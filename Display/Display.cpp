#include "stdafx.h"
#include "../Display/Display.h"
#include "../Display/Camera.h"
#include "../Display/Effect.h"
#include "../Display/Texture.h"
#include "../Display/Surface.h"
#include "../Display/RenderTarget.h"
#include "../Display/PostProcess.h"
#include "../Display/NormalProcess.h"
#include "../Display/Font.h"
#include "../Display/RenderStage.h"
#include "../Core/Scripting.h"
#include "../Core/Util.h"
#include "../Core/Octree.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Display::DisplayFormatMap Display::s_mDisplayFormat;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayVertexBuffer::DisplayVertexBuffer(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_uBufferSize(0),
		m_uVertexSize(0),
		m_pVertexDecl(NULL),
		m_pVertexBuffer(NULL)
	{

	}

	DisplayVertexBuffer::~DisplayVertexBuffer()
	{

	}

	bool DisplayVertexBuffer::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo) && (0 != pInfo->m_uBufferSize) && (NULL != pInfo->m_pVertexElement);

		if (false != bResult)
		{
			Release();
			m_uBufferSize = pInfo->m_uBufferSize;
			m_uVertexSize = pInfo->m_uVertexSize;
			HRESULT hResult =  m_rDisplay.GetDevicePtr()->CreateVertexBuffer(
				m_uBufferSize,
				D3DUSAGE_WRITEONLY,
				0,
				D3DPOOL_DEFAULT,
				&m_pVertexBuffer,
				NULL);
			bResult = SUCCEEDED(hResult);
		}

		if (false != bResult)
		{
			HRESULT hResult = m_rDisplay.GetDevicePtr()->CreateVertexDeclaration(pInfo->m_pVertexElement, &m_pVertexDecl);
			bResult = SUCCEEDED(hResult);
		}

		return bResult;
	}

	void DisplayVertexBuffer::Update()
	{
		
	}

	void DisplayVertexBuffer::Release()
	{
		if (NULL != m_pVertexDecl)
		{
			m_pVertexDecl->Release();
			m_pVertexDecl = NULL;
		}
		if (NULL != m_pVertexBuffer)
		{
			m_pVertexBuffer->Release();
			m_pVertexBuffer = NULL;
		}
	}

	bool DisplayVertexBuffer::Set(const VoidPtr _pData)
	{
		VoidPtr pLockedData = NULL;
		HRESULT hResult = m_pVertexBuffer->Lock(0, m_uBufferSize, &pLockedData, 0);
		if (SUCCEEDED(hResult))
		{
			memcpy(pLockedData, _pData, m_uBufferSize);
			m_pVertexBuffer->Unlock();
		}
		return (SUCCEEDED(hResult));
	}

	bool DisplayVertexBuffer::Use()
	{
		VertexBufferPtr pCurrentVertexBuffer;
		unsigned int uOffset;
		unsigned int uStride;
		HRESULT hResult = m_rDisplay.GetDevicePtr()->GetStreamSource(0, &pCurrentVertexBuffer, &uOffset, &uStride);
		if ((SUCCEEDED(hResult)) && (pCurrentVertexBuffer != m_pVertexBuffer))
		{
			hResult = m_rDisplay.GetDevicePtr()->SetStreamSource(0, m_pVertexBuffer, 0, m_uVertexSize);
		}
		if (SUCCEEDED(hResult))
		{
			VertexDeclPtr pCurrentVertexDecl;
			hResult = m_rDisplay.GetDevicePtr()->GetVertexDeclaration(&pCurrentVertexDecl);
			if ((SUCCEEDED(hResult)) && (pCurrentVertexDecl != m_pVertexDecl))
			{
				hResult = m_rDisplay.GetDevicePtr()->SetVertexDeclaration(m_pVertexDecl);
			}
		}
		return (SUCCEEDED(hResult));
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayIndexBuffer::DisplayIndexBuffer(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_uBufferSize(0),
		m_b16Bits(false),
		m_pIndexBuffer(NULL)
	{

	}

	DisplayIndexBuffer::~DisplayIndexBuffer()
	{

	}

	bool DisplayIndexBuffer::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo) && (0 != pInfo->m_uBufferSize);

		if (false != bResult)
		{
			Release();
			m_b16Bits = pInfo->m_b16Bits;
			m_uBufferSize = pInfo->m_uBufferSize * ((false != m_b16Bits) ? sizeof(Word) : sizeof(UInt));
			HRESULT hResult =  m_rDisplay.GetDevicePtr()->CreateIndexBuffer(
				m_uBufferSize,
				D3DUSAGE_WRITEONLY,
				(false != m_b16Bits) ? D3DFMT_INDEX16 : D3DFMT_INDEX32,
				D3DPOOL_DEFAULT,
				&m_pIndexBuffer,
				NULL);
			bResult = SUCCEEDED(hResult);
		}

		return bResult;
	}

	void DisplayIndexBuffer::Update()
	{

	}

	void DisplayIndexBuffer::Release()
	{
		if (NULL != m_pIndexBuffer)
		{
			m_pIndexBuffer->Release();
			m_pIndexBuffer = NULL;
		}
	}

	bool DisplayIndexBuffer::Set(const VoidPtr _pData)
	{
		VoidPtr pLockedData = NULL;
		HRESULT hResult = m_pIndexBuffer->Lock(0, m_uBufferSize, &pLockedData, 0);
		if (SUCCEEDED(hResult))
		{
			memcpy(pLockedData, _pData, m_uBufferSize);
			m_pIndexBuffer->Unlock();
		}
		return (SUCCEEDED(hResult));
	}

	bool DisplayIndexBuffer::Use()
	{
		IndexBufferPtr pCurrentIndexBuffer;
		HRESULT hResult = m_rDisplay.GetDevicePtr()->GetIndices(&pCurrentIndexBuffer);
		if ((SUCCEEDED(hResult)) && (pCurrentIndexBuffer != m_pIndexBuffer))
		{
			hResult = m_rDisplay.GetDevicePtr()->SetIndices(m_pIndexBuffer);
		}
		return (SUCCEEDED(hResult));
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayObject::DisplayObject()
	:	CoreObject(),
		m_oWorld(),
		m_pMaterial(NULL),
		m_uRenderPass(0)
	{
		D3DXMatrixIdentity(&m_oWorld);
	}

	DisplayObject::~DisplayObject()
	{

	}

	void DisplayObject::SetWorldMatrix(MatrixRef _rWorld)
	{
		m_oWorld = _rWorld;
	}

	MatrixPtr DisplayObject::GetWorldMatrix()
	{
		return &m_oWorld;
	}

	void DisplayObject::SetMaterial(DisplayMaterialPtr _pMaterial)
	{
		m_pMaterial = _pMaterial;
	}

	DisplayMaterialPtr DisplayObject::GetMaterial()
	{
		return m_pMaterial;
	}

	void DisplayObject::SetRenderStage(const Key& _uRenderPass)
	{
		m_uRenderPass = _uRenderPass;
	}

	const Key& DisplayObject::GetRenderStage() const
	{
		return m_uRenderPass;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Display::Display()
	:	CoreObject(),
		m_pDirect3D(NULL),
		m_pDevice(NULL),
		m_vRenderList(),
		m_vUpdateList(),
		m_mViewports(),
		m_mCameras(),
		m_pCurrentCamera(NULL),
		m_pMaterialManager(NULL),
		m_pTextureManager(NULL),
		m_pSurfaceManager(NULL),
		m_pFontManager(NULL),
		m_pWorldMatrix(NULL),
		m_pPostProcesses(NULL),
		m_pDispFXPP(NULL),
		m_pEffectPP(NULL),
		m_pPostProcessGeometry(NULL),
		m_pRTChain(NULL),
		m_pNormalProcesses(NULL),
		m_pCurrentNormalProcess(NULL),
		m_pCurrentVertexBuffer(NULL),
		m_pCurrentIndexBuffer(NULL),
		m_pCurrentRenderStage(NULL),
		m_oWorldInvTransposeMatrix(),
		m_uWidth(0),
		m_uHeight(0)
	{
		if (NULL == GetInstance())
		{
			SetInstance(this);
		}
	}

	Display::~Display()
	{
		if (this == GetInstance())
		{
			SetInstance(NULL);
		}
	}

	bool Display::Create(const boost::any& _rConfig)
	{
		WindowData* pWindowData = boost::any_cast<WindowData*>(_rConfig);
		bool bResult = (NULL != pWindowData);

		if (false != bResult)
		{
			Release();
			m_pDirect3D = Direct3DCreate9(D3D_SDK_VERSION);
			bResult = (NULL != m_pDirect3D);
		}
		if (false != bResult)
		{
			bResult = OpenVideo(*pWindowData);
		}

		return bResult;
	}

	void Display::Update()
	{
		m_pSurfaceManager->Update();
		m_pTextureManager->Update();
		m_pFontManager->Update();
		m_pMaterialManager->Update();

		m_pRTChain->SetImmediateWrite(false);

		DisplayRenderStagePtrVec::iterator iRS = m_vRenderStages.begin();
		DisplayRenderStagePtrVec::iterator iEnd = m_vRenderStages.end();
		while (iEnd != iRS)
		{
			m_pCurrentRenderStage = *iRS;
			RenderPass(m_pCurrentRenderStage);
			++iRS;
		}
		m_pCurrentRenderStage = NULL;

		// copy back to back buffer
		if (false == m_pRTChain->GetImmediateWrite())
		{
			if (SUCCEEDED(m_pDevice->BeginScene()))
			{
				TexturePtr pFinalRenderTex = static_cast<TexturePtr>(m_pRTChain->GetTexture(0)->GetBase());
				m_pEffectPP->SetTechnique("RenderScene");
				m_pEffectPP->SetTexture("g_ColorTex", pFinalRenderTex);
				UINT cPasses;
				m_pEffectPP->Begin(&cPasses, 0);
				m_pPostProcessGeometry->RenderBegin();
				for(UINT p = 0; p < cPasses; ++p)
				{
					m_pEffectPP->BeginPass(p);
					m_pPostProcessGeometry->Render();
					m_pEffectPP->EndPass();
				}
				m_pPostProcessGeometry->RenderEnd();
				m_pEffectPP->End();
				m_pDevice->EndScene();
			}
		}

		m_pDevice->Present(NULL, NULL, NULL, NULL);
	}

	void Display::Release()
	{
		CloseVideo();

		if (NULL != m_pDirect3D)
		{
			m_pDirect3D->Release();
			m_pDirect3D = NULL;
		}
	}

	bool Display::OpenVideo(WindowData& _rWindowData)
	{
		bool bResult = (NULL == m_pDevice);
		HRESULT hResult;

		if (false != bResult)
		{
			D3DPRESENT_PARAMETERS oD3DPP;
			memset(&oD3DPP, 0 ,sizeof(D3DPRESENT_PARAMETERS));

			oD3DPP.EnableAutoDepthStencil = TRUE;
			oD3DPP.AutoDepthStencilFormat = D3DFORMAT(_rWindowData.m_uDXDepthFormat);
			oD3DPP.Windowed = _rWindowData.m_bFullScreen ? FALSE : TRUE;
			oD3DPP.BackBufferWidth = _rWindowData.m_oClientRect.right;
			oD3DPP.BackBufferHeight = _rWindowData.m_oClientRect.bottom;
			oD3DPP.BackBufferFormat = _rWindowData.m_bFullScreen ? D3DFORMAT(_rWindowData.m_uDXColorFormat) : D3DFMT_UNKNOWN;
			oD3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
			oD3DPP.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

			m_uWidth = _rWindowData.m_oClientRect.right;
			m_uHeight = _rWindowData.m_oClientRect.bottom;


			UINT AdapterToUse=D3DADAPTER_DEFAULT;
			D3DDEVTYPE DeviceType=D3DDEVTYPE_HAL;
#if SHIPPING_VERSION
			// When building a shipping version, disable PerfHUD (opt-out)
#else // SHIPPING_VERSION
			// Look for 'NVIDIA PerfHUD' adapter
			// If it is present, override default settings
			for (UINT Adapter=0;Adapter<m_pDirect3D->GetAdapterCount();Adapter++)
			{
				D3DADAPTER_IDENTIFIER9 Identifier;
				HRESULT Res;
				Res = m_pDirect3D->GetAdapterIdentifier(Adapter,0,&Identifier);
				if (strstr(Identifier.Description,"PerfHUD") != 0)
				{
					AdapterToUse=Adapter;
					DeviceType=D3DDEVTYPE_REF;
					break;
				}
			}
#endif // SHIPPING_VERSION

			hResult = m_pDirect3D->CreateDevice(
				AdapterToUse,
				DeviceType,
				_rWindowData.m_hWnd,
				D3DCREATE_HARDWARE_VERTEXPROCESSING,
				&oD3DPP,
				&m_pDevice);

			bResult = SUCCEEDED(hResult);
		}

		if (false != bResult)
		{
			m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
			m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		}

		if (false != bResult)
		{
			m_pMaterialManager = new DisplayMaterialManager(*this);
			bResult = m_pMaterialManager->Create(boost::any(0));
		}

		if (false != bResult)
		{
			m_pTextureManager = new DisplayTextureManager(*this);
			bResult = m_pTextureManager->Create(boost::any(0));
		}

		if (false != bResult)
		{
			m_pSurfaceManager = new DisplaySurfaceManager(*this);
			bResult = m_pSurfaceManager->Create(boost::any(0));
		}

		if (false != bResult)
		{
			m_pFontManager = new DisplayFontManager(*this);
			bResult = m_pFontManager->Create(boost::any(0));
		}

		if (false != bResult)
		{
			DisplayRenderTargetGeometry::CreateInfo oRTGCInfo = { m_uWidth, m_uHeight };
			DisplayRenderTargetChain::CreateInfo oRTCCInfo = { "GBUFFERS", m_uWidth, m_uHeight, D3DFORMAT(_rWindowData.m_uDXGBufferFormat), _rWindowData.m_uDXGBufferCount, _rWindowData.m_aDXGBufferFormat };
			m_pPostProcessGeometry = new DisplayRenderTargetGeometry(*this);
			m_pRTChain = new DisplayRenderTargetChain(*this);
			bResult =  m_pPostProcessGeometry->Create(boost::any(&oRTGCInfo))
				&& m_pRTChain->Create(boost::any(&oRTCCInfo))
				&& m_pMaterialManager->LoadEffect("MRT", "data/effects/simplepost.fx")
				&& (m_pDispFXPP = m_pMaterialManager->GetEffect("MRT"))
				&& (m_pEffectPP = m_pDispFXPP->GetEffect());
			m_pPostProcesses = NULL;
			m_pNormalProcesses = NULL;
		}

		return bResult;
	}

	void Display::CloseVideo()
	{
		#pragma message("implement a generic notify system to enable unload/release data before a system shutdown")

		if (NULL != m_pRTChain)
		{
			m_pRTChain->Release();
			m_pRTChain = NULL;
		}

		if (NULL != m_pPostProcessGeometry)
		{
			m_pPostProcessGeometry->Release();
			delete m_pPostProcessGeometry;
			m_pPostProcessGeometry = NULL;
		}

		if (NULL != m_pDispFXPP)
		{
			m_pMaterialManager->UnloadEffect("MRT");
			m_pDispFXPP = NULL;
		}

		if (NULL != m_pFontManager)
		{
			m_pFontManager->Release();
			delete m_pFontManager;
			m_pFontManager = NULL;
		}

		if (NULL != m_pSurfaceManager)
		{
			m_pSurfaceManager->Release();
			delete m_pSurfaceManager;
			m_pSurfaceManager = NULL;
		}

		if (NULL != m_pTextureManager)
		{
			m_pTextureManager->Release();
			delete m_pTextureManager;
			m_pTextureManager = NULL;
		}

		if (NULL != m_pMaterialManager)
		{
			m_pMaterialManager->Release();
			delete m_pMaterialManager;
			m_pMaterialManager = NULL;
		}

		if (NULL != m_pDevice)
		{
			m_pDevice->Release();
			m_pDevice = NULL;
		}

		m_mViewports.clear();

		while (m_mCameras.end() != m_mCameras.begin())
		{
			ReleaseCamera(m_mCameras.begin()->first);
			m_mCameras.erase(m_mCameras.begin());
		}
	}

	void Display::UpdateRequest(CoreObjectPtr _pCoreObject)
	{
		if (m_vUpdateList.end() == find(m_vUpdateList.begin(), m_vUpdateList.end(), _pCoreObject))
		{
			m_vUpdateList.push_back(_pCoreObject);
		}
	}

	void Display::RenderRequest(const Key& _uRenderPassKey, DisplayObjectPtr _pDisplayObject)
	{
		DisplayRenderStagePtr pRS = m_mRenderStages[_uRenderPassKey];
		if (NULL != pRS)
		{
			pRS->RenderRequest(_pDisplayObject);
		}
	}

	DisplayVertexBufferPtr Display::CreateVertexBuffer(DisplayVertexBuffer::CreateInfo& _rCreateInfo)
	{
		DisplayVertexBufferPtr pVertexBuffer = new DisplayVertexBuffer(*this);
		if (false == pVertexBuffer->Create(boost::any(&_rCreateInfo)))
		{
			ReleaseVertexBuffer(pVertexBuffer);
			pVertexBuffer = NULL;
		}
		return pVertexBuffer;
	}

	bool Display::SetCurrentVertexBuffer(DisplayVertexBufferPtr _pVertexBuffer)
	{
		bool bResult = true;
		if (m_pCurrentVertexBuffer != _pVertexBuffer)
		{
			m_pCurrentVertexBuffer = _pVertexBuffer;
			if (NULL != m_pCurrentVertexBuffer)
			{
				bResult = m_pCurrentVertexBuffer->Use();
			}
		}
		return bResult;
	}

	DisplayVertexBufferPtr Display::GetCurrentVertexBuffer()
	{
		return m_pCurrentVertexBuffer;
	}

	void Display::ReleaseVertexBuffer(DisplayVertexBufferPtr _pVertexBuffer)
	{
		_pVertexBuffer->Release();
		delete _pVertexBuffer;
	}

	DisplayIndexBufferPtr Display::CreateIndexBuffer(DisplayIndexBuffer::CreateInfo& _rCreateInfo)
	{
		DisplayIndexBufferPtr pIndexBuffer = new DisplayIndexBuffer(*this);
		if (false == pIndexBuffer->Create(boost::any(&_rCreateInfo)))
		{
			ReleaseIndexBuffer(pIndexBuffer);
			pIndexBuffer = NULL;
		}
		return pIndexBuffer;
	}

	bool Display::SetCurrentIndexBuffer(DisplayIndexBufferPtr _pIndexBuffer)
	{
		bool bResult = true;
		if (m_pCurrentIndexBuffer != _pIndexBuffer)
		{
			m_pCurrentIndexBuffer = _pIndexBuffer;
			if (NULL != m_pCurrentIndexBuffer)
			{
				bResult = m_pCurrentIndexBuffer->Use();
			}
		}
		return bResult;
	}

	DisplayIndexBufferPtr Display::GetCurrentIndexBuffer()
	{
		return m_pCurrentIndexBuffer;
	}

	void Display::ReleaseIndexBuffer(DisplayIndexBufferPtr _pIndexBuffer)
	{
		_pIndexBuffer->Release();
		delete _pIndexBuffer;
	}

	DevicePtr Display::GetDevicePtr() const
	{
		return m_pDevice;
	}

	void Display::GetResolution(unsigned int& _uWidth, unsigned int& _uHeight) const
	{
		_uWidth = m_uWidth;
		_uHeight = m_uHeight;
	}

	DisplayMaterialManagerPtr Display::GetMaterialManager()
	{
		return m_pMaterialManager;
	}

	DisplayTextureManagerPtr Display::GetTextureManager()
	{
		return m_pTextureManager;
	}

	DisplaySurfaceManagerPtr Display::GetSurfaceManager()
	{
		return m_pSurfaceManager;
	}

	DisplayFontManagerPtr Display::GetFontManager()
	{
		return m_pFontManager;
	}

	void Display::SetCurrentWorldMatrix(MatrixPtr _pMatrix)
	{
		m_pWorldMatrix = _pMatrix;
		D3DXMatrixInverse(&m_oWorldInvTransposeMatrix, NULL, m_pWorldMatrix);
		D3DXMatrixTranspose(&m_oWorldInvTransposeMatrix, &m_oWorldInvTransposeMatrix);
	}

	MatrixPtr Display::GetCurrentWorldMatrix()
	{
		return m_pWorldMatrix;
	}

	MatrixPtr Display::GetCurrentWorldInvTransposeMatrix()
	{
		return &m_oWorldInvTransposeMatrix;
	}

	void Display::MRTRenderBeginPass(UIntRef _uIndex)
	{
		if (NULL != m_pRTChain)
		{
			m_pRTChain->RenderBeginPass(_uIndex);
		}
	}

	void Display::MRTRenderEndPass()
	{
		if (NULL != m_pRTChain)
		{
			m_pRTChain->RenderEndPass();
		}
	}

	void Display::SetNormalProcessesList(DisplayNormalProcessPtrVecPtr _pNormalProcesses)
	{
		m_pNormalProcesses = _pNormalProcesses;
	}

	void Display::SetPostProcessesList(DisplayPostProcessPtrVecPtr _pPostProcesses)
	{
		m_pPostProcesses = _pPostProcesses;
	}

	DisplayObjectPtr Display::GetPostProcessGeometry()
	{
		return m_pPostProcessGeometry;
	}

	DisplayRenderTargetChainPtr Display::GetRenderTargetChain()
	{
		return m_pRTChain;
	}

	DisplayNormalProcessPtr Display::GetCurrentNormalProcess()
	{
		return m_pCurrentNormalProcess;
	}

	DisplayCameraPtr Display::CreateCamera(const Key& _uNameKey, LuaObjectRef _rLuaObject)
	{
		DisplayCameraPtr pCamera = GetCamera(_uNameKey);
		bool bResult = (NULL == pCamera);

		if (false != bResult)
		{
			string strViewportName;
			DisplayCamera::CreateInfo oDCCInfo;
			pCamera = new DisplayCamera(*this);
			Scripting::Lua::Get(_rLuaObject, "depth_near", oDCCInfo.m_fZNear, oDCCInfo.m_fZNear);
			Scripting::Lua::Get(_rLuaObject, "depth_far", oDCCInfo.m_fZFar, oDCCInfo.m_fZFar);
			Scripting::Lua::Get(_rLuaObject, "fovy", oDCCInfo.m_fDegreeFovy, oDCCInfo.m_fDegreeFovy);
			Scripting::Lua::Get(_rLuaObject, "viewport", string(""), strViewportName);
			Scripting::Lua::Get(_rLuaObject, "perspective_mode", oDCCInfo.m_bPerspectiveMode, oDCCInfo.m_bPerspectiveMode);
			if (false == strViewportName.empty())
			{
				const Key uViewportKey = MakeKey(strViewportName);
				ViewportPtr pViewport = GetViewport(uViewportKey);
				if (NULL != pViewport)
				{
					oDCCInfo.m_fX = float(pViewport->X) / float(m_uWidth);
					oDCCInfo.m_fY = float(pViewport->Y) / float(m_uHeight);
					oDCCInfo.m_fWidth = float(pViewport->Width) / float(m_uWidth);
					oDCCInfo.m_fHeight = float(pViewport->Height) / float(m_uHeight);
					oDCCInfo.m_fAspectRatio = float(pViewport->Width) / float(pViewport->Height);
				}
			}
			Scripting::Lua::Get(_rLuaObject, "aspect_ratio", oDCCInfo.m_fAspectRatio, oDCCInfo.m_fAspectRatio);
			Scripting::Lua::Get(_rLuaObject, "position", oDCCInfo.m_oPos, oDCCInfo.m_oPos);
			Scripting::Lua::Get(_rLuaObject, "rotation", oDCCInfo.m_oRot, oDCCInfo.m_oRot);
			bool bResult = pCamera->Create(boost::any(&oDCCInfo));
			if (false != bResult)
			{
				m_mCameras[_uNameKey] = pCamera;
			}
		}

		return pCamera;
	}

	void Display::ReleaseCamera(const Key& _uNameKey)
	{
		DisplayCameraPtrMap::iterator iPair = m_mCameras.find(_uNameKey);
		if (m_mCameras.end() != iPair)
		{
			DisplayCameraPtr pCamera =  iPair->second;
			pCamera->Release();
			delete pCamera;
			m_mCameras.erase(iPair);
		}
	}

	DisplayCameraPtr Display::GetCamera(const Key& _uNameKey)
	{
		DisplayCameraPtrMap::iterator iPair = m_mCameras.find(_uNameKey);
		DisplayCameraPtr pResult = (m_mCameras.end() != iPair) ? iPair->second : NULL;
		return pResult;
	}

	DisplayCameraPtr Display::GetCurrentCamera()
	{
		return m_pCurrentCamera;
	}

	void Display::SetCurrentCamera(DisplayCameraPtr _pCamera)
	{
		m_pCurrentCamera = _pCamera;
	}

	bool Display::AddViewport(const Key& _uNameKey, ViewportRef _rViewPort)
	{
		ViewportMap::iterator iPair = m_mViewports.find(_uNameKey);
		bool bResult = (m_mViewports.end() == iPair);

		if (false != bResult)
		{
			m_mViewports[_uNameKey] = _rViewPort;
		}

		return bResult;
	}

	ViewportPtr Display::GetViewport(const Key& _uNameKey)
	{
		ViewportMap::iterator iPair = m_mViewports.find(_uNameKey);
		return (m_mViewports.end() == iPair) ? NULL : &iPair->second;
	}

	void Display::AddRenderStages(DisplayRenderStagePtrVec _vRenderPasses)
	{
		DisplayRenderStagePtrVec::iterator iRS = _vRenderPasses.begin();
		DisplayRenderStagePtrVec::iterator iEnd = _vRenderPasses.end();
		while (iEnd != iRS)
		{
			DisplayRenderStagePtr pRS = *iRS;
			m_vRenderStages.push_back(pRS);
			m_mRenderStages[pRS->GetNameKey()] = pRS;
			++iRS;
		}
	}

	void Display::RemoveRenderStages(DisplayRenderStagePtrVec _vRenderPasses)
	{
		DisplayRenderStagePtrVec::iterator iRS = _vRenderPasses.begin();
		DisplayRenderStagePtrVec::iterator iEnd = _vRenderPasses.end();
		while (iEnd != iRS)
		{
			DisplayRenderStagePtr pRS = *iRS;

			DisplayRenderStagePtrVec::iterator iRPToErase = find(m_vRenderStages.begin(), m_vRenderStages.end(), pRS);
			if (m_vRenderStages.end() != iRPToErase)
			{
				m_vRenderStages.erase(iRPToErase);
			}

			DisplayRenderStagePtrMap::iterator iPair = m_mRenderStages.find(pRS->GetNameKey());
			if (m_mRenderStages.end() != iPair)
			{
				m_mRenderStages.erase(iPair);
			}

			++iRS;
		}
	}

	DisplayRenderStagePtr Display::GetCurrentRenderStage()
	{
		return m_pCurrentRenderStage;
	}

	void Display::RenderUpdate()
	{
		CoreObjectPtrVec::iterator iCoreObject = m_vUpdateList.begin();
		CoreObjectPtrVec::iterator iEnd = m_vUpdateList.end();
		while (iEnd != iCoreObject)
		{
			(*iCoreObject)->Update();
			++iCoreObject;
		}
	}

	void Display::RenderPass(DisplayRenderStagePtr _pRP)
	{
		const UInt uBlack = D3DCOLOR_XRGB(0, 0, 0);
		const UInt uBlue = D3DCOLOR_XRGB(16, 32, 64);
		const UInt uClearColor = uBlack;

		//vsoutput(__FUNCTION__" : render stage %x\n", _pRP->GetNameKey());

		_pRP->Update();

		// Render scene to buffers
		if ((NULL != m_pNormalProcesses) && (false == m_pNormalProcesses->empty()))
		{
			DisplayNormalProcessPtrVec::iterator iNormalProcess = m_pNormalProcesses->begin();
			DisplayNormalProcessPtrVec::iterator iEnd = m_pNormalProcesses->end();
			while (iEnd != iNormalProcess)
			{
				m_pCurrentNormalProcess = *iNormalProcess;
				m_pCurrentNormalProcess->RenderBegin();
				Render(_pRP);
				m_pCurrentNormalProcess->RenderEnd();

				++iNormalProcess;
			}
			m_pCurrentNormalProcess = NULL;
			m_pNormalProcesses = NULL;
		}
		else
		{
			m_pRTChain->EnableAllRenderTargets();
			m_pRTChain->RenderBegin(DisplayRenderTarget::ERenderMode_NORMALPROCESS);
			m_pRTChain->RenderBeginPass(0);
			m_pDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, uClearColor, 1.0f, 0L);
			Render(_pRP);
			m_pRTChain->RenderEndPass();
			m_pRTChain->RenderEnd();
		}

		if ((NULL != m_pPostProcesses) && (false == m_pPostProcesses->empty()))
		{
			// Apply post processes effects
			m_pRTChain->EnableAllRenderTargets();
			m_pRTChain->RenderBegin(DisplayRenderTarget::ERenderMode_POSTPROCESS);
			DisplayPostProcessPtrVec::iterator iPostProcess = m_pPostProcesses->begin();
			DisplayPostProcessPtrVec::iterator iEnd = m_pPostProcesses->end();
			while (iEnd != iPostProcess)
			{
				DisplayPostProcessPtr pPostProcess = *iPostProcess;
				pPostProcess->RenderBegin();
				pPostProcess->Update();
				pPostProcess->RenderEnd();
				++iPostProcess;
			}
			m_pRTChain->RenderEnd();
			m_pPostProcesses = NULL;
		}

		_pRP->GetRenderList().clear();
	}

	void Display::Render(DisplayRenderStagePtr _pRP)
	{
		m_pCurrentCamera->Update();

		RenderUpdate();

		// only use registered objects for this pass
		DisplayObjectPtrVec& vDisplayObjects = _pRP->GetRenderList();
		DisplayObjectPtrVec::iterator iDisplayObject = vDisplayObjects.begin();
		DisplayObjectPtrVec::iterator iEnd = vDisplayObjects.end();
		while (iEnd != iDisplayObject)
		{
			DisplayObjectPtr pDisplayObject = *iDisplayObject;
			DisplayMaterialPtr pMaterial = pDisplayObject->GetMaterial();
			DisplayEffectPtr pEffect = pMaterial->GetEffect();
			if (m_vRenderList.end() == find(m_vRenderList.begin(), m_vRenderList.end(), pEffect))
			{
				m_vRenderList.push_back(pEffect);
			}
			pEffect->RenderRequest(pMaterial);
			pMaterial->RenderRequest(pDisplayObject);
			++iDisplayObject;
		}

		struct RenderEffectFunction
		{
			void operator()(DisplayEffectPtr _pDisplayEffect)
			{
				_pDisplayEffect->Render();
			}
		};
		for_each(m_vRenderList.begin(), m_vRenderList.end(), RenderEffectFunction());
		m_vRenderList.clear();
	}

	unsigned int Display::GetFormatBitsPerPixel(const D3DFORMAT& _eFormat)
	{
		switch (_eFormat)
		{
			//case D3DFMT_UNKNOWN:
			case D3DFMT_R8G8B8:
			{
				return 24;
			}
			case D3DFMT_A8R8G8B8:
			case D3DFMT_X8R8G8B8:
			case D3DFMT_A2B10G10R10:
			case D3DFMT_A8B8G8R8:
			case D3DFMT_X8B8G8R8:
			case D3DFMT_G16R16:
			case D3DFMT_A2R10G10B10:
			case D3DFMT_X8L8V8U8:
			case D3DFMT_Q8W8V8U8:
			case D3DFMT_V16U16:
			case D3DFMT_A2W10V10U10:
			case D3DFMT_D32:
			case D3DFMT_D24S8:
			case D3DFMT_D24X8:
			case D3DFMT_D24X4S4:
			case D3DFMT_D32F_LOCKABLE:
			case D3DFMT_D24FS8:
		#if !defined(D3D_DISABLE_9EX)
			case D3DFMT_D32_LOCKABLE:
		#endif // !D3D_DISABLE_9EX
			case D3DFMT_INDEX32:
			case D3DFMT_G16R16F:
			case D3DFMT_R32F:
			{
				return 32;
			}
			case D3DFMT_R5G6B5:
			case D3DFMT_X1R5G5B5:
			case D3DFMT_A1R5G5B5:
			case D3DFMT_A4R4G4B4:
			case D3DFMT_A8R3G3B2:
			case D3DFMT_X4R4G4B4:
			case D3DFMT_A8L8:
			case D3DFMT_V8U8:
			case D3DFMT_L6V5U5:
			case D3DFMT_R8G8_B8G8:
			case D3DFMT_G8R8_G8B8:
			case D3DFMT_D16_LOCKABLE:
			case D3DFMT_D15S1:
			case D3DFMT_D16:
			case D3DFMT_L16:
			case D3DFMT_INDEX16:
			case D3DFMT_R16F:
			case D3DFMT_CxV8U8:
			{
				return 16;
			}
			case D3DFMT_R3G3B2:
			case D3DFMT_A8:
			case D3DFMT_A8P8:
			case D3DFMT_P8:
			case D3DFMT_L8:
			case D3DFMT_A4L4:
		#if !defined(D3D_DISABLE_9EX)
			case D3DFMT_S8_LOCKABLE:
		#endif // !D3D_DISABLE_9EX
			{
				return 8;
			}
			case D3DFMT_A16B16G16R16:
			case D3DFMT_G32R32F:
			case D3DFMT_Q16W16V16U16:
			case D3DFMT_A16B16G16R16F:
			{
				return 64;
			}
			case D3DFMT_A32B32G32R32F:
			{
				return 128;
			}
		#if !defined(D3D_DISABLE_9EX)
			case D3DFMT_A1:
			{
				return 1;
			}
		#endif // !D3D_DISABLE_9EX
			case D3DFMT_UYVY:
			case D3DFMT_YUY2:
			case D3DFMT_DXT1:
			case D3DFMT_DXT2:
			case D3DFMT_DXT3:
			case D3DFMT_DXT4:
			case D3DFMT_DXT5:
			case D3DFMT_VERTEXDATA:
			case D3DFMT_MULTI2_ARGB8:
		#if !defined(D3D_DISABLE_9EX)
			case D3DFMT_BINARYBUFFER:
		#endif // !D3D_DISABLE_9EX
			{
				return 0;
			}
		}
		return 0;
	}

	bool Display::IsPowerOf2(const unsigned int& _uValue, UIntPtr _pPowerLevel)
	{
		unsigned int uTemp = _uValue;
		unsigned int uBitsCount = 0;
		bool bResult = false;

		if (NULL == _pPowerLevel)
		{
			while (1 != uTemp)
			{
				uBitsCount = (0x1 == (0x1 & uTemp)) ? (uBitsCount + 1) : uBitsCount;
				uTemp >>= 1;
			}
			uBitsCount = (0x1 == (0x1 & uTemp)) ? (uBitsCount + 1) : uBitsCount;
		}
		else
		{
			(*_pPowerLevel) = 0;
			while (1 != uTemp)
			{
				uBitsCount = (0x1 == (0x1 & uTemp)) ? (uBitsCount + 1) : uBitsCount;
				uTemp >>= 1;
				++(*_pPowerLevel);
			}
			uBitsCount = (0x1 == (0x1 & uTemp)) ? (uBitsCount + 1) : uBitsCount;
		}

		bResult = (1 == uBitsCount); // is it a power of 2 number ??

		return bResult;
	}

	D3DFORMAT Display::StringToDisplayFormat(const string& _strFormatName, const D3DFORMAT& _uDefaultFormat)
	{
		if (false != s_mDisplayFormat.empty())
		{
			InitDisplayFormatMap();
		}
		Key uFormatNameKey = MakeKey(_strFormatName);
		if (s_mDisplayFormat.end() == s_mDisplayFormat.find(uFormatNameKey))
		{
			return _uDefaultFormat;
		}
		return s_mDisplayFormat[uFormatNameKey];
	}

	D3DFORMAT Display::KeyToDisplayFormat(const Key& _uFormatNameKey, const D3DFORMAT& _uDefaultFormat)
	{
		if (false != s_mDisplayFormat.empty())
		{
			InitDisplayFormatMap();
		}
		if (s_mDisplayFormat.end() == s_mDisplayFormat.find(_uFormatNameKey))
		{
			return _uDefaultFormat;
		}
		return s_mDisplayFormat[_uFormatNameKey];
	}

	void Display::InitDisplayFormatMap()
	{
		#define AddToDisplayFormatMap(Format) s_mDisplayFormat[MakeKey(string(#Format))] = Format;

		AddToDisplayFormatMap(D3DFMT_UNKNOWN);
		AddToDisplayFormatMap(D3DFMT_R8G8B8);
		AddToDisplayFormatMap(D3DFMT_A8R8G8B8);
		AddToDisplayFormatMap(D3DFMT_X8R8G8B8);
		AddToDisplayFormatMap(D3DFMT_A2B10G10R10);
		AddToDisplayFormatMap(D3DFMT_A8B8G8R8);
		AddToDisplayFormatMap(D3DFMT_X8B8G8R8);
		AddToDisplayFormatMap(D3DFMT_G16R16);
		AddToDisplayFormatMap(D3DFMT_A2R10G10B10);
		AddToDisplayFormatMap(D3DFMT_X8L8V8U8);
		AddToDisplayFormatMap(D3DFMT_Q8W8V8U8);
		AddToDisplayFormatMap(D3DFMT_V16U16);
		AddToDisplayFormatMap(D3DFMT_A2W10V10U10);
		AddToDisplayFormatMap(D3DFMT_D32);
		AddToDisplayFormatMap(D3DFMT_D24S8);
		AddToDisplayFormatMap(D3DFMT_D24X8);
		AddToDisplayFormatMap(D3DFMT_D24X4S4);
		AddToDisplayFormatMap(D3DFMT_D32F_LOCKABLE);
		AddToDisplayFormatMap(D3DFMT_D24FS8);
#if !defined(D3D_DISABLE_9EX)
		AddToDisplayFormatMap(D3DFMT_D32_LOCKABLE);
#endif // !D3D_DISABLE_9EX
		AddToDisplayFormatMap(D3DFMT_INDEX32);
		AddToDisplayFormatMap(D3DFMT_G16R16F);
		AddToDisplayFormatMap(D3DFMT_R32F);
		AddToDisplayFormatMap(D3DFMT_R5G6B5);
		AddToDisplayFormatMap(D3DFMT_X1R5G5B5);
		AddToDisplayFormatMap(D3DFMT_A1R5G5B5);
		AddToDisplayFormatMap(D3DFMT_A4R4G4B4);
		AddToDisplayFormatMap(D3DFMT_A8R3G3B2);
		AddToDisplayFormatMap(D3DFMT_X4R4G4B4);
		AddToDisplayFormatMap(D3DFMT_A8L8);
		AddToDisplayFormatMap(D3DFMT_V8U8);
		AddToDisplayFormatMap(D3DFMT_L6V5U5);
		AddToDisplayFormatMap(D3DFMT_R8G8_B8G8);
		AddToDisplayFormatMap(D3DFMT_G8R8_G8B8);
		AddToDisplayFormatMap(D3DFMT_D16_LOCKABLE);
		AddToDisplayFormatMap(D3DFMT_D15S1);
		AddToDisplayFormatMap(D3DFMT_D16);
		AddToDisplayFormatMap(D3DFMT_L16);
		AddToDisplayFormatMap(D3DFMT_INDEX16);
		AddToDisplayFormatMap(D3DFMT_R16F);
		AddToDisplayFormatMap(D3DFMT_CxV8U8);
		AddToDisplayFormatMap(D3DFMT_R3G3B2);
		AddToDisplayFormatMap(D3DFMT_A8);
		AddToDisplayFormatMap(D3DFMT_A8P8);
		AddToDisplayFormatMap(D3DFMT_P8);
		AddToDisplayFormatMap(D3DFMT_L8);
		AddToDisplayFormatMap(D3DFMT_A4L4);
#if !defined(D3D_DISABLE_9EX)
		AddToDisplayFormatMap(D3DFMT_S8_LOCKABLE);
#endif // !D3D_DISABLE_9EX
		AddToDisplayFormatMap(D3DFMT_A16B16G16R16);
		AddToDisplayFormatMap(D3DFMT_G32R32F);
		AddToDisplayFormatMap(D3DFMT_Q16W16V16U16);
		AddToDisplayFormatMap(D3DFMT_A16B16G16R16F);
		AddToDisplayFormatMap(D3DFMT_A32B32G32R32F);
#if !defined(D3D_DISABLE_9EX)
		AddToDisplayFormatMap(D3DFMT_A1);
#endif // !D3D_DISABLE_9EX
		AddToDisplayFormatMap(D3DFMT_UYVY);
		AddToDisplayFormatMap(D3DFMT_YUY2);
		AddToDisplayFormatMap(D3DFMT_DXT1);
		AddToDisplayFormatMap(D3DFMT_DXT2);
		AddToDisplayFormatMap(D3DFMT_DXT3);
		AddToDisplayFormatMap(D3DFMT_DXT4);
		AddToDisplayFormatMap(D3DFMT_DXT5);
		AddToDisplayFormatMap(D3DFMT_VERTEXDATA);
		AddToDisplayFormatMap(D3DFMT_MULTI2_ARGB8);
#if !defined(D3D_DISABLE_9EX)
		AddToDisplayFormatMap(D3DFMT_BINARYBUFFER);
#endif // !defined(D3D_DISABLE_9EX)

		#undef AddToDisplayFormatMap
	}
}
