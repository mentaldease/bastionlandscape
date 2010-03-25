#include "stdafx.h"
#include "../Display/FontBMF.h"
#include "../Display/Effect.h"
#include "../Display/EffectParam.h"

namespace ElixirEngine
{
	namespace BitmapFont
	{
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		#define SV2	sizeof(Vector2)
		#define SV3	sizeof(Vector3)
		#define SV4	sizeof(Vector4)

		VertexElement VertexFont::s_VertexElement[3] =
		{
			{ 0,	0,						D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
			{ 0,	1 * SV3,				D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
			D3DDECL_END()
		};

		#undef SV2
		#undef SV3
		#undef SV4

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		DisplayFontText::DisplayFontText()
		:	ElixirEngine::DisplayFontText(),
			m_wstrText(),
			m_f4Color(0.0f, 0.0f, 0.0f, 0.0f),
			m_pFont(NULL),
			m_pVertex(NULL),
			m_pPreviousVertexBuffer(NULL),
			m_uPreviousVertexDecl(0),
			m_uPreviousVBOffset(0),
			m_uPreviousVBStride(0),
			m_uVertexCount(0),
			m_bTextChanged(false),
			m_bSizeChanged(false),
			m_bRebuildText(false)
		{

		}

		DisplayFontText::~DisplayFontText()
		{

		}

		bool DisplayFontText::Create(const boost::any& _rConfig)
		{
			Release();

			m_pFont = boost::any_cast<DisplayFontPtr>(_rConfig);
			const bool bResult = (NULL != m_pFont);

			return (NULL != m_pFont);
		}

		void DisplayFontText::Release()
		{
			if (NULL != m_pVertex)
			{
				m_pFont->m_rFontLoader.GetVertexPool().Free(m_pVertex);
				m_pVertex = NULL;
			}
			DisplayObject::Release();
		}

		void DisplayFontText::RenderBegin()
		{
			DisplayPtr pDisplay = Display::GetInstance();
			m_uPreviousVertexDecl = pDisplay->GetCurrentVertexDeclaration();
			if (m_uPreviousVertexDecl != m_pFont->GetLoader().GetVertexDecl())
			{
				pDisplay->SetVertexDeclaration(m_pFont->GetLoader().GetVertexDecl());
			}
			const static Key uBitmapFontDiffuse = MakeKey(string("BITMAPFONTDIFFUSE"));
			pDisplay->GetMaterialManager()->SetVector4BySemantic(uBitmapFontDiffuse, &m_f4Color);
		}

		void DisplayFontText::Render()
		{
			if (false != m_bRebuildText)
			{
				BuildText();
				m_bTextChanged = false;
				m_bSizeChanged = false;
				m_bRebuildText = false;
			}

			if (NULL != m_pVertex)
			{
				Display::GetInstance()->GetDevicePtr()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, m_uVertexCount - 2, m_pVertex, sizeof(VertexFont));
			}
		}

		void DisplayFontText::RenderEnd()
		{
			if ((m_uPreviousVertexDecl != m_pFont->GetLoader().GetVertexDecl()) && (0 != m_uPreviousVertexDecl))
			{
				Display::GetInstance()->SetVertexDeclaration(m_uPreviousVertexDecl);
			}
		}

		void DisplayFontText::SetText(const wstring& _wstrText)
		{
			m_bSizeChanged = (m_wstrText.length() != _wstrText.length());
			m_bTextChanged = (m_wstrText != _wstrText) || ((false == _wstrText.empty()) && (NULL == m_pVertex));
			m_bRebuildText = m_bSizeChanged || m_bTextChanged;
			m_wstrText = _wstrText;
		}

		void DisplayFontText::SetColor(const Vector4& _f4Color)
		{
			m_f4Color = _f4Color;
		}

		void DisplayFontText::BuildText()
		{
			// +4 vertex for each char
			// +2 vertex to link to next char
			// -2 for the last one char
			const UInt uCharCount = UInt(m_wstrText.length());
			const UInt uVertexCount = uCharCount * (4 + 2) - 2;

			if ((NULL != m_pVertex) && (false != m_bSizeChanged))
			{
				m_pFont->m_rFontLoader.GetVertexPool().Free(m_pVertex);
				m_pVertex = NULL;
			}

			m_uVertexCount = uVertexCount;

			if ((NULL == m_pVertex) && (0 < m_uVertexCount))
			{
				m_pVertex = m_pFont->m_rFontLoader.GetVertexPool().Alloc(m_uVertexCount);
			}

			if (NULL != m_pVertex)
			{
				VertexFontPtr pVertex = m_pVertex;
				float fXOffset = 0.0f;
				float fYOffset = 0.0f;
				float fZOffset = 0.0f;
				const float fScale = 1.0f;
				UInt wcPrevChar = 0;

				for (UInt i = 0 ; uCharCount > i ; ++i)
				{
					UInt wcChar = m_wstrText[i];
					BlockCharRef rChar = m_pFont->m_mBlockChars[wcChar];
					float fPageID = float(rChar.m_uPage);

					//{
					//	char szBuffer[1024];
					//	const char* pBuffer = &szBuffer[0];
					//	sprintf(szBuffer, "fheight=%f fwidth=%f fx=%f fy=%f adv=%d offx=%d offy=%d ID=%u\n", rChar.m_fHeight, rChar.m_fWidth, rChar.m_fX, rChar.m_fY, rChar.m_sXAdvance, rChar.m_sXOffset, rChar.m_sYOffset, rChar.m_uID);
					//	wchar_t wszBuffer[1024];
					//	mbsrtowcs(wszBuffer, &pBuffer, strlen(szBuffer) + 1, NULL);
					//	OutputDebugString(wszBuffer);
					//}

					fXOffset += float(m_pFont->GetKerning(wcPrevChar, wcChar));

					// top left
					{
						VertexFontRef rVertex = pVertex[0];
						rVertex.m_f3Pos.x = fXOffset + 0.0f;
						rVertex.m_f3Pos.y = fYOffset + 0.0f;
						rVertex.m_f3Pos.z = fZOffset + 0.0f;
						rVertex.m_f2UV = Vector3(rChar.m_fX, rChar.m_fY, fPageID);
						rVertex.m_f3Pos.x *= fScale;
						rVertex.m_f3Pos.y *= fScale;
					}
					// bottom left
					{
						VertexFontRef rVertex = pVertex[1];
						rVertex.m_f3Pos.x = fXOffset + 0.0f;
						rVertex.m_f3Pos.y = fYOffset - float(rChar.m_sYOffset + rChar.m_uHeight);
						rVertex.m_f3Pos.z = fZOffset + 0.0f;
						rVertex.m_f2UV = Vector3(rChar.m_fX, rChar.m_fY + rChar.m_fHeight, fPageID);
						rVertex.m_f3Pos.x *= fScale;
						rVertex.m_f3Pos.y *= fScale;
					}
					// top right
					{
						VertexFontRef rVertex = pVertex[2];
						rVertex.m_f3Pos.x = fXOffset + float(rChar.m_sXOffset + rChar.m_uWidth);
						rVertex.m_f3Pos.y = fYOffset + 0.0f;
						rVertex.m_f3Pos.z = fZOffset + 0.0f;
						rVertex.m_f2UV = Vector3(rChar.m_fX + rChar.m_fWidth, rChar.m_fY, fPageID);
						rVertex.m_f3Pos.x *= fScale;
						rVertex.m_f3Pos.y *= fScale;
					}
					// bottom right
					{
						VertexFontRef rVertex = pVertex[3];
						rVertex.m_f3Pos.x = fXOffset + float(rChar.m_sXOffset + rChar.m_uWidth);
						rVertex.m_f3Pos.y = fYOffset - float(rChar.m_sYOffset + rChar.m_uHeight);
						rVertex.m_f3Pos.z = fZOffset + 0.0f;
						rVertex.m_f2UV = Vector3(rChar.m_fX + rChar.m_fWidth, rChar.m_fY + rChar.m_fHeight, fPageID);
						rVertex.m_f3Pos.x *= fScale;
						rVertex.m_f3Pos.y *= fScale;
					}

					// link to next char
					if ((uCharCount - 1) > i)
					{
						fXOffset += float(rChar.m_sXAdvance);
						wcPrevChar = wcChar;

						pVertex[4] = pVertex[3];

						wcChar = m_wstrText[i + 1];
						const float fKerning = float(m_pFont->GetKerning(wcPrevChar, wcChar));
						BlockCharRef rNextChar = m_pFont->m_mBlockChars[wcChar];
						fPageID = float(rNextChar.m_uPage);
						VertexFontRef rVertex = pVertex[5];
						rVertex.m_f3Pos.x = fXOffset + 0.0f + fKerning;
						rVertex.m_f3Pos.y = fYOffset + 0.0f;
						rVertex.m_f3Pos.z = fZOffset + 0.0f;
						rVertex.m_f2UV = Vector3(rNextChar.m_fX, rNextChar.m_fY, fPageID);
						rVertex.m_f3Pos.x *= fScale;
						rVertex.m_f3Pos.y *= fScale;

						pVertex += (4 + 2);
					}
				}
			}
		}

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		DisplayFont::DisplayFont(DisplayFontManagerRef _rFontManager, DisplayFontLoaderRef _rFontLoader)
		:	ElixirEngine::DisplayFont(_rFontManager),
			m_rFontLoader(_rFontLoader)
		{
			m_oBlockInfo.m_sFontSize = 0;
			m_oBlockInfo.m_uBitField = 0;
			m_oBlockInfo.m_uCharSet = 0;
			m_oBlockInfo.m_uStretchH = 0;
			m_oBlockInfo.m_uAA = 0;
			m_oBlockInfo.m_uPaddingUp = 0;
			m_oBlockInfo.m_uPaddingRight = 0;
			m_oBlockInfo.m_uPaddingDown = 0;
			m_oBlockInfo.m_uPaddingLeft = 0;
			m_oBlockInfo.m_uSpacingHoriz = 0;
			m_oBlockInfo.m_uSpacingVert = 0;
			m_oBlockInfo.m_uOutline = 0;
			m_oBlockInfo.m_strFontName.clear();

			m_oBlockCommon.m_uLineHeight = 0;
			m_oBlockCommon.m_uBase = 0;
			m_oBlockCommon.m_uScaleW = 0;
			m_oBlockCommon.m_uScaleH = 0;
			m_oBlockCommon.m_uPages = 0;
			m_oBlockCommon.m_uBitField = 0;
			m_oBlockCommon.m_uAlphaChannel = 0;
			m_oBlockCommon.m_uRedChannel = 0;
			m_oBlockCommon.m_uGreenChannel = 0;
			m_oBlockCommon.m_uBlueChannel = 0;
		}

		DisplayFont::~DisplayFont()
		{
		}

		bool DisplayFont::Create(const boost::any& _rConfig)
		{
			Release();

			const string& strFileName = boost::any_cast<const string&>(_rConfig);
			FilePtr pFile = FS::GetRoot()->OpenFile(strFileName, FS::EOpenMode_READBINARY);
			bool bResult = (NULL != pFile);

			if (false != bResult)
			{
				Byte uVersion = 0;
				bResult = CheckHeader(pFile, uVersion);
				if (false != bResult)
				{
					Byte uBlockType = 0;
					UInt uBlockSise = 0;
					do 
					{
						pFile->Read(&uBlockType, sizeof(Byte));

						// we must read something before detecting end of file.
						if (false != pFile->EndOfFile())
						{
							break;
						}

						pFile->Read(&uBlockSise, sizeof(UInt));

						switch (uBlockType)
						{
							case 1:
							{
								bResult = ReadBlockInfo(pFile, uBlockSise);
								break;
							}
							case 2:
							{
								bResult = ReadBlockCommon(pFile, uBlockSise);
								break;
							}
							case 3:
							{
								bResult = ReadBlockPages(pFile, uBlockSise);
								break;
							}
							case 4:
							{
								bResult = ReadBlockChars(pFile, uBlockSise);
								break;
							}
							case 5:
							{
								bResult = ReadBlockKerning(pFile, uBlockSise);
								break;
							}
							default:
							{
								bResult = false;
								break;
							}
						}
					}
					while (false != bResult);
				}
				FS::GetRoot()->CloseFile(pFile);
			}

			if (false != bResult)
			{
				string strPath;
				FS::GetPathWithoutFileName(strFileName, strPath, true);
				bResult = LoadTextures(strPath);
			}

			return bResult;
		}

		void DisplayFont::Update()
		{

		}

		void DisplayFont::Release()
		{
			DisplayTextureManagerPtr pTextureManager = Display::GetInstance()->GetTextureManager();
			for (UInt i = 0 ; m_oBlockCommon.m_uPages > i ; ++i)
			{
				const Key uKeyName = MakeKey(m_oBlockPages.m_vPageNames[i]);
				pTextureManager->Unload(uKeyName);
			}

			m_mTextures.clear();
			m_oBlockPages.m_vPageNames.clear();
			m_mBlockChars.clear();
			m_vBlockKernings.clear();
		}

		ElixirEngine::DisplayFontTextPtr DisplayFont::CreateText()
		{
			DisplayFontTextPtr pResult = m_rFontLoader.GetTextPool().Alloc();
			if (NULL != pResult)
			{
				if (false == pResult->Create(boost::any(this)))
				{
					ReleaseText(pResult);
					pResult = NULL;
				}
			}
			return pResult;
		}

		void DisplayFont::ReleaseText(ElixirEngine::DisplayFontTextPtr _pText)
		{
			m_rFontLoader.GetTextPool().Free(static_cast<TextPool::TPtr>(_pText));
		}

		DisplayRef DisplayFont::GetDisplay()
		{
			return m_rFontManager.GetDisplay();
		}

		DisplayFontLoaderRef DisplayFont::GetLoader()
		{
			return m_rFontLoader;
		}

		bool DisplayFont::CheckHeader(FilePtr _pFile, Byte& _uVersion)
		{
			char aHeader[4];
			_pFile->Read(aHeader, 4 * sizeof(char));
			const bool bResult = ('B' == aHeader[0]) && ('M' == aHeader[1]) && ('F' == aHeader[2]);
			_uVersion = (false != bResult) ? Byte(aHeader[3]) : 0;
			return bResult;
		}

		bool DisplayFont::ReadBlockInfo(FilePtr _pFile, UInt& _uBlockSise)
		{
			bool bResult = true;

			_pFile->Read(&m_oBlockInfo.m_sFontSize, sizeof(short));
			_pFile->Read(&m_oBlockInfo.m_uBitField, sizeof(Byte	));
			_pFile->Read(&m_oBlockInfo.m_uCharSet, sizeof(Byte));
			_pFile->Read(&m_oBlockInfo.m_uStretchH, sizeof(Word));
			_pFile->Read(&m_oBlockInfo.m_uAA, sizeof(Byte));
			_pFile->Read(&m_oBlockInfo.m_uPaddingUp, sizeof(Byte));
			_pFile->Read(&m_oBlockInfo.m_uPaddingRight, sizeof(Byte));
			_pFile->Read(&m_oBlockInfo.m_uPaddingDown, sizeof(Byte));
			_pFile->Read(&m_oBlockInfo.m_uPaddingLeft, sizeof(Byte));
			_pFile->Read(&m_oBlockInfo.m_uSpacingHoriz, sizeof(Byte));
			_pFile->Read(&m_oBlockInfo.m_uSpacingVert, sizeof(Byte));
			_pFile->Read(&m_oBlockInfo.m_uOutline, sizeof(Byte));

			{
				// because of memory alignment sizeof(BlockInfo) + sizeof(string) may not give 14
				// see Bitmap Font binary format documentation.
				//const UInt uLength = _uBlockSise - sizeof(BlockInfo) + sizeof(string);
				const UInt uLength = _uBlockSise - 14;
				CharPtr pBuffer = new char[uLength + 1];
				pBuffer[uLength] = '\0';

				_pFile->Read(pBuffer, uLength);
				m_oBlockInfo.m_strFontName = pBuffer;

				delete[] pBuffer;
			}

			return bResult;
		}

		bool DisplayFont::ReadBlockCommon(FilePtr _pFile, UInt& _uBlockSise)
		{
			bool bResult = true;

			_pFile->Read(&m_oBlockCommon.m_uLineHeight, sizeof(Word));	
			_pFile->Read(&m_oBlockCommon.m_uBase, sizeof(Word));
			_pFile->Read(&m_oBlockCommon.m_uScaleW, sizeof(Word));
			_pFile->Read(&m_oBlockCommon.m_uScaleH, sizeof(Word));
			_pFile->Read(&m_oBlockCommon.m_uPages, sizeof(Word));
			_pFile->Read(&m_oBlockCommon.m_uBitField, sizeof(Byte));
			_pFile->Read(&m_oBlockCommon.m_uAlphaChannel, sizeof(Byte));
			_pFile->Read(&m_oBlockCommon.m_uRedChannel, sizeof(Byte));
			_pFile->Read(&m_oBlockCommon.m_uGreenChannel, sizeof(Byte));
			_pFile->Read(&m_oBlockCommon.m_uBlueChannel, sizeof(Byte));

			return bResult;
		}

		bool DisplayFont::ReadBlockPages(FilePtr _pFile, UInt& _uBlockSise)
		{
			bool bResult = (0 == (_uBlockSise % m_oBlockCommon.m_uPages));

			if (false != bResult)
			{
				const UInt uLength = _uBlockSise / m_oBlockCommon.m_uPages;
				CharPtr pBuffer = new char[uLength + 1];
				pBuffer[uLength] = '\0';

				m_oBlockPages.m_vPageNames.resize(m_oBlockCommon.m_uPages);
				for (UInt i = 0 ; m_oBlockCommon.m_uPages > i ; ++i)
				{
					_pFile->Read(pBuffer, uLength);
					m_oBlockPages.m_vPageNames[i] = pBuffer;
				}

				delete[] pBuffer;
			}

			return bResult;
		}

		bool DisplayFont::ReadBlockChars(FilePtr _pFile, UInt& _uBlockSise)
		{
			// see Bitmap Font binary format documentation.
			const UInt uSubBlockSize = 20;
			bool bResult = (0 == (_uBlockSise % uSubBlockSize));

			if (false != bResult)
			{
				const UInt uCount = _uBlockSise / uSubBlockSize;
				for (UInt i = 0 ; uCount > i ; ++i)
				{
					UInt uID = 0;
					_pFile->Read(&uID, sizeof(UInt));
					bResult = (m_mBlockChars.end() == m_mBlockChars.find(uID));
					if (false == bResult)
					{
						break;
					}

					BlockCharRef rBlockChar = m_mBlockChars[uID];
					rBlockChar.m_uID = uID;
					_pFile->Read(&rBlockChar.m_uX, sizeof(Word));
					_pFile->Read(&rBlockChar.m_uY, sizeof(Word));
					_pFile->Read(&rBlockChar.m_uWidth, sizeof(Word));
					_pFile->Read(&rBlockChar.m_uHeight, sizeof(Word));
					_pFile->Read(&rBlockChar.m_sXOffset, sizeof(short));
					_pFile->Read(&rBlockChar.m_sYOffset, sizeof(short));
					_pFile->Read(&rBlockChar.m_sXAdvance, sizeof(short));
					_pFile->Read(&rBlockChar.m_uPage, sizeof(Byte));
					_pFile->Read(&rBlockChar.m_uChannel, sizeof(Byte));

					rBlockChar.m_fX = (float(rBlockChar.m_uX) + 0.5f) / float(m_oBlockCommon.m_uScaleW);
					rBlockChar.m_fY = (float(rBlockChar.m_uY) + 0.5f) / float(m_oBlockCommon.m_uScaleH);
					rBlockChar.m_fWidth = float(rBlockChar.m_uWidth) / float(m_oBlockCommon.m_uScaleW);
					rBlockChar.m_fHeight = float(rBlockChar.m_uHeight) / float(m_oBlockCommon.m_uScaleH);
				}
			}

			return bResult;
		}

		bool DisplayFont::ReadBlockKerning(FilePtr _pFile, UInt& _uBlockSise)
		{
			// see Bitmap Font binary format documentation.
			const UInt uSubBlockSize = 10;
			bool bResult = (0 == (_uBlockSise % uSubBlockSize));

			if (false != bResult)
			{
				const UInt uCount = _uBlockSise / uSubBlockSize;
				m_vBlockKernings.resize(uCount);
				for (UInt i = 0 ; uCount > i ; ++i)
				{
					BlockKerningRef rBlockKerning = m_vBlockKernings[i];
					_pFile->Read(&rBlockKerning.m_uFirst, sizeof(UInt));
					_pFile->Read(&rBlockKerning.m_uSecond, sizeof(UInt));
					_pFile->Read(&rBlockKerning.m_sAmount, sizeof(short));
				}
			}

			return bResult;
		}

		bool DisplayFont::LoadTextures(const string& _strPath)
		{
			DisplayTextureManagerPtr pTextureManager = Display::GetInstance()->GetTextureManager();
			string strFileName;
			bool bResult = true;

			for (UInt i = 0 ; m_oBlockCommon.m_uPages > i ; ++i)
			{
				strFileName = _strPath;
				strFileName += m_oBlockPages.m_vPageNames[i];
				bResult = pTextureManager->Load(m_oBlockPages.m_vPageNames[i], strFileName, DisplayTexture::EType_2D);
				if (false == bResult)
				{
					break;
				}

				const Key uKeyName = MakeKey(m_oBlockPages.m_vPageNames[i]);
				DisplayTexturePtr pTexture = pTextureManager->Get(uKeyName);
				bResult = (NULL != pTexture);
				if (false == bResult)
				{
					break;
				}

				const string strName = boost::str(boost::format("BITMAPFONTTEX%1%") % i);
				const Key uSemanticKeyName = MakeKey(strName);
				m_mTextures[uSemanticKeyName] = pTexture;

				pTextureManager->SetBySemantic(uSemanticKeyName, pTexture);
			}

			return bResult;
		}

		short DisplayFont::GetKerning(const wchar_t _wcFirst, const wchar_t _wcSecond)
		{
			BlockKerningVec::iterator iKerning = m_vBlockKernings.begin();
			BlockKerningVec::iterator iEnd = m_vBlockKernings.end();
			short sResult = 0;

			while (iEnd != iKerning)
			{
				BlockKerningRef rKerning = *iKerning;
				if ((_wcFirst == rKerning.m_uFirst) && (_wcSecond == rKerning.m_uSecond))
				{
					sResult = rKerning.m_sAmount;
					break;
				}
				++iKerning;
			}

			return sResult;
		}

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		DisplayFontLoader::DisplayFontLoader(DisplayFontManagerRef _rFontManager)
		:	ElixirEngine::DisplayFontLoader(_rFontManager),
			m_pVertexPool(NULL),
			m_pTextPool(NULL),
			m_uVertexDecl(NULL)
		{

		}

		DisplayFontLoader::~DisplayFontLoader()
		{

		}

		bool DisplayFontLoader::Create(const boost::any& _rConfig)
		{
			CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
			bool bResult = (NULL != pInfo);

			if (false != bResult)
			{
				m_pVertexPool = new VertexPool();
				const UInt uCharCount = pInfo->m_uVertexCount;
				bResult = m_pVertexPool->Create(boost::any(uCharCount));
			}

			if (false != bResult)
			{
				m_pTextPool = new TextPool();
				const UInt uTextCount = pInfo->m_uTextCount;
				bResult = m_pTextPool->Create(boost::any(uTextCount));
			}

			if (false != bResult)
			{
				m_uVertexDecl = Display::GetInstance()->CreateVertexDeclaration(VertexFont::s_VertexElement);
				bResult = (0 != m_uVertexDecl);
			}

			DisplayPtr pDisplay = Display::GetInstance();
			pDisplay->GetMaterialManager()->RegisterParamCreator(MakeKey(string("BITMAPFONTTEX0")), boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1));
			pDisplay->GetMaterialManager()->RegisterParamCreator(MakeKey(string("BITMAPFONTTEX1")), boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1));
			pDisplay->GetMaterialManager()->RegisterParamCreator(MakeKey(string("BITMAPFONTTEX2")), boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1));
			pDisplay->GetMaterialManager()->RegisterParamCreator(MakeKey(string("BITMAPFONTTEX3")), boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1));
			pDisplay->GetMaterialManager()->RegisterParamCreator(MakeKey(string("BITMAPFONTDIFFUSE")), boost::bind(&DisplayEffectParamVECTOR4::CreateParam, _1));

			return bResult;
		}

		void DisplayFontLoader::Release()
		{
			DisplayPtr pDisplay = Display::GetInstance();
			pDisplay->GetMaterialManager()->UnregisterParamCreator(MakeKey(string("BITMAPFONTTEX0")));
			pDisplay->GetMaterialManager()->UnregisterParamCreator(MakeKey(string("BITMAPFONTTEX1")));
			pDisplay->GetMaterialManager()->UnregisterParamCreator(MakeKey(string("BITMAPFONTTEX2")));
			pDisplay->GetMaterialManager()->UnregisterParamCreator(MakeKey(string("BITMAPFONTTEX3")));
			pDisplay->GetMaterialManager()->UnregisterParamCreator(MakeKey(string("BITMAPFONTDIFFUSE")));

			if (0 != m_uVertexDecl)
			{
				pDisplay->ReleaseVertexDeclaration(m_uVertexDecl);
				m_uVertexDecl = 0;
			}

			if (NULL != m_pVertexPool)
			{
				m_pVertexPool->Release();
				delete m_pVertexPool;
				m_pVertexPool = NULL;
			}
		}

		ElixirEngine::DisplayFontPtr DisplayFontLoader::Load(const string& _strFileName)
		{
			ElixirEngine::DisplayFontPtr pResult = new DisplayFont(m_rFontManager, *this);
			const bool bResult = pResult->Create(boost::any(_strFileName));
			if (false == bResult)
			{
				Unload(pResult);
				pResult = NULL;
			}
			return pResult;
		}

		void DisplayFontLoader::Unload(ElixirEngine::DisplayFontPtr _pFont)
		{
			_pFont->Release();
			delete _pFont;
		}

		VertexPoolRef DisplayFontLoader::GetVertexPool()
		{
			return *m_pVertexPool;
		}

		TextPoolRef DisplayFontLoader::GetTextPool()
		{
			return *m_pTextPool;
		}

		Key DisplayFontLoader::GetVertexDecl()
		{
			return m_uVertexDecl;
		}
	}
}
