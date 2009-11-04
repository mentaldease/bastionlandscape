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

	class CameraListener;
	typedef CameraListener* CameraListenerPtr;
	typedef CameraListener& CameraListenerRef;

	struct WaterData;
	typedef WaterData* WaterDataPtr;
	typedef WaterData& WaterDataRef;
}

#endif // __APPLICATIONTYPES_H__
