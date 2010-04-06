#include "stdafx.h"
#include "../Core/Scripting.h"
#include "../Display/Camera.h"
#include "../Display/Effect.h"
#include "../Display/EffectParam.h"
#include "../Display/EffectStateManager.h"
#include "../Core/File.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayMaterialManager::DisplayMaterialManager(DisplayRef _rDisplay)
	:	CoreObject(),
		m_mEffects(),
		m_mMaterials(),
		m_mParamCreators(),
		m_mFloatInfo(),
		m_mVector2Info(),
		m_mVector3Info(),
		m_mVector4Info(),
		m_mMatrixInfo(),
		m_rDisplay(_rDisplay),
		m_pIncludeInterface(NULL),
		m_strIncludeBasePath()
	{

	}

	DisplayMaterialManager::~DisplayMaterialManager()
	{

	}

	bool DisplayMaterialManager::Create(const boost::any& _rConfig)
	{
		Release();

		bool bResult = m_oParamsBuffer.Reserve(1024 * 1024);

		m_vDefaultParamKeys.resize(ECommonParamSemantic_COUNT);
		std::string aDefaultNames[ECommonParamSemantic_COUNT] =
		{
			"WORLDVIEWPROJ", "WORLD", "VIEW", "WORLDVIEW", "VIEWINV", "VIEWPROJ", "PROJ", "WORLDINVTRANSPOSE",
			"ENVIRONMENTTEX", "NORMALTEX", "DIFFUSETEX", "CAMERAPOS", "FRUSTUMCORNERS", "DIFFUSECOLOR",
			"RT2D00", "RT2D01", "RT2D02", "RT2D03", "RT2D04", "RT2D05", "RT2D06", "RT2D07",
			"ORT2D00", "ORT2D01", "ORT2D02", "ORT2D03", "ORT2D04", "ORT2D05", "ORT2D06", "ORT2D07",
			"TEX2D00", "TEX2D01", "TEX2D02", "TEX2D03", "TEX2D04", "TEX2D05", "TEX2D06", "TEX2D07"
		};

		for (UInt i = 0 ; ECommonParamSemantic_COUNT > i ; ++i)
		{
			m_vDefaultParamKeys[i] = MakeKey(aDefaultNames[i]);
		}

		m_vCurrentParamKeys = m_vDefaultParamKeys;

		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_WORLDVIEWPROJ]] = boost::bind(&DisplayEffectParamWORLDVIEWPROJ::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_WORLD]] = boost::bind(&DisplayEffectParamWORLD::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_VIEW]] = boost::bind(&DisplayEffectParamVIEW::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_WORLDVIEW]] = boost::bind(&DisplayEffectParamWORLDVIEW::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_VIEWINV]] = boost::bind(&DisplayEffectParamVIEWINV::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_VIEWPROJ]] = boost::bind(&DisplayEffectParamVIEWPROJ::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_PROJ]] = boost::bind(&DisplayEffectParamPROJ::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_WORLDINVTRANSPOSE]] = boost::bind(&DisplayEffectParamWORLDINVTRANSPOSE::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_ENVIRONMENTTEX]] = boost::bind(&DisplayEffectParamENVIRONMENTTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_NORMALTEX]] = boost::bind(&DisplayEffectParamNORMALTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_DIFFUSETEX]] = boost::bind(&DisplayEffectParamDIFFUSETEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_CAMERAPOS]] = boost::bind(&DisplayEffectParamVECTOR3::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_FRUSTUMCORNERS]] = boost::bind(&DisplayEffectParamFRUSTUMCORNERS::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_DIFFUSECOLOR]] = boost::bind(&DisplayEffectParamVECTOR4::CreateParam, _1);
		// render target texture
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_RT2D00]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_RT2D01]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_RT2D02]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_RT2D03]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_RT2D04]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_RT2D05]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_RT2D06]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_RT2D07]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		// original render target texture (rendered during normal process mode)
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_ORT2D00]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_ORT2D01]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_ORT2D02]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_ORT2D03]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_ORT2D04]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_ORT2D05]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_ORT2D06]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_ORT2D07]] = boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1);
		// standard texture
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_TEX2D00]] = boost::bind(&DisplayEffectParamDIFFUSETEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_TEX2D01]] = boost::bind(&DisplayEffectParamDIFFUSETEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_TEX2D02]] = boost::bind(&DisplayEffectParamDIFFUSETEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_TEX2D03]] = boost::bind(&DisplayEffectParamDIFFUSETEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_TEX2D04]] = boost::bind(&DisplayEffectParamDIFFUSETEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_TEX2D05]] = boost::bind(&DisplayEffectParamDIFFUSETEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_TEX2D06]] = boost::bind(&DisplayEffectParamDIFFUSETEX::CreateParam, _1);
		m_mParamCreators[m_vDefaultParamKeys[ECommonParamSemantic_TEX2D07]] = boost::bind(&DisplayEffectParamDIFFUSETEX::CreateParam, _1);

		m_pIncludeInterface = new DisplayEffectInclude;
		bResult = m_pIncludeInterface->Create(boost::any(0));

		return bResult;
	}

	void DisplayMaterialManager::Update()
	{
		m_oParamsBuffer.Clear();
	}

	void DisplayMaterialManager::Release()
	{
		UnloadAll();

		if (NULL != m_pIncludeInterface)
		{
			delete m_pIncludeInterface;
			m_pIncludeInterface = NULL;
		}

		m_mParamCreators.clear();
		m_vCurrentParamKeys.clear();
		m_vDefaultParamKeys.clear();
	}

	bool DisplayMaterialManager::CreateMaterial(const Key& _uNameKey, LuaObjectRef _rLuaObject)
	{
		DisplayMaterialPtrMap::iterator iPair = m_mMaterials.find(_uNameKey);
		bool bResult = (m_mMaterials.end() == iPair);

		if (false != bResult)
		{
			string strEffectPath;
			string strEffectName;
			DisplayEffectPtr pEffect;
			DisplayMaterialPtr pMaterial = NULL;

			strEffectPath = _rLuaObject["effect"].GetString();
			if (false != bResult)
			{
				FS::GetFileNameWithoutExt(strEffectPath, strEffectName);
				pEffect = GetEffect(strEffectName);
				if (NULL == pEffect)
				{
					bResult = LoadEffect(strEffectName, strEffectPath);
					pEffect = (false != bResult) ? GetEffect(strEffectName) : NULL;
				}
			}
			if (false != bResult)
			{
				DisplayMaterial::CreateInfo oDMCInfo = { pEffect, &_rLuaObject };
				pMaterial = new DisplayMaterial(*this);
				bResult = pMaterial->Create(boost::any(&oDMCInfo));
			}
			if (false != bResult)
			{
				m_mMaterials[_uNameKey] = pMaterial;
			}
			else if (NULL != pMaterial)
			{
				pMaterial->Release();
				delete pMaterial;
			}
		}

		return bResult;
	}

	void DisplayMaterialManager::UnloadMaterial(const string& _strName)
	{
		Key uKey = MakeKey(_strName);
		UnloadMaterial(uKey);
	}

	void DisplayMaterialManager::UnloadMaterial(const Key& _uNameKey)
	{
		DisplayMaterialPtrMap::iterator iPair = m_mMaterials.find(_uNameKey);
		bool bResult = (m_mMaterials.end() != iPair);
		if (false != bResult)
		{
			iPair->second->Release();
			delete iPair->second;
			m_mMaterials.erase(iPair);
		}
	}

	DisplayMaterialPtr DisplayMaterialManager::GetMaterial(const string& _strName)
	{
		const Key uKey = MakeKey(_strName);
		return GetMaterial(uKey);
	}

	DisplayMaterialPtr DisplayMaterialManager::GetMaterial(const Key& _strNameKey)
	{
		DisplayMaterialPtrMap::iterator iPair = m_mMaterials.find(_strNameKey);
		bool bResult = (m_mMaterials.end() != iPair);
		DisplayMaterialPtr pResult = (false != bResult) ? iPair->second : NULL;
		return pResult;
	}

	bool DisplayMaterialManager::LoadEffect(const string& _strName, const string& _strPath)
	{
		Key uKey = MakeKey(_strName);
		DisplayEffectPtrMap::iterator iPair = m_mEffects.find(uKey);
		bool bResult = (m_mEffects.end() == iPair);
		if (false != bResult)
		{
			DisplayEffect::CreateInfo oDECInfo = { _strPath, true };
			DisplayEffectPtr pEffect = new DisplayEffect(m_rDisplay);
			bResult = pEffect->Create(boost::any(&oDECInfo));
			if (false != bResult)
			{
				m_mEffects[uKey] = pEffect;
			}
			else
			{
				pEffect->Release();
				delete pEffect;
			}
		}
		return bResult;
	}

	void DisplayMaterialManager::UnloadEffect(const string& _strName)
	{
		Key uKey = 0;
		uKey = MakeKey(_strName);
		DisplayEffectPtrMap::iterator iPair = m_mEffects.find(uKey);
		bool bResult = (m_mEffects.end() != iPair);
		if (false != bResult)
		{
			iPair->second->Release();
			delete iPair->second;
			m_mEffects.erase(iPair);
		}
	}

	DisplayEffectPtr DisplayMaterialManager::GetEffect(const string& _strName)
	{
		Key uKey = 0;
		uKey = MakeKey(_strName);
		DisplayEffectPtrMap::iterator iPair = m_mEffects.find(uKey);
		bool bResult = (m_mEffects.end() != iPair);
		DisplayEffectPtr pResult = (false != bResult) ? iPair->second : NULL;
		return pResult;
	}

	DisplayEffectParamPtr DisplayMaterialManager::CreateParam(const string& _strSemanticName, const boost::any& _rConfig)
	{
		const Key uKey = MakeKey(_strSemanticName);
		return CreateParam(uKey, _rConfig);
	}

	bool DisplayMaterialManager::RegisterParamCreator(const Key& _uSemanticNameKey, CreateParamFunc _Func)
	{
		CreateParamFuncMap::iterator iPair = m_mParamCreators.find(_uSemanticNameKey);
		bool bResult (m_mParamCreators.end() == iPair);

		if (false != bResult)
		{
			m_mParamCreators[_uSemanticNameKey] = _Func;
		}

		return bResult;
	}

	bool DisplayMaterialManager::UnregisterParamCreator(const Key& _uSemanticNameKey)
	{
		CreateParamFuncMap::iterator iPair = m_mParamCreators.find(_uSemanticNameKey);
		bool bResult (m_mParamCreators.end() != iPair);

		if (false != bResult)
		{
			m_mParamCreators.erase(iPair);
		}

		return bResult;
	}

	DisplayEffectParamPtr DisplayMaterialManager::CreateParam(const Key& _uSemanticNameKey, const boost::any& _rConfig)
	{
		CreateParamFuncMap::iterator iCreateParamFunc = m_mParamCreators.find(_uSemanticNameKey);
		DisplayEffectParamPtr pResult = NULL;

		if (m_mParamCreators.end() != iCreateParamFunc)
		{
			CreateParamFunc pCreateParam = iCreateParamFunc->second;
			pResult = pCreateParam(_rConfig);
		}

		return pResult;
	}

	void DisplayMaterialManager::ReleaseParam(DisplayEffectParamPtr _pParam)
	{
		_pParam->Release();
		delete _pParam;
	}

	void DisplayMaterialManager::UnloadAll()
	{
		struct MaterialReleaseAndDeleteFunction
		{
			void operator()(pair<Key, DisplayMaterialPtr> _iPair)
			{
				_iPair.second->Release();
				delete _iPair.second;
			}
		};
		for_each(m_mMaterials.begin(), m_mMaterials.end(), MaterialReleaseAndDeleteFunction());
		m_mMaterials.clear();

		struct EffectReleaseAndDeleteFunction
		{
			void operator()(pair<Key, DisplayEffectPtr> _iPair)
			{
				_iPair.second->Release();
				delete _iPair.second;
			}
		};
		for_each(m_mEffects.begin(), m_mEffects.end(), EffectReleaseAndDeleteFunction());
		m_mEffects.clear();
	}

	DisplayRef DisplayMaterialManager::GetDisplay()
	{
		return m_rDisplay;
	}

	DisplayMemoryBufferPtr DisplayMaterialManager::GetParamsMemory()
	{
		return &m_oParamsBuffer;
	}

	void DisplayMaterialManager::SetFloatBySemantic(const Key& _uSemanticKey, FloatPtr _pData)
	{
		m_mFloatInfo[_uSemanticKey] = _pData;
	}

	FloatPtr DisplayMaterialManager::GetFloatBySemantic(const Key& _uSemanticKey)
	{
		return m_mFloatInfo[_uSemanticKey];
	}

	void DisplayMaterialManager::SetVector2BySemantic(const Key& _uSemanticKey, Vector2* _pData)
	{
		m_mVector2Info[_uSemanticKey] = _pData;
	}

	Vector2* DisplayMaterialManager::GetVector2BySemantic(const Key& _uSemanticKey)
	{
		return m_mVector2Info[_uSemanticKey];
	}

	void DisplayMaterialManager::SetVector3BySemantic(const Key& _uSemanticKey, Vector3* _pData)
	{
		m_mVector3Info[_uSemanticKey] = _pData;
	}

	Vector3* DisplayMaterialManager::GetVector3BySemantic(const Key& _uSemanticKey)
	{
		return m_mVector3Info[_uSemanticKey];
	}

	void DisplayMaterialManager::SetVector4BySemantic(const Key& _uSemanticKey, Vector4* _pData)
	{
		m_mVector4Info[_uSemanticKey] = _pData;
	}

	Vector4* DisplayMaterialManager::GetVector4BySemantic(const Key& _uSemanticKey)
	{
		return m_mVector4Info[_uSemanticKey];
	}

	void DisplayMaterialManager::SetMatrixBySemantic(const Key& _uSemanticKey, MatrixPtr _pData)
	{
		m_mMatrixInfo[_uSemanticKey] = _pData;
	}

	MatrixPtr DisplayMaterialManager::GetMatrixBySemantic(const Key& _uSemanticKey)
	{
		return m_mMatrixInfo[_uSemanticKey];
	}

	void DisplayMaterialManager::SetStructBySemantic(const Key& _uSemanticKey, VoidPtr _pData, const UInt _uSize)
	{
		StructDataRef rInfo = m_mStructInfo[_uSemanticKey];
		rInfo.m_pData = _pData;
		rInfo.m_uSize = _uSize;
	}

	VoidPtr DisplayMaterialManager::GetStructBySemantic(const Key& _uSemanticKey, UIntRef _uSize)
	{
		StructDataRef rInfo = m_mStructInfo[_uSemanticKey];
		_uSize = rInfo.m_uSize;
		return rInfo.m_pData;
	}

	bool DisplayMaterialManager::OverrideCommonParamSemantic(const ECommonParamSemantic _uCommonParam, const Key _uNewParamKey)
	{
		bool bResult = (UInt(ECommonParamSemantic_COUNT) > UInt(_uCommonParam)) && (m_mParamCreators.end() == m_mParamCreators.find(_uNewParamKey));
		if (false != bResult)
		{
			const Key uCurrentParamKey = m_vCurrentParamKeys[_uCommonParam];
			m_mParamCreators[_uNewParamKey] = m_mParamCreators[uCurrentParamKey];
			m_vCurrentParamKeys[_uCommonParam] = _uNewParamKey;
			m_mParamCreators.erase(m_mParamCreators.find(uCurrentParamKey));
		}
		return bResult;
	}

	bool DisplayMaterialManager::ResetCommonParamSemantic(const ECommonParamSemantic _uCommonParam)
	{
		Key uNewParamKey = 0;
		bool bResult = (UInt(ECommonParamSemantic_COUNT) > UInt(_uCommonParam))
			&& (uNewParamKey = m_vDefaultParamKeys[_uCommonParam])
			&& (m_mParamCreators.end() == m_mParamCreators.find(uNewParamKey));
		if (false != bResult)
		{
			const Key uCurrentParamKey = m_vCurrentParamKeys[_uCommonParam];
			m_mParamCreators[uNewParamKey] = m_mParamCreators[uCurrentParamKey];
			m_vCurrentParamKeys[_uCommonParam] = uNewParamKey;
			m_mParamCreators.erase(m_mParamCreators.find(uCurrentParamKey));
		}
		return bResult;
	}

	void DisplayMaterialManager::SetEffectIncludeBasePath(const string& _strPath)
	{
		m_strIncludeBasePath = _strPath;
	}

	const string& DisplayMaterialManager::GetEffectIncludeBasePath()
	{
		return m_strIncludeBasePath;
	}

	DisplayEffectIncludePtr DisplayMaterialManager::GetEffectIncludeInterface()
	{
		return m_pIncludeInterface;
	}
}
