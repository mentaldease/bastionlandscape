#ifndef __EFFECTPARAM_H__
#define __EFFECTPARAM_H__

#include "../Display/DisplayTypes.h"
#include "../Display/Texture.h"
#include "../Display/Camera.h"
#include "../Core/Config.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParam : public CoreObject
	{
	public:
		struct CreateInfo
		{
			ConfigPtr			m_pConfig;
			ConfigShortcutPtr	m_pShortcut;
			DisplayMaterialPtr	m_pDisplayMaterial;
		};

	public:
		DisplayEffectParam(DisplayMaterialRef _rDisplayMaterial)
		:	m_rDisplayMaterial(_rDisplayMaterial),
			m_hData(NULL)
		{

		}

		virtual ~DisplayEffectParam()
		{

		}

		virtual bool Use() = 0;

	protected:
		DisplayMaterialRef	m_rDisplayMaterial;
		Handle				m_hData;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamWORLDVIEWPROJ : public DisplayEffectParam
	{
	public:
		DisplayEffectParamWORLDVIEWPROJ(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial),
			m_oWVP(),
			m_pWorld(NULL),
			m_pViewProj(NULL)
		{

		}

		virtual ~DisplayEffectParamWORLDVIEWPROJ()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}

			return bResult;
		}

		virtual bool Use()
		{
			m_pViewProj = m_rDisplayMaterial.GetMaterialManager().GetDisplay().GetCurrentCamera()->GetMatrix(DisplayCamera::EMatrix_VIEWPROJ);
			m_pWorld = m_rDisplayMaterial.GetMaterialManager().GetDisplay().GetCurrentWorldMatrix();
			if ((NULL != m_pWorld) && (NULL != m_pViewProj))
			{
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetMatrix(m_hData, D3DXMatrixMultiply(&m_oWVP, m_pWorld, m_pViewProj));
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamWORLDVIEWPROJ(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		Matrix		m_oWVP;
		MatrixPtr	m_pWorld;
		MatrixPtr	m_pViewProj;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamWORLD : public DisplayEffectParam
	{
	public:
		DisplayEffectParamWORLD(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial),
			m_pWorld(NULL)
		{

		}

		virtual ~DisplayEffectParamWORLD()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}

			return bResult;
		}

		virtual bool Use()
		{
			m_pWorld = m_rDisplayMaterial.GetMaterialManager().GetDisplay().GetCurrentWorldMatrix();
			if (NULL != m_pWorld)
			{
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetMatrix(m_hData, m_pWorld);
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamWORLD(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		MatrixPtr	m_pWorld;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamVIEW : public DisplayEffectParam
	{
	public:
		DisplayEffectParamVIEW(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial),
			m_pView(NULL)
		{

		}

		virtual ~DisplayEffectParamVIEW()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}

			return bResult;
		}

		virtual bool Use()
		{
			m_pView = m_rDisplayMaterial.GetMaterialManager().GetDisplay().GetCurrentCamera()->GetMatrix(DisplayCamera::EMatrix_VIEW);
			if (NULL != m_pView)
			{
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetMatrix(m_hData, m_pView);
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamVIEW(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		MatrixPtr	m_pView;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamVIEWINV : public DisplayEffectParam
	{
	public:
		DisplayEffectParamVIEWINV(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial),
			m_pViewInv(NULL)
		{

		}

		virtual ~DisplayEffectParamVIEWINV()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}

			return bResult;
		}

		virtual bool Use()
		{
			m_pViewInv = m_rDisplayMaterial.GetMaterialManager().GetDisplay().GetCurrentCamera()->GetMatrix(DisplayCamera::EMatrix_VIEWINV);
			if (NULL != m_pViewInv)
			{
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetMatrix(m_hData, m_pViewInv);
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamVIEWINV(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		MatrixPtr	m_pViewInv;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamVIEWPROJ : public DisplayEffectParam
	{
	public:
		DisplayEffectParamVIEWPROJ(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial),
			m_pViewProj(NULL)
		{

		}

		virtual ~DisplayEffectParamVIEWPROJ()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}

			return bResult;
		}

		virtual bool Use()
		{
			m_pViewProj = m_rDisplayMaterial.GetMaterialManager().GetDisplay().GetCurrentCamera()->GetMatrix(DisplayCamera::EMatrix_VIEWPROJ);
			if (NULL != m_pViewProj)
			{
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetMatrix(m_hData, m_pViewProj);
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamVIEWPROJ(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		MatrixPtr	m_pViewProj;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamWORLDINVTRANSPOSE : public DisplayEffectParam
	{
	public:
		DisplayEffectParamWORLDINVTRANSPOSE(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial),
			m_pWorld(NULL)
		{

		}

		virtual ~DisplayEffectParamWORLDINVTRANSPOSE()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}

			return bResult;
		}

		virtual bool Use()
		{
			m_pWorld = m_rDisplayMaterial.GetMaterialManager().GetDisplay().GetCurrentWorldInvTransposeMatrix();
			if (NULL != m_pWorld)
			{
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetMatrix(m_hData, m_pWorld);
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamWORLDINVTRANSPOSE(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		MatrixPtr	m_pWorld;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamENVIRONMENTTEX : public DisplayEffectParam
	{
	public:
		DisplayEffectParamENVIRONMENTTEX(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial),
			m_pTexture(NULL)
		{

		}

		virtual ~DisplayEffectParamENVIRONMENTTEX()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}
			if (false != bResult)
			{
				bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "name", m_strName);
			}
			if (false != bResult)
			{
				bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "value", m_strPath);
			}

			return bResult;
		}

		virtual bool Use()
		{
			if (NULL == m_pTexture)
			{
				DisplayTextureManagerPtr pTextureManager = m_rDisplayMaterial.GetMaterialManager().GetDisplay().GetTextureManager();
				if (false != pTextureManager->Load(m_strName, m_strPath, DisplayTexture::EType_CUBE))
				{
					m_pTexture = pTextureManager->Get(m_strName);
				}
			}
			if (NULL != m_pTexture)
			{
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetTexture(m_hData, m_pTexture->GetBase());
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamENVIRONMENTTEX(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		DisplayTexturePtr	m_pTexture;
		string				m_strName;
		string				m_strPath;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamNORMALTEX : public DisplayEffectParam
	{
	public:
		DisplayEffectParamNORMALTEX(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial),
			m_pTexture(NULL)
		{

		}

		virtual ~DisplayEffectParamNORMALTEX()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}
			if (false != bResult)
			{
				bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "name", m_strName);
			}
			if (false != bResult)
			{
				bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "value", m_strPath);
			}

			return bResult;
		}

		virtual bool Use()
		{
			if (NULL == m_pTexture)
			{
				DisplayTextureManagerPtr pTextureManager = m_rDisplayMaterial.GetMaterialManager().GetDisplay().GetTextureManager();
				if (false != pTextureManager->Load(m_strName, m_strPath, DisplayTexture::EType_2D))
				{
					m_pTexture = pTextureManager->Get(m_strName);
				}
			}
			if (NULL != m_pTexture)
			{
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetTexture(m_hData, m_pTexture->GetBase());
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamNORMALTEX(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		DisplayTexturePtr	m_pTexture;
		string				m_strName;
		string				m_strPath;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamFLOAT : public DisplayEffectParam
	{
	public:
		DisplayEffectParamFLOAT(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial)
		{

		}

		virtual ~DisplayEffectParamFLOAT()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}
			if (false != bResult)
			{
				m_uSemanticKey = MakeKey(strSemanticName);
			}

			return bResult;
		}

		virtual bool Use()
		{
			m_pData = m_rDisplayMaterial.GetMaterialManager().GetFloatBySemantic(m_uSemanticKey);
			if (NULL != m_pData)
			{
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetFloat(m_hData, *m_pData);
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamFLOAT(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		FloatPtr	m_pData;
		Key			m_uSemanticKey;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamVECTOR2 : public DisplayEffectParam
	{
	public:
		DisplayEffectParamVECTOR2(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial)
		{

		}

		virtual ~DisplayEffectParamVECTOR2()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}
			if (false != bResult)
			{
				m_uSemanticKey = MakeKey(strSemanticName);
			}

			return bResult;
		}

		virtual bool Use()
		{
			m_pData = m_rDisplayMaterial.GetMaterialManager().GetVector2BySemantic(m_uSemanticKey);
			if (NULL != m_pData)
			{
				m_oData.x = m_pData->x;
				m_oData.y = m_pData->y;
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetVector(m_hData, &m_oData);
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamVECTOR2(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		Vector4		m_oData;
		Vector2*	m_pData;
		Key			m_uSemanticKey;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamVECTOR3 : public DisplayEffectParam
	{
	public:
		DisplayEffectParamVECTOR3(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial)
		{

		}

		virtual ~DisplayEffectParamVECTOR3()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}
			if (false != bResult)
			{
				m_uSemanticKey = MakeKey(strSemanticName);
			}

			return bResult;
		}

		virtual bool Use()
		{
			m_pData = m_rDisplayMaterial.GetMaterialManager().GetVector3BySemantic(m_uSemanticKey);
			if (NULL != m_pData)
			{
				m_oData.x = m_pData->x;
				m_oData.y = m_pData->y;
				m_oData.z = m_pData->z;
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetVector(m_hData, &m_oData);
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamVECTOR3(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		Vector4		m_oData;
		Vector3*	m_pData;
		Key			m_uSemanticKey;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamVECTOR4 : public DisplayEffectParam
	{
	public:
		DisplayEffectParamVECTOR4(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial)
		{

		}

		virtual ~DisplayEffectParamVECTOR4()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}
			if (false != bResult)
			{
				m_uSemanticKey = MakeKey(strSemanticName);
			}

			return bResult;
		}

		virtual bool Use()
		{
			m_pData = m_rDisplayMaterial.GetMaterialManager().GetVector4BySemantic(m_uSemanticKey);
			if (NULL != m_pData)
			{
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetVector(m_hData, m_pData);
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamVECTOR4(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		Vector4*	m_pData;
		Key			m_uSemanticKey;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamMATRIX : public DisplayEffectParam
	{
	public:
		DisplayEffectParamMATRIX(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial)
		{

		}

		virtual ~DisplayEffectParamMATRIX()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}
			if (false != bResult)
			{
				m_uSemanticKey = MakeKey(strSemanticName);
			}

			return bResult;
		}

		virtual bool Use()
		{
			m_pData = m_rDisplayMaterial.GetMaterialManager().GetMatrixBySemantic(m_uSemanticKey);
			if (NULL != m_pData)
			{
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetMatrix(m_hData, m_pData);
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamMATRIX(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		MatrixPtr	m_pData;
		Key			m_uSemanticKey;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamDIFFUSETEX : public DisplayEffectParam
	{
	public:
		DisplayEffectParamDIFFUSETEX(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial),
			m_pTexture(NULL)
		{

		}

		virtual ~DisplayEffectParamDIFFUSETEX()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}
			if (false != bResult)
			{
				bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "name", m_strName);
			}
			if (false != bResult)
			{
				bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "value", m_strPath);
			}

			return bResult;
		}

		virtual bool Use()
		{
			if (NULL == m_pTexture)
			{
				DisplayTextureManagerPtr pTextureManager = m_rDisplayMaterial.GetMaterialManager().GetDisplay().GetTextureManager();
				if (false != pTextureManager->Load(m_strName, m_strPath, DisplayTexture::EType_2D))
				{
					m_pTexture = pTextureManager->Get(m_strName);
				}
			}
			if (NULL != m_pTexture)
			{
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetTexture(m_hData, m_pTexture->GetBase());
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamDIFFUSETEX(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		DisplayTexturePtr	m_pTexture;
		string				m_strName;
		string				m_strPath;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamSEMANTICTEX : public DisplayEffectParam
	{
	public:
		DisplayEffectParamSEMANTICTEX(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial),
			m_pTexture(NULL)
		{

		}

		virtual ~DisplayEffectParamSEMANTICTEX()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}
			if (false != bResult)
			{
				m_uSemanticKey = MakeKey(strSemanticName);
			}

			return bResult;
		}

		virtual bool Use()
		{
			DisplayTextureManagerPtr pTextureManager = m_rDisplayMaterial.GetMaterialManager().GetDisplay().GetTextureManager();
			m_pTexture = pTextureManager->GetBySemantic(m_uSemanticKey);
			if (NULL != m_pTexture)
			{
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetTexture(m_hData, m_pTexture->GetBase());
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamSEMANTICTEX(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		DisplayTexturePtr	m_pTexture;
		Key					m_uSemanticKey;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectParamFRUSTUMCORNERS : public DisplayEffectParam
	{
	public:
		DisplayEffectParamFRUSTUMCORNERS(DisplayMaterialRef _rDisplayMaterial)
		:	DisplayEffectParam(_rDisplayMaterial)
		{

		}

		virtual ~DisplayEffectParamFRUSTUMCORNERS()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
			string strSemanticName;
			bool bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "semantic", strSemanticName);

			if (false != bResult)
			{
				m_hData = pInfo->m_pDisplayMaterial->GetEffect()->GetEffect()->GetParameterBySemantic(NULL, strSemanticName.c_str());
				bResult = (NULL != m_hData);
			}
			if (false != bResult)
			{
				m_uSemanticKey = MakeKey(strSemanticName);
			}

			return bResult;
		}

		virtual bool Use()
		{
			Vector3* pData = m_rDisplayMaterial.GetMaterialManager().GetVector3BySemantic(m_uSemanticKey);
			if (NULL != pData)
			{
				for (UInt i = 0 ; DisplayCamera::EFrustumCorner_COUNT > i ; ++i)
				{
					m_aData[i] = Vector4(pData[i].x, pData[i].y, pData[i].z, 0.0f);
				}
				HRESULT hResult = m_rDisplayMaterial.GetEffect()->GetEffect()->SetVectorArray(m_hData, m_aData, DisplayCamera::EFrustumCorner_COUNT);
				return SUCCEEDED(hResult);
			}
			return false;
		}

		static DisplayEffectParamPtr CreateParam(const boost::any& _rConfig)
		{
			DisplayEffectParam::CreateInfo* pDEPCInfo = boost::any_cast<DisplayEffectParam::CreateInfo*>(_rConfig);
			DisplayEffectParamPtr pParam = new DisplayEffectParamFRUSTUMCORNERS(*(pDEPCInfo->m_pDisplayMaterial));
			if (false == pParam->Create(_rConfig))
			{
				pParam->Release();
				delete pParam;
				pParam = NULL;
			}
			return pParam;
		}

	protected:
		Vector4	m_aData[DisplayCamera::EFrustumCorner_COUNT];
		Key		m_uSemanticKey;

	private:
	};
}

#endif // __EFFECTPARAM_H__