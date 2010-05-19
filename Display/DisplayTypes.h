#ifndef __DISPLAYTYPES_H__
#define __DISPLAYTYPES_H__

#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif // D3D_DEBUG_INFO

#include <d3d9.h>
#include <d3dx9.h>

#define DISPLAY_TEST_MRT	1

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	typedef IDirect3D9						Direct3D;
	typedef Direct3D*						Direct3DPtr;
	typedef IDirect3DDevice9				Device;
	typedef Device*							DevicePtr;
	typedef Device&							DeviceRef;
	typedef D3DXVECTOR2						Vector2;
	typedef D3DXVECTOR3						Vector3;
	typedef D3DXVECTOR4						Vector4;
	typedef Vector2*						Vector2Ptr;
	typedef Vector3*						Vector3Ptr;
	typedef Vector4*						Vector4Ptr;
	typedef map<Key, Vector2*>				Vector2PtrMap;
	typedef map<Key, Vector3*>				Vector3PtrMap;
	typedef map<Key, Vector4*>				Vector4PtrMap;
	typedef vector<Vector2>					Vector2Vec;
	typedef vector<Vector3>					Vector3Vec;
	typedef vector<Vector4>					Vector4Vec;
	typedef D3DXMATRIX						Matrix;
	typedef Matrix*							MatrixPtr;
	typedef Matrix&							MatrixRef;
	typedef map<Key, MatrixPtr>				MatrixPtrMap;
	typedef D3DVERTEXELEMENT9				VertexElement;
	typedef VertexElement*					VertexElementPtr;
	typedef IDirect3DVertexBuffer9			VertexBuffer;
	typedef VertexBuffer*					VertexBufferPtr;
	typedef IDirect3DVertexDeclaration9*	VertexDeclPtr;
	typedef map<Key, VertexDeclPtr>			VertexDeclPtrMap;
	typedef IDirect3DIndexBuffer9			IndexBuffer;
	typedef IndexBuffer*					IndexBufferPtr;
	typedef ID3DXEffect*					EffectPtr;
	typedef D3DXEFFECT_DESC					EffectDesc;
	typedef D3DXPARAMETER_DESC				EffectParamDesc;
	typedef EffectParamDesc*				EffectParamDescPtr;
	typedef EffectDesc*						EffectDescPtr;
	typedef LPD3DXBUFFER					BufferPtr;
	typedef D3DXHANDLE						Handle;
	typedef map<Key, Handle>				HandleMap;
	typedef HandleMap&						HandleMapRef;
	typedef D3DVIEWPORT9					Viewport;
	typedef Viewport*						ViewportPtr;
	typedef Viewport&						ViewportRef;
	typedef IDirect3DTexture9				Texture;
	typedef Texture*						TexturePtr;
	typedef IDirect3DCubeTexture9			CubeTexture;
	typedef CubeTexture*					CubeTexturePtr;
	typedef LPDIRECT3DBASETEXTURE9			BaseTexturePtr;
	typedef D3DXPLANE						Plane;
	typedef Plane*							PlanePtr;
	typedef IDirect3DSurface9				Surface;
	typedef Surface*						SurfacePtr;
	typedef Surface&						SurfaceRef;
	typedef D3DXIMAGE_INFO					ImageInfo;
	typedef ImageInfo*						ImageInfoPtr;
	typedef ImageInfo&						ImageInfoRef;
	typedef D3DLOCKED_RECT					LockedRect;
	typedef LockedRect*						LockedRectPtr;
	typedef LockedRect&						LockedRectRef;
	typedef D3DSURFACE_DESC					SurfaceDesc;
	typedef SurfaceDesc*					SurfaceDescPtr;
	typedef SurfaceDesc&					SurfaceDescRef;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Display;
	typedef Display* DisplayPtr;
	typedef Display& DisplayRef;

	class DisplayObject;
	typedef DisplayObject* DisplayObjectPtr;
	typedef DisplayObject& DisplayObjectRef;
	typedef vector<DisplayObjectPtr> DisplayObjectPtrVec;

	class DisplayVertexBuffer;
	typedef DisplayVertexBuffer* DisplayVertexBufferPtr;
	typedef DisplayVertexBuffer& DisplayVertexBufferRef;
	typedef vector<DisplayVertexBufferPtr> DisplayVertexBufferPtrVec;
	typedef map<Key, DisplayVertexBufferPtr> DisplayVertexBufferPtrMap;

	class DisplayIndexBuffer;
	typedef DisplayIndexBuffer* DisplayIndexBufferPtr;
	typedef DisplayIndexBuffer& DisplayIndexBufferRef;
	typedef vector<DisplayIndexBufferPtr> DisplayIndexBufferPtrVec;
	typedef map<Key, DisplayIndexBufferPtr> DisplayIndexBufferPtrMap;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayCamera;
	typedef DisplayCamera* DisplayCameraPtr;
	typedef DisplayCamera& DisplayCameraRef;
	typedef vector<DisplayCameraPtr> DisplayCameraPtrVec;
	typedef map<Key, DisplayCameraPtr> DisplayCameraPtrMap;

	struct AABB;
	typedef AABB* AABBPtr;
	typedef AABB& AABBRef;

	typedef map<Key, Viewport>	ViewportMap;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayEffect;
	typedef DisplayEffect* DisplayEffectPtr;
	typedef DisplayEffect& DisplayEffectRef;
	typedef vector<DisplayEffectPtr> DisplayEffectPtrVec;
	typedef map<Key, DisplayEffectPtr> DisplayEffectPtrMap;

	class DisplayMaterial;
	typedef DisplayMaterial* DisplayMaterialPtr;
	typedef DisplayMaterial& DisplayMaterialRef;
	typedef vector<DisplayMaterialPtr> DisplayMaterialPtrVec;
	typedef map<Key, DisplayMaterialPtr> DisplayMaterialPtrMap;

	class DisplayMaterialManager;
	typedef DisplayMaterialManager* DisplayMaterialManagerPtr;
	typedef DisplayMaterialManager& DisplayMaterialManagerRef;

	class DisplayEffectParam;
	typedef DisplayEffectParam* DisplayEffectParamPtr;
	typedef DisplayEffectParam& DisplayEffectParamRef;
	typedef vector<DisplayEffectParamPtr> DisplayEffectParamPtrVec;

	typedef boost::function<DisplayEffectParamPtr (const boost::any& _rConfig)> CreateParamFunc;
	typedef map<Key, CreateParamFunc> CreateParamFuncMap;

	class DisplayEffectInclude;
	typedef DisplayEffectInclude* DisplayEffectIncludePtr;
	typedef DisplayEffectInclude& DisplayEffectIncludeRef;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayTextureManager;
	typedef DisplayTextureManager* DisplayTextureManagerPtr;
	typedef DisplayTextureManager& DisplayTextureManagerRef;

	class DisplayTexture;
	typedef DisplayTexture* DisplayTexturePtr;
	typedef DisplayTexture& DisplayTextureRef;
	typedef map<Key, DisplayTexturePtr> DisplayTexturePtrMap;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplaySurfaceManager;
	typedef DisplaySurfaceManager* DisplaySurfaceManagerPtr;
	typedef DisplaySurfaceManager& DisplaySurfaceManagerRef;

	class DisplaySurface;
	typedef DisplaySurface* DisplaySurfacePtr;
	typedef DisplaySurface& DisplaySurfaceRef;
	typedef map<Key, DisplaySurfacePtr> DisplaySurfacePtrMap;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayRenderTarget;
	typedef DisplayRenderTarget* DisplayRenderTargetPtr;
	typedef DisplayRenderTarget& DisplayRenderTargetRef;
	typedef vector<DisplayRenderTargetPtr> DisplayRenderTargetPtrVec;

	class DisplayRenderTargetChain;
	typedef DisplayRenderTargetChain* DisplayRenderTargetChainPtr;
	typedef DisplayRenderTargetChain& DisplayRenderTargetChainRef;
	typedef map<Key, DisplayRenderTargetChainPtr> DisplayRenderTargetChainPtrMap;

	class DisplayRenderTargetGeometry;
	typedef DisplayRenderTargetGeometry* DisplayRenderTargetGeometryPtr;
	typedef DisplayRenderTargetGeometry& DisplayRenderTargetGeometryRef;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayPostProcess;
	typedef DisplayPostProcess* DisplayPostProcessPtr;
	typedef DisplayPostProcess& DisplayPostProcessRef;
	typedef vector<DisplayPostProcessPtr> DisplayPostProcessPtrVec;
	typedef DisplayPostProcessPtrVec* DisplayPostProcessPtrVecPtr;
	typedef map<Key, DisplayPostProcessPtr> DisplayPostProcessPtrMap;
	typedef DisplayPostProcessPtrMap* DisplayPostProcessPtrMapPtr;

	class DisplayNormalProcess;
	typedef DisplayNormalProcess* DisplayNormalProcessPtr;
	typedef DisplayNormalProcess& DisplayNormalProcessRef;
	typedef vector<DisplayNormalProcessPtr> DisplayNormalProcessPtrVec;
	typedef DisplayNormalProcessPtrVec* DisplayNormalProcessPtrVecPtr;
	typedef map<Key, DisplayNormalProcessPtr> DisplayNormalProcessPtrMap;
	typedef DisplayNormalProcessPtrMap* DisplayNormalProcessPtrMapPtr;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayFontText;
	typedef DisplayFontText* DisplayFontTextPtr;
	typedef DisplayFontText& DisplayFontTextRef;

	class DisplayFont;
	typedef DisplayFont* DisplayFontPtr;
	typedef DisplayFont& DisplayFontRef;
	typedef vector<DisplayFontPtr> DisplayFontPtrVec;

	class DisplayFontLoader;
	typedef DisplayFontLoader* DisplayFontLoaderPtr;
	typedef DisplayFontLoader& DisplayFontLoaderRef;
	typedef map<Key, DisplayFontLoaderPtr> DisplayFontLoaderPtrMap;
	typedef vector<DisplayFontLoaderPtr> DisplayFontLoaderPtrVec;

	class DisplayFontManager;
	typedef DisplayFontManager* DisplayFontManagerPtr;
	typedef DisplayFontManager& DisplayFontManagerRef;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayRenderStage;
	typedef DisplayRenderStage* DisplayRenderStagePtr;
	typedef DisplayRenderStage& DisplayRenderStageRef;
	typedef map<Key, DisplayRenderStagePtr> DisplayRenderStagePtrMap;
	typedef vector<DisplayRenderStagePtr> DisplayRenderStagePtrVec;
	typedef map<Key, DisplayObjectPtrVec> DisplayRenderRequestListMap;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct GeometryHelperVertex;
	typedef GeometryHelperVertex* GeometryHelperVertexPtr;
	typedef GeometryHelperVertex& GeometryHelperVertexRef;
	typedef vector<GeometryHelperVertex> GeometryHelperVertexVec;
	typedef GeometryHelperVertexVec::iterator GeometryHelperVertexVecIt;

	struct GeometryHelperVertexColor;
	typedef GeometryHelperVertexColor* GeometryHelperVertexColorPtr;
	typedef GeometryHelperVertexColor& GeometryHelperVertexColorRef;
	typedef vector<GeometryHelperVertexColor> GeometryHelperVertexColorVec;
	typedef GeometryHelperVertexColorVec::iterator GeometryHelperVertexColorVecIt;

	class DisplayGeometrySphere;
	typedef DisplayGeometrySphere* DisplayGeometrySpherePtr;
	typedef DisplayGeometrySphere& DisplayGeometrySphereRef;

	class DisplayGeometryLineManager;
	typedef DisplayGeometryLineManager* DisplayGeometryLineManagerPtr;
	typedef DisplayGeometryLineManager& DisplayGeometryLineManagerRef;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class OctreeTraverseFuncFrustum;
	typedef OctreeTraverseFuncFrustum* OctreeTraverseFuncFrustumPtr;
	typedef OctreeTraverseFuncFrustum& OctreeTraverseFuncFrustumRef;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayStateManager;
	typedef DisplayStateManager* DisplayStateManagerPtr;
	typedef DisplayStateManager& DisplayStateManagerRef;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayObjectDummy;
	typedef DisplayObjectDummy* DisplayObjectDummyPtr;
	typedef DisplayObjectDummy& DisplayObjectDummyRef;

	class DisplayComponent;
	typedef DisplayComponent* DisplayComponentPtr;
	typedef DisplayComponent& DisplayComponentRef;

	typedef ComponentFactory<DisplayComponent> DisplayComponentFactory;
}

#endif // __DISPLAYTYPES_H__
