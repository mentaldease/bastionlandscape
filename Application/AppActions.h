#ifndef __APPACTIONS_H__
#define __APPACTIONS_H__

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	enum EAppAction
	{
		EAppAction_UNKNOWN,
		EAppAction_CAMERA_INC_SPEED,
		EAppAction_CAMERA_DEC_SPEED,
		EAppAction_CAMERA_STRAFE_FRONT,
		EAppAction_CAMERA_STRAFE_BACK,
		EAppAction_CAMERA_MOVE_FRONT,
		EAppAction_CAMERA_MOVE_BACK,
		EAppAction_CAMERA_MOVE_RIGHT,
		EAppAction_CAMERA_MOVE_LEFT,
		EAppAction_CAMERA_MOVE_UP,
		EAppAction_COUNT // Always last enum member
	};
}

#endif // __APPACTIONS_H__
