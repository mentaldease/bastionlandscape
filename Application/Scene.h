#ifndef __SCENE_H__
#define __SCENE_H__

#include "../Application/ApplicationIncludes.h"
#include "../Application/Water.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Scene : public CoreObject
	{
	public:
		struct CreateInfo
		{
			string	m_strPath;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

	public:
		Scene(ApplicationRef _rApplication);
		virtual ~Scene();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		void PreUpdate();
		void DrawOverlayText(const float _fX, const float _fY, const wstring& _wstrText, const Vector4& _oColor);

	protected:
		bool CreateFromLuaConfig(CreateInfoPtr _pInfo);
		bool CreateLoadRenderTargets(LuaObjectRef _rLuaObject);
		bool CreateLoadMaterials(LuaObjectRef _rLuaObject);
		bool CreateLoadLandscapes(LuaObjectRef _rLuaObject);
		bool CreateLoadLandscape(LuaObjectRef _rLuaObject);
		bool CreateLoadWaterDataList(LuaObjectRef _rLuaObject);
		bool CreateLoadWaterData(LuaObjectRef _rLuaObject, WaterDataRef _rWaterData);
		bool CreateLoadCameras(LuaObjectRef _rLuaObject);
		bool CreateLoadRenderPasses(LuaObjectRef _rLuaObject);
		bool CreateLoadRenderPass(LuaObjectRef _rLuaObject);

	protected:
		ApplicationRef				m_rApplication;
		CoreObjectPtrMap			m_mAllObjects;
		LandscapePtrMap				m_mLandscapes;
		DisplayMaterialPtrMap		m_mMaterials;
		DisplayTexturePtrMap		m_mAdditionalRTs;
		DisplayCameraPtrMap			m_mCameras;
		DisplayRenderPassPtrMap		m_mRenderPasses;
		DisplayRenderPassPtrVec		m_vRenderPasses;
		Vector4						m_oLightDir;
		WaterDataPtr				m_pWaterData;
		UInt						m_uWaterDataCount;
		string						m_strName;
		float						m_fWaterLevel;
		Key							m_uWaterLevelKey;
		Key							m_uWaterDataKey;

		DisplayFontTextPtr			m_pUIText;
		DisplayFontPtr				m_pUIFont;
		DisplayMaterialPtr			m_pUIMaterial;
		wstring						m_wstrText;

	private:
	};
}

#endif // __SCENE_H__
