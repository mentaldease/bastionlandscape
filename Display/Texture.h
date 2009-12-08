#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "../Core/Core.h"
#include "../Display/DisplayTypes.h"
#include "../Display/Display.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayTexture : public CoreObject
	{
	public:
		enum EType
		{
			EType_2D,
			EType_CUBE,
		};

		enum EUsage
		{
			EUsage_DEFAULT,
			EUsage_RENDERTARGET,
		};

		struct CreateInfo
		{
			bool			m_bLoadMode;
			string			m_strName;
			string			m_strPath;
			EType			m_eType;
			unsigned int	m_uWidth;
			unsigned int	m_uHeight;
			D3DFORMAT		m_eFormat;
			EUsage			m_eUsage;
			bool			m_bMipmap;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

	public:
		DisplayTexture(DisplayRef _rDisplay);
		virtual ~DisplayTexture();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual BaseTexturePtr GetBase();
		virtual SurfaceDescPtr GetDesc(const D3DCUBEMAP_FACES& _eFace);

	protected:
		bool Load(CreateInfoRef _rInfo);
		bool New(CreateInfoRef _rInfo);

	protected:
		SurfaceDesc		m_aSurfaceDescs[6];
		DisplayRef		m_rDisplay;
		EType			m_eType;
		TexturePtr		m_pTexture;
		CubeTexturePtr	m_pCubeTexture;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayTextureManager : public CoreObject
	{
	public:
		DisplayTextureManager(DisplayRef _rDisplay);
		virtual ~DisplayTextureManager();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		bool Load(const string& _strName, const string& _strPath, const DisplayTexture::EType& _eType);
		bool New(const string& _strName, const unsigned int& _uWidth, const unsigned int& _uHeight, const D3DFORMAT& _eFormat, const bool& _bMipmap, const DisplayTexture::EType& _eType, const DisplayTexture::EUsage& _eUsage);
		void Unload(const string& _strName);
		void Unload(const Key& _strNameKey);
		DisplayTexturePtr Get(const string& _strName);
		DisplayTexturePtr Get(const Key& _strNameKey);
		void UnloadAll();

		DisplayTexturePtr GetBySemantic(const Key& _uSemanticKey);
		void SetBySemantic(const Key& _uSemanticKey, DisplayTexturePtr _pTexture);

	protected:
		DisplayRef				m_rDisplay;
		DisplayTexturePtrMap	m_mTextures;
		DisplayTexturePtrMap	m_mSemanticTextures;
	};

}

#endif // __TEXTURE_H__
