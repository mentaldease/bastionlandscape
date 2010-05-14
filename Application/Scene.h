#ifndef __SCENE_H__
#define __SCENE_H__

#include "../Application/ApplicationIncludes.h"
#include "../Application/Water.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class ScenePicker : public CoreObject
	{
	public:
		struct CreateInfo
		{
			ScenePtr	m_pScene;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

	public:
		ScenePicker();

		virtual bool Create(const boost::any& _rConfig);

		void Activate(const bool _bState);
		void Update(OctreeObjectPtrVecRef _rvOctreeObjects);
		MatrixPtr GetWorldMatrix();
		DisplayObjectPtr GetPickedObject();

	protected:
		void PickObjects(const Vector3& _f3RayBegin, const Vector3& _f3RayEnd, CoreObjectPtrVec& _rvObjects, OctreeObjectPtrVecRef _rvOctreeObjects);
		void UpdatePicking(const Vector3Ptr _f3RayBegin, const Vector3Ptr _f3RayEnd, Vector3Ptr _f3Out, OctreeObjectPtrVecRef _rvOctreeObjects);

	protected:
		ScenePtr			m_pScene;
		OctreeObjectPtrVec	m_vTraversedObjects;
		Vector3				m_f3RayBegin;
		Vector3				m_f3RayEnd;
		Vector3				m_f3Pick;
		DisplayObjectPtr	m_pPickCursorObject;
		DisplayObjectPtr	m_pPickedObject;
		bool				m_bActive;
	};

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

		static bool RegisterClasses();
		static void UnregisterClasses();
		static bool RegisterClass(const Key& _uClassNameKey, CreateClassFunc _Func);
		static bool UnregisterClass(const Key& _uClassNameKey);

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		void PreUpdate();
		void DrawOverlayText(const float _fX, const float _fY, const wstring& _wstrText, const Vector4& _f4Color);
		ApplicationRef GetApplication();
		Vector4 GetLightDir();
		OctreePtr GetOctree();
		CoreObjectPtr GetHierarchyObject(const Key _uNameKey);
		void ActivatePicking(const bool _bState);
		void NewDummy();
		void AddLandscape(LandscapePtr _pLandscape);
		WaterDataPtr GetWaterData(UIntRef _uCount);
		bool GetWaterLevel(const Vector3& _f3Pos, FloatRef _fLevel);

	protected:
		bool CreateFromLuaConfig(CreateInfoPtr _pInfo);
		bool CreateLoadRenderTargets(LuaObjectRef _rLuaObject);
		bool CreateLoadMaterials(LuaObjectRef _rLuaObject);
		bool CreateLoadOctree(LuaObjectRef _rLuaObject);
		bool CreateLoadWaterDataList(LuaObjectRef _rLuaObject);
		bool CreateLoadWaterData(LuaObjectRef _rLuaObject, WaterDataRef _rWaterData);
		bool CreateLoadCameras(LuaObjectRef _rLuaObject);
		bool CreateLoadRenderStages(LuaObjectRef _rLuaObject);
		bool CreateLoadRenderStage(LuaObjectRef _rLuaObject);
		bool CreateLoadHierarchy(LuaObjectRef _rLuaObject);

		void AddObject(DisplayObjectPtr _pObject);
		void UpdateDebug(OctreeObjectPtrVec& _rvOctreeObjects, OctreeNodePtrVec& _rvNodes);

	protected:
		static CoreObjectPtr CreateClassLandscape(LuaObjectRef _rTable, ScenePtr _pScene);
		static CoreObjectPtr CreateClassShpere(LuaObjectRef _rTable, ScenePtr _pScene);
		static CoreObjectPtr CreateClassSky(LuaObjectRef _rTable, ScenePtr _pScene);
		static CoreObjectPtr CreateClassLineManager(LuaObjectRef _rTable, ScenePtr _pScene);

	protected:
		static CreateClassFuncMap		s_mClasses;

		ApplicationRef					m_rApplication;
		CoreObjectPtrMap				m_mHierarchy;
		DisplayMaterialPtrMap			m_mMaterials;
		DisplayTexturePtrMap			m_mAdditionalRTs;
		DisplayCameraPtrMap				m_mCameras;
		DisplayRenderStagePtrMap		m_mRenderStages;
		DisplayRenderStagePtrVec		m_vRenderStages;
		CoreObjectPtrVec				m_vDummies;
		LandscapePtrVec					m_vLandscapes;
		Vector4							m_f4LightDir;
		WaterDataPtr					m_pWaterData;
		OctreePtr						m_pOctree;
		OctreeTraverseFuncFrustumPtr	m_pOTFFrustum;
		DisplayPtr						m_pDisplay;
		UInt							m_uWaterDataCount;
		string							m_strName;
		float							m_fWaterLevel;
		Key								m_uWaterLevelKey;
		Key								m_uWaterDataKey;
		Key								m_uFrustumModeKey;

		DebugTextOverlayPtr				m_pUITextOverlay;
		Key								m_uUIMainFontLabel;
		Key								m_uUIRenderStage;

		float							m_fDayTime;
		float							m_fVerticalOffset;

		ScenePicker						m_oPicker;
	};
}

#endif // __SCENE_H__
