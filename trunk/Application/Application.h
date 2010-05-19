#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "../Application/ApplicationIncludes.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	#define WATER_COUNT	4

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Application : public CoreObject
	{
	public:
		enum EStateMode
		{
			EStateMode_UNINITIALIZED,
			EStateMode_ERROR,
			EStateMode_INITIALING_WINDOW,
			EStateMode_INITIALING_DISPLAY,
			EStateMode_INITIALING_INPUT,
			EStateMode_INITIALING_FS,
			EStateMode_INITIALING_SHADERS,
			EStateMode_READY,
			EStateMode_RUNNING,
			EStateMode_QUIT,
		};

	public:
		Application();
		virtual ~Application();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		const EStateMode& GetStateMode() const;
		const GraphicConfigData& GetGraphicConfigData() const;

		DisplayPtr GetDisplay();
		const float& GetDeltaTime() const;

		void SetMousePos(const float _fX, const float _fY);
		const Vector3& GetMousePos();

		void SetData(const Key _uDataKey, VoidPtr _pData);
		VoidPtr GetData(const Key _uDataKey);

	protected:
		typedef boost::function<void()> UpdateFunction;

		struct CameraParams
		{
			CameraParams();

			void Reset(const bool _bResetSpeed = true);

			float	m_fMoveSpeed;
			float	m_fFront;
			float	m_fFront2;
			float	m_fStrafe;
			float	m_fUp;
		};

	protected:
		void LoadScene();
		void RenderScene();
		void UpdateInput();
		void UpdateSpectatorCamera(const float& _fElapsedTime);
		bool GetLuaConfigParameters();
		bool AddViewportFromLua(LuaObjectRef _rLuaObject);
		void Log(const string& _strFormat, ...);
		bool CreateActions();
		void ProcessActions(UInt _uActionID);
		void ProcessPendingAction(UInt _uActionID);
		void ReleaseActions();

	protected:
		GraphicConfigData			m_oGraphicConfig;
		CameraParams				m_oCameraParams;
		VoidPtrMap					m_mDataExchanger;
		Vector3						m_f3MousePos;
		EStateMode					m_eStateMode;
		DisplayPtr					m_pDisplay;
		ScenePtr					m_pScene;
		LandscapePtr				m_pLandscape;
		LandscapeLayerManagerPtr	m_pLandscapeLayerManager;
		UpdateFunction				m_pUpdateFunction;
		FSPtr						m_pFSRoot;
		FSPtr						m_pFSNative;
		FSPtr						m_pFSMemory;
		InputPtr					m_pInput;
		InputDevicePtr				m_pKeyboard;
		InputDevicePtr				m_pMouse;
		TimePtr						m_pTime;
		unsigned int				m_uUpdateTimerID;
		unsigned int				m_uProfileTimerID;
		float						m_fRelativeTime;
		float						m_fElapsedTime;
		float						m_fCameraMoveSpeed;
		Byte						m_aKeysInfo[256];
		Byte						m_aKeysInfoOld[256];
		DIMouseState				m_oMouseInfo;
		DIMouseState				m_oMouseInfoOld;
		CameraListenerPtr			m_pCameraListener;
		LuaStatePtr					m_pLuaState;
		DisplayCameraPtr			m_pMainCamera;
		ActionKeybindingManagerPtr	m_pKeybinds;
		ActionDispatcherPtr			m_pActionDispatcher;
		FilePtr						m_pLog;
		ActionCallbackFunc			m_pActionCallback;
		JobManagerPtr				m_pJobManager;
		JobPtr						m_pOneJob;
		EntityPtr					m_pSelectedEntity;
		Key							m_uProcessAction;
		UInt						m_uPendingAction;

	private:
	};
}

#endif // __APPLICATION_H__
