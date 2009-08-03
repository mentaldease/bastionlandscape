#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "../Core/Core.h"
#include "../Core/FileNative.h"
#include "../Display/Display.h"
#include "../Landscape/Landscape.h"
#include "../Input/Input.h"
#include "../Core/Time.h"
using namespace ElixirEngine;

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

	protected:
		typedef boost::function<void()> UpdateFunction;

	protected:
		void LoadLandscape();
		void RenderLandscape();

		void UpdateSpectatorCamera(const float& _fElapsedTime);

	protected:
		WindowData		m_oWindow;
		EStateMode		m_eStateMode;
		DisplayPtr		m_pDisplay;
		LandscapePtr	m_pLandscape;
		UpdateFunction	m_pUpdateFunction;
		FSPtr			m_pFSRoot;
		FSPtr			m_pFSNative;
		InputPtr		m_pInput;
		InputDevicePtr	m_pKeyboard;
		InputDevicePtr	m_pMouse;
		TimePtr			m_pTime;
		unsigned int	m_uRLTimerID;
		float			m_fRelativeTime;

	private:
	};
}

#endif // __APPLICATION_H__
