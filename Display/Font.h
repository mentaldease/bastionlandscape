#ifndef __FONT_H__
#define __FONT_H__

#include "../Display/Display.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayFontText : public DisplayObject
	{
	public:
		DisplayFontText();
		virtual ~DisplayFontText();

		virtual void SetWorldMatrix(MatrixRef _rWorld) = 0;
		virtual void SetText(const wstring& _wstrText) = 0;
		virtual void SetColor(const Vector4& _f4Color) = 0;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayFont : public CoreObject
	{
	public:
		DisplayFont(DisplayFontManagerRef _rFontManager);
		virtual ~DisplayFont();

		virtual DisplayFontTextPtr CreateText() = 0;
		virtual void ReleaseText(DisplayFontTextPtr _pText) = 0;
		virtual DisplayRef GetDisplay() = 0;

	protected:
		DisplayFontManagerRef	m_rFontManager;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayFontLoader : public CoreObject
	{
	public:
		DisplayFontLoader(DisplayFontManagerRef _rFontManager);
		virtual ~DisplayFontLoader();

		virtual DisplayFontPtr Load(const string& _strFileName) = 0;
		virtual void Unload(DisplayFontPtr _pFont) = 0;

	protected:
		DisplayFontManagerRef	m_rFontManager;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayFontManager : public CoreObject
	{
	public:
		DisplayFontManager(DisplayRef _rDisplay);
		virtual ~DisplayFontManager();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		bool Load(const Key& _uNameKey, const string& _strFileName);
		void Unload(const Key& _uNameKey);
		DisplayFontPtr Get(const Key& _uNameKey);

		DisplayRef GetDisplay();

	protected:
		struct Link
		{
			Link();
			Link(DisplayFontPtr _pFont, DisplayFontLoaderPtr _pLoader);

			DisplayFontPtr			m_pFont;
			DisplayFontLoaderPtr	m_pLoader;
		};
		typedef map<Key, Link> LinkMap;

	protected:
		bool RegisterLoader(const Key& _uExtensionKey, DisplayFontLoaderPtr _pLoader);
		void UnregisterLoader(const Key& _uExtensionKey);
		DisplayFontLoaderPtr GetLoader(const Key& _uExtensionKey);

	protected:
		DisplayRef				m_rDisplay;
		LinkMap					m_mFonts;
		DisplayFontLoaderPtrMap	m_mLoaders;
	};
}

#endif
