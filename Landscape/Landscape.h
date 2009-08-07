#ifndef __LANDSCAPE_H__
#define __LANDSCAPE_H__

#include "../Core/Core.h"
#include "../Display/Display.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Landscape;
	typedef Landscape* LandscapePtr;
	typedef Landscape& LandscapeRef;
	typedef vector<LandscapePtr> LandscapePtrVec;
	typedef map<Key, LandscapePtr> LandscapePtrMap;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class LandscapeChunk : public CoreObject
	{
	public:
		LandscapeChunk(LandscapeRef _rLandscape, DisplayRef _rDisplay);
		virtual ~LandscapeChunk();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();
		virtual void Render();

	protected:
		DisplayRef		m_rDisplay;
		LandscapeRef	m_rLandscape;
		unsigned int	m_uStartVertexIndex;

	private:
	};
	typedef LandscapeChunk*				LandscapeChunkPtr;
	typedef vector<LandscapeChunkPtr>	LandscapeChunkPtrVec;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Landscape : public DisplayObject
	{
	public:
		enum EVertexFormat
		{
			EFormat_UNKNOWN,
			EFormat_DEFAULT,
			EFormat_LIQUID,
		};

		struct OpenInfo
		{
			string			m_strName;
			unsigned int	m_uQuadSize;
			unsigned int	m_uGridWidth;
			unsigned int	m_uGridDepth;
			EVertexFormat	m_eFormat;
			string			m_strHeightmap;
		};

		struct VertexDefault
		{
			static VertexElement s_VertexElement[4];

			Vector3	m_oPosition;
			Vector3	m_oNormal;
			Vector4	m_oColor;
		};
		typedef VertexDefault* VertexDefaultPtr;

		struct VertexLiquid
		{
			static VertexElement s_VertexElement[6];

			Vector3	m_oPosition;
			Vector3	m_oNormal;
			Vector3	m_oBiNormal;
			Vector3	m_oTangent;
			Vector2	m_oUV;
		};
		typedef VertexLiquid* VertexLiquidPtr;

		struct GlobalInfo
		{
			GlobalInfo();

			bool Create(const OpenInfo& _rOpenInfo);
			void Release();

			string			m_strName;
			unsigned int	m_uQuadSize;
			unsigned int	m_uGridWidth;
			unsigned int	m_uGridDepth;
			unsigned int	m_uChunkCount;
			unsigned int	m_uVertexCount;
			unsigned int	m_uVertexPerRawCount;
			unsigned int	m_uRawCount;
			unsigned int	m_uStripSize;
		};

	public:
		Landscape(DisplayRef _rDisplay);
		virtual ~Landscape();

		static EVertexFormat StringToVertexFormat(const string& _strFormat);

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();
		virtual void Render();

		bool Open(const OpenInfo& _rOpenInfo);
		void Close();

		const GlobalInfo& GetGlobalInfo() const;

	protected:
		bool CreateVertexBufferDefault();
		bool CreateVertexBufferLiquid();

	protected:
		GlobalInfo				m_oGlobalInfo;
		LandscapeChunkPtrVec	m_vGrid;
		DisplayVertexBufferPtr	m_pVertexBuffer;
		DisplayIndexBufferPtr	m_pIndexBuffer;
		VoidPtr					m_pVertexes;
		UIntPtr					m_pIndexes;
		EVertexFormat			m_eFormat;

	private:
	};
}

#endif // __LANDSCAPE_H__
