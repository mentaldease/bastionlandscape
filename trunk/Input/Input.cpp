#include "stdafx.h"
#include "../Input/Input.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	InputPtr Input::s_pInput = NULL;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Input::CreateInfo::CreateInfo()
	:	m_hInstance(NULL),
		m_hWnd(NULL),
		m_bCreateDefaultKeyboard(false),
		m_bCreateDefaultMouse(false),
		m_uDefaultKeyboardKey(0),
		m_uDefaultMouseKey(0),
		m_uCooperativeLevelKeyboard(0),
		m_uCooperativeLevelMouse(0)
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Input::Input()
	:	CoreObject(),
		m_pDI(NULL),
		m_mDevices(),
		m_pKeyboard(NULL),
		m_pMouse(NULL),
		m_uKeyboardKey(0),
		m_uMouseKey(0)
	{

	}

	Input::~Input()
	{

	}

	void Input::SetRoot(InputPtr _pInput)
	{
		s_pInput = _pInput;
	}

	InputPtr Input::GetRoot()
	{
		return s_pInput;
	}

	bool Input::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			Release();
			if ((false != pInfo->m_bCreateDefaultKeyboard) || (false != pInfo->m_bCreateDefaultMouse))
			{
				HRESULT hResult = DirectInput8Create(pInfo->m_hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, NULL);
				bResult = SUCCEEDED(hResult);
			}
		}
		if (false != bResult)
		{
			if (false != pInfo->m_bCreateDefaultKeyboard)
			{
				InputDeviceKeyboard::CreateInfo oIDKCInfo = { this, pInfo->m_hWnd, pInfo->m_uCooperativeLevelKeyboard };
				m_pKeyboard = new InputDeviceKeyboard(*this);
				bResult = m_pKeyboard->Create(boost::any(&oIDKCInfo));
				if (false != bResult)
				{
					bResult = RegisterDevice(pInfo->m_uDefaultKeyboardKey, m_pKeyboard);
				}
				if (false != bResult)
				{
					m_uKeyboardKey = pInfo->m_uDefaultKeyboardKey;
				}
			}
			if ((false != bResult) && (false != pInfo->m_bCreateDefaultMouse))
			{
				InputDeviceMouse::CreateInfo oIDMCInfo = { this, pInfo->m_hWnd, pInfo->m_uCooperativeLevelMouse };
				m_pMouse = new InputDeviceMouse(*this);
				bResult = m_pMouse->Create(boost::any(&oIDMCInfo));
				if (false != bResult)
				{
					bResult = RegisterDevice(pInfo->m_uDefaultMouseKey, m_pMouse);
				}
				if (false != bResult)
				{
					m_uMouseKey = pInfo->m_uDefaultMouseKey;
				}
			}
		}

		return bResult;
	}

	void Input::Update()
	{
		struct DeviceUpdateFunction
		{
			void operator() (InputDevicePtrPair _oPair)
			{
				(_oPair.second)->Update();
			}
		};
		for_each(m_mDevices.begin(), m_mDevices.end(), DeviceUpdateFunction());
	}

	void Input::Release()
	{
		m_mDevices.clear();

		if (NULL != m_pMouse)
		{
			m_pMouse->Release();
			delete m_pMouse;
			m_pMouse = NULL;
		}
		if (NULL != m_pKeyboard)
		{
			m_pKeyboard->Release();
			delete m_pKeyboard;
			m_pKeyboard = NULL;
		}
		if (NULL != m_pDI)
		{
			m_pDI->Release();
			m_pDI = NULL;
		}
	}

	DIPtr Input::GetDirectInput()
	{
		return m_pDI;
	}

	bool Input::RegisterDevice(const Key& _rDeviceKey, InputDevicePtr _pDevice)
	{
		bool bResult = (m_mDevices.end() == m_mDevices.find(_rDeviceKey)) && (NULL != _pDevice);

		if (false != bResult)
		{
			m_mDevices[_rDeviceKey] = _pDevice;
		}

		return bResult;
	}

	bool Input::UnregisterDevice(const Key& _rDeviceKey, InputDevicePtr _pDevice)
	{
		InputDevicePtrMap::iterator iPair = m_mDevices.find(_rDeviceKey);
		bool bResult = (m_mDevices.end() != iPair) && (iPair->second == _pDevice);

		if (false != bResult)
		{
			m_mDevices.erase(iPair);
		}

		return bResult;
	}

	InputDevicePtr Input::GetDevice(const Key& _rDeviceKey)
	{
		InputDevicePtrMap::iterator iPair = m_mDevices.find(_rDeviceKey);
		InputDevicePtr pResult = (m_mDevices.end() != iPair) ? iPair->second : NULL;
		return pResult;
	}

	bool Input::GetDeviceInfo(const Key& _rDeviceKey, VoidPtr _pData)
	{
		InputDevicePtr pResult = GetDevice(_rDeviceKey);
		bool bResult = (NULL != pResult);

		if (false != bResult)
		{
			bResult = pResult->GetInfo(_pData);
		}

		return bResult;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	InputDevice::InputDevice(InputRef _rInput)
	:	CoreObject(),
		m_rInput(_rInput)
	{

	}

	InputDevice::~InputDevice()
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	InputDeviceKeyboard::InputDeviceKeyboard(InputRef _rInput)
	:	InputDevice(_rInput),
		m_pDevice(NULL)
	{

	}

	InputDeviceKeyboard::~InputDeviceKeyboard()
	{

	}

	bool InputDeviceKeyboard::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			Release();
			DIPtr pDI = pInfo->m_pInput->GetDirectInput();
			HRESULT hResult = pDI->CreateDevice(GUID_SysKeyboard, &m_pDevice, NULL); 
			bResult = SUCCEEDED(hResult);
		}
		if (false != bResult)
		{
			bResult = SUCCEEDED(m_pDevice->SetDataFormat(&c_dfDIKeyboard));
		}
		if (false != bResult)
		{
			bResult = SUCCEEDED(m_pDevice->SetCooperativeLevel(pInfo->m_hWnd, pInfo->m_uCooperativeLevel));
		}

		return bResult;
	}

	void InputDeviceKeyboard::Update()
	{
		m_pDevice->Acquire();
		m_pDevice->GetDeviceState(256 * sizeof(unsigned char), m_aKeys);
	}

	void InputDeviceKeyboard::Release()
	{
		if (NULL != m_pDevice)
		{
			m_pDevice->Release();
			m_pDevice = NULL;
		}
	}

	bool InputDeviceKeyboard::GetInfo(VoidPtr _pData)
	{
		memcpy(_pData, m_aKeys, 256 * sizeof(unsigned char));
		return true;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	InputDeviceMouse::InputDeviceMouse(InputRef _rInput)
	:	InputDevice(_rInput),
		m_pDevice(NULL)
	{

	}

	InputDeviceMouse::~InputDeviceMouse()
	{

	}

	bool InputDeviceMouse::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			DIPtr pDI = pInfo->m_pInput->GetDirectInput();
			HRESULT hResult = pDI->CreateDevice(GUID_SysMouse, &m_pDevice, NULL); 
			bResult = SUCCEEDED(hResult);
		}
		if (false != bResult)
		{
			bResult = SUCCEEDED(m_pDevice->SetDataFormat(&c_dfDIMouse2));
		}
		if (false != bResult)
		{
			bResult = SUCCEEDED(m_pDevice->SetCooperativeLevel(pInfo->m_hWnd, pInfo->m_uCooperativeLevel));
		}

		return bResult;
	}

	void InputDeviceMouse::Update()
	{
		m_pDevice->Acquire();
		m_pDevice->GetDeviceState(sizeof(DIMouseState), &m_oState);
	}

	void InputDeviceMouse::Release()
	{
		if (NULL != m_pDevice)
		{
			m_pDevice->Release();
			m_pDevice = NULL;
		}
	}

	bool InputDeviceMouse::GetInfo(VoidPtr _pData)
	{
		memcpy(_pData, &m_oState, sizeof(DIMouseState));
		return true;
	}
}
