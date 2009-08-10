#ifndef __LANDSCAPE_H__
#define __LANDSCAPE_H__

#include "../Core/Core.h"
#include "../Display/Display.h"
#include "../Landscape/LandscapeTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class LandscapeChunk : public CoreObject
	{
	public:
		enum ESubChild
		{
			ESubChild_NORTHWEST,
			ESubChild_NORTHEAST,
			ESubChild_SOUTHWEST,
			ESubChild_SOUTHEAST,
			ESubChild_COUNT // last enum member
		};

		struct CreateInfo
		{
			unsigned int	m_uX;
			unsigned int	m_uZ;
		};

	public:
		LandscapeChunk(LandscapeRef _rLandscape, DisplayRef _rDisplay, const unsigned int& _uLOD);
		virtual ~LandscapeChunk();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();
		virtual void Render();

		void Traverse(LandscapeChunkPtrVecRef _rRenderList);

	protected:
		DisplayRef				m_rDisplay;
		LandscapeRef			m_rLandscape;
		unsigned int			m_uStartVertexIndex;
		unsigned int			m_uLOD;
		LandscapeChunkPtr		m_pParent;
		LandscapeChunkPtr		m_pChildren[ESubChild_COUNT];

	private:
	};

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
			unsigned int	m_uGridSize;
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

		struct LODInfo
		{
			unsigned int	m_uStartIndex;
			unsigned int	m_uStripSize;
			unsigned int	m_uLODGridSize;
			unsigned int	m_uLODQuadSize;
		};
		typedef LODInfo* LODInfoPtr;

		struct GlobalInfo
		{
			GlobalInfo();

			bool Create(const OpenInfo& _rOpenInfo);
			void Release();
			bool IsPowerOf2(const unsigned int& _uValue, unsigned int* _pPowerLevel = NULL);

			string			m_strName;
			unsigned int	m_uQuadSize;
			unsigned int	m_uGridSize;
			unsigned int	m_uChunkCount;
			unsigned int	m_uVertexCount;
			unsigned int	m_uVertexPerRawCount;
			unsigned int	m_uRawCount;
			unsigned int	m_uStripSize;
			unsigned int	m_uLODCount;
			unsigned int	m_uTotalLODStripSize;
			LODInfoPtr	m_pLODs;
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
		bool CreateIndexBuffer();
		bool CreateChunks();

	protected:
		GlobalInfo				m_oGlobalInfo;
		LandscapeChunkPtrVec	m_vGrid;
		LandscapeChunkPtrVec	m_vRenderList;
		DisplayVertexBufferPtr	m_pVertexBuffer;
		DisplayIndexBufferPtr	m_pIndexBuffer;
		VoidPtr					m_pVertexes;
		UIntPtr					m_pIndexes;
		EVertexFormat			m_eFormat;

	private:
	};
}

#endif // __LANDSCAPE_H__
