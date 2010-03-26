#ifndef __FONTBMF_H__
#define __FONTBMF_H__

#include "../Display/Font.h"
#include "../Core/File.h"
#include "../Core/Pool.h"

namespace ElixirEngine
{
	namespace BitmapFont
	{
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		class DisplayFont;
		typedef DisplayFont* DisplayFontPtr;
		typedef DisplayFont& DisplayFontRef;

		class DisplayFontText;
		typedef DisplayFontText* DisplayFontTextPtr;
		typedef DisplayFontText& DisplayFontTextRef;

		class DisplayFontLoader;
		typedef DisplayFontLoader* DisplayFontLoaderPtr;
		typedef DisplayFontLoader& DisplayFontLoaderRef;

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		// see Bitmap Font binary format documentation.

		struct BlockInfo
		{
			enum EBitField
			{
				EBitField_SMOOTH		= 0x01 << 0,
				EBitField_UNICODE		= 0x01 << 1,
				EBitField_ITALIC		= 0x01 << 2,
				EBitField_BOLD			= 0x01 << 3,
				EBitField_FIXEDHEIGHT	= 0x01 << 4,
				EBitField_RESERVED5		= 0x01 << 5,
				EBitField_RESERVED6		= 0x01 << 6,
				EBitField_RESERVED7		= 0x01 << 7,
			};

			short	m_sFontSize;
			Byte	m_uBitField;
			Byte	m_uCharSet;
			Word	m_uStretchH;
			Byte	m_uAA;
			Byte	m_uPaddingUp;
			Byte	m_uPaddingRight;
			Byte	m_uPaddingDown;
			Byte	m_uPaddingLeft;
			Byte	m_uSpacingHoriz;
			Byte	m_uSpacingVert;
			Byte	m_uOutline;
			string	m_strFontName;
		};

		struct BlockCommon
		{
			enum EBitField
			{
				EBitField_RESERVED0		= 0x01 << 0,
				EBitField_RESERVED1		= 0x01 << 1,
				EBitField_RESERVED2		= 0x01 << 2,
				EBitField_RESERVED3		= 0x01 << 3,
				EBitField_RESERVED4		= 0x01 << 4,
				EBitField_RESERVED5		= 0x01 << 5,
				EBitField_RESERVED6		= 0x01 << 6,
				EBitField_PACKED		= 0x01 << 7,
			};

			Word	m_uLineHeight;
			Word	m_uBase;
			Word	m_uScaleW;
			Word	m_uScaleH;
			Word	m_uPages;
			Byte	m_uBitField;
			Byte	m_uAlphaChannel;
			Byte	m_uRedChannel;
			Byte	m_uGreenChannel;
			Byte	m_uBlueChannel;
		};

		struct BlockPages
		{
			StringVec	m_vPageNames;
		};

		struct BlockChar
		{
			UInt	m_uID;
			Word	m_uX;
			Word	m_uY;
			Word	m_uWidth;
			Word	m_uHeight;
			short	m_sXOffset;
			short	m_sYOffset;
			short	m_sXAdvance;
			Byte	m_uPage;
			Byte	m_uChannel;
			// Followings are not part of BMF binary format.
			// They have been added to ease vertex initialization.
			float	m_fX;
			float	m_fY;
			float	m_fWidth;
			float	m_fHeight;
		};
		typedef BlockChar* BlockCharPtr;
		typedef BlockChar& BlockCharRef;
		typedef vector<BlockChar> BlockCharVec;
		typedef map<UInt, BlockChar> BlockCharMap;

		struct BlockKerning
		{
			UInt	m_uFirst;
			UInt	m_uSecond;
			short	m_sAmount;
		};
		typedef BlockKerning* BlockKerningPtr;
		typedef BlockKerning& BlockKerningRef;
		typedef vector<BlockKerning> BlockKerningVec;

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		struct VertexFont
		{
			Vector3	m_f3Pos;	// xyz
			Vector3	m_f2UV;		// u, v, texture index
			static VertexElement s_VertexElement[3];
		};
		typedef VertexFont* VertexFontPtr;
		typedef VertexFont& VertexFontRef;

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		typedef Pool<VertexFont> VertexFontPool;
		typedef VertexFontPool* VertexFontPoolPtr;
		typedef VertexFontPool& VertexFontPoolRef;

		template<typename T>
		class FontObjectBuffer : public CoreObject
		{
		public:
			typedef T* TPtr;
			typedef T& TRef;
			typedef Pool<T> TPool;
			typedef TPool* TPoolPtr;
			typedef TPool& TPoolRef;

		public:
			FontObjectBuffer()
			:	CoreObject(),
				m_pPool(NULL),
				m_uMax(0)
			{

			}

			virtual ~FontObjectBuffer()
			{

			}

			virtual bool Create(const boost::any& _rConfig)
			{
				return Reserve(boost::any_cast<const UInt>(_rConfig));
			}

			virtual void Update()
			{

			}

			virtual void Release()
			{
				if (NULL != m_pPool)
				{
					delete m_pPool;
					m_pPool = NULL;
					m_uMax = 0;
				}
			}

