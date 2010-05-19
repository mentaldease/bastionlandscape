#include "stdafx.h"
#include "../Core/File.h"
#include "../Display/Font.h"
#include "../Display/FontBMF.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayFontText::DisplayFontText()
	:	DisplayObject()
	{

	}

	DisplayFontText::~DisplayFontText()
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayFont::DisplayFont(DisplayFontManagerRef _rFontManager)
	:	CoreObject(),
		m_rFontManager(_rFontManager)
	{

	}

	DisplayFont::~DisplayFont()
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayFontLoader::DisplayFontLoader(DisplayFontManagerRef _rFontManager)
	:	m_rFontManager(_rFontManager)
	{

	}

	DisplayFontLoader::~DisplayFontLoader()
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayFontManager::DisplayFontManager(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay)
	{

	}

	DisplayFontManager::~DisplayFontManager()
	{

	}

	bool DisplayFontManager::Create(const boost::any& _rConfig)
	{
		Release();

		DisplayFontLoaderPtr pLoader = new BitmapFont::DisplayFontLoader(*this);
		BitmapFont::DisplayFontLoader::CreateInfo BMFDLCInfo = { 5000, 100 };
		bool bResult = pLoader->Create(boost::any(&BMFDLCInfo));
		if (false != bResult)
		{
			RegisterLoader(MakeKey(string("fnt")), pLoader);
		}

		return bResult;
	}

	void DisplayFontManager::Update()
	{
		LinkMap::iterator iPair = m_mFonts.begin();
		LinkMap::iterator iEnd = m_mFonts.end();
		while (iEnd != iPair)
		{
			Link& rLink = iPair->second;
			rLink.m_pFont->Update();
			++iPair;
		}
	}

	void DisplayFontManager::Release()
	{
		{
			LinkMap::iterator iPair = m_mFonts.begin();
			LinkMap::iterator iEnd = m_mFonts.end();
			while (iEnd != iPair)
			{
				Link& rLink = iPair->second;
				rLink.m_pLoader->Unload(rLink.m_pFont);
				++iPair;
			}
			m_mFonts.clear();
		}
		{
			DisplayFontLoaderPtrMap::iterator iPair = m_mLoaders.begin();
			DisplayFontLoaderPtrMap::iterator iEnd = m_mLoaders.end();
			while (iEnd != iPair)
			{
				CoreObject::ReleaseDeleteReset(iPair->second);
				++iPair;
			}
			m_mLoaders.clear();
		}
	}

	bool DisplayFontManager::RegisterLoader(const Key& _uExtensionKey, DisplayFontLoaderPtr _pLoader)
	{
		DisplayFontLoaderPtr pLoader = GetLoader(_uExtensionKey);
		bool bResult = (NULL == pLoader) && (NULL != _pLoader);

		if (false != bResult)
		{
			m_mLoaders[_uExtensionKey] = _pLoader;
		}

		return bResult;
	}

	void DisplayFontManager::UnregisterLoader(const Key& _uExtensionKey)
	{
		DisplayFontLoaderPtr pLoader = GetLoader(_uExtensionKey);
		if (NULL != pLoader)
		{
			LinkMap::iterator iPair = m_mFonts.begin();
			LinkMap::iterator iEnd = m_mFonts.end();
			while (iEnd != iPair)
			{
				Link& rLink = iPair->second;
				if (pLoader == rLink.m_pLoader)
				{
					const Key uNameKey = iPair->first;
					rLink.m_pLoader->Unload(rLink.m_pFont);
					LinkMap::iterator iPairToErase = iPair;
					++iPair;
					m_mFonts.erase(iPairToErase);
					continue;
				}
				++iPair;
			}
			{
				DisplayFontLoaderPtrMap::iterator iPair = m_mLoaders.find(_uExtensionKey);
				m_mLoaders.erase(iPair);
			}
		}
	}

	bool DisplayFontManager::Load(const Key& _uNameKey, const string& _strFileName)
	{
		bool bResult = (NULL != Get(_uNameKey));

		if (false == bResult)
		{
			string strExtension;
			FS::GetFileExt(_strFileName, strExtension);
			const Key uExtensionKey = MakeKey(strExtension);
			DisplayFontLoaderPtr pLoader = GetLoader(uExtensionKey);
			if (NULL != pLoader)
			{
				DisplayFontPtr pFont = pLoader->Load(_strFileName);
				if (NULL != pFont)
				{
					m_mFonts[_uNameKey] = Link(pFont, pLoader);
					bResult = true;
				}
			}
		}

		return bResult;
	}

	void DisplayFontManager::Unload(const Key& _uNameKey)
	{
		LinkMap::iterator iPair = m_mFonts.find(_uNameKey);
		if (m_mFonts.end() != iPair)
		{
			Link& rLink = iPair->second;
			rLink.m_pLoader->Unload(rLink.m_pFont);
			m_mFonts.erase(iPair);
		}
	}

	DisplayFontPtr DisplayFontManager::Get(const Key& _uNameKey)
	{
		DisplayFontPtr pResult = NULL;
		LinkMap::iterator iPair = m_mFonts.find(_uNameKey);
		if (m_mFonts.end() != iPair)
		{
			Link& rLink = iPair->second;
			pResult = rLink.m_pFont;
		}
		return pResult;
	}

	DisplayRef DisplayFontManager::GetDisplay()
	{
		return m_rDisplay;
	}

	DisplayFontLoaderPtr DisplayFontManager::GetLoader(const Key& _uExtensionKey)
	{
		DisplayFontLoaderPtrMap::iterator iPair = m_mLoaders.find(_uExtensionKey);
		DisplayFontLoaderPtr pResult = (m_mLoaders.end() != iPair) ? iPair->second : NULL;
		return pResult;
	}

	DisplayFontManager::Link::Link()
	:	m_pFont(NULL),
		m_pLoader(NULL)
	{

	}

	DisplayFontManager::Link::Link(DisplayFontPtr _pFont, DisplayFontLoaderPtr _pLoader)
	:	m_pFont(_pFont),
		m_pLoader(_pLoader)
	{

	}

}