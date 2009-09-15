#ifndef __LANDSCAPE_H__
#define __LANDSCAPE_H__

#include "../Core/Core.h"
#include "../Display/Display.h"
#include "../Display/Surface.h"
#include "../Landscape/LandscapeTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	#define LANDSCAPE_USE_HIGHEST_LOD_ONLY	1
	#define LANDSCAPE_USE_MORPHING			0

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
#if LANDSCAPE_USE_MORPHING
		static VertexElement s_VertexElement[6];
#else // LANDSCAPE_USE_MORPHING
		static VertexElement s_VertexElement[5];
#endif // LANDSCAPE_USE_MORPHING

		VertexDefaultRef operator = (VertexIndependentRef _rVertexIndependent);

		Vector3	m_oPosition;
#if LANDSCAPE_USE_MORPHING
		Vector3	m_oPosition2;
#endif // LANDSCAPE_USE_MORPHING
		Vector3	m_oNormal;
		Vector4	m_oColor;
		Vector2	m_oUV;
	};

	struct VertexLiquid
	{
		static VertexElement s_VertexElement[6];

		VertexLiquidRef operator = (VertexIndependentRef _rVertexIndependent);

		Vector3	m_oPosition;
		Vector3	m_oNormal;
		Vector3	m_oBiNormal;
		Vector3	m_oTangent;
		Vector2	m_oUV;
	};

	struct VertexIndependent
	{
		struct LODVertexLink
		{
			unsigned int	m_uLOD;
			unsigned int	m_uIndex;
		};
		typedef vector<LODVertexLink> LODVertexLinkVec;

		void AddLink(const unsigned int& _uLOD, const unsigned int& _uIndex);

		Vector3				m_oPosition;
#if LANDSCAPE_USE_MORPHING
		Vector3				m_oPosition2;
#endif // LANDSCAPE_USE_MORPHING
		Vector3				m_oNormal;
		Vector2				m_oUV;
		LODVertexLinkVec	m_vLinks;
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
			unsigned int			m_uStartIndex;
			unsigned int			m_uStripSize;
			unsigned int			m_uGridSize;
			unsigned int			m_uQuadSize;
			unsigned int			m_uGeometricError;
			unsigned int			m_uVertexPerRawCount;
			unsigned int			m_uRawCount;
			unsigned int			m_uVertexCount;
			unsigned int			m_uNumVertices;
			unsigned int			m_uIncrement;
			DisplayVertexBufferPtr	m_pVertexBuffer;
			VoidPtr					m_pVertexes;
			VertexIndependentPtr	m_pVertexesIndependent;
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
		void GetVertexPosition(const LODInfo& _rLODInfo, const unsigned int& _uIndexBufferIndex, const unsigned int& _uVertexStartIndex, Vector3& _rPosition);
		bool SetIndices();
		bool UseLODVertexBuffer(const unsigned int& _uLOD);

		unsigned int m_uOutOfFrustum;

	protected:
		bool CreateVertexBufferIndependent();
		void ComputeVertexIndependentMorphs(LODInfoRef _rLODInfo);
		void ComputeVertexIndependentNormals(LODInfoRef _rLODInfo);
		bool CreateVertexBufferDefault();
		bool CreateVertexBufferLiquid();
		bool CreateIndexBuffer();
		bool CreateChunks();
		bool LoadHeightmap(const string& _strFileName);

		void InterpolatePixelA8R8G8B8(const DisplaySurface::UVInfo& _rUVInfo, ByteRef _uRed, ByteRef _uGreen, ByteRef _uBlue, ByteRef _uAlpha);

	protected:
		GlobalInfo					m_oGlobalInfo;
		LandscapeChunkPtrVec		m_vGrid;
		LandscapeChunkPtrVec		m_vRenderList;
		DisplayVertexBufferPtrVec	m_vVertexBuffers;
		VoidPtrVec					m_vVertexes;
		DisplayVertexBufferPtr		m_pCurrentVertexBuffer;
		DisplayIndexBufferPtr		m_pIndexBuffer;
		UIntPtr						m_pIndexes;
		VertexIndependentPtrVec		m_vVertexesIndependent;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class LandscapeChunk : public DisplayObject
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

		virtual void RenderBegin();
		virtual void Render();

		void Traverse(LandscapeChunkPtrVecRef _rRenderList, const Vector3& _rCamPos, const float& _fPixelSize);
		unsigned int GetLODID() const;

	protected:
		LandscapeRef			m_rLandscape;
		unsigned int			m_uStartVertexIndex;
		unsigned int			m_uLOD;
		LandscapeChunkPtr		m_pParent;
		LandscapeChunkPtr		m_pChildren[ESubChild_COUNT];
		Vector3					m_oCenter;
		Vector3					m_oExtends;
		Landscape::LODInfoPtr	m_pLODInfo;
		float					m_fMorphFactor;

	private:
	};
}

#endif // __LANDSCAPE_H__
