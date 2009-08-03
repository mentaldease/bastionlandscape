#ifndef __INPUT_H__
#define __INPUT_H__

#include "../Core/Core.h"
#include "../Input/InputTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Input : public CoreObject
	{
	public:
		struct CreateInfo
		{
			CreateInfo();

			HINSTANCE	m_hInstance;
			HWND		m_hWnd;
			bool		m_bCreateDefaultKeyboard;
			bool		m_bCreateDefaultMouse;
			Key			m_uDefaultKeyboardKey;	// if m_bCreateDefaultKeyboard is true then m_uDefaultKeyboardKey will be used as device key.
			Key			m_uDefaultMouseKey;		// if m_bCreateDefaultMouse is true then m_uDefaultMouseKey will be used as device key.
			DWORD		m_uCooperativeLevelKeyboard;
			DWORD		m_uCooperativeLevelMouse;
		};

	public:
		Input();
		virtual ~Input();

		static void SetRoot(InputPtr _pInput);
		static InputPtr GetRoot();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		DIPtr GetDirectInput();

		bool RegisterDevice(const Key& _rDeviceKey, InputDevicePtr _pDevice);
		bool UnregisterDevice(const Key& _rDeviceKey, InputDevicePtr _pDevice);
		InputDevicePtr GetDevice(const Key& _rDeviceKey);
		bool GetDeviceInfo(const Key& _rDeviceKey, VoidPtr _pData);

	protected:
		static InputPtr		s_pInput;

		DIPtr				m_pDI;
		InputDevicePtrMap	m_mDevices;
		InputDevicePtr		m_pKeyboard;
		InputDevicePtr		m_pMouse;
		Key					m_uKeyboardKey;
		Key					m_uMouseKey;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class InputDevice : public CoreObject
	{
	public:
		InputDevice(InputRef _rInput);
		virtual ~InputDevice();

		virtual bool GetInfo(VoidPtr _pData) = 0;

	protected:
		InputRef	m_rInput;

	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class InputDeviceKeyboard : public InputDevice
	{
	public:
		struct CreateInfo
		{
			InputPtr	m_pInput;
			HWND		m_hWnd;
			DWORD		m_uCooperativeLevel;
		};

	public:
		InputDeviceKeyboard(InputRef _rInput);
		virtual ~InputDeviceKeyboard();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual bool GetInfo(VoidPtr _pData);

	protected:
		DIDevicePtr		m_pDevice;
		unsigned char	m_aKeys[256];
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class InputDeviceMouse : public InputDevice
	{
	public:
		struct CreateInfo
		{
			InputPtr	m_pInput;
			HWND		m_hWnd;
			DWORD		m_uCooperativeLevel;
		};

	public:
		InputDeviceMouse(InputRef _rInput);
		virtual ~InputDeviceMouse();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual bool GetInfo(VoidPtr _pData);

	protected:
		DIDevicePtr		m_pDevice;
		DIMouseState	m_oState;
	};
}

#endif // __INPUT_H__
