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

	protected:

	protected:
		DisplayEffectPtrMap		m_mEffects;
		DisplayMaterialPtrMap	m_mMaterials;
		CreateParamFuncMap		m_mParamCreators;
		DisplayRef				m_rDisplay;

	private:
	};
}

#endif // __EFFECT_H__
