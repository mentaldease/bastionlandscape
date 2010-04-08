#ifndef __APPLICATIONTYPES_H__
#define __APPLICATIONTYPES_H__

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Application;
	typedef Application* ApplicationPtr;
	typedef Application& ApplicationRef;

	class Scene;
	typedef Scene* ScenePtr;
	typedef Scene& SceneRef;

	class Sky;
	typedef Sky* SkyPtr;
	typedef Sky& SkyRef;

	class CameraListener;
	typedef CameraListener* CameraListenerPtr;
	typedef CameraListener& CameraListenerRef;

	struct WaterData;
	typedef WaterData* WaterDataPtr;
	typedef WaterData& WaterDataRef;

	class DebugTextOverlay;
	typedef DebugTextOverlay* DebugTextOverlayPtr;
	typedef DebugTextOverlay& DebugTextOverlayRef;

	typedef boost::function<CoreObjectPtr (LuaObjectRef _rLuaObject, ScenePtr _pScene)> CreateClassFunc;
	typedef map<Key, CreateClassFunc> CreateClassFuncMap;

	class ActionKeybindingManager;
	typedef ActionKeybindingManager* ActionKeybindingManagerPtr;
	typedef ActionKeybindingManager& ActionKeybindingManagerRef;
}

#endif // __APPLICATIONTYPES_H__
