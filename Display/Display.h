#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "../Core/Core.h"
#include "../Core/ObjectCreator.h"
#include "../Core/Profiling.h"
#include "../Core/Pool.h"
#include "../Display/DisplayTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	typedef ObjectCreator<DisplayVertexBuffer> DisplayVertexBufferCreator;
	typedef ObjectCreator<DisplayIndexBuffer> DisplayIndexBufferCreator;

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

		static DisplayVertexBufferPtr NewInstance();
		static void DeleteInstance(DisplayVertexBufferPtr _pObject);

	protected:
		DisplayRef		m_rDisplay;
		unsigned int	m_uBufferSize;
		unsigned int	m_uVertexSize;
		Key				m_uVertexDecl;
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

		static DisplayIndexBufferPtr NewInstance();
		static void DeleteInstance(DisplayIndexBufferPtr _pObject);

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
		struct BoundingMesh;
		typedef BoundingMesh* BoundingMeshPtr;
		typedef BoundingMesh& BoundingMeshRef;

		struct BoundingMesh
		{
			void Clear();

			void AddVertex(const float _fX, const float _fY, const float _fZ);
			void AddTriangle(const UInt _uI1, const UInt _uI2, const UInt _uI3);

			void Transform(BoundingMeshRef _rBoundingMesh, MatrixRef _rm4Transform);

			Vector3Vec	m_vVertex;
			UIntVec		m_vTriangles; // 3 index per triangle
		};

	public:
		DisplayObject();
		virtual ~DisplayObject();

		virtual void Release();

		virtual void SetWorldMatrix(MatrixRef _rWorld);
		virtual MatrixPtr GetWorldMatrix();
		virtual void SetMaterial(DisplayMaterialPtr _pMaterial);
		virtual DisplayMaterialPtr GetMaterial();
		virtual void SetRenderStage(const Key& _uRenderPass);
		virtual const Key& GetRenderStage() const;

		virtual void RenderBegin() {};
		virtual void Render() = 0;
		virtual void RenderEnd() {};

		virtual bool RayIntersect(const Vector3& _f3RayBegin, const Vector3& _f3RayEnd, Vector3& _f3Intersect);
		virtual BoundingMeshRef GetBoundingMesh();
		virtual void GetTransformedBoundingMesh(BoundingMeshRef _rBoundingMesh, MatrixRef _rm4Transform);

		virtual void SetComponent(ComponentPtr _pComponent);
		virtual ComponentPtr GetComponent();

	protected:
		Matrix				m_m4World;
		BoundingMesh		m_oBoundingMesh;
		DisplayMaterialPtr	m_pMaterial;
		ComponentPtr		m_pComponent;
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

		bool OpenVideo(GraphicConfigDataRef _rGraphicConfigData);
		void CloseVideo();

		void UpdateRequest(CoreObjectPtr _pCoreObject);
		void RenderRequest(const Key& _uRenderPassKey, DisplayObjectPtr _pDisplayObject);
		void RenderRequest(DisplayObjectPtr _pDisplayObject);

		Key CreateVertexBufferKey(DisplayVertexBuffer::CreateInfo& _rCreateInfo);
		bool SetCurrentVertexBufferKey(const Key _uVertexBuffer);
		Key GetCurrentVertexBufferKey();
		void ReleaseVertexBufferKey(Key _uVertexBuffer);
		bool SetVertexBufferKeyData(const Key _uVertexBuffer, const VoidPtr _pData);

		Key CreateIndexBufferKey(DisplayIndexBuffer::CreateInfo& _rCreateInfo);
		bool SetCurrentIndexBufferKey(const Key _uIndexBuffer);
		Key GetCurrentIndexBufferKey();
		void ReleaseIndexBufferKey(const Key _uIndexBuffer);
		bool SetIndexBufferKeyData(const Key _uIndexBuffer, const VoidPtr _pData);

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
		DisplayStateManagerPtr GetStateManagerInterface();

		Key CreateVertexDeclaration(VertexElementPtr _pVertexElements);
		bool SetVertexDeclaration(const Key _uVertexDeclaration);
		Key GetCurrentVertexDeclaration();
		void ReleaseVertexDeclaration(const Key _uVertexDeclaration);

		void Unproject(const Vector3Ptr _pf3In, Vector3Ptr _pf3Out, DisplayCameraPtr _pCamera = NULL, const MatrixPtr _pObjectWorld = NULL);

		ComponentPtr NewComponent(EntityRef _rEntity, const boost::any& _rConfig);
		void DeleteComponent(ComponentPtr _pComponent);

		static unsigned int GetFormatBitsPerPixel(const D3DFORMAT& _eFormat);
		static bool IsPowerOf2(const unsigned int& _uValue, UIntPtr _pPowerLevel = NULL);
		static D3DFORMAT StringToDisplayFormat(const string& _strFormatName, const D3DFORMAT& _uDefaultFormat);
		static D3DFORMAT KeyToDisplayFormat(const Key& _uFormatNameKey, const D3DFORMAT& _uDefaultFormat);

	protected:
		typedef map<Key, D3DFORMAT>	DisplayFormatMap;
		static DisplayFormatMap s_mDisplayFormat;
		static void InitDisplayFormatMap();

		void RenderUpdate();
		void RenderStage(DisplayRenderStagePtr _pRS);
		void Render(DisplayRenderStagePtr _pRS);

		DisplayVertexBufferPtr CreateVertexBuffer(DisplayVertexBuffer::CreateInfo& _rCreateInfo);
		bool SetCurrentVertexBuffer(DisplayVertexBufferPtr _pVertexBuffer);
		DisplayVertexBufferPtr GetCurrentVertexBuffer();
		void ReleaseVertexBuffer(DisplayVertexBufferPtr _pVertexBuffer);

		DisplayIndexBufferPtr CreateIndexBuffer(DisplayIndexBuffer::CreateInfo& _rCreateInfo);
		bool SetCurrentIndexBuffer(DisplayIndexBufferPtr _pIndexBuffer);
		DisplayIndexBufferPtr GetCurrentIndexBuffer();
		void ReleaseIndexBuffer(DisplayIndexBufferPtr _pIndexBuffer);

		Direct3DPtr						m_pDirect3D;
		DevicePtr						m_pDevice;
		DisplayEffectPtrVec				m_vRenderList;
		CoreObjectPtrVec				m_vUpdateList;
		ViewportMap						m_mViewports;
		DisplayCameraPtrMap				m_mCameras;
		DisplayRenderStagePtrVec		m_vRenderStages;
		DisplayRenderStagePtrMap		m_mRenderStages;
		DisplayRenderRequestListMap		m_mRenderRequests;
		DisplayVertexBufferCreator		m_oVertexBuffers;
		DisplayIndexBufferCreator		m_oIndexBuffer;
		VertexDeclPtrMap				m_mVertexDecls;
		DisplayComponentFactory			m_oComponentFactory;
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
		DisplayPostProcessPtr			m_pCurrentPostProcess;
		DisplayVertexBufferPtr			m_pCurrentVertexBuffer;
		DisplayIndexBufferPtr			m_pCurrentIndexBuffer;
		DisplayRenderStagePtr			m_pCurrentRenderStage;
		DisplayStateManagerPtr			m_pStateManagerInterface;
		Matrix							m_m4WorldInvTransposeMatrix;
		Key								m_uCurrentVertexBuffer;
		Key								m_uCurrentIndexBuffer;
		Key								m_uCurrentVertexDecl;
		Key								m_uVertexDeclID;
		UInt							m_uWidth;
		UInt							m_uHeight;
		UInt							m_bDepthBytes;

	private:
	};
}

#endif // __DISPLAY_H__
