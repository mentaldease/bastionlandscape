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

		struct CreateInfo
		{
			string	m_strName;
			string	m_strPath;
			EType	m_eType;
		};

	public:
		DisplayTexture(DisplayRef _rDisplay);
		virtual ~DisplayTexture();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual BaseTexturePtr GetBase();

	protected:
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
		void Unload(const string& _strName);
		DisplayTexturePtr Get(const string& _strName);
		void UnloadAll();

	protected:
		DisplayRef				m_rDisplay;
		DisplayTexturePtrMap	m_mTextures;
	};

}

#endif // __TEXTURE_H__
