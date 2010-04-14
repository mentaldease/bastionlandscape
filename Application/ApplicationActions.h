#ifndef __APPLICATIONACTIONS_H__
#define __APPLICATIONACTIONS_H__

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
		EAppAction_PATH_CREATE,
		EAppAction_ENTITY_CREATE,
		EAppAction_COUNT // Always last enum member
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class ActionDispatcher : public ElixirEngine::CoreObject
	{
	public:
		ActionDispatcher();
		virtual ~ActionDispatcher();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		void SetBindings(ActionKeybindingManagerPtr _pBindingManager);
		bool RegisterActionCallback(UInt _uActionID, const Key _uCallbackID, ActionCallbackFunc _pCallback);
		void UnregisterActionCallback(UInt _uActionID, const Key _uCallbackID);

	protected:
		ActionKeybindingManagerPtr	m_pBindingManager;
		ActionCallbackFuncMultiMap	m_mCallbacks;
	};
}

#endif // __APPLICATIONACTIONS_H__
