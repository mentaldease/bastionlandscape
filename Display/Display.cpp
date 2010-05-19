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
#include "../Display/EffectStateManager.h"
#include "../Display/DisplayComponent.h"
#include "../Core/Scripting.h"
#include "../Core/Util.h"
#include "../Core/Octree.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Display::DisplayFormatMap Display::s_mDisplayFormat;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Display::Display()
	:	CoreObject(),
		m_pDirect3D(NULL),
		m_pDevice(NULL),
		m_vRenderList(),
		m_vUpdateList(),
		m_mViewports(),
		m_mCameras(),
		m_vRenderStages(),
		m_mRenderStages(),
		m_mRenderRequests(),
		m_oVertexBuffers(),
		m_oIndexBuffer(),
		m_mVertexDecls(),
		m_oComponentFactory(),
		m_pCurrentCamera(NULL),
		m_pMaterialManager(NULL),
		m_pTextureManager(NULL),
		m_pSurfaceManager(NULL),
		m_pFontManager(NULL),
		m_pWorldMatrix(NULL),
		m_pPostProcesses(NULL),
		m_pDispFXPP(NULL),
		m_pEffectPP(NULL),
		m_pPostProcessGeometry(NULL),
		m_pRTChain(NULL),
		m_pNormalProcesses(NULL),
		m_pCurrentNormalProcess(NULL),
		m_pCurrentPostProcess(NULL),
		m_pCurrentVertexBuffer(NULL),
		m_pCurrentIndexBuffer(NULL),
		m_pCurrentRenderStage(NULL),
		m_pStateManagerInterface(NULL),
		m_m4WorldInvTransposeMatrix(),
		m_uCurrentVertexBuffer(0),
		m_uCurrentIndexBuffer(0),
		m_uCurrentVertexDecl(0),
		m_uVertexDeclID(0),
		m_uWidth(0),
		m_uHeight(0),
		m_bDepthBytes(0)
	{
		if (NULL == GetInstance())
		{
			SetInstance(this);
		}
	}

	Display::~Display()
	{
		if (this == GetInstance())
		{
			SetInstance(NULL);
		}
	}

	bool Display::Create(const boost::any& _rConfig)
	{
		GraphicConfigData* pGraphicConfigData = boost::any_cast<GraphicConfigData*>(_rConfig);
		bool bResult = (NULL != pGraphicConfigData);

		if (false != bResult)
		{
			Release();
			m_pDirect3D = Direct3DCreate9(D3D_SDK_VERSION);
			bResult = (NULL != m_pDirect3D);
		}
		if (false != bResult)
		{
			bResult = OpenVideo(*pGraphicConfigData);
		}

		return bResult;
	}

	void Display::Update()
	{
		PROFILING(__FUNCTION__);

		{
			PROFILING(__FUNCTION__" [MANAGERS UPDATE]");
			m_pSurfaceManager->Update();
			m_pTextureManager->Update();
			m_pFontManager->Update();
			m_pMaterialManager->Update();
			m_pRTChain->SetImmediateWrite(false);
		}

		{
			PROFILING(__FUNCTION__" [STAGES]");
			DisplayRenderStagePtrVec::iterator iRS = m_vRenderStages.begin();
			DisplayRenderStagePtrVec::iterator iEnd = m_vRenderStages.end();
			while (iEnd != iRS)
			{
				m_pCurrentRenderStage = *iRS;
				RenderStage(m_pCurrentRenderStage);
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
				m_pEffectPP->Begin(&cPasses, EFFECT_RENDER_FLAGS);
				m_pPostProcessGeometry->RenderBegin();
				for(UINT p = 0; p < cPasses; ++p)
				{
					m_pStateManagerInterface->BeginPass(p);
					m_pEffectPP->BeginPass(p);
					m_pPostProcessGeometry->Render();
					m_pEffectPP->EndPass();
					m_pStateManagerInterface->EndPass();
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
	}

	void Display::Release()
	{
		CloseVideo();

		m_oIndexBuffer.Release();
		m_oVertexBuffers.Release();

		if (NULL != m_pDirect3D)
		{
			m_pDirect3D->Release();
			m_pDirect3D = NULL;
		}
	}

	bool Display::OpenVideo(GraphicConfigDataRef _rGraphicConfigData)
	{
		bool bResult = (NULL == m_pDevice);
		HRESULT hResult;

		if (false != bResult)
		{
			D3DPRESENT_PARAMETERS oD3DPP;
			memset(&oD3DPP, 0 ,sizeof(D3DPRESENT_PARAMETERS));

			oD3DPP.EnableAutoDepthStencil = TRUE;
			oD3DPP.AutoDepthStencilFormat = D3DFORMAT(_rGraphicConfigData.m_uDXDepthFormat);
			oD3DPP.Windowed = _rGraphicConfigData.m_bFullScreen ? FALSE : TRUE;
			oD3DPP.BackBufferWidth = _rGraphicConfigData.m_oClientRect.right;
			oD3DPP.BackBufferHeight = _rGraphicConfigData.m_oClientRect.bottom;
			oD3DPP.BackBufferFormat = _rGraphicConfigData.m_bFullScreen ? D3DFORMAT(_rGraphicConfigData.m_uDXColorFormat) : D3DFMT_UNKNOWN;
			oD3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
			oD3DPP.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

			m_uWidth = _rGraphicConfigData.m_oClientRect.right;
			m_uHeight = _rGraphicConfigData.m_oClientRect.bottom;
			m_bDepthBytes = GetFormatBitsPerPixel(D3DFORMAT(_rGraphicConfigData.m_uDXDepthFormat)) / 8;


			UINT AdapterToUse=D3DADAPTER_DEFAULT;
			D3DDEVTYPE DeviceType=D3DDEVTYPE_HAL;
#if SHIPPING_VERSION
			// When building a shipping version, disable PerfHUD (opt-out)
#else // SHIPPING_VERSION
			// Look for 'NVIDIA PerfHUD' adapter
			// If it is present, override default settings
			for (UINT Adapter=0;Adapter<m_pDirect3D->GetAdapterCount();Adapter++)
			{
				D3DADAPTER_IDENTIFIER9 Identifier;
				HRESULT Res;
				Res = m_pDirect3D->GetAdapterIdentifier(Adapter,0,&Identifier);
				if (strstr(Identifier.Description,"PerfHUD") != 0)
				{
					AdapterToUse=Adapter;
					DeviceType=D3DDEVTYPE_REF;
					break;
				}
			}
#endif // SHIPPING_VERSION

			hResult = m_pDirect3D->CreateDevice(
				AdapterToUse,
				DeviceType,
				_rGraphicConfigData.m_hWnd,
				D3DCREATE_HARDWARE_VERTEXPROCESSING,
				&oD3DPP,
				&m_pDevice);

			bResult = SUCCEEDED(hResult);
		}

		if (false != bResult)
		{
			bResult = m_oComponentFactory.Create(boost::any(0));
		}

		if (false != bResult)
		{
			// DisplayStateManager is NOT a CoreObject derived class, no Create call
			m_pStateManagerInterface = new DisplayStateManager;
			m_pStateManagerInterface->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
			m_pStateManagerInterface->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		}

		if (false != bResult)
		{
			m_pMaterialManager = new DisplayMaterialManager(*this);
			bResult = m_pMaterialManager->Create(boost::any(0));
		}

		if (false != bResult)
		{
			m_pTextureManager = new DisplayTextureManager(*this);
			bResult = m_pTextureManager->Create(boost::any(0));
		}

		if (false != bResult)
		{
			m_pSurfaceManager = new DisplaySurfaceManager(*this);
			bResult = m_pSurfaceManager->Create(boost::any(0));
		}

		if (false != bResult)
		{
			m_pFontManager = new DisplayFontManager(*this);
			bResult = m_pFontManager->Create(boost::any(0));
		}

		if (false != bResult)
		{
			DisplayRenderTargetGeometry::CreateInfo oRTGCInfo = { m_uWidth, m_uHeight };
			DisplayRenderTargetChain::CreateInfo oRTCCInfo = { "GBUFFERS", &_rGraphicConfigData, m_uWidth, m_uHeight };
			m_pPostProcessGeometry = new DisplayRenderTargetGeometry(*this);
			m_pRTChain = new DisplayRenderTargetChain(*this);
			bResult =  m_pPostProcessGeometry->Create(boost::any(&oRTGCInfo))
				&& m_pRTChain->Create(boost::any(&oRTCCInfo))
				&& m_pMaterialManager->LoadEffect("MRT", "data/effects/simplepost.fx")
				&& (m_pDispFXPP = m_pMaterialManager->GetEffect("MRT"))
				&& (m_pEffectPP = m_pDispFXPP->GetEffect());
			m_pPostProcesses = NULL;
			m_pNormalProcesses = NULL;
		}

		if (false != bResult)
		{
			m_pStateManagerInterface->Reset();
		}

		return bResult;
	}

	void Display::CloseVideo()
	{
		#pragma message("implement a generic notify system to enable unload/release data before a system shutdown")

		if (NULL != m_pRTChain)
		{
			m_pRTChain->Release();
			delete m_pRTChain;
			m_pRTChain = NULL;
		}

		if (NULL != m_pPostProcessGeometry)
		{
			m_pPostProcessGeometry->Release();
			delete m_pPostProcessGeometry;
			m_pPostProcessGeometry = NULL;
		}

		if (NULL != m_pDispFXPP)
		{
			m_pMaterialManager->UnloadEffect("MRT");
			m_pDispFXPP = NULL;
		}

		if (NULL != m_pFontManager)
		{
			m_pFontManager->Release();
			delete m_pFontManager;
			m_pFontManager = NULL;
		}

		if (NULL != m_pSurfaceManager)
		{
			m_pSurfaceManager->Release();
			delete m_pSurfaceManager;
			m_pSurfaceManager = NULL;
		}

		if (NULL != m_pTextureManager)
		{
			m_pTextureManager->Release();
			delete m_pTextureManager;
			m_pTextureManager = NULL;
		}

		if (NULL != m_pMaterialManager)
		{
			m_pMaterialManager->Release();
			delete m_pMaterialManager;
			m_pMaterialManager = NULL;
		}

		// DisplayStateManager is NOT a CoreObject derived class, no Release call
		if (NULL != m_pStateManagerInterface)
		{
			delete m_pStateManagerInterface;
			m_pStateManagerInterface = NULL;
		}

		if (NULL != m_pDevice)
		{
			m_pDevice->Release();
			m_pDevice = NULL;
		}

		m_mViewports.clear();

		while (m_mCameras.end() != m_mCameras.begin())
		{
			ReleaseCamera(m_mCameras.begin()->first);
			m_mCameras.erase(m_mCameras.begin());
		}

		m_oComponentFactory.Release();
	}

	void Display::UpdateRequest(CoreObjectPtr _pCoreObject)
	{
		if (m_vUpdateList.end() == find(m_vUpdateList.begin(), m_vUpdateList.end(), _pCoreObject))
		{
			m_vUpdateList.push_back(_pCoreObject);
		}
	}

	void Display::RenderRequest(const Key& _uRenderPassKey, DisplayObjectPtr _pDisplayObject)
	{
		DisplayRenderStagePtr pRS = m_mRenderStages[_uRenderPassKey];
		if (NULL != pRS)
		{
			pRS->RenderRequest(_pDisplayObject);
		}
	}

	void Display::RenderRequest(DisplayObjectPtr _pDisplayObject)
	{
		RenderRequest(_pDisplayObject->GetRenderStage(), _pDisplayObject);
	}

	Key Display::CreateVertexBufferKey(DisplayVertexBuffer::CreateInfo& _rCreateInfo)
	{
		return m_oVertexBuffers.New(boost::any(&_rCreateInfo));
	}

	bool Display::SetCurrentVertexBufferKey(const Key _uVertexBuffer)
	{
		bool bResult = true;
		if (m_uCurrentVertexBuffer != _uVertexBuffer)
		{
			m_uCurrentVertexBuffer = _uVertexBuffer;
			m_pCurrentVertexBuffer = m_oVertexBuffers.Get(_uVertexBuffer);
			if (NULL != m_pCurrentVertexBuffer)
			{
				bResult = m_pCurrentVertexBuffer->Use();
			}
		}
		return bResult;
	}

	Key Display::GetCurrentVertexBufferKey()
	{
		return m_uCurrentVertexBuffer;	
	}

	void Display::ReleaseVertexBufferKey(Key _uVertexBuffer)
	{
		m_oVertexBuffers.Delete(_uVertexBuffer);
		m_uCurrentVertexBuffer = (m_uCurrentIndexBuffer == _uVertexBuffer) ? 0 : m_uCurrentVertexBuffer;
	}

	bool Display::SetVertexBufferKeyData(const Key _uVertexBuffer, const VoidPtr _pData)
	{
		DisplayVertexBufferPtr pVertexBuffer = m_oVertexBuffers.Get(_uVertexBuffer);
		bool bResult = (NULL != pVertexBuffer);
		if (false != bResult)
		{
			bResult = pVertexBuffer->Set(_pData);
		}
		return bResult;
	}

	Key Display::CreateIndexBufferKey(DisplayIndexBuffer::CreateInfo& _rCreateInfo)
	{
		return m_oIndexBuffer.New(boost::any(&_rCreateInfo));
	}

	bool Display::SetCurrentIndexBufferKey(const Key _uIndexBuffer)
	{
		bool bResult = true;
		if (m_uCurrentIndexBuffer != _uIndexBuffer)
		{
			m_uCurrentIndexBuffer = _uIndexBuffer;
			m_pCurrentIndexBuffer = m_oIndexBuffer.Get(_uIndexBuffer);
			if (NULL != m_pCurrentIndexBuffer)
			{
				bResult = m_pCurrentIndexBuffer->Use();
			}
		}
		return bResult;
	}

	Key Display::GetCurrentIndexBufferKey()
	{
		return m_uCurrentIndexBuffer;
	}

	void Display::ReleaseIndexBufferKey(const Key _uIndexBuffer)
	{
		m_oIndexBuffer.Delete(_uIndexBuffer);
		m_uCurrentIndexBuffer = (m_uCurrentIndexBuffer == _uIndexBuffer) ? 0 : m_uCurrentIndexBuffer;
	}

	bool Display::SetIndexBufferKeyData(const Key _uIndexBuffer, const VoidPtr _pData)
	{
		DisplayIndexBufferPtr pIndexBuffer = m_oIndexBuffer.Get(_uIndexBuffer);
		bool bResult = (NULL != pIndexBuffer);
		if (false != bResult)
		{
			bResult = pIndexBuffer->Set(_pData);
		}
		return bResult;
	}

	DisplayVertexBufferPtr Display::CreateVertexBuffer(DisplayVertexBuffer::CreateInfo& _rCreateInfo)
	{
		DisplayVertexBufferPtr pVertexBuffer = new DisplayVertexBuffer(*this);
		if (false == pVertexBuffer->Create(boost::any(&_rCreateInfo)))
		{
			ReleaseVertexBuffer(pVertexBuffer);
			pVertexBuffer = NULL;
		}
		return pVertexBuffer;
	}

	bool Display::SetCurrentVertexBuffer(DisplayVertexBufferPtr _pVertexBuffer)
	{
		bool bResult = true;
		m_uCurrentVertexBuffer = 0;
		if (m_pCurrentVertexBuffer != _pVertexBuffer)
		{
			m_pCurrentVertexBuffer = _pVertexBuffer;
			if (NULL != m_pCurrentVertexBuffer)
			{
				bResult = m_pCurrentVertexBuffer->Use();
			}
		}
		return bResult;
	}

	DisplayVertexBufferPtr Display::GetCurrentVertexBuffer()
	{
		return m_pCurrentVertexBuffer;
	}

	void Display::ReleaseVertexBuffer(DisplayVertexBufferPtr _pVertexBuffer)
	{
		_pVertexBuffer->Release();
		delete _pVertexBuffer;
	}

	DisplayIndexBufferPtr Display::CreateIndexBuffer(DisplayIndexBuffer::CreateInfo& _rCreateInfo)
	{
		DisplayIndexBufferPtr pIndexBuffer = new DisplayIndexBuffer(*this);
		if (false == pIndexBuffer->Create(boost::any(&_rCreateInfo)))
		{
			ReleaseIndexBuffer(pIndexBuffer);
			pIndexBuffer = NULL;
		}
		return pIndexBuffer;
	}

	bool Display::SetCurrentIndexBuffer(DisplayIndexBufferPtr _pIndexBuffer)
	{
		bool bResult = true;
		m_uCurrentIndexBuffer = 0;
		if (m_pCurrentIndexBuffer != _pIndexBuffer)
		{
			m_pCurrentIndexBuffer = _pIndexBuffer;
			if (NULL != m_pCurrentIndexBuffer)
			{
				bResult = m_pCurrentIndexBuffer->Use();
			}
		}
		return bResult;
	}

	DisplayIndexBufferPtr Display::GetCurrentIndexBuffer()
	{
		return m_pCurrentIndexBuffer;
	}

	void Display::ReleaseIndexBuffer(DisplayIndexBufferPtr _pIndexBuffer)
	{
		_pIndexBuffer->Release();
		delete _pIndexBuffer;
	}

	DevicePtr Display::GetDevicePtr() const
	{
		return m_pDevice;
	}

	void Display::GetResolution(unsigned int& _uWidth, unsigned int& _uHeight) const
	{
		_uWidth = m_uWidth;
		_uHeight = m_uHeight;
	}

	DisplayMaterialManagerPtr Display::GetMaterialManager()
	{
		return m_pMaterialManager;
	}

	DisplayTextureManagerPtr Display::GetTextureManager()
	{
		return m_pTextureManager;
	}

	DisplaySurfaceManagerPtr Display::GetSurfaceManager()
	{
		return m_pSurfaceManager;
	}

	DisplayFontManagerPtr Display::GetFontManager()
	{
		return m_pFontManager;
	}

	void Display::SetCurrentWorldMatrix(MatrixPtr _pMatrix)
	{
		m_pWorldMatrix = _pMatrix;
		D3DXMatrixInverse(&m_m4WorldInvTransposeMatrix, NULL, m_pWorldMatrix);
		D3DXMatrixTranspose(&m_m4WorldInvTransposeMatrix, &m_m4WorldInvTransposeMatrix);
	}

	MatrixPtr Display::GetCurrentWorldMatrix()
	{
		return m_pWorldMatrix;
	}

	MatrixPtr Display::GetCurrentWorldInvTransposeMatrix()
	{
		return &m_m4WorldInvTransposeMatrix;
	}

	void Display::MRTRenderBeginPass(UIntRef _uIndex)
	{
		if (NULL != m_pRTChain)
		{
			m_pRTChain->RenderBeginPass(_uIndex);
		}
	}

	void Display::MRTRenderEndPass()
	{
		if (NULL != m_pRTChain)
		{
			m_pRTChain->RenderEndPass();
		}
	}

	void Display::SetNormalProcessesList(DisplayNormalProcessPtrVecPtr _pNormalProcesses)
	{
		m_pNormalProcesses = _pNormalProcesses;
	}

	void Display::SetPostProcessesList(DisplayPostProcessPtrVecPtr _pPostProcesses)
	{
		m_pPostProcesses = _pPostProcesses;
	}

	DisplayObjectPtr Display::GetPostProcessGeometry()
	{
		return m_pPostProcessGeometry;
	}

	DisplayRenderTargetChainPtr Display::GetRenderTargetChain()
	{
		return m_pRTChain;
	}

	DisplayNormalProcessPtr Display::GetCurrentNormalProcess()
	{
		return m_pCurrentNormalProcess;
	}

	DisplayCameraPtr Display::CreateCamera(const Key& _uNameKey, LuaObjectRef _rLuaObject)
	{
		DisplayCameraPtr pCamera = GetCamera(_uNameKey);
		bool bResult = (NULL == pCamera);

		if (false != bResult)
		{
			string strViewportName;
			DisplayCamera::CreateInfo oDCCInfo;
			pCamera = new DisplayCamera(*this);
			Scripting::Lua::Get(_rLuaObject, "depth_near", oDCCInfo.m_fZNear, oDCCInfo.m_fZNear);
			Scripting::Lua::Get(_rLuaObject, "depth_far", oDCCInfo.m_fZFar, oDCCInfo.m_fZFar);
			Scripting::Lua::Get(_rLuaObject, "fovy", oDCCInfo.m_fDegreeFovy, oDCCInfo.m_fDegreeFovy);
			Scripting::Lua::Get(_rLuaObject, "viewport", string(""), strViewportName);
			Scripting::Lua::Get(_rLuaObject, "perspective_mode", oDCCInfo.m_bPerspectiveMode, oDCCInfo.m_bPerspectiveMode);
			if (false == strViewportName.empty())
			{
				const Key uViewportKey = MakeKey(strViewportName);
				ViewportPtr pViewport = GetViewport(uViewportKey);
				if (NULL != pViewport)
				{
					oDCCInfo.m_fX = float(pViewport->X) / float(m_uWidth);
					oDCCInfo.m_fY = float(pViewport->Y) / float(m_uHeight);
					oDCCInfo.m_fWidth = float(pViewport->Width) / float(m_uWidth);
					oDCCInfo.m_fHeight = float(pViewport->Height) / float(m_uHeight);
					oDCCInfo.m_fAspectRatio = float(pViewport->Width) / float(pViewport->Height);
				}
			}
			Scripting::Lua::Get(_rLuaObject, "aspect_ratio", oDCCInfo.m_fAspectRatio, oDCCInfo.m_fAspectRatio);
			Scripting::Lua::Get(_rLuaObject, "position", oDCCInfo.m_oPos, oDCCInfo.m_oPos);
			Scripting::Lua::Get(_rLuaObject, "rotation", oDCCInfo.m_oRot, oDCCInfo.m_oRot);
			bool bResult = pCamera->Create(boost::any(&oDCCInfo));
			if (false != bResult)
			{
				m_mCameras[_uNameKey] = pCamera;
			}
		}

		return pCamera;
	}

	void Display::ReleaseCamera(const Key& _uNameKey)
	{
		DisplayCameraPtrMap::iterator iPair = m_mCameras.find(_uNameKey);
		if (m_mCameras.end() != iPair)
		{
			DisplayCameraPtr pCamera =  iPair->second;
			pCamera->Release();
			delete pCamera;
			m_mCameras.erase(iPair);
		}
	}

	DisplayCameraPtr Display::GetCamera(const Key& _uNameKey)
	{
		DisplayCameraPtrMap::iterator iPair = m_mCameras.find(_uNameKey);
		DisplayCameraPtr pResult = (m_mCameras.end() != iPair) ? iPair->second : NULL;
		return pResult;
	}

	DisplayCameraPtr Display::GetCurrentCamera()
	{
		return m_pCurrentCamera;
	}

	void Display::SetCurrentCamera(DisplayCameraPtr _pCamera)
	{
		m_pCurrentCamera = _pCamera;
	}

	bool Display::AddViewport(const Key& _uNameKey, ViewportRef _rViewPort)
	{
		ViewportMap::iterator iPair = m_mViewports.find(_uNameKey);
		bool bResult = (m_mViewports.end() == iPair);

		if (false != bResult)
		{
			m_mViewports[_uNameKey] = _rViewPort;
		}

		return bResult;
	}

	ViewportPtr Display::GetViewport(const Key& _uNameKey)
	{
		ViewportMap::iterator iPair = m_mViewports.find(_uNameKey);
		return (m_mViewports.end() == iPair) ? NULL : &iPair->second;
	}

	void Display::AddRenderStages(DisplayRenderStagePtrVec _vRenderPasses)
	{
		DisplayRenderStagePtrVec::iterator iRS = _vRenderPasses.begin();
		DisplayRenderStagePtrVec::iterator iEnd = _vRenderPasses.end();
		while (iEnd != iRS)
		{
			DisplayRenderStagePtr pRS = *iRS;
			m_vRenderStages.push_back(pRS);
			m_mRenderStages[pRS->GetNameKey()] = pRS;
			++iRS;
		}
	}

	void Display::RemoveRenderStages(DisplayRenderStagePtrVec _vRenderPasses)
	{
		DisplayRenderStagePtrVec::iterator iRS = _vRenderPasses.begin();
		DisplayRenderStagePtrVec::iterator iEnd = _vRenderPasses.end();
		while (iEnd != iRS)
		{
			DisplayRenderStagePtr pRS = *iRS;

			DisplayRenderStagePtrVec::iterator iRPToErase = find(m_vRenderStages.begin(), m_vRenderStages.end(), pRS);
			if (m_vRenderStages.end() != iRPToErase)
			{
				m_vRenderStages.erase(iRPToErase);
			}

			DisplayRenderStagePtrMap::iterator iPair = m_mRenderStages.find(pRS->GetNameKey());
			if (m_mRenderStages.end() != iPair)
			{
				m_mRenderStages.erase(iPair);
			}

			++iRS;
		}
	}

	DisplayRenderStagePtr Display::GetCurrentRenderStage()
	{
		return m_pCurrentRenderStage;
	}

	DisplayStateManagerPtr Display::GetStateManagerInterface()
	{
		return m_pStateManagerInterface;
	}

	Key Display::CreateVertexDeclaration(VertexElementPtr _pVertexElements)
	{
		VertexDeclPtr pVertexDecl = NULL;
		Key uResult = 0;
		if (SUCCEEDED(m_pDevice->CreateVertexDeclaration(_pVertexElements, &pVertexDecl)))
		{
			uResult = ++m_uVertexDeclID;
			m_mVertexDecls[uResult] = pVertexDecl;
		}
		return uResult;
	}

	bool Display::SetVertexDeclaration(const Key _uVertexDeclaration)
	{
		VertexDeclPtrMap::iterator iPair = m_mVertexDecls.find(_uVertexDeclaration);
		bool bResult = (m_mVertexDecls.end() != iPair);
		if ((false != bResult) && (m_uCurrentVertexDecl != _uVertexDeclaration))
		{
			VertexDeclPtr pVertexDecl = iPair->second;
			m_pDevice->SetVertexDeclaration(pVertexDecl);
			m_uCurrentVertexDecl = _uVertexDeclaration;
		}
		return bResult;
	}

	Key Display::GetCurrentVertexDeclaration()
	{
		return m_uCurrentVertexDecl;
	}

	void Display::ReleaseVertexDeclaration(const Key _uVertexDeclaration)
	{
		VertexDeclPtrMap::iterator iPair = m_mVertexDecls.find(_uVertexDeclaration);
		bool bResult = (m_mVertexDecls.end() != iPair);
		if (false != bResult)
		{
			VertexDeclPtr pVertexDecl = iPair->second;
			pVertexDecl->Release();
			m_uCurrentVertexDecl = (m_uCurrentVertexDecl == _uVertexDeclaration) ? 0 : m_uCurrentVertexDecl;
		}
	}

	void Display::Unproject(const Vector3Ptr _pf3In, Vector3Ptr _pf3Out, DisplayCameraPtr _pCamera, const MatrixPtr _pObjectWorld)
	{
		Matrix mIdentity;
		D3DXMatrixIdentity(&mIdentity);
		_pCamera = (NULL == _pCamera) ? m_pCurrentCamera : _pCamera;
		D3DXVec3Unproject(
			_pf3Out,
			_pf3In,
			_pCamera->GetCurrentViewport(),
			_pCamera->GetMatrix(DisplayCamera::EMatrix_PROJ),
			_pCamera->GetMatrix(DisplayCamera::EMatrix_VIEW),
			(NULL != _pObjectWorld) ? _pObjectWorld : &mIdentity);
	}

	ComponentPtr Display::NewComponent(EntityRef _rEntity, const boost::any& _rConfig)
	{
		return m_oComponentFactory.New(_rEntity, _rConfig);
	}

	void Display::DeleteComponent(ComponentPtr _pComponent)
	{
		m_oComponentFactory.Delete(_pComponent);
	}

	void Display::RenderUpdate()
	{
		PROFILING(__FUNCTION__);
		CoreObjectPtrVec::iterator iCoreObject = m_vUpdateList.begin();
		CoreObjectPtrVec::iterator iEnd = m_vUpdateList.end();
		while (iEnd != iCoreObject)
		{
			(*iCoreObject)->Update();
			++iCoreObject;
		}
	}

	void Display::RenderStage(DisplayRenderStagePtr _pRS)
	{
		PROFILING(__FUNCTION__);
		const UInt uBlack = D3DCOLOR_XRGB(0, 0, 0);
		const UInt uBlue = D3DCOLOR_XRGB(16, 32, 64);
		const UInt uClearColor = uBlack;

		//vsoutput(__FUNCTION__" : render stage %x\n", _pRS->GetNameKey());

		_pRS->Update();

		// Render scene to buffers
		if ((NULL != m_pNormalProcesses) && (false == m_pNormalProcesses->empty()))
		{
			DisplayNormalProcessPtrVec::iterator iNormalProcess = m_pNormalProcesses->begin();
			DisplayNormalProcessPtrVec::iterator iEnd = m_pNormalProcesses->end();
			while (iEnd != iNormalProcess)
			{
				m_pCurrentNormalProcess = *iNormalProcess;
				m_pCurrentNormalProcess->RenderBegin();
				Render(_pRS);
				m_pCurrentNormalProcess->RenderEnd();

				++iNormalProcess;
			}
			m_pCurrentNormalProcess = NULL;
			m_pNormalProcesses = NULL;
		}
		else
		{
			m_pRTChain->EnableAllRenderTargets();
			m_pRTChain->RenderBegin(DisplayRenderTarget::ERenderMode_NORMALPROCESS);
			m_pRTChain->RenderBeginPass(0);
			m_pDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, uClearColor, 1.0f, 0L);
			Render(_pRS);
			m_pRTChain->RenderEndPass();
			m_pRTChain->RenderEnd();
		}

		if ((NULL != m_pPostProcesses) && (false == m_pPostProcesses->empty()))
		{
			// Apply post processes effects
			m_pRTChain->EnableAllRenderTargets();
			m_pRTChain->RenderBegin(DisplayRenderTarget::ERenderMode_POSTPROCESS);
			DisplayPostProcessPtrVec::iterator iPostProcess = m_pPostProcesses->begin();
			DisplayPostProcessPtrVec::iterator iEnd = m_pPostProcesses->end();
			while (iEnd != iPostProcess)
			{
				m_pCurrentPostProcess = *iPostProcess;
				m_pCurrentPostProcess->RenderBegin();
				m_pCurrentPostProcess->Update();
				m_pCurrentPostProcess->RenderEnd();
				++iPostProcess;
			}
			m_pRTChain->RenderEnd();
			m_pCurrentPostProcess = NULL;
			m_pPostProcesses = NULL;
		}

		_pRS->GetRenderList().clear();
	}

	void Display::Render(DisplayRenderStagePtr _pRS)
	{
		PROFILING(__FUNCTION__);
		m_pCurrentCamera->Update();

		RenderUpdate();

		{
			PROFILING(__FUNCTION__" [REQUESTS]");
			// only use registered objects for this pass
			DisplayObjectPtrVec& vDisplayObjects = _pRS->GetRenderList();
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
			vDisplayObjects.clear();
		}

		{
			PROFILING(__FUNCTION__" [RENDER]");
			DisplayEffectPtrVec::iterator iEffect = m_vRenderList.begin();
			DisplayEffectPtrVec::iterator iEnd = m_vRenderList.end();
			while (iEnd != iEffect)
			{
				(*iEffect)->Render();
				++iEffect;
			}
			m_vRenderList.clear();
		}
	}
}
