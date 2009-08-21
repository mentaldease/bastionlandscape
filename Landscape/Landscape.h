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

	#define LANDSCAPE_CHUNK_VERTEXANDINDEXBUFFER	1

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	enum ELandscapeVertexFormat
	{
		ELandscapeVertexFormat_UNKNOWN,
		ELandscapeVertexFormat_DEFAULT,
		ELandscapeVertexFormat_LIQUID,
	};

	struct VertexDefault
	{
		static VertexElement s_VertexElement[4];

		Vector3	m_oPosition;
		Vector3	m_oNormal;
		Vector4	m_oColor;
	};

	struct VertexLiquid
	{
		static VertexElement s_VertexElement[6];

		Vector3	m_oPosition;
		Vector3	m_oNormal;
		Vector3	m_oBiNormal;
		Vector3	m_oTangent;
		Vector2	m_oUV;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Landscape : public DisplayObject
	{
	public:
		struct OpenInfo
		{
			string					m_strName;
			unsigned int			m_uQuadSize;
			unsigned int			m_uGridSize;
			ELandscapeVertexFormat	m_eFormat;
			string					m_strHeightmap;
			float					m_fPixelErrorMax;
			float					m_fFloorScale;
			float					m_fHeightScale;
		};

		struct LODInfo
		{
			unsigned int	m_uStartIndex;
			unsigned int	m_uStripSize;
			unsigned int	m_uLODGridSize;
			unsigned int	m_uLODQuadSize;
			unsigned int	m_uGeometricError;
		};
		typedef LODInfo* LODInfoPtr;
		typedef LODInfo& LODInfoRef;

		struct GlobalInfo
		{
			GlobalInfo();

			bool Create(const OpenInfo& _rOpenInfo);
			void Release();
			bool IsPowerOf2(const unsigned int& _uValue, unsigned int* _pPowerLevel = NULL);

			string					m_strName;
			unsigned int			m_uQuadSize;
			unsigned int			m_uGridSize;
			unsigned int			m_uChunkCount;
			unsigned int			m_uVertexCount;
			unsigned int			m_uVertexPerRawCount;
			unsigned int			m_uRawCount;
			unsigned int			m_uStripSize;
			unsigned int			m_uLODCount;
			unsigned int			m_uTotalLODStripSize;
			LODInfoPtr				m_pLODs;
			float					m_fPixelErrorMax;
			float					m_fFloorScale;
			float					m_fHeightScale;
			ELandscapeVertexFormat	m_eFormat;
		};

	public:
		Landscape(DisplayRef _rDisplay);
		virtual ~Landscape();

		static ELandscapeVertexFormat StringToVertexFormat(const string& _strFormat);

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();
		virtual void Render();

		bool Open(const OpenInfo& _rOpenInfo);
		void Close();

		const GlobalInfo& GetGlobalInfo() const;
		void GetVertexPosition(const unsigned int& _uIndexBufferIndex, const unsigned int& _uVertexStartIndex, Vector3& _rPosition);

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
#if LANDSCAPE_CHUNK_VERTEXANDINDEXBUFFER
		UIntPtr					m_pIndexesShadow;
#endif // LANDSCAPE_CHUNK_VERTEXANDINDEXBUFFER

	private:
	};

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
#if LANDSCAPE_CHUNK_VERTEXANDINDEXBUFFER
			unsigned int	m_uLOD;
			VoidPtr			m_pVertexes;
#endif // LANDSCAPE_CHUNK_VERTEXANDINDEXBUFFER
		};

	public:
		LandscapeChunk(LandscapeRef _rLandscape, DisplayRef _rDisplay, const unsigned int& _uLOD);
		virtual ~LandscapeChunk();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();
		virtual void Render();

		void Traverse(LandscapeChunkPtrVecRef _rRenderList, const Vector3& _rCamPos, const float& _fPixelSize);

#if LANDSCAPE_CHUNK_VERTEXANDINDEXBUFFER
	protected:
		template<typename T>
		bool CreateVertexBuffer(const CreateInfo& _rInfo);
#endif // LANDSCAPE_CHUNK_VERTEXANDINDEXBUFFER

	protected:
		DisplayRef				m_rDisplay;
		LandscapeRef			m_rLandscape;
		unsigned int			m_uStartVertexIndex;
		unsigned int			m_uLOD;
		LandscapeChunkPtr		m_pParent;
		LandscapeChunkPtr		m_pChildren[ESubChild_COUNT];
		Vector3					m_oCenter;
		Vector3					m_oExtends;
		Landscape::LODInfoPtr	m_pLODInfo;
#if LANDSCAPE_CHUNK_VERTEXANDINDEXBUFFER
		DisplayVertexBufferPtr	m_pVertexBuffer;
		VoidPtr					m_pVertexes;
		unsigned int			m_uVertexCount;
#endif // LANDSCAPE_CHUNK_VERTEXANDINDEXBUFFER

	private:
	};
}

#endif // __LANDSCAPE_H__
