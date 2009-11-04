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

	protected:
		bool CreateFromLuaConfig(CreateInfoPtr _pInfo);
		bool CreateLoadMaterials(LuaObjectRef _rLuaObject);
		bool CreateLoadLandscapes(LuaObjectRef _rLuaObject);
		bool CreateLoadLandscape(LuaObjectRef _rLuaObject);
		bool CreateLoadPostProcesses(LuaObjectRef _rLuaObject);
		bool CreateLoadPostProcess(LuaObjectRef _rLuaObject);
		bool CreateLoadNormalProcesses(LuaObjectRef _rLuaObject);
		bool CreateLoadNormalProcess(LuaObjectRef _rLuaObject);
		bool CreateLoadWaterDataList(LuaObjectRef _rLuaObject);
		bool CreateLoadWaterData(LuaObjectRef _rLuaObject, WaterDataRef _rWaterData);

	protected:
		ApplicationRef				m_rApplication;
		CoreObjectPtrMap			m_mAllObjects;
		LandscapePtrMap				m_mLandscapes;
		DisplayMaterialPtrMap		m_mMaterials;
		DisplayPostProcessPtrMap	m_mPostProcesses;
		DisplayPostProcessPtrVec	m_vPostProcesses;
		DisplayNormalProcessPtrMap	m_mNormalProcesses;
		DisplayNormalProcessPtrVec	m_vNormalProcesses;
		Vector4						m_oLightDir;
		WaterDataPtr				m_pWaterData;
		UInt						m_uWaterDataCount;
		string						m_strName;
		float						m_fWaterLevel;
		Key							m_uWaterLevelKey;
		Key							m_uWaterDataKey;

	private:
	};
}

#endif // __SCENE_H__
