#ifndef __DISPLAYTYPES_H__
#define __DISPLAYTYPES_H__

#include <d3d9.h>
#include <d3dx9.h>

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
	typedef D3DXMATRIX						Matrix;
	typedef Matrix*							MatrixPtr;
	typedef Matrix&							MatrixRef;
	typedef D3DVERTEXELEMENT9				VertexElement;
	typedef VertexElement*					VertexElementPtr;
	typedef IDirect3DVertexBuffer9			VertexBuffer;
	typedef VertexBuffer*					VertexBufferPtr;
	typedef IDirect3DVertexDeclaration9*	VertexDeclPtr;
	typedef IDirect3DIndexBuffer9			IndexBuffer;
	typedef IndexBuffer*					IndexBufferPtr;
	typedef ID3DXEffect*					EffectPtr;
	typedef LPD3DXBUFFER					BufferPtr;
	typedef D3DXHANDLE						Handle;
	typedef D3DVIEWPORT9					Viewport;
	typedef Viewport*						ViewportPtr;
	typedef Viewport&						ViewportRef;
	typedef IDirect3DTexture9				Texture;
	typedef Texture*						TexturePtr;
	typedef IDirect3DCubeTexture9			CubeTexture;
	typedef CubeTexture*					CubeTexturePtr;
	typedef LPDIRECT3DBASETEXTURE9			BaseTexturePtr;

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

	class DisplayIndexBuffer;
	typedef DisplayIndexBuffer* DisplayIndexBufferPtr;
	typedef DisplayIndexBuffer& DisplayIndexBufferRef;

	class DisplayCamera;
	typedef DisplayCamera* DisplayCameraPtr;
	typedef DisplayCamera& DisplayCameraRef;

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
}

#endif // __DISPLAYTYPES_H__
