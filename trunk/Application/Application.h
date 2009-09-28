#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "../Application/ApplicationIncludes.h"

namespace BastionGame
{
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
		const WindowData& GetWindowData() const;

		DisplayPtr GetDisplay();

	protected:
		typedef boost::function<void()> UpdateFunction;

	protected:
		void LoadScene();
		void RenderScene();

		void UpdateSpectatorCamera(const float& _fElapsedTime);

	protected:
		WindowData					m_oWindow;
		EStateMode					m_eStateMode;
		DisplayPtr					m_pDisplay;
		ScenePtr					m_pScene;
		LandscapePtr				m_pLandscape;
		LandscapeLayerManagerPtr	m_pLandscapeLayerManager;
		UpdateFunction				m_pUpdateFunction;
		FSPtr						m_pFSRoot;
		FSPtr						m_pFSNative;
		InputPtr					m_pInput;
		InputDevicePtr				m_pKeyboard;
		InputDevicePtr				m_pMouse;
		TimePtr						m_pTime;
		unsigned int				m_uRLTimerID;
		float						m_fRelativeTime;
		float						m_fCameraMoveSpeed;
		unsigned char				m_aKeysInfo[256];
		unsigned char				m_aKeysInfoOld[256];
		DIMouseState				m_oMouseInfo;
		DIMouseState				m_oMouseInfoOld;
		Vector4						m_oLightDir;

	private:
	};
}

#endif // __APPLICATION_H__
