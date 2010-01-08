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

		VertexElement VertexFont::s_VertexElement[4] =
		{
			{ 0,	0,						D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
			{ 0,	1 * SV3,				D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,		0 },
			{ 0,	1 * SV3 + SV4,			D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
			D3DDECL_END()
		};

		#undef SV2
		#undef SV3
		#undef SV4

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
#if 1
#else
		VertexBufferPtr VertexBuffer::s_pInstance = NULL;

		VertexBuffer::VertexBuffer()
		:	CoreObject(),
			m_pPool(NULL),
			m_uMaxVertex(0)
		{
		}

		VertexBuffer::~VertexBuffer()
		{
			if (this == s_pInstance)
			{
				s_pInstance = NULL;
			}
		}

		VertexBufferPtr VertexBuffer::GetInstance()
		{
			return s_pInstance;
		}

		void VertexBuffer::SetInstance(VertexBufferPtr _pInstance)
		{
			s_pInstance = _pInstance;
		}

		bool VertexBuffer::Create(const boost::any& _rConfig)
		{
			return Reserve(boost::any_cast<const UInt>(_rConfig));
		}

		void VertexBuffer::Update()
		{

		}

		void VertexBuffer::Release()
		{
			if (NULL != m_pPool)
			{
				delete m_pPool;
				m_pPool = NULL;
			}
			m_uMaxVertex = 0;
		}

		VertexFontPtr VertexBuffer::Alloc(UIntRef _uSize)
		{
			return m_pPool->Alloc(_uSize);
		}

		void VertexBuffer::Free(VertexFontPtr _pFont)
		{
			m_pPool->Free(_pFont);
		}

		bool VertexBuffer::Reserve(UInt _uSize)
		{
			if ((NULL != m_pPool) && (m_uMaxVertex != _uSize))
			{
				delete m_pPool;
				m_pPool = NULL;
			}

			m_uMaxVertex = _uSize;
			if (0 < m_uMaxVertex)
			{
				m_pPool = new VertexFontPool(m_uMaxVertex);
			}

			return ((0 < m_uMaxVertex) && (m_uMaxVertex == m_pPool->Size()));
		}
#endif

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		DisplayFontText::DisplayFontText()
		:	ElixirEngine::DisplayFontText(),
			m_wstrText(),
			m_pFont(NULL),
			m_pVertex(NULL),
			m_uVertexCount(0),
			m_bTextChanged(false),
			m_bSizeChanged(false)
		{

		}

		DisplayFontText::~DisplayFontText()
		{

		}

		bool DisplayFontText::Create(const boost::any& _rConfig)
		{
			m_pFont = boost::any_cast<DisplayFontPtr>(_rConfig);
			return (NULL != m_pFont);
		}

		void DisplayFontText::Release()
		{
			if (NULL != m_pVertex)
			{
				m_pFont->m_rFontLoader.GetVertexBuffer().Free(m_pVertex);
				m_pVertex = NULL;
			}
		}

		void DisplayFontText::Render()
		{
			if (false != m_bTextChanged)
			{
				BuildText();
				m_bTextChanged = false;
			}

			if (NULL != m_pVertex)
			{
				Display::GetInstance()->GetDevicePtr()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, m_uVertexCount - 2, m_pVertex, sizeof(VertexFont));
			}
		}

		void DisplayFontText::SetWorldMatrix(MatrixRef _rWorld)
		{
			m_oWorld = _rWorld;
		}

		void DisplayFontText::SetText(const wstring& _wstrText)
		{
			m_bSizeChanged = (m_wstrText.length() != _wstrText.length());
			m_bTextChanged = (m_wstrText != _wstrText);
			m_wstrText = _wstrText;
		}

		void DisplayFontText::BuildText()
		{
			if (NULL != m_pVertex)
			{
				m_pFont->m_rFontLoader.GetVertexBuffer().Free(m_pVertex);
			}

			// +4 vertex for each char
			// +2 vertex to link to next char
			// -2 except the last one
			const UInt uCharCount = UInt(m_wstrText.length());
			m_uVertexCount = uCharCount * (4 + 2) - 2;
			m_pVertex = m_pFont->m_rFontLoader.GetVertexBuffer().Alloc(m_uVertexCount);

			if (NULL != m_pVertex)
			{
				const Vector4 oWhite(1.0f, 1.0f, 1.0f, 1.0f);
				VertexFontPtr pVertex = m_pVertex;
				float fXOffset = 0.0f;
				float fYOffset = 0.0f;
				float fZOffset = 0.0f;

				for (UInt i = 0 ; uCharCount > i ; ++i)
				{
					wchar_t wcChar = m_wstrText[i];
					BlockChar& rChar = m_pFont->m_mBlockChars[wcChar];

					// top left
					VertexFontRef rVertex = m_pVertex[0];
					rVertex.m_oPos.x = fXOffset + 0.0f;
					rVertex.m_oPos.y = fYOffset + 0.0f;
					rVertex.m_oPos.z = fZOffset + 0.0f;
					rVertex.m_oUV = Vector3(rChar.m_fX, rChar.m_fY, float(rChar.m_uPage));
					rVertex.m_oColor = oWhite;
					// bottom left
					rVertex = m_pVertex[1];
					rVertex.m_oPos.x = fXOffset + 0.0f;
					rVertex.m_oPos.y = fYOffset + float(rChar.m_uYOffset);
					rVertex.m_oPos.z = fZOffset + 0.0f;
					rVertex.m_oUV = Vector3(rChar.m_fX, rChar.m_fY + rChar.m_fHeight, float(rChar.m_uPage));
					rVertex.m_oColor = oWhite;
					// top right
					rVertex = m_pVertex[2];
					rVertex.m_oPos.x = fXOffset + float(rChar.m_uXOffset);
					rVertex.m_oPos.y = fYOffset + 0.0f;
					rVertex.m_oPos.z = fZOffset + 0.0f;
					rVertex.m_oUV = Vector3(rChar.m_fX + rChar.m_fWidth, rChar.m_fY, float(rChar.m_uPage));
					rVertex.m_oColor = oWhite;
					// bottom right
					rVertex = m_pVertex[3];
					rVertex.m_oPos.x = fXOffset + float(rChar.m_uXOffset);
					rVertex.m_oPos.y = fYOffset + float(rChar.m_uYOffset);
					rVertex.m_oPos.z = fZOffset + 0.0f;
					rVertex.m_oUV = Vector3(rChar.m_fX + rChar.m_fWidth, rChar.m_fY + rChar.m_fHeight, float(rChar.m_uPage));
					rVertex.m_oColor = oWhite;

					fXOffset += float(rChar.m_uXAdvance);

					// link to next char
					if ((uCharCount - 1) > i)
					{
						m_pVertex[4] = m_pVertex[3];

						wcChar = m_wstrText[i + 1];
						rChar = m_pFont->m_mBlockChars[wcChar];
						rVertex = m_pVertex[5];
						rVertex.m_oPos.x = fXOffset + 0.0f;
						rVertex.m_oPos.y = fYOffset + 0.0f;
						rVertex.m_oPos.z = fZOffset + 0.0f;
						rVertex.m_oUV = Vector3(rChar.m_fX, rChar.m_fY, float(rChar.m_uPage));
						rVertex.m_oColor = oWhite;

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
		}

		DisplayFont::~DisplayFont()
		{
		}

		bool DisplayFont::Create(const boost::any& _rConfig)
		{
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
			ElixirEngine::DisplayFontTextPtr pResult = new DisplayFontText();
			if (false == pResult->Create(boost::any(this)))
			{
				ReleaseText(pResult);
				pResult = NULL;
			}
			return pResult;
		}

		void DisplayFont::ReleaseText(ElixirEngine::DisplayFontTextPtr _pText)
		{
			_pText->Release();
			delete _pText;
		}

		DisplayRef DisplayFont::GetDisplay()
		{
			return m_rFontManager.GetDisplay();
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
					BlockCharRef rBlockChar = m_mBlockChars[uID];
					rBlockChar.m_uID = uID;
					_pFile->Read(&rBlockChar.m_uX, sizeof(Word));
					_pFile->Read(&rBlockChar.m_uY, sizeof(Word));
					_pFile->Read(&rBlockChar.m_uWidth, sizeof(Word));
					_pFile->Read(&rBlockChar.m_uHeight, sizeof(Word));
					_pFile->Read(&rBlockChar.m_uXOffset, sizeof(Word));
					_pFile->Read(&rBlockChar.m_uYOffset, sizeof(Word));
					_pFile->Read(&rBlockChar.m_uXAdvance, sizeof(Word));
					_pFile->Read(&rBlockChar.m_uPage, sizeof(Byte));
					_pFile->Read(&rBlockChar.m_uChannel, sizeof(Byte));

					rBlockChar.m_fX = float(rBlockChar.m_uX) / float(m_oBlockCommon.m_uScaleW);
					rBlockChar.m_fY = float(rBlockChar.m_uY) / float(m_oBlockCommon.m_uScaleH);
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

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		DisplayFontLoader::DisplayFontLoader(DisplayFontManagerRef _rFontManager)
		:	ElixirEngine::DisplayFontLoader(_rFontManager),
			m_pVertexBuffer(NULL),
			m_pTextBuffer(NULL)
		{

		}

		DisplayFontLoader::~DisplayFontLoader()
		{

		}

		bool DisplayFontLoader::Create(const boost::any& _rConfig)
		{
			bool bResult = true;

			if (false != bResult)
			{
				m_pVertexBuffer = new VertexBuffer();
				const UInt uCharCount = 5000;
				bResult = m_pVertexBuffer->Create(boost::any(uCharCount));
			}

			if (false != bResult)
			{
				m_pTextBuffer = new TextBuffer();
				const UInt uTextCount = 100;
				bResult = m_pTextBuffer->Create(boost::any(uTextCount));
			}

			DisplayPtr pDisplay = Display::GetInstance();
			pDisplay->GetMaterialManager()->RegisterParamCreator(MakeKey(string("BITMAPFONTTEX0")), boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1));
			pDisplay->GetMaterialManager()->RegisterParamCreator(MakeKey(string("BITMAPFONTTEX1")), boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1));
			pDisplay->GetMaterialManager()->RegisterParamCreator(MakeKey(string("BITMAPFONTTEX2")), boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1));
			pDisplay->GetMaterialManager()->RegisterParamCreator(MakeKey(string("BITMAPFONTTEX3")), boost::bind(&DisplayEffectParamSEMANTICTEX::CreateParam, _1));

			return bResult;
		}

		void DisplayFontLoader::Release()
		{
			if (NULL != m_pVertexBuffer)
			{
				m_pVertexBuffer->Release();
				delete m_pVertexBuffer;
				m_pVertexBuffer = NULL;
			}

			DisplayPtr pDisplay = Display::GetInstance();
			pDisplay->GetMaterialManager()->UnregisterParamCreator(MakeKey(string("BITMAPFONTTEX0")));
			pDisplay->GetMaterialManager()->UnregisterParamCreator(MakeKey(string("BITMAPFONTTEX1")));
			pDisplay->GetMaterialManager()->UnregisterParamCreator(MakeKey(string("BITMAPFONTTEX2")));
			pDisplay->GetMaterialManager()->UnregisterParamCreator(MakeKey(string("BITMAPFONTTEX3")));
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

		VertexBufferRef DisplayFontLoader::GetVertexBuffer()
		{
			return *m_pVertexBuffer;
		}

		TextBufferRef DisplayFontLoader::GetTextBuffer()
		{
			return *m_pTextBuffer;
		}
	}
}
