#include "stdafx.h"
#include "../Application/Application.h"
#include "../Application/Scene.h"
#include "../Application/ActionKeybinding.h"
#include "../Application/ApplicationActions.h"
#include <stdarg.h>

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class CameraListener : public CoreObject
	{
	public:
		CameraListener(DisplayRef _rDisplay)
		:	CoreObject(),
			m_pCamera(NULL),
			m_rDisplay(_rDisplay),
			m_oReflection(),
			m_uReflectionKey(MakeKey(string("reflection"))),
			m_uReflection2Key(MakeKey(string("reflection2"))),
			m_uWaterLevelKey(MakeKey(string("WATERLEVEL"))),
			m_uWaterDataKey(MakeKey(string("WATERDATA")))
		{

		}

		virtual ~CameraListener()
		{

		}

		virtual bool Create(const boost::any& _rConfig)
		{
			bool bResult = true;
			return bResult;
		}

		virtual void Update()
		{
			const Key uProcessNameKey = m_rDisplay.GetCurrentNormalProcess()->GetNameKey();
			m_pCamera = m_rDisplay.GetCurrentCamera();
			if ((NULL != m_pCamera) && (NULL != m_rDisplay.GetCurrentNormalProcess())
				&& ((m_uReflectionKey == uProcessNameKey) || (m_uReflection2Key == uProcessNameKey)))
			{
				FloatPtr pWaterLevel = m_rDisplay.GetMaterialManager()->GetFloatBySemantic(m_uWaterLevelKey);
				UInt uWaterDataSize;
				WaterDataPtr pWaterData = static_cast<WaterDataPtr>(m_rDisplay.GetMaterialManager()->GetStructBySemantic(m_uWaterDataKey, uWaterDataSize));
				if (NULL != pWaterData)
				{
					if (m_uReflectionKey == uProcessNameKey)
					{
						pWaterLevel = &pWaterData[0].m_fWaterLevel;
						//vsoutput(__FUNCTION__" : reflection camera\n");
					}
					else if (m_uReflection2Key == uProcessNameKey)
					{
						pWaterLevel = &pWaterData[1].m_fWaterLevel;
						//vsoutput(__FUNCTION__" : reflection2 camera\n");
					}
				}
				Vector3 oWaterLevel(0.0f, *pWaterLevel, 0.0f);
				Vector3 oReflectDir(0.0f, 1.0f, 0.0f);
				D3DXPlaneFromPointNormal(&m_oReflectPlane, &oWaterLevel, &oReflectDir);
				m_pCamera->SetReflection(true, &m_oReflectPlane);
				m_pCamera->SetClipPlanes(1, &m_oReflectPlane);
			}
			else if (NULL != m_pCamera)
			{
				m_pCamera->SetReflection(false);
				m_pCamera->SetClipPlanes(0, NULL);
				//vsoutput(__FUNCTION__" : main camera\n");
			}
		}

	protected:
		DisplayCameraPtr	m_pCamera;
		DisplayRef			m_rDisplay;
		Matrix				m_oReflection;
		Plane				m_oReflectPlane;
		Key					m_uReflectionKey;
		Key					m_uReflection2Key;
		Key					m_uWaterLevelKey;
		Key					m_uWaterDataKey;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Application::CameraParams::CameraParams()
	{
		Reset();
	}

	void Application::CameraParams::Reset(const bool _bResetSpeed)
	{
		m_fMoveSpeed = (false == _bResetSpeed) ? m_fMoveSpeed : 0.0f;
		m_fFront = 0.0f;
		m_fFront2 = 0.0f;
		m_fStrafe = 0.0f;
		m_fUp = 0.0f;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Application::Application()
	:	CoreObject(),
		m_oWindow(),
		m_eStateMode(EStateMode_UNINITIALIZED),
		m_pDisplay(NULL),
		m_pScene(NULL),
		m_pLandscapeLayerManager(NULL),
		m_pUpdateFunction(NULL),
		m_pFSRoot(NULL),
		m_pFSNative(NULL),
		m_pFSMemory(NULL),
		m_pInput(NULL),
		m_pKeyboard(NULL),
		m_pMouse(NULL),
		m_uUpdateTimerID(0xffffffff),
		m_uProfileTimerID(0xffffffff),
		m_fRelativeTime(0.0f),
		m_fCameraMoveSpeed(100.0f),
		m_pCameraListener(NULL),
		m_pLuaState(NULL),
		m_pCamera(NULL),
		m_pKeybinds(NULL),
		m_pActionDispatcher(NULL),
		m_pLog(NULL),
		m_pJobManager(NULL),
		m_pOneJob(NULL),
		m_uProcessAction(MakeKey(string("ProcessAction")))
	{
		m_pActionCallback = boost::bind(&Application::ProcessActions, this, _1);
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
			m_uUpdateTimerID = m_pTime->CreateTimer(false);
			m_uProfileTimerID = m_pTime->CreateTimer(true);
			Profiling::SetTimer(m_uProfileTimerID);
		}

		if (false != bResult)
		{
			const UInt uBitSize = sizeof(DWORD_PTR) * 8;
			DWORD_PTR ProcMask;
			DWORD_PTR SysMask;
			GetProcessAffinityMask(GetCurrentProcess(), &ProcMask, &SysMask);
			UInt uCount = 0;
			for (UInt i = 0 ; uBitSize > i ; ++i)
			{
				uCount = (0 != (ProcMask & 1)) ? uCount + 1 : uCount;
				ProcMask >>= 1;
			}
			JobManager::CreateInfo oJMCInfo = { uCount };
			m_pJobManager = new JobManager;
			bResult = m_pJobManager->Create(boost::any(&oJMCInfo));
		}

		if (false != bResult)
		{
			m_eStateMode = EStateMode_INITIALING_FS;
			m_pFSRoot = new FS;
			m_pFSNative = new FSNative;
			m_pFSMemory = new FSMemory;
			bResult = m_pFSRoot->Create(boost::any(0))
				&& m_pFSNative->Create(boost::any(0))
				&& m_pFSMemory->Create(boost::any(0))
				&& m_pFSRoot->AddFS("NATIVE", m_pFSNative)
				&& m_pFSRoot->AddFS("MEMORY", m_pFSMemory);
			if (false != bResult)
			{
				FS::SetRoot(m_pFSRoot);
				m_pLog = FS::GetRoot()->OpenFile("MEMORY@Log.txt", FS::EOpenMode_CREATETEXT);
				Log("BASTION log info\n");
			}
		}

		if (false != bResult)
		{
			m_pLuaState = Scripting::Lua::CreateState();
			bResult = (NULL != m_pLuaState);
		}

		if (false != bResult)
		{
			m_oWindow = *(boost::any_cast<WindowData*>(_rConfig));
			Scripting::Lua::SetStateInstance(m_pLuaState);
			GetLuaConfigParameters();
		}

		if (false != bResult)
		{
			m_eStateMode = EStateMode_INITIALING_WINDOW;
			m_eStateMode = (NULL != (*m_oWindow.m_pCreateWindow)(m_oWindow)) ? m_eStateMode : EStateMode_ERROR;
			bResult = (EStateMode_ERROR != m_eStateMode);
		}

		if (false != bResult)
		{
			m_eStateMode = EStateMode_INITIALING_DISPLAY;
			m_pDisplay = new Display;
			bResult = m_pDisplay->Create(boost::any(&m_oWindow));
			m_eStateMode = (false != bResult) ? EStateMode_READY : EStateMode_ERROR;
			if (false != bResult)
			{
				LuaStatePtr pLuaState = Scripting::Lua::GetStateInstance();
				LuaObject oGlobals = pLuaState->GetGlobals();
				LuaObject oConfig = oGlobals["bastion_config"];
				LuaObject oGraphics = oConfig["graphics"];
				LuaObject oViewports = oGraphics["viewports"];
				if (false == oViewports.IsNil())
				{
					const int sCount = oViewports.GetCount();
					for (int i = 0 ; (sCount > i) && (false != bResult) ; ++i)
					{
						bResult = AddViewportFromLua(oViewports[i + 1]);
					}
				}
			}
		}

		if (false != bResult)
		{
			m_eStateMode = EStateMode_INITIALING_SHADERS;
			DisplayMaterialManagerPtr pMaterialManager = m_pDisplay->GetMaterialManager();

			pMaterialManager->SetEffectIncludeBasePath("Data/Effects");
			//pMaterialManager->OverrideCommonParamSemantic(DisplayMaterialManager::ECommonParamSemantic_WORLDVIEWPROJ, MakeKey(string("WVP")));

			//
			pMaterialManager->RegisterParamCreator(MakeKey(string("TIME")), boost::bind(&DisplayEffectParamFLOAT::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("LIGHTDIR")), boost::bind(&DisplayEffectParamVECTOR4::CreateParam, _1));
			// landscape
			pMaterialManager->RegisterParamCreator(MakeKey(string("NOISETEX")), boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("MORPHFACTOR")), boost::bind(&DisplayEffectParamFLOAT::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("ATLASDIFFUSETEX")), boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("ATLASLUTTEX")), boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("ATLASDIFFUSEINFO")), boost::bind(&DisplayEffectParamVECTOR4::CreateParam, _1));
			// misc
			pMaterialManager->RegisterParamCreator(MakeKey(string("USERMATRIX00")), boost::bind(&DisplayEffectParamMATRIX::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("USERMATRIX01")), boost::bind(&DisplayEffectParamMATRIX::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("USERMATRIX02")), boost::bind(&DisplayEffectParamMATRIX::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("USERMATRIX03")), boost::bind(&DisplayEffectParamMATRIX::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("USERMATRIX04")), boost::bind(&DisplayEffectParamMATRIX::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("USERMATRIX05")), boost::bind(&DisplayEffectParamMATRIX::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("USERMATRIX06")), boost::bind(&DisplayEffectParamMATRIX::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("USERMATRIX07")), boost::bind(&DisplayEffectParamMATRIX::CreateParam, _1));
			// sky
			pMaterialManager->RegisterParamCreator(MakeKey(string("SKY_BETARAYLEIGH")), boost::bind(&DisplayEffectParamVECTOR3::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("SKY_BETAMIE")), boost::bind(&DisplayEffectParamVECTOR3::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("SKY_SUNCOLORINTENSITY")), boost::bind(&DisplayEffectParamVECTOR4::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("SKY_ONEOVERRAYLEIGHMIE")), boost::bind(&DisplayEffectParamVECTOR3::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("SKY_BETADASHRAYLEIGH")), boost::bind(&DisplayEffectParamVECTOR3::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("SKY_BETADASHMIE")), boost::bind(&DisplayEffectParamVECTOR3::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("SKY_HGDATA")), boost::bind(&DisplayEffectParamVECTOR3::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("SKY_SUNPOSITION")), boost::bind(&DisplayEffectParamVECTOR3::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("SKY_HAZEINTENSITY")), boost::bind(&DisplayEffectParamFLOAT::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("SKY_HAZEHEIGHT")), boost::bind(&DisplayEffectParamFLOAT::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(MakeKey(string("SKY_HAZECOLOR")), boost::bind(&DisplayEffectParamVECTOR3::CreateParam, _1));

			pMaterialManager->SetFloatBySemantic(MakeKey(string("TIME")), &m_fRelativeTime);
		}

		if (false != bResult)
		{
			m_eStateMode = EStateMode_INITIALING_INPUT;
			Input::CreateInfo oICInfo;
			m_pInput = new Input;
			oICInfo.m_bCreateDefaultKeyboard = true;
			oICInfo.m_bCreateDefaultMouse = true;
			oICInfo.m_uCooperativeLevelKeyboard = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
			oICInfo.m_uCooperativeLevelMouse = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
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
			Scene::RegisterClasses();

			m_pKeyboard = m_pInput->GetDevice(MakeKey(string("DIKEYBOARD")));
			m_pMouse = m_pInput->GetDevice(MakeKey(string("DIMOUSE")));
			bResult = (NULL != m_pKeyboard) && (NULL != m_pMouse);
			if (false != bResult)
			{
				memset(m_aKeysInfoOld, 0, sizeof(Byte) * 256);
				memset(m_aKeysInfo, 0, sizeof(Byte) * 256);
				memset(&m_oMouseInfoOld, 0, sizeof(DIMouseState));
				memset(&m_oMouseInfo, 0, sizeof(DIMouseState));
			}
		}

		if (false != bResult)
		{
			bResult = CreateActions();
		}

		if (false != bResult)
		{
			m_pUpdateFunction = boost::bind(&Application::LoadScene, this);
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
			m_eStateMode = (NULL != m_pUpdateFunction) ? m_eStateMode : EStateMode_QUIT;
		}
	}

	void Application::Release()
	{
		if (NULL != m_pCameraListener)
		{
			m_pCamera->RemoveListener(m_pCameraListener);
			m_pCameraListener->Release();
			delete m_pCameraListener;
			m_pCameraListener = NULL;
		}

		if (NULL != m_pScene)
		{
			m_pScene->Release();
			delete m_pScene;
			m_pScene = NULL;
		}

		ReleaseActions();

		if (NULL != m_pInput)
		{
			Input::SetRoot(NULL);
			m_pInput->Release();
			delete m_pInput;
			m_pInput = NULL;
			m_pKeyboard = NULL;
			m_pMouse = NULL;
		}

		Scene::UnregisterClasses();

		if (NULL != m_pLandscapeLayerManager)
		{
			m_pLandscapeLayerManager->Release();
			delete m_pLandscapeLayerManager;
			m_pLandscapeLayerManager = NULL;
			LandscapeLayerManager::SetInstance(NULL);
		}

		DisplayMaterialManagerPtr pMaterialManager = m_pDisplay->GetMaterialManager();
		if (NULL != pMaterialManager)
		{
			pMaterialManager->UnregisterParamCreator(MakeKey(string("SKY_BETARAYLEIGH")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("SKY_BETAMIE")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("SKY_SUNCOLORINTENSITY")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("SKY_ONEOVERRAYLEIGHMIE")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("SKY_BETADASHRAYLEIGH")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("SKY_BETADASHMIE")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("SKY_HGDATA")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("SKY_SUNPOSITION")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("SKY_HAZEINTENSITY")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("SKY_HAZEHEIGHT")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("SKY_HAZECOLOR")));

			pMaterialManager->UnregisterParamCreator(MakeKey(string("TIME")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("LIGHTDIR")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("NOISETEX")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("MORPHFACTOR")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("ATLASDIFFUSETEX")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("ATLASLUTTEX")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("ATLASDIFFUSEINFO")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("USERMATRIX00")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("USERMATRIX01")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("USERMATRIX02")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("USERMATRIX03")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("USERMATRIX04")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("USERMATRIX05")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("USERMATRIX06")));
			pMaterialManager->UnregisterParamCreator(MakeKey(string("USERMATRIX07")));

			//pMaterialManager->ResetCommonParamSemantic(DisplayMaterialManager::ECommonParamSemantic_WORLDVIEWPROJ);
		}

		if (NULL != m_pDisplay)
		{
			m_pDisplay->Release();
			delete m_pDisplay;
			m_pDisplay = NULL;
		}

		if (NULL != m_pLuaState)
		{
			Scripting::Lua::ReleaseState(m_pLuaState);
			m_pLuaState = NULL;
		}

		if ((NULL != m_pFSRoot) && (NULL != m_pFSMemory))
		{
			Profiling::OutputInfo(m_pLog);
			m_pFSRoot->CloseFile(m_pLog);
			m_pLog = NULL;
		}

		if ((NULL != m_pFSRoot) && (NULL != m_pFSMemory))
		{
			m_pFSRoot->RemoveFS("MEMORY", m_pFSMemory);
			m_pFSMemory->Release();
			delete m_pFSMemory;
			m_pFSMemory = NULL;
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
			FS::SetRoot(NULL);
			m_pFSRoot->Release();
			delete m_pFSRoot;
			m_pFSRoot = NULL;
		}

		if (NULL != m_pJobManager)
		{
			m_pJobManager->Release();
			delete m_pJobManager;
			m_pJobManager = NULL;
		}

		if (NULL != m_pOneJob)
		{
			m_pOneJob->Release();
			delete m_pOneJob;
			m_pOneJob = NULL;
		}

		if (NULL != m_pTime)
		{
			Time::SetRoot(NULL);
			float fTemp;
			m_pTime->ReleaseTimer(m_uProfileTimerID, fTemp);
			m_pTime->ReleaseTimer(m_uUpdateTimerID, fTemp);
			m_pTime->Release();
			delete m_pTime;
			m_pTime = NULL;
		}

		m_eStateMode = EStateMode_UNINITIALIZED;
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

	const float& Application::GetDeltaTime() const
	{
		return m_fElapsedTime;
	}

	void Application::LoadScene()
	{
		if (NULL == m_pScene)
		{
			Scene::CreateInfo oSCInfo = { "data/scenes/scenetest00.lua" };
			m_pScene = new Scene(*this);
			bool bResult = m_pScene->Create(boost::any(&oSCInfo));
			if (false != bResult)
			{
				Profiling::OutputInfo(m_pLog);
				Profiling::Clear();

				m_pCamera = m_pDisplay->GetCamera(MakeKey(string("scenecamera00")));
				m_pCameraListener = new CameraListener(*m_pDisplay);
				bool bResult = m_pCameraListener->Create(boost::any(0));
				if (false != bResult)
				{
					m_pCamera->AddListener(m_pCameraListener);
					m_pUpdateFunction = boost::bind(&Application::RenderScene, this);
				}
			}
			if (false == bResult)
			{
				m_pScene->Release();
				delete m_pScene;
				m_pScene = NULL;
			}
		}
	}

	void Application::RenderScene()
	{
		PROFILING(__FUNCTION__);
		if (m_pTime->ResetTimer(m_uUpdateTimerID, m_fElapsedTime))
		{
			m_fElapsedTime /= 1000.0f;
			m_fRelativeTime += m_fElapsedTime;
			UpdateInput();
			m_pDisplay->UpdateRequest(m_pScene);
			m_pDisplay->Update();
		}
	}

	void Application::UpdateInput()
	{
		m_pInput->Update();

		memcpy(m_aKeysInfoOld, m_aKeysInfo, sizeof(Byte) * 256);
		m_pKeyboard->GetInfo(m_aKeysInfo);
		memcpy(&m_oMouseInfoOld, &m_oMouseInfo, sizeof(DIMouseState));
		m_pMouse->GetInfo(&m_oMouseInfo);

		m_pActionDispatcher->Update();

		UpdateSpectatorCamera(m_fElapsedTime);
	}

	void Application::UpdateSpectatorCamera(const float& _fElapsedTime)
	{
		PROFILING(__FUNCTION__);

		// hack : if mouse movement delta if too hight then reset
		if ((-1000 >= m_oMouseInfo.lX) || (1000 <= m_oMouseInfo.lX) || (-1000 >= m_oMouseInfo.lY) || (1000 <= m_oMouseInfo.lY))
		{
			m_oMouseInfo.lX = 0;
			m_oMouseInfo.lY = 0;
			m_oMouseInfo.lZ = 0;
			vsoutput(__FUNCTION__" : mouse reseted\n");
		}

		Vector3& rCamPos = m_pCamera->GetPosition();
		Vector3& rCamRot = m_pCamera->GetRotation();
		Vector3 oCamFrontDir;
		Vector3 oCamRightDir;
		Vector3 oCamUpDir;
		const float fCameraMoveSpeed = m_fCameraMoveSpeed * _fElapsedTime;
		const float fCameraRotSpeed = 10.0f * _fElapsedTime;

		if ( ( 0 != m_oMouseInfo.rgbButtons[0] ) || ( 0 != m_oMouseInfo.rgbButtons[1] ) || ( 0 != m_oMouseInfo.rgbButtons[2] ) )
		{
			rCamRot.y += m_oMouseInfo.lX * fCameraRotSpeed;
			rCamRot.x += m_oMouseInfo.lY * fCameraRotSpeed;
			//vsoutput(__FUNCTION__" : mouse %d %d\n", m_oMouseInfo.lX, m_oMouseInfo.lY);
		}

		m_pCamera->GetDirs(oCamFrontDir, oCamRightDir, oCamUpDir);
		Vector3 oFrontDir(oCamFrontDir.x, 0.0f, oCamFrontDir.z);
		D3DXVec3Normalize(&oFrontDir, &oFrontDir);
		rCamPos += oFrontDir * m_oCameraParams.m_fFront2 * fCameraMoveSpeed;
		rCamPos += oCamFrontDir * m_oCameraParams.m_fFront * fCameraMoveSpeed;
		rCamPos += oCamRightDir * m_oCameraParams.m_fStrafe * fCameraMoveSpeed;
		rCamPos.y += m_oCameraParams.m_fUp * fCameraMoveSpeed;

		m_oCameraParams.Reset(false);
	}

	void Application::GetLuaConfigParameters()
	{
		if (false != Scripting::Lua::Loadfile("data/bastion_config.lua"))
		{
			LuaStatePtr pLuaState = Scripting::Lua::GetStateInstance();
			LuaObject oGlobals = pLuaState->GetGlobals();
			LuaObject oConfig = oGlobals["bastion_config"];
			LuaObject oGraphics = oConfig["graphics"];

			m_oWindow.m_bFullScreen = oGraphics["fullscreen"].GetBoolean();
			m_oWindow.m_oClientRect.right = oGraphics["width"].GetInteger();
			m_oWindow.m_oClientRect.bottom = oGraphics["height"].GetInteger();
			m_oWindow.m_fZNear = oGraphics["depth_near"].GetFloat();
			m_oWindow.m_fZFar = oGraphics["depth_far"].GetFloat();
			m_oWindow.m_uDXGBufferCount = oGraphics["gbuffer_count"].GetInteger();
			m_oWindow.m_uDXGBufferCount = (WindowData::c_uMaxGBuffers < m_oWindow.m_uDXGBufferCount) ? WindowData::c_uMaxGBuffers : m_oWindow.m_uDXGBufferCount;

			string strFormat = oGraphics["color_format"].GetString();
			m_oWindow.m_uDXColorFormat = Display::StringToDisplayFormat(strFormat, D3DFORMAT(m_oWindow.m_uDXColorFormat));
			strFormat = oGraphics["depth_format"].GetString();
			m_oWindow.m_uDXDepthFormat = Display::StringToDisplayFormat(strFormat, D3DFORMAT(m_oWindow.m_uDXDepthFormat));

			LuaObject oGBuffers = oGraphics["gbuffers_format"];
			for (UInt i = 0 ; m_oWindow.m_uDXGBufferCount > i ; ++i)
			{
				strFormat = oGBuffers[i + 1].GetString();
				m_oWindow.m_aDXGBufferFormat[i] = Display::StringToDisplayFormat(strFormat, D3DFORMAT(m_oWindow.m_aDXGBufferFormat[i]));
			}
		}
	}


	bool Application::AddViewportFromLua(LuaObjectRef _rLuaObject)
	{
		const string strName = _rLuaObject["name"].GetString();
		const Key uNameKey = MakeKey(strName);
		bool bResult = (NULL == m_pDisplay->GetViewport(uNameKey));

		if (false != bResult)
		{
			unsigned int uTemp[2];
			m_pDisplay->GetResolution(uTemp[0], uTemp[1]);
			const float fX = _rLuaObject["x"].GetFloat();
			const float fY = _rLuaObject["y"].GetFloat();
			const float fWidth = _rLuaObject["width"].GetFloat();
			const float fHeight = _rLuaObject["height"].GetFloat();

			Viewport oViewport;
			oViewport.X = (unsigned int)(fX * uTemp[0]);
			oViewport.Y = (unsigned int)(fY * uTemp[1]);
			oViewport.Width = (unsigned int)(fWidth * uTemp[0]);
			oViewport.Height = (unsigned int)(fHeight * uTemp[1]);
			oViewport.MinZ = 0.0f;
			oViewport.MaxZ = 1.0f;

			m_pDisplay->AddViewport(uNameKey, oViewport);
		}

		return bResult;
	}

	void Application::Log(const string& _strFormat, ...)
	{
		if (NULL != m_pLog)
		{
			const size_t uBufferSize = 4 * 1024;
			char szbuffer[uBufferSize];
			va_list vaArgs;
			const char* pbuffer = &szbuffer[0];

			va_start(vaArgs, _strFormat);
			const int sSize = vsnprintf_s(szbuffer, uBufferSize, _TRUNCATE, _strFormat.c_str(), vaArgs);
			va_end(vaArgs);

			m_pLog->Write(szbuffer, sSize);
		}
	}
}
