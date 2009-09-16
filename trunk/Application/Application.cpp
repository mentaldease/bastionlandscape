#include "stdafx.h"
#include "../Application/Application.h"
#include "../Display/Camera.h"
#include "../Display/Effect.h"
#include "../Display/EffectParam.h"
#include "../Display/Camera.h"

#include "../Application/Scene.h"

#include <boost/bind.hpp>

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Application::Application()
	:	CoreObject(),
		m_oWindow(),
		m_eStateMode(EStateMode_UNINITIALIZED),
		m_pDisplay(NULL),
		m_pScene(NULL),
		m_pLandscape(NULL),
		m_pLandscapeLayerManager(NULL),
		m_pUpdateFunction(NULL),
		m_pFSRoot(NULL),
		m_pFSNative(NULL),
		m_pInput(NULL),
		m_pKeyboard(NULL),
		m_pMouse(NULL),
		m_uRLTimerID(0xffffffff),
		m_fRelativeTime(0.0f),
		m_fCameraMoveSpeed(100.0f)
	{
		m_oLightDir = Vector4(0.0f, -1.0f, 1.0f, 0.0f);
		D3DXVec4Normalize(&m_oLightDir, &m_oLightDir);
		DisplayEffectParamLIGHTDIR::s_pLightDir = &m_oLightDir;
	}

	Application::~Application()
	{

	}

	bool Application::Create(const boost::any& _rConfig)
	{
		bool bResult = (EStateMode_UNINITIALIZED == m_eStateMode);

		if (false != bResult)
		{
			m_pTime = new Time;
			bResult = m_pTime->Create(boost::any(0));
			Time::SetRoot(m_pTime);
			m_uRLTimerID = m_pTime->CreateTimer(false);
		}

		if (false != bResult)
		{
			m_eStateMode = EStateMode_INITIALING_FS;
			m_pFSRoot = new FS;
			m_pFSNative = new FSNative;
			bResult = m_pFSRoot->Create(boost::any(0))
				&& m_pFSNative->Create(boost::any(0))
				&& m_pFSRoot->AddFS("NATIVE", m_pFSNative);
			if (false != bResult)
			{
				FS::SetRoot(m_pFSRoot);
			}
		}

		if (false != bResult)
		{
			m_eStateMode = EStateMode_INITIALING_WINDOW;
			m_oWindow = *(boost::any_cast<WindowData*>(_rConfig));
			m_eStateMode = (NULL != (*m_oWindow.m_pCreateWindow)(m_oWindow)) ? m_eStateMode : EStateMode_ERROR;
			bResult = (EStateMode_ERROR != m_eStateMode);
		}

		if (false != bResult)
		{
			m_eStateMode = EStateMode_INITIALING_DISPLAY;
			m_pDisplay = new Display;
			bResult = m_pDisplay->Create(boost::any(&m_oWindow));
			m_eStateMode = (false != bResult) ? EStateMode_READY : EStateMode_ERROR;
		}

		if (false != bResult)
		{
			m_eStateMode = EStateMode_INITIALING_SHADERS;
			DisplayEffectParamTIME::s_fTime = &m_fRelativeTime;
			//DisplayMaterialManagerPtr pMaterialManager = m_pDisplay->GetMaterialManager();
			//bResult = pMaterialManager->LoadMaterial("basic00", "data/materials/basic00.material")
			//	&& pMaterialManager->LoadMaterial("water00", "data/materials/water00.material");
		}

		if (false != bResult)
		{
			m_eStateMode = EStateMode_INITIALING_INPUT;
			Input::CreateInfo oICInfo;
			m_pInput = new Input;
			oICInfo.m_bCreateDefaultKeyboard = true;
			oICInfo.m_bCreateDefaultMouse = true;
			oICInfo.m_uCooperativeLevelKeyboard = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE;
			oICInfo.m_uCooperativeLevelMouse = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE;
			oICInfo.m_hWnd = m_oWindow.m_hWnd;
			oICInfo.m_hInstance = m_oWindow.m_hInstance;
			oICInfo.m_uDefaultKeyboardKey = MakeKey(string("DIKEYBOARD"));
			oICInfo.m_uDefaultMouseKey = MakeKey(string("DIMOUSE"));
			bResult = m_pInput->Create(boost::any(&oICInfo));
			Input::SetRoot((false != bResult) ? m_pInput : NULL);
		}

		if (false != bResult)
		{
			m_pLandscapeLayerManager = new LandscapeLayerManager(*m_pDisplay);
			bResult = m_pLandscapeLayerManager->Create(boost::any(0));
			LandscapeLayerManager::SetInstance((false != bResult) ? m_pLandscapeLayerManager : NULL);
		}

		if (false != bResult)
		{
			m_pKeyboard = m_pInput->GetDevice(MakeKey(string("DIKEYBOARD")));
			m_pMouse = m_pInput->GetDevice(MakeKey(string("DIMOUSE")));
			bResult = (NULL != m_pKeyboard) && (NULL != m_pMouse);
			if (false != bResult)
			{
				memset(m_aKeysInfoOld, 0, sizeof(unsigned char) * 256);
				memset(m_aKeysInfo, 0, sizeof(unsigned char) * 256);
				memset(&m_oMouseInfoOld, 0, sizeof(DIMouseState));
				memset(&m_oMouseInfo, 0, sizeof(DIMouseState));
				//m_pUpdateFunction = boost::bind(&Application::LoadLandscape, this);
				m_pUpdateFunction = boost::bind(&Application::LoadScene, this);
			}
		}

		if (false != bResult)
		{
			m_eStateMode = EStateMode_READY;
		}

		return (EStateMode_READY == m_eStateMode);
	}

	void Application::Update()
	{
		if ((EStateMode_READY == m_eStateMode) || (EStateMode_RUNNING == m_eStateMode))
		{
			MSG msg;
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (!TranslateAccelerator(msg.hwnd, m_oWindow.m_hAccelTable, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				m_eStateMode = (WM_QUIT != msg.message) ? EStateMode_RUNNING : EStateMode_QUIT;
			}
			else if (NULL != m_pUpdateFunction)
			{
				m_pUpdateFunction();
			}
		}
	}

	void Application::Release()
	{
		if (NULL != m_pLandscapeLayerManager)
		{
			LandscapeLayerManager::SetInstance(NULL);
			m_pLandscapeLayerManager->Release();
			delete m_pLandscapeLayerManager;
			m_pLandscapeLayerManager = NULL;
		}
		if (NULL != m_pInput)
		{
			Input::SetRoot(NULL);
			m_pInput->Release();
			delete m_pInput;
			m_pInput = NULL;
			m_pKeyboard = NULL;
			m_pMouse = NULL;
		}
		if (NULL != m_pLandscape)
		{
			m_pLandscape->Close();
			m_pLandscape->Release();
			m_pLandscape = NULL;
		}
		if (NULL != m_pScene)
		{
			m_pScene->Release();
			delete m_pScene;
			m_pScene = NULL;
		}
		if (NULL != m_pDisplay)
		{
			m_pDisplay->Release();
			delete m_pDisplay;
			m_pDisplay = NULL;
		}
		if ((NULL != m_pFSRoot) && (NULL != m_pFSNative))
		{
			m_pFSRoot->RemoveFS("NATIVE", m_pFSNative);
			m_pFSNative->Release();
			delete m_pFSNative;
			m_pFSNative = NULL;
		}
		if (NULL != m_pFSRoot)
		{
			m_pFSRoot->Release();
			delete m_pFSRoot;
			m_pFSRoot = NULL;
		}
		if (NULL != m_pTime)
		{
			Time::SetRoot(NULL);
			m_pTime->Release();
			delete m_pTime;
			m_pTime = NULL;
		}
	}

	const Application::EStateMode& Application::GetStateMode() const
	{
		return m_eStateMode;
	}

	const WindowData& Application::GetWindowData() const
	{
		return m_oWindow;
	}

	DisplayPtr Application::GetDisplay()
	{
		return m_pDisplay;
	}

	void Application::LoadLandscape()
	{
		if (NULL == m_pLandscape)
		{
			Landscape::OpenInfo oLOInfo = { "Landscape00", 16, 1, ELandscapeVertexFormat_LIQUID, "" };
			m_pLandscape = new Landscape(*m_pDisplay);
			if ((false == m_pLandscape->Create(boost::any(0)))
				|| (false == m_pLandscape->Open(oLOInfo)))
			{
				m_pLandscape->Release();
				delete m_pLandscape;
				m_pLandscape = NULL;
			}
			else
			{
				m_pDisplay->GetCurrentCamera()->GetPosition() = Vector3(0.0f, 2.0f, -0.0f);
				m_pLandscape->SetMaterial(m_pDisplay->GetMaterialManager()->GetMaterial("water00"));
				m_pUpdateFunction = boost::bind(&Application::RenderLandscape, this);
			}
		}
	}

	void Application::RenderLandscape()
	{
		float fElapsedTime;
		if (m_pTime->ResetTimer(m_uRLTimerID, fElapsedTime))
		{
			fElapsedTime /= 1000.0f;
			m_fRelativeTime += fElapsedTime;
			m_pInput->Update();
			UpdateSpectatorCamera(fElapsedTime);
			m_pLandscape->Update();
			m_pDisplay->Update();
		}
	}

	void Application::LoadScene()
	{
		if (NULL == m_pScene)
		{
			Scene::CreateInfo oSCInfo = { "data/scenes/test00.scene" };
			m_pScene = new Scene(*this);
			if (false == m_pScene->Create(boost::any(&oSCInfo)))
			{
				m_pScene->Release();
				delete m_pScene;
				m_pScene = NULL;
			}
			else
			{
				m_pDisplay->GetCurrentCamera()->GetPosition() = Vector3(0.0f, 2.0f, -10.0f);
				m_pUpdateFunction = boost::bind(&Application::RenderScene, this);
			}
		}
	}

	void Application::RenderScene()
	{
		float fElapsedTime;
		if (m_pTime->ResetTimer(m_uRLTimerID, fElapsedTime))
		{
			fElapsedTime /= 1000.0f;
			m_fRelativeTime += fElapsedTime;
			m_pInput->Update();
			UpdateSpectatorCamera(fElapsedTime);
			m_pScene->Update();
			m_pDisplay->Update();
		}
	}

	void Application::UpdateSpectatorCamera(const float& _fElapsedTime)
	{
		memcpy(m_aKeysInfoOld, m_aKeysInfo, sizeof(unsigned char) * 256);
		m_pKeyboard->GetInfo(m_aKeysInfo);
		memcpy(&m_oMouseInfoOld, &m_oMouseInfo, sizeof(DIMouseState));
		m_pMouse->GetInfo(&m_oMouseInfo);

		if ((m_aKeysInfoOld[DIK_MULTIPLY]) && (!m_aKeysInfo[DIK_MULTIPLY]))
		{
			m_fCameraMoveSpeed *= 2.0f;
		}
		if ((m_aKeysInfoOld[DIK_DIVIDE]) && (!m_aKeysInfo[DIK_DIVIDE]))
		{
			m_fCameraMoveSpeed /= 2.0f;
			if (1.0f > m_fCameraMoveSpeed)
			{
				m_fCameraMoveSpeed = 1.0f;
			}
		}

		Vector3& rCamPos = m_pDisplay->GetCurrentCamera()->GetPosition();
		Vector3& rCamRot = m_pDisplay->GetCurrentCamera()->GetRotation();
		Vector3 oCamFrontDir;
		Vector3 oCamRightDir;
		Vector3 oCamUpDir;
		const float fCameraMoveSpeed = m_fCameraMoveSpeed * _fElapsedTime;
		const float fCameraRotSpeed = 10.0f * _fElapsedTime;

		if ( ( 0 != m_oMouseInfo.rgbButtons[0] ) || ( 0 != m_oMouseInfo.rgbButtons[1] ) || ( 0 != m_oMouseInfo.rgbButtons[2] ) )
		{
			rCamRot.y += m_oMouseInfo.lX * fCameraRotSpeed;
			rCamRot.x += m_oMouseInfo.lY * fCameraRotSpeed;
		}

		m_pDisplay->GetCurrentCamera()->GetDirs( oCamFrontDir, oCamRightDir, oCamUpDir, true );
		if (!((m_aKeysInfo[DIK_RSHIFT]) || (m_aKeysInfo[DIK_LSHIFT])))
		{
			rCamPos += oCamFrontDir * ( m_aKeysInfo[DIK_UP] | m_aKeysInfo[DIK_W] ? 1.0f : 0.0f ) * fCameraMoveSpeed;
			rCamPos -= oCamFrontDir * ( m_aKeysInfo[DIK_DOWN] | m_aKeysInfo[DIK_S] ? 1.0f : 0.0f ) * fCameraMoveSpeed;
		}
		else
		{
			Vector3 oFrontDir(oCamFrontDir.x, 0.0f, oCamFrontDir.z);
			D3DXVec3Normalize(&oFrontDir, &oFrontDir);
			rCamPos += oFrontDir * ( m_aKeysInfo[DIK_UP] | m_aKeysInfo[DIK_W] ? 1.0f : 0.0f ) * fCameraMoveSpeed;
			rCamPos -= oFrontDir * ( m_aKeysInfo[DIK_DOWN] | m_aKeysInfo[DIK_S] ? 1.0f : 0.0f ) * fCameraMoveSpeed;
		}
		rCamPos += oCamRightDir * ( m_aKeysInfo[DIK_RIGHT] | m_aKeysInfo[DIK_D] ? 1.0f : 0.0f ) * fCameraMoveSpeed;
		rCamPos -= oCamRightDir * ( m_aKeysInfo[DIK_LEFT] | m_aKeysInfo[DIK_A] ? 1.0f : 0.0f ) * fCameraMoveSpeed;
		rCamPos.y += ( m_aKeysInfo[DIK_SPACE] ? 1.0f : 0.0f ) * fCameraMoveSpeed;
	}
}
