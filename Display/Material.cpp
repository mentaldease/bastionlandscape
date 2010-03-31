#include "stdafx.h"
#include "../Core/Scripting.h"
#include "../Display/Camera.h"
#include "../Display/Effect.h"
#include "../Display/EffectParam.h"
#include "../Core/File.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayMaterial::DisplayMaterial(DisplayMaterialManagerRef _rMaterialManager)
	:	CoreObject(),
		m_rMaterialManager(_rMaterialManager),
		m_pEffect(NULL),
		m_vRenderList(),
		m_vParams(),
		m_hTechnique(NULL),
		m_uPassCount(0)
	{

	}

	DisplayMaterial::~DisplayMaterial()
	{

	}

	bool DisplayMaterial::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo) && (NULL != pInfo->m_pEffect);

		if (false != bResult)
		{
			Release();
			m_pEffect = pInfo->m_pEffect;
			if (NULL != pInfo->m_pLuaObject)
			{
				bResult = CreateFromLuaConfig(pInfo);
			}
		}

		return bResult;
	}

	void DisplayMaterial::Update()
	{

	}

	void DisplayMaterial::Release()
	{
		struct ParamReleaseAndDeleteFunction
		{
			ParamReleaseAndDeleteFunction(DisplayMaterialManager& _rMaterialManager)
				:	m_rMaterialManager(_rMaterialManager)
			{
			}

			void operator()(DisplayEffectParamPtr _pParam)
			{
				m_rMaterialManager.ReleaseParam(_pParam);
			}

			DisplayMaterialManager&	m_rMaterialManager;
		};
		for_each(m_vParams.begin(), m_vParams.end(), ParamReleaseAndDeleteFunction(m_rMaterialManager));
		m_vParams.clear();
	}

	void DisplayMaterial::RenderRequest(DisplayObjectPtr _pDisplayObject)
	{
		if (m_vRenderList.end() == find(m_vRenderList.begin(), m_vRenderList.end(), _pDisplayObject))
		{
			m_vRenderList.push_back(_pDisplayObject);
		}
	}

	void DisplayMaterial::Render()
	{
		m_pEffect->GetEffect()->SetTechnique(m_hTechnique);
#if 0
		for_each(m_vRenderList.begin(), m_vRenderList.end(), RenderObjectFunction(this));
#else
		DisplayPtr pDisplay = Display::GetInstance();
		EffectPtr pEffect = GetEffect()->GetEffect();
		size_t uCount = m_vRenderList.size();
		UInt uPassCount;
		pEffect->Begin(&uPassCount, 0);
		for (UInt uPass = 0 ; uPass < uPassCount ; ++uPass)
		{
			pDisplay->MRTRenderBeginPass(uPass);
			pEffect->BeginPass(uPass);
			for (size_t i = 0 ; uCount > i ; ++i)
			{
				DisplayObjectPtr pDisplayObject = m_vRenderList[i];
				pDisplayObject->RenderBegin();
				pDisplay->SetCurrentWorldMatrix(pDisplayObject->GetWorldMatrix());
				UseParams();
				pEffect->CommitChanges();
				pDisplayObject->Render();
				pDisplayObject->RenderEnd();
			}
			pEffect->EndPass();
			pDisplay->MRTRenderEndPass();
		}
		pEffect->End();
#endif
		m_vRenderList.clear();
	}

	void DisplayMaterial::UseParams()
	{
		struct UseParamFunction
		{
			void operator() (DisplayEffectParamPtr _pDisplayEffectParam)
			{
				_pDisplayEffectParam->Use();
			}
		};

		for_each(m_vParams.begin(), m_vParams.end(), UseParamFunction());
	}

	DisplayEffectPtr DisplayMaterial::GetEffect()
	{
		return m_pEffect;
	}

	DisplayMaterialManagerRef DisplayMaterial::GetMaterialManager()
	{
		return m_rMaterialManager;
	}

	UInt DisplayMaterial::GetPassCount()
	{
		return m_uPassCount;
	}

	bool DisplayMaterial::CreateFromLuaConfig(CreateInfoPtr _pInfo)
	{
		bool bResult = true;

		map<Key, int> mConfigParams;
		LuaObject oParams = (*_pInfo->m_pLuaObject)["params"];
		if (false == oParams.IsNil())
		{
			const int uCount = oParams.GetCount();
			string strSemanticValue;

			// pre-store all config info for parameters
			for (int i = 0 ; uCount > i ; ++i)
			{
				LuaObject oParam = oParams[i + 1];

				strSemanticValue = oParam["semantic"].GetString();
				const Key uSemanticKey = MakeKey(strSemanticValue);
				bResult = (mConfigParams.end() == mConfigParams.find(uSemanticKey));
				if (false == bResult)
				{
					break;
				}

				mConfigParams[uSemanticKey] = i;
			}
		}

		// create parameters
		if (false != bResult)
		{
			HandleMapRef rHandles = m_pEffect->GetHandles();
			HandleMap::iterator iHandle = rHandles.begin();
			HandleMap::iterator iEnd = rHandles.end();
			while (iEnd != iHandle)
			{
				const Key uSemanticKey = iHandle->first;
				map<Key, int>::iterator iParam = mConfigParams.find(uSemanticKey);
				LuaObject oLuaParam(Scripting::Lua::GetStateInstance());
				if (mConfigParams.end() != iParam)
				{
					oLuaParam = oParams[iParam->second + 1];
				}
				DisplayEffectParam::CreateInfo oDEPCInfo(oLuaParam, this, iHandle->second, uSemanticKey);
				DisplayEffectParamPtr pParam = m_rMaterialManager.CreateParam(uSemanticKey, boost::any(&oDEPCInfo));
				if (NULL == pParam)
				{
					const string& rstrName = m_pEffect->GetNameBySemanticKey(uSemanticKey);
					bResult = false;
					break;
				}
				if (mConfigParams.end() != iParam)
				{
					mConfigParams.erase(iParam);
				}
				m_vParams.push_back(pParam);
				++iHandle;
			}
		}

		if (false != bResult)
		{
			const char* pszTechniqueValue = (*_pInfo->m_pLuaObject)["technique"].GetString();
			m_hTechnique = m_pEffect->GetEffect()->GetTechniqueByName(pszTechniqueValue);
			bResult = (NULL != m_hTechnique);
			if (false != bResult)
			{
				D3DXTECHNIQUE_DESC oTechDesc;
				bResult = SUCCEEDED(m_pEffect->GetEffect()->GetTechniqueDesc(m_hTechnique, &oTechDesc));
				if (false != bResult)
				{
					m_uPassCount = oTechDesc.Passes;
				}
			}
		}

		return bResult;
	}
}
