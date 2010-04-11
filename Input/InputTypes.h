#ifndef __INPUTTYPES_H__
#define __INPUTTYPES_H__

#include <dinput.h>

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	typedef LPDIRECTINPUT8			DIPtr;
	typedef LPDIRECTINPUTDEVICE8	DIDevicePtr;
	typedef DIMOUSESTATE2			DIMouseState;
	typedef DIMouseState*			DIMouseStatePtr;
	typedef DIMouseState&			DIMouseStateRef;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Input;
	typedef Input* InputPtr;
	typedef Input& InputRef;

	class InputDevice;
	typedef InputDevice* InputDevicePtr;
	typedef InputDevice& InputDeviceRef;
	typedef map<Key, InputDevicePtr> InputDevicePtrMap;
	typedef pair<Key, InputDevicePtr> InputDevicePtrPair;
}

#endif // __INPUTTYPES_H__