			TPtr Alloc(const UInt _uSize = 1)
			{
				return m_pPool->Alloc(_uSize);
			}

			void Free(TPtr _pT)
			{
				m_pPool->Free(_pT);
			}

		protected:
			bool Reserve(UInt _uSize)
			{
				if ((NULL != m_pPool) && (m_uMax != _uSize))
				{
					delete m_pPool;
					m_pPool = NULL;
				}

				m_uMax = _uSize;
				if (0 < m_uMax)
				{
					m_pPool = new TPool(m_uMax);
				}

				return ((0 < m_uMax) && (m_uMax == m_pPool->Capacity()));
			}

		protected:
			TPoolPtr	m_pPool;
			UInt		m_uMax;
		};

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		typedef FontObjectBuffer<VertexFont> VertexPool;
		typedef VertexPool* VertexPoolPtr;
		typedef VertexPool& VertexPoolRef;

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		class DisplayFontText : public ElixirEngine::DisplayFontText
		{
		public:
			struct CreateInfo
			{
				DisplayPtr		m_pDisplay;
				DisplayFontPtr	m_pFont;
			};
			typedef CreateInfo* CreateInfoPtr;
			typedef CreateInfo& CreateInfoRef;

		public:
			DisplayFontText();
			virtual ~DisplayFontText();

			virtual bool Create(const boost::any& _rConfig);
			virtual void Release();

			virtual void RenderBegin();
			virtual void Render();
			virtual void RenderEnd();
			virtual bool RenderBeginRecord();
			virtual bool RenderRecord();
			virtual bool RenderEndRecord();

			virtual void SetText(const wstring& _wstrText);
			virtual void SetColor(const Vector4& _f4Color);

		protected:
			void BuildText();

		protected:
			wstring							m_wstrText;
			Vector4							m_f4Color;
			DisplayFontPtr					m_pFont;
			VertexFontPtr					m_pVertex;
			ElixirEngine::VertexBufferPtr	m_pPreviousVertexBuffer;
			Key								m_uPreviousVertexDecl;
			unsigned int					m_uPreviousVBOffset;
			unsigned int					m_uPreviousVBStride;
			UInt							m_uVertexCount;
			bool							m_bTextChanged;
			bool							m_bSizeChanged;
			bool							m_bRebuildText;
		};
		typedef FontObjectBuffer<DisplayFontText> TextPool;
		typedef TextPool* TextPoolPtr;
		typedef TextPool& TextPoolRef;

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		class DisplayFont : public ElixirEngine::DisplayFont
		{
			friend class DisplayFontText;

		public:
			DisplayFont(DisplayFontManagerRef _rFontManager, DisplayFontLoaderRef _rFontLoader);
			virtual ~DisplayFont();

			virtual bool Create(const boost::any& _rConfig);
			virtual void Update();
			virtual void Release();

			virtual ElixirEngine::DisplayFontTextPtr CreateText();
			virtual void ReleaseText(ElixirEngine::DisplayFontTextPtr _pText);
			virtual DisplayRef GetDisplay();

			DisplayFontLoaderRef GetLoader();

		protected:
			bool CheckHeader(FilePtr _pFile, Byte& _uVersion);
			bool ReadBlockInfo(FilePtr _pFile, UInt& _uBlockSise);
			bool ReadBlockCommon(FilePtr _pFile, UInt& _uBlockSise);
			bool ReadBlockPages(FilePtr _pFile, UInt& _uBlockSise);
			bool ReadBlockChars(FilePtr _pFile, UInt& _uBlockSise);
			bool ReadBlockKerning(FilePtr _pFile, UInt& _uBlockSise);

			bool LoadTextures(const string& _strPath);
			short GetKerning(const wchar_t _wcFirst, const wchar_t _wcSecond);

		protected:
			BlockInfo				m_oBlockInfo;
			BlockCommon				m_oBlockCommon;
			BlockPages				m_oBlockPages;
			BlockCharMap			m_mBlockChars;
			BlockKerningVec			m_vBlockKernings;
			DisplayTexturePtrMap	m_mTextures;
			DisplayFontLoaderRef	m_rFontLoader;
		};

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		class DisplayFontLoader : public ElixirEngine::DisplayFontLoader
		{
		public:
			struct CreateInfo
			{
				UInt	m_uVertexCount;
				UInt	m_uTextCount;
			};
			typedef CreateInfo* CreateInfoPtr;
			typedef CreateInfo& CreateInfoRef;

		public:
			DisplayFontLoader(DisplayFontManagerRef _rFontManager);
			virtual ~DisplayFontLoader();

			virtual bool Create(const boost::any& _rConfig);
			virtual void Release();

			virtual ElixirEngine::DisplayFontPtr Load(const string& _strFileName);
			virtual void Unload(ElixirEngine::DisplayFontPtr _pFont);

			VertexPoolRef GetVertexPool();
			TextPoolRef GetTextPool();
			Key GetVertexDecl();

		protected:
			VertexPoolPtr	m_pVertexPool;
			TextPoolPtr		m_pTextPool;
			Key				m_uVertexDecl;
		};
	}
}

#endif
