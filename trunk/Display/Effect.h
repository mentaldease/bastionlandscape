#ifndef __EFFECT_H__
#define __EFFECT_H__

#include "../Display/Display.h"
#include "../Core/CoreTypes.h"

namespace ElixirEngine
{
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

		void RegisterParamCreator(const Key& _uSemanticNameKey, CreateParamFunc _Func);
		void UnregisterParamCreator(const Key& _uSemanticNameKey);
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
		DisplayRef				m_rDisplay;

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
