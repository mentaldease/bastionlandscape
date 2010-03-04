#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "../Core/Core.h"
#include "../Display/DisplayTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	template<typename T>
	struct RenderFunction
	{
		void Do(T* pObject)
		{
			pObject->Render();
		}
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayVertexBuffer : public CoreObject
	{
	public:
		struct CreateInfo
		{
			unsigned int		m_uBufferSize;
			unsigned int		m_uVertexSize;
			VertexElementPtr	m_pVertexElement;
		};

	public:
		DisplayVertexBuffer(DisplayRef _rDisplay);
		virtual ~DisplayVertexBuffer();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		bool Set(const VoidPtr _pData);
		bool Use();

	protected:
		DisplayRef		m_rDisplay;
		unsigned int	m_uBufferSize;
		unsigned int	m_uVertexSize;
		VertexDeclPtr	m_pVertexDecl;
		VertexBufferPtr	m_pVertexBuffer;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayIndexBuffer : public CoreObject
	{
	public:
		struct CreateInfo
		{
			unsigned int	m_uBufferSize;
			bool			m_b16Bits;
		};

	public:
		DisplayIndexBuffer(DisplayRef _rDisplay);
		virtual ~DisplayIndexBuffer();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		bool Set(const VoidPtr _pData);
		bool Use();

	protected:
		DisplayRef		m_rDisplay;
		unsigned int	m_uBufferSize;
		bool			m_b16Bits;
		IndexBufferPtr	m_pIndexBuffer;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayObject : public CoreObject
	{
	public:
		DisplayObject();
		virtual ~DisplayObject();

		virtual MatrixPtr GetWorldMatrix();

		virtual void SetMaterial(DisplayMaterialPtr _pMaterial);
		virtual DisplayMaterialPtr GetMaterial();

		virtual void SetRenderStage(const Key& _uRenderPass);
		virtual const Key& GetRenderStage() const;

		virtual void RenderBegin() {};
		virtual void Render() = 0;
		virtual void RenderEnd() {};

	protected:
		Matrix				m_oWorld;
		DisplayMaterialPtr	m_pMaterial;
		Key					m_uRenderPass;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Display : public CoreObject, public WeakSingleton<Display>
	{
	public:
		Display();
		virtual ~Display();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		bool OpenVideo(WindowData& _rWindowData);
		void CloseVideo();

		void UpdateRequest(CoreObjectPtr _pCoreObject);
		void RenderRequest(const Key& _uRenderPassKey, DisplayObjectPtr _pDisplayObject);

		DisplayVertexBufferPtr CreateVertexBuffer(DisplayVertexBuffer::CreateInfo& _rCreateInfo);
		bool SetCurrentVertexBuffer(DisplayVertexBufferPtr _pVertexBuffer);
		DisplayVertexBufferPtr GetCurrentVertexBuffer();
		void ReleaseVertexBuffer(DisplayVertexBufferPtr _pVertexBuffer);

		DisplayIndexBufferPtr CreateIndexBuffer(DisplayIndexBuffer::CreateInfo& _rCreateInfo);
		bool SetCurrentIndexBuffer(DisplayIndexBufferPtr _pIndexBuffer);
		DisplayIndexBufferPtr GetCurrentIndexBuffer();
		void ReleaseIndexBuffer(DisplayIndexBufferPtr _pIndexBuffer);

		DevicePtr GetDevicePtr() const;
		void GetResolution(unsigned int& _uWidth, unsigned int& _uHeight) const;
		DisplayMaterialManagerPtr GetMaterialManager();
		DisplayTextureManagerPtr GetTextureManager();
		DisplaySurfaceManagerPtr GetSurfaceManager();
		DisplayFontManagerPtr GetFontManager();

		void SetCurrentWorldMatrix(MatrixPtr _pMatrix);
		MatrixPtr GetCurrentWorldMatrix();
		MatrixPtr GetCurrentWorldInvTransposeMatrix();

		void MRTRenderBeginPass(UIntRef _uIndex);
		void MRTRenderEndPass();
		void SetNormalProcessesList(DisplayNormalProcessPtrVecPtr _pNormalProcesses);
		void SetPostProcessesList(DisplayPostProcessPtrVecPtr _pPostProcesses);
		DisplayObjectPtr GetPostProcessGeometry();
		DisplayRenderTargetChainPtr GetRenderTargetChain();
		DisplayNormalProcessPtr GetCurrentNormalProcess();

		DisplayCameraPtr CreateCamera(const Key& _uNameKey, LuaObjectRef _rLuaObject);
		void ReleaseCamera(const Key& _uNameKey);
		DisplayCameraPtr GetCamera(const Key& _uNameKey);
		DisplayCameraPtr GetCurrentCamera();
		void SetCurrentCamera(DisplayCameraPtr _pCamera);
		bool AddViewport(const Key& _uNameKey, ViewportRef _rViewPort);
		ViewportPtr GetViewport(const Key& _uNameKey);

		void AddRenderStages(DisplayRenderStagePtrVec _vRenderPasses);
		void RemoveRenderStages(DisplayRenderStagePtrVec _vRenderPasses);
		DisplayRenderStagePtr GetCurrentRenderStage();

		static unsigned int GetFormatBitsPerPixel(const D3DFORMAT& _eFormat);
		static bool IsPowerOf2(const unsigned int& _uValue, UIntPtr _pPowerLevel = NULL);
		static D3DFORMAT StringToDisplayFormat(const string& _strFormatName, const D3DFORMAT& _uDefaultFormat);
		static D3DFORMAT KeyToDisplayFormat(const Key& _uFormatNameKey, const D3DFORMAT& _uDefaultFormat);

	protected:
		typedef map<Key, D3DFORMAT>	DisplayFormatMap;
		static DisplayFormatMap s_mDisplayFormat;
		static void InitDisplayFormatMap();

		void RenderUpdate();
		void RenderPass(DisplayRenderStagePtr _pRP);
		void Render(DisplayRenderStagePtr _pRP);

		Direct3DPtr						m_pDirect3D;
		DevicePtr						m_pDevice;
		DisplayEffectPtrVec				m_vRenderList;
		CoreObjectPtrVec				m_vUpdateList;
		ViewportMap						m_mViewports;
		DisplayCameraPtrMap				m_mCameras;
		DisplayRenderStagePtrVec		m_vRenderStages;
		DisplayRenderStagePtrMap		m_mRenderStages;
		DisplayRenderRequestListMap		m_mRenderRequests;
		DisplayCameraPtr				m_pCurrentCamera;
		DisplayMaterialManagerPtr		m_pMaterialManager;
		DisplayTextureManagerPtr		m_pTextureManager;
		DisplaySurfaceManagerPtr		m_pSurfaceManager;
		DisplayFontManagerPtr			m_pFontManager;
		MatrixPtr						m_pWorldMatrix;
		DisplayPostProcessPtrVecPtr		m_pPostProcesses;
		DisplayEffectPtr				m_pDispFXPP;
		EffectPtr						m_pEffectPP;
		DisplayRenderTargetGeometryPtr	m_pPostProcessGeometry;
		DisplayRenderTargetChainPtr		m_pRTChain;
		DisplayNormalProcessPtrVecPtr	m_pNormalProcesses;
		DisplayNormalProcessPtr			m_pCurrentNormalProcess;
		DisplayVertexBufferPtr			m_pCurrentVertexBuffer;
		DisplayIndexBufferPtr			m_pCurrentIndexBuffer;
		DisplayRenderStagePtr			m_pCurrentRenderStage;
		Matrix							m_oWorldInvTransposeMatrix;
		unsigned int					m_uWidth;
		unsigned int					m_uHeight;

	private:
	};
}

#endif // __DISPLAY_H__
