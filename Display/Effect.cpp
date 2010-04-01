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

	DisplayMemoryBuffer::DisplayMemoryBuffer()
	:	m_pBuffer(NULL),
		m_uCapacity(0),
		m_uSize(0)
	{

	}

	DisplayMemoryBuffer::~DisplayMemoryBuffer()
	{
		if (NULL != m_pBuffer)
		{
			delete[] m_pBuffer;
		}
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayMaterialManager::StructData::StructData()
	:	m_pData(NULL),
		m_uSize(0)
	{
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayEffectInclude::DisplayEffectInclude()
	:	CoreObject(),
		ID3DXInclude(),
		m_vBuffers()
	{

	}

	DisplayEffectInclude::~DisplayEffectInclude()
	{

	}

	bool DisplayEffectInclude::Create(const boost::any& _rConfig)
	{
		return true;
	}

	void DisplayEffectInclude::Release()
	{

	}

	HRESULT DisplayEffectInclude::Open(
		D3DXINCLUDE_TYPE IncludeType,
		LPCSTR pFileName,
		LPCVOID pParentData,
		LPCVOID *ppData,
		UINT * pBytes
		)
	{
		string strFileName;
		FS::ComposePath(strFileName, Display::GetInstance()->GetMaterialManager()->GetIncludeBasePath(), string(pFileName));
		FilePtr pFile = NULL;

		HRESULT hResult = (false == strFileName.empty()) ? S_OK : E_FAIL;
		if (SUCCEEDED(hResult))
		{
			pFile = FS::GetRoot()->OpenFile(strFileName, FS::EOpenMode_READTEXT);
			hResult = (NULL != pFile) ? S_OK : E_FAIL;
		}
		if (SUCCEEDED(hResult))
		{
			int sSize = pFile->Size();
			char* pSourceCode = new char[sSize];
			sSize = pFile->Read(pSourceCode, sSize);
			FS::GetRoot()->CloseFile(pFile);
			m_vBuffers.push_back(pSourceCode);
			*ppData = pSourceCode;
			*pBytes = sSize;
		}

		return hResult;
	}

	HRESULT DisplayEffectInclude::Close(
		LPCVOID pData
		)
	{
		CharPtr pSourceCode = (CharPtr)pData;
		CharPtrVec::iterator iSourceCode = find(m_vBuffers.begin(), m_vBuffers.end(), pSourceCode);

		HRESULT hResult = (m_vBuffers.end() != iSourceCode) ? S_OK : E_FAIL;
		if (SUCCEEDED(hResult))
		{
			delete[] pSourceCode;
			m_vBuffers.erase(iSourceCode);
		}

		return hResult;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayEffect::DisplayEffect(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_pEffect(NULL),
		m_vRenderList()
	{

	}

	DisplayEffect::~DisplayEffect()
	{

	}

	bool DisplayEffect::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		FilePtr pFile = NULL;
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			Release();
			pFile = FS::GetRoot()->OpenFile(pInfo->m_strPath, pInfo->m_bIsText ? FS::EOpenMode_READTEXT : FS::EOpenMode_READBINARY);
			bResult = (NULL != pFile);
		}
		if (false != bResult)
		{
			int sSize = pFile->Size();
			char* pSourceCode = new char[sSize];
			sSize = pFile->Read(pSourceCode, sSize);
			FS::GetRoot()->CloseFile(pFile);

			BufferPtr pCompilErrors;
			HRESULT hResult = D3DXCreateEffect(
				m_rDisplay.GetDevicePtr(),
				pSourceCode,
				sSize,
				NULL, // D3DXMACRO Defines,
				m_rDisplay.GetMaterialManager()->GetIncludeInterface(), // LPD3DXINCLUDE Includes,
				/*D3DXSHADER_SKIPOPTIMIZATION | */D3DXSHADER_DEBUG | D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY,
				NULL, // LPD3DXEFFECTPOOL Pool,
				&m_pEffect,
				&pCompilErrors);

			delete[] pSourceCode;

			bResult = SUCCEEDED(hResult);
			if (false == bResult)
			{
				char* ptr = static_cast<char*>(pCompilErrors->GetBufferPointer());
				int a = 0;
				++a;
			}
			else
			{
				m_strName = pInfo->m_strPath;
				m_mSemantics[0] = string("");
				bResult = GetParameters();
			}

			if (NULL != pCompilErrors)
			{
				pCompilErrors->Release();
			}
		}

		return bResult;
	}

	void DisplayEffect::Update()
	{

	}

	void DisplayEffect::Release()
	{
		if (NULL != m_pEffect)
		{
			m_pEffect->Release();
			m_pEffect = NULL;
		}
		m_mSemantics.clear();
		m_mHandles.clear();
	}

	void DisplayEffect::RenderRequest(DisplayMaterialPtr _pDisplayMaterial)
	{
		if (m_vRenderList.end() == find(m_vRenderList.begin(), m_vRenderList.end(), _pDisplayMaterial))
		{
			m_vRenderList.push_back(_pDisplayMaterial);
		}
	}

	void DisplayEffect::Render()
	{
		struct RenderMaterialFunction
		{
			void operator() (DisplayMaterialPtr _pDisplayMaterial)
			{
				_pDisplayMaterial->Render();
			}
		};

		static RenderMaterialFunction oRMF;
		for_each(m_vRenderList.begin(), m_vRenderList.end(), RenderMaterialFunction());
		m_vRenderList.clear();
	}

	EffectPtr DisplayEffect::GetEffect()
	{
		return m_pEffect;
	}

	Handle DisplayEffect::GetHandleBySemanticKey(const Key& _uKey)
	{
		HandleMap::iterator iHandle = m_mHandles.find(_uKey);
		return ((m_mHandles.end() != iHandle) ? iHandle->second : NULL);
	}

	HandleMapRef DisplayEffect::GetHandles()
	{
		return m_mHandles;
	}

	const string& DisplayEffect::GetNameBySemanticKey(const Key& _uKey)
	{
		map<Key, string>::const_iterator iPair = m_mSemantics.find(_uKey);
		if (m_mSemantics.end() != iPair)
		{
			return iPair->second;
		}
		return m_mSemantics[0];
	}

	bool DisplayEffect::GetParameters()
	{
		EffectDesc oEffectDesc;
		bool bResult = SUCCEEDED(m_pEffect->GetDesc(&oEffectDesc));

		if (false != bResult)
		{
			EffectParamDesc oParamDesc;
			Key uSemanticKey;
			Handle hParam;

			for (UInt i = 0 ; oEffectDesc.Parameters > i ; ++i)
			{
				hParam = m_pEffect->GetParameter(NULL, i);
				bResult = (NULL != hParam) && SUCCEEDED(m_pEffect->GetParameterDesc(hParam, &oParamDesc));
				if (false == bResult)
				{
					break;
				}
				if (NULL != oParamDesc.Semantic)
				{
					uSemanticKey = MakeKey(string(oParamDesc.Semantic));
					bResult = (NULL == GetHandleBySemanticKey(uSemanticKey));
				}
				else
				{
					continue;
				}
				if (false == bResult)
				{
					break;
				}
				m_mHandles[uSemanticKey] = hParam;
				m_mSemantics[uSemanticKey] = string(oParamDesc.Semantic);
			}
		}

		return bResult;
	}
}
