#include "stdafx.h"
#include "../Display/Display.h"
#include "../Display/Camera.h"
#include "../Display/Effect.h"
#include "../Display/Texture.h"
#include "../Display/Surface.h"
#include "../Display/RenderTarget.h"
#include "../Display/PostProcess.h"
#include "../Display/NormalProcess.h"
#include "../Display/Font.h"
#include "../Display/RenderStage.h"
#include "../Core/Scripting.h"
#include "../Core/Util.h"
#include "../Core/Octree.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	bool Display::UpdateRecord()
	{
		PROFILING(__FUNCTION__);

		bool bResult = true;

		m_oCommands.FreeAll();
		m_vCommands.clear();

		{
			PROFILING(__FUNCTION__" [MANAGERS UPDATE]");
			m_pSurfaceManager->Update();
			m_pTextureManager->Update();
			m_pFontManager->Update();
			m_pMaterialManager->Update();
			//m_pRTChain->SetImmediateWrite(false);
		}

		{
			PROFILING(__FUNCTION__" [STAGES]");
			DisplayRenderStagePtrVec::iterator iRS = m_vRenderStages.begin();
			DisplayRenderStagePtrVec::iterator iEnd = m_vRenderStages.end();
			while (iEnd != iRS)
			{
				m_pCurrentRenderStage = *iRS;
				RecordCommand(NewCommand(EDisplayCommand_SETSTAGE, this));
				RenderStageRecord(m_pCurrentRenderStage);
				++iRS;
			}
			m_pCurrentRenderStage = NULL;
		}

		// copy back to back buffer
		if (false == m_pRTChain->GetImmediateWrite())
		{
			PROFILING(__FUNCTION__" [IMMEDIATE WRITE]");
			if (SUCCEEDED(m_pDevice->BeginScene()))
			{
				TexturePtr pFinalRenderTex = static_cast<TexturePtr>(m_pRTChain->GetTexture(0)->GetBase());
				m_pEffectPP->SetTechnique("RenderScene");
				m_pEffectPP->SetTexture("g_ColorTex", pFinalRenderTex);
				UINT cPasses;
				m_pEffectPP->Begin(&cPasses, 0);
				m_pPostProcessGeometry->RenderBegin();
				for(UINT p = 0; p < cPasses; ++p)
				{
					m_pEffectPP->BeginPass(p);
					m_pPostProcessGeometry->Render();
					m_pEffectPP->EndPass();
				}
				m_pPostProcessGeometry->RenderEnd();
				m_pEffectPP->End();
				m_pDevice->EndScene();
			}
		}

		{
			PROFILING(__FUNCTION__" [PRESENT]");
			m_pDevice->Present(NULL, NULL, NULL, NULL);
		}

		return bResult;
	}

	bool Display::RenderStageRecord(DisplayRenderStagePtr _pRP)
	{
		PROFILING(__FUNCTION__);
		const UInt uBlack = D3DCOLOR_XRGB(0, 0, 0);
		const UInt uBlue = D3DCOLOR_XRGB(16, 32, 64);
		const UInt uClearColor = uBlack;
		bool bResult = true;

		//vsoutput(__FUNCTION__" : render stage %x\n", _pRP->GetNameKey());

		_pRP->UpdateRecord();

		// Render scene to buffers
		if ((NULL != m_pNormalProcesses) && (false == m_pNormalProcesses->empty()))
		{
			RecordCommand(NewCommand(EDisplayCommand_BEGINNORMALPROCESSES, this));
			DisplayNormalProcessPtrVec::iterator iNormalProcess = m_pNormalProcesses->begin();
			DisplayNormalProcessPtrVec::iterator iEnd = m_pNormalProcesses->end();
			while (iEnd != iNormalProcess)
			{
				m_pCurrentNormalProcess = *iNormalProcess;
				m_pCurrentNormalProcess->RenderBeginRecord();
				RenderRecord(_pRP);
				m_pCurrentNormalProcess->RenderEndRecord();

				++iNormalProcess;
			}
			RecordCommand(NewCommand(EDisplayCommand_ENDNORMALPROCESSES, this));
			m_pCurrentNormalProcess = NULL;
			m_pNormalProcesses = NULL;
		}
		else
		{
			RecordCommand(NewCommand(EDisplayCommand_BEGINNONORMALPROCESSES, this));
			//m_pRTChain->EnableAllRenderTargets();
			//m_pRTChain->RenderBegin(DisplayRenderTarget::ERenderMode_NORMALPROCESS);
			//m_pRTChain->RenderBeginPass(0);
			//m_pDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, uClearColor, 1.0f, 0L);
			RenderRecord(_pRP);
			RecordCommand(NewCommand(EDisplayCommand_ENDNONORMALPROCESSES, this));
			//m_pRTChain->RenderEndPass();
			//m_pRTChain->RenderEnd();
		}

		if ((NULL != m_pPostProcesses) && (false == m_pPostProcesses->empty()))
		{
			RecordCommand(NewCommand(EDisplayCommand_BEGINPOSTPROCESSES, this));
			// Apply post processes effects
			//m_pRTChain->EnableAllRenderTargets();
			//m_pRTChain->RenderBegin(DisplayRenderTarget::ERenderMode_POSTPROCESS);
			DisplayPostProcessPtrVec::iterator iPostProcess = m_pPostProcesses->begin();
			DisplayPostProcessPtrVec::iterator iEnd = m_pPostProcesses->end();
			while (iEnd != iPostProcess)
			{
				DisplayPostProcessPtr pPostProcess = *iPostProcess;
				pPostProcess->RenderBeginRecord();
				pPostProcess->UpdateRecord();
				pPostProcess->RenderEndRecord();
				++iPostProcess;
			}
			RecordCommand(NewCommand(EDisplayCommand_ENDPOSTPROCESSES, this));
			//m_pRTChain->RenderEnd();
			m_pPostProcesses = NULL;
		}

		_pRP->GetRenderList().clear();

		return bResult;
	}

	bool Display::RenderRecord(DisplayRenderStagePtr _pRP)
	{
		PROFILING(__FUNCTION__);

		bool bResult = m_pCurrentCamera->UpdateRecord();

		if (false != bResult)
		{
			RenderUpdate();

			{
				PROFILING(__FUNCTION__" [REQUESTS]");
				// only use registered objects for this pass
				DisplayObjectPtrVec& vDisplayObjects = _pRP->GetRenderList();
				DisplayObjectPtrVec::iterator iDisplayObject = vDisplayObjects.begin();
				DisplayObjectPtrVec::iterator iEnd = vDisplayObjects.end();
				while (iEnd != iDisplayObject)
				{
					DisplayObjectPtr pDisplayObject = *iDisplayObject;
					DisplayMaterialPtr pMaterial = pDisplayObject->GetMaterial();
					DisplayEffectPtr pEffect = pMaterial->GetEffect();
					if (m_vRenderList.end() == find(m_vRenderList.begin(), m_vRenderList.end(), pEffect))
					{
						m_vRenderList.push_back(pEffect);
					}
					pEffect->RenderRequest(pMaterial);
					pMaterial->RenderRequest(pDisplayObject);
					++iDisplayObject;
				}
			}

			{
				PROFILING(__FUNCTION__" [RENDER]");
				struct RecordEffectFunction
				{
					RecordEffectFunction()
					:	m_bResult(true)
					{

					}

					void operator()(DisplayEffectPtr _pDisplayEffect)
					{
						m_bResult &= _pDisplayEffect->RenderRecord();
					}

					bool	m_bResult;
				};
				RecordEffectFunction funcRecord;
				for_each(m_vRenderList.begin(), m_vRenderList.end(), funcRecord);
				m_vRenderList.clear();

				bResult &= funcRecord.m_bResult;
			}
		}

		return bResult;
	}

	bool Display::RecordCommand(CoreCommandPtr _pCommand)
	{
		bool bResult = true;

		switch (_pCommand->m_uID)
		{
			case EDisplayCommand_SETSTAGE:
			{
				bResult = _pCommand->AddArg(m_pCurrentRenderStage);
				break;
			}
			case EDisplayCommand_BEGINNORMALPROCESSES:
			{
				break;
			}
			case EDisplayCommand_ENDNORMALPROCESSES:
			{
				break;
			}
			case EDisplayCommand_BEGINNONORMALPROCESSES:
			{
				break;
			}
			case EDisplayCommand_ENDNONORMALPROCESSES:
			{
				break;
			}
			case EDisplayCommand_BEGINPOSTPROCESSES:
			{
				break;
			}
			case EDisplayCommand_ENDPOSTPROCESSES:
			{
				break;
			}
			default:
			{
				bResult = false;
				break;
			}
		}

		return bResult;
	}
}
