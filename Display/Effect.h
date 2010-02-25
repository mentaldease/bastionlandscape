#ifndef __EFFECT_H__
#define __EFFECT_H__

#include "../Display/Display.h"
#include "../Core/CoreTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffectInclude : public CoreObject, public ID3DXInclude
	{
	public:
		DisplayEffectInclude();
		virtual ~DisplayEffectInclude();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Release();

		HRESULT __stdcall Open(
			D3DXINCLUDE_TYPE IncludeType,
			LPCSTR pFileName,
			LPCVOID pParentData,
			LPCVOID *ppData,
			UINT * pBytes
			);

		HRESULT __stdcall Close(
			LPCVOID pData
			);

	protected:
		CharPtrVec	m_vBuffers;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffect : public CoreObject
	{
	public:
		struct CreateInfo
		{
			string	m_strPath;
			bool	m_bIsText;
		};

	public:
		DisplayEffect(DisplayRef _rDisplay);
		virtual ~DisplayEffect();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual void RenderRequest(DisplayMaterialPtr _pDisplayMaterial);
		virtual void Render();

		virtual EffectPtr GetEffect();

		Handle GetHandleBySemanticKey(const Key& _uKey);
		HandleMapRef GetHandles();

	protected:
		bool GetParameters();

	protected:
		DisplayRef				m_rDisplay;
		EffectPtr				m_pEffect;
		DisplayMaterialPtrVec	m_vRenderList;
		HandleMap				m_mHandles;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayMaterial : public CoreObject
	{
	public:
		struct CreateInfo
		{
			DisplayEffectPtr	m_pEffect;
			LuaObjectPtr		m_pLuaObject;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInforef;

	public:
		DisplayMaterial(DisplayMaterialManagerRef _rMaterialManager);
		virtual ~DisplayMaterial();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual void RenderRequest(DisplayObjectPtr _pDisplayObject);
		virtual void Render();
		virtual void UseParams();

		virtual DisplayEffectPtr GetEffect();
		virtual DisplayMaterialManagerRef GetMaterialManager();

	protected:
		bool CreateFromLuaConfig(CreateInfoPtr _pInfo);

	protected:
		DisplayMaterialManagerRef	m_rMaterialManager;
		DisplayEffectPtr			m_pEffect;
		DisplayObjectPtrVec			m_vRenderList;
		DisplayEffectParamPtrVec	m_vParams;
		Handle						m_hTechnique;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayMaterialManager : public CoreObject
	{
	public:
		// this enum enables the application to redefine effect parameters semantics
		// that are internally used by various display classes.
		enum ECommonParamSemantic
		{
			// common
			ECommonParamSemantic_WORLDVIEWPROJ,
			ECommonParamSemantic_WORLD,
			ECommonParamSemantic_VIEW,
			ECommonParamSemantic_VIEWINV,
			ECommonParamSemantic_VIEWPROJ,
			ECommonParamSemantic_PROJ,
			ECommonParamSemantic_WORLDINVTRANSPOSE,
			ECommonParamSemantic_ENVIRONMENTTEX,
			ECommonParamSemantic_NORMALTEX,
			ECommonParamSemantic_DIFFUSETEX,
			ECommonParamSemantic_CAMERAPOS,
			ECommonParamSemantic_FRUSTUMCORNERS,
			ECommonParamSemantic_DIFFUSECOLOR,
			// render target texture
			ECommonParamSemantic_RT2D00,
			ECommonParamSemantic_RT2D01,
			ECommonParamSemantic_RT2D02,
			ECommonParamSemantic_RT2D03,
			ECommonParamSemantic_RT2D04,
			ECommonParamSemantic_RT2D05,
			ECommonParamSemantic_RT2D06,
			ECommonParamSemantic_RT2D07,
			// original render target texture (rendered during normal process mode)
			ECommonParamSemantic_ORT2D00,
			ECommonParamSemantic_ORT2D01,
			ECommonParamSemantic_ORT2D02,
			ECommonParamSemantic_ORT2D03,
			ECommonParamSemantic_ORT2D04,
			ECommonParamSemantic_ORT2D05,
			ECommonParamSemantic_ORT2D06,
			ECommonParamSemantic_ORT2D07,
			// standard texture
			ECommonParamSemantic_TEX2D00,
			ECommonParamSemantic_TEX2D01,
			ECommonParamSemantic_TEX2D02,
			ECommonParamSemantic_TEX2D03,
			ECommonParamSemantic_TEX2D04,
			ECommonParamSemantic_TEX2D05,
			ECommonParamSemantic_TEX2D06,
			ECommonParamSemantic_TEX2D07,
			ECommonParamSemantic_COUNT // always last enum member
		};
	public:
		DisplayMaterialManager(DisplayRef _rDisplay);
		virtual ~DisplayMaterialManager();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		bool CreateMaterial(const Key& _uNameKey, LuaObjectRef _rLuaObject);
		void UnloadMaterial(const string& _strName);
		void UnloadMaterial(const Key& _uNameKey);
		DisplayMaterialPtr GetMaterial(const string& _strName);
		DisplayMaterialPtr GetMaterial(const Key& _strNameKey);

		bool LoadEffect(const string& _strName, const string& _strPath);
		void UnloadEffect(const string& _strName);
		DisplayEffectPtr GetEffect(const string& _strName);

		bool RegisterParamCreator(const Key& _uSemanticNameKey, CreateParamFunc _Func);
		bool UnregisterParamCreator(const Key& _uSemanticNameKey);
		DisplayEffectParamPtr CreateParam(const string& _strSemanticName, const boost::any& _rConfig);
		DisplayEffectParamPtr CreateParam(const Key& _uSemanticNameKey, const boost::any& _rConfig);
		void ReleaseParam(DisplayEffectParamPtr _pParam);

		void UnloadAll();

		DisplayRef GetDisplay();

		void SetFloatBySemantic(const Key& _uSemanticKey, FloatPtr _pData);
		FloatPtr GetFloatBySemantic(const Key& _uSemanticKey);
		void SetVector2BySemantic(const Key& _uSemanticKey, Vector2* _pData);
		Vector2* GetVector2BySemantic(const Key& _uSemanticKey);
		void SetVector3BySemantic(const Key& _uSemanticKey, Vector3* _pData);
		Vector3* GetVector3BySemantic(const Key& _uSemanticKey);
		void SetVector4BySemantic(const Key& _uSemanticKey, Vector4* _pData);
		Vector4* GetVector4BySemantic(const Key& _uSemanticKey);
		void SetMatrixBySemantic(const Key& _uSemanticKey, MatrixPtr _pData);
		MatrixPtr GetMatrixBySemantic(const Key& _uSemanticKey);
		void SetStructBySemantic(const Key& _uSemanticKey, VoidPtr _pData, const UInt _uSize);
		VoidPtr GetStructBySemantic(const Key& _uSemanticKey, UIntRef _uSize);

		bool OverrideCommonParamSemantic(const ECommonParamSemantic _uCommonParam, const Key _uNewParamKey);
		bool ResetCommonParamSemantic(const ECommonParamSemantic _uCommonParam);

		void SetIncludeBasePath(const string& _strPath);
		const string& GetIncludeBasePath();
		DisplayEffectIncludePtr GetIncludeInterface();

	protected:
		struct StructData
		{
			StructData();
			VoidPtr	m_pData;
			UInt	m_uSize;
		};
		typedef StructData* StructDataPtr;
		typedef StructData& StructDataRef;
		typedef map<Key, StructData> StructDataMap;

	protected:
		DisplayEffectPtrMap		m_mEffects;
		DisplayMaterialPtrMap	m_mMaterials;
		CreateParamFuncMap		m_mParamCreators;
		FloatPtrMap				m_mFloatInfo;
		Vector2PtrMap			m_mVector2Info;
		Vector3PtrMap			m_mVector3Info;
		Vector4PtrMap			m_mVector4Info;
		MatrixPtrMap			m_mMatrixInfo;
		StructDataMap			m_mStructInfo;
		KeyVec					m_vCurrentParamKeys;
		KeyVec					m_vDefaultParamKeys;
		DisplayRef				m_rDisplay;
		DisplayEffectIncludePtr	m_pIncludeInterface;
		string					m_strIncludeBasePath;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct RenderObjectFunction
	{
		RenderObjectFunction(DisplayMaterialPtr _pMaterial)
		:	m_pMaterial(_pMaterial)
		{

		}

		void operator() (DisplayObjectPtr _pDisplayObject)
		{
			DisplayRef rDisplay = m_pMaterial->GetMaterialManager().GetDisplay();
			EffectPtr pEffect = m_pMaterial->GetEffect()->GetEffect();
			UInt uPassCount;
			pEffect->Begin(&uPassCount, 0);
			_pDisplayObject->RenderBegin();
			rDisplay.SetCurrentWorldMatrix(_pDisplayObject->GetWorldMatrix());
			for (UInt uPass = 0 ; uPass < uPassCount ; ++uPass)
			{
				rDisplay.MRTRenderBeginPass(uPass);
				pEffect->BeginPass(uPass);
				m_pMaterial->UseParams();
				pEffect->CommitChanges();
				_pDisplayObject->Render();
				pEffect->EndPass();
				rDisplay.MRTRenderEndPass();
			}
			_pDisplayObject->RenderEnd();
			pEffect->End();
		}

		DisplayMaterialPtr	m_pMaterial;
	};
}

#endif // __EFFECT_H__
