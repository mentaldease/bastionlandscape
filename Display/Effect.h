#ifndef __EFFECT_H__
#define __EFFECT_H__

#include "../Display/Display.h"
#include "../Core/Config.h"

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

	protected:
		DisplayRef				m_rDisplay;
		EffectPtr				m_pEffect;
		DisplayMaterialPtrVec	m_vRenderList;

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
			ConfigPtr			m_pConfig;
			DisplayEffectPtr	m_pEffect;
		};

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

		bool LoadMaterial(const string& _strName, const string& _strPath);
		void UnloadMaterial(const string& _strName);
		void UnloadMaterial(const Key& _uNameKey);
		DisplayMaterialPtr GetMaterial(const string& _strName);

		bool LoadEffect(const string& _strName, const string& _strPath);
		void UnloadEffect(const string& _strName);
		DisplayEffectPtr GetEffect(const string& _strName);

		DisplayEffectParamPtr CreateParam(const string& _strSemanticName, const boost::any& _rConfig);
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

	protected:

	protected:
		DisplayEffectPtrMap		m_mEffects;
		DisplayMaterialPtrMap	m_mMaterials;
		CreateParamFuncMap		m_mParamCreators;
		FloatPtrMap				m_mFloatInfo;
		Vector2PtrMap			m_mVector2Info;
		Vector3PtrMap			m_mVector3Info;
		Vector4PtrMap			m_mVector4Info;
		MatrixPtrMap			m_mMatrixInfo;
		DisplayRef				m_rDisplay;

	private:
	};
}

#endif // __EFFECT_H__
