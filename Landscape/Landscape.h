#ifndef __LANDSCAPE_H__
#define __LANDSCAPE_H__

#include "../Core/Core.h"
#include "../Core/Octree.h"
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

	struct LandscapeVertexDefault
	{
#if LANDSCAPE_USE_MORPHING
		static VertexElement s_VertexElement[7];
#else // LANDSCAPE_USE_MORPHING
		static VertexElement s_VertexElement[6];
#endif // LANDSCAPE_USE_MORPHING

		LandscapeVertexDefaultRef operator = (LandscapeVertexIndependentRef _rVertexIndependent);

		Vector3	m_f3Position;
#if LANDSCAPE_USE_MORPHING
		Vector3	m_f3Position2;
#endif // LANDSCAPE_USE_MORPHING
		Vector3	m_f3Normal;
		Vector4	m_f4Color;
		Vector2	m_f2UV;
		Vector3	m_f3UV2; // slope, height, water level
	};

	struct LandscapeVertexLiquid
	{
		static VertexElement s_VertexElement[6];

		LandscapeVertexLiquidRef operator = (LandscapeVertexIndependentRef _rVertexIndependent);

		Vector3	m_f3Position;
		Vector3	m_f3Normal;
		Vector3	m_f3BiNormal;
		Vector3	m_f3Tangent;
		Vector2	m_f2UV;
	};

	struct LandscapeVertexIndependent
	{
		struct LODVertexLink
		{
			UInt	m_uLOD;
			UInt	m_uIndex;
		};
		typedef vector<LODVertexLink> LODVertexLinkVec;

		void AddLink(const UInt& _uLOD, const UInt& _uIndex);

		LODVertexLinkVec	m_vLinks;
		Vector3				m_f3Position;
#if LANDSCAPE_USE_MORPHING
		Vector3				m_f3Position2;
#endif // LANDSCAPE_USE_MORPHING
		Vector3				m_f3Normal;
		Vector2				m_f2UV;
		float				m_fNormalizedSlope;
		float				m_fNormalizedHeight;
		UInt				m_uWaterLevel;
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
			OctreePtr				m_pOctree;
			UInt					m_uQuadSize;
			UInt					m_uGridSize;
			ELandscapeVertexFormat	m_eFormat;
			string					m_strHeightmap;
			string					m_strLayersConfig;
			float					m_fPixelErrorMax;
			float					m_fFloorScale;
			float					m_fHeightScale;
			Key						m_uRenderStageKey;
		};

		struct LODInfo
		{
			UInt							m_uStartIndex;
			UInt							m_uStripSize;
			UInt							m_uGridSize;
			UInt							m_uQuadSize;
			UInt							m_uGeometricError;
			UInt							m_uVertexPerRowCount;
			UInt							m_uRowCount;
			UInt							m_uVertexCount;
			UInt							m_uNumVertices;
			UInt							m_uIncrement;
			Key								m_uVertexBuffer;
			VoidPtr							m_pVertexes;
			LandscapeVertexIndependentPtr	m_pVertexesIndependent;
		};
		typedef LODInfo* LODInfoPtr;
		typedef LODInfo& LODInfoRef;

		struct GlobalInfo
		{
			GlobalInfo();

			void Reset();
			bool Create(const OpenInfo& _rOpenInfo);
			void Release();

			string					m_strName;
			LODInfoPtr				m_pLODs;
			OctreePtr				m_pOctree;
			Key						m_uRenderStageKey;
			UInt					m_uQuadSize;
			UInt					m_uGridSize;
			UInt					m_uChunkCount;
			UInt					m_uVertexCount;
			UInt					m_uVertexPerRowCount;
			UInt					m_uRowCount;
			UInt					m_uStripSize;
			UInt					m_uLODCount;
			UInt					m_uTotalLODStripSize;
			float					m_fPixelErrorMax;
			float					m_fFloorScale;
			float					m_fHeightScale;
			float					m_fMinHeight;
			float					m_fMaxHeight;
			ELandscapeVertexFormat	m_eFormat;
		};

	public:
		Landscape(DisplayRef _rDisplay);
		virtual ~Landscape();

		static ELandscapeVertexFormat StringToVertexFormat(const string& _strFormat);

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();
		virtual void SetWorldMatrix(MatrixRef _rWorld);
		virtual void SetMaterial(DisplayMaterialPtr _pMaterial);
		virtual void SetRenderStage(const Key& _uRenderPass);
		virtual void Render();

		bool Open(const OpenInfo& _rOpenInfo);
		void Close();

		const GlobalInfo& GetGlobalInfo() const;
		void GetVertexPosition(LODInfoRef _rLODInfo, const UInt& _uIndexBufferIndex, const UInt& _uVertexStartIndex, Vector3& _rPosition);
		bool SetIndices();
		bool UseLODVertexBuffer(const UInt& _uLOD);
		void UseLayering();
		const UIntPtr GetIndices() const;
		void UpdateObjectLocation(DisplayObjectPtr _pObject);
		bool GetWaterIndex(const Vector3& _f3Pos, UIntRef _uLevel);

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
		GlobalInfo							m_oGlobalInfo;
		LandscapeChunkPtrVec				m_vGrid;
		LandscapeChunkPtrVec				m_vRenderList;
		KeyVec								m_vVertexBuffers;
		VoidPtrVec							m_vVertexes;
		LandscapeVertexIndependentPtrVec	m_vVertexesIndependent;
		Key									m_uCurrentVertexBuffer;
		Key									m_uIndexBuffer;
		UIntPtr								m_pIndexes;
		LandscapeLayeringPtr				m_pLayering;
		DisplayPtr							m_pDisplay;
		Key									m_uRenderStageKey;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class LandscapeChunk : public DisplayObject, public OctreeObject
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
			UInt	m_uX;
			UInt	m_uZ;
		};

	public:
		LandscapeChunk(LandscapeRef _rLandscape, OctreeRef _rOctree, const UInt& _uLOD);
		virtual ~LandscapeChunk();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual void SetWorldMatrix(MatrixRef _rWorld);
		virtual void SetMaterial(DisplayMaterialPtr _pMaterial);
		virtual void SetRenderStage(const Key& _uRenderPass);
		virtual void RenderBegin();
		virtual void Render();
		virtual bool RayIntersect(const Vector3& _f3RayBegin, const Vector3& _f3RayEnd, Vector3& _f3Intersect);

		void Traverse(LandscapeChunkPtrVecRef _rRenderList, const Vector3& _rCamPos, const float& _fPixelSize);
		UInt GetLODID() const;
		void UpdateObjectLocation(DisplayObjectPtr _pObject);
		bool GetWaterIndex(const Vector3& _f3Pos, UIntRef _uLevel);

	protected:
		bool CreateBoundingMesh();

	protected:
		static Key				s_uMorphFactorKey;

		LandscapeRef			m_rLandscape;
		UInt					m_uStartVertexIndex;
		UInt					m_uLOD;
		LandscapeChunkPtr		m_pParent;
		LandscapeChunkPtr		m_pChildren[ESubChild_COUNT];
		Vector3					m_f3AABB[2]; // 0 = (left, bottom, near), 1 = (right, top, far)
		Vector3					m_f3Center;
		Vector3					m_f3Extends;
		Landscape::LODInfoPtr	m_pLODInfo;
		DisplayPtr				m_pDisplay;
		float					m_fMorphFactor;
		UInt					m_uIndexX;
		UInt					m_uIndexZ;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class LandscapeLayering : public CoreObject
	{
	public:
		struct CreateInfo
		{
			string	m_strPath;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

		struct Layer
		{
			bool Evaluate(const float& _fSlope, const float& _fHeight);

			UInt	m_uAtlasIndex;
			float	m_fMinHeight;
			float	m_fMaxHeight;
			float	m_fMinSlope;
			float	m_fMaxSlope;
		};
		typedef Layer*			LayerPtr;
		typedef Layer&			LayerRef;
		typedef vector<Layer>	LayerVec;

	public:
		LandscapeLayering(LandscapeLayerManagerRef _rLandscapeLayerManager);
		virtual ~LandscapeLayering();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		DisplayTexturePtr GetAtlas();
		DisplayTexturePtr GetSlopeAndHeightLUT();
		DisplayTexturePtr GetNoise();
		Vector4& GetShaderInfo();

	protected:
		bool CreateFromLuaConfig(CreateInfoRef _rInfo);
		bool CreateAtlas(LuaObjectRef _rLuaObject);
		bool CreateSlopeAndHeightLUT(LuaObjectRef _rLuaObject);
		bool CreateNoise(LuaObjectRef _rLuaObject);

	protected:
		LayerVec					m_vLayers;
		LandscapeLayerManagerRef	m_rLandscapeLayerManager;
		DisplayTexturePtr			m_pAtlas;
		DisplayTexturePtr			m_pSlopeAndHeightLUT;
		DisplayTexturePtr			m_pNoise;
		Vector4						m_oShaderInfo;
		string						m_strAtlasName;
		string						m_strSAHLUTName;
		string						m_strNoiseName;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class LandscapeLayerManager : public CoreObject
	{
	public:
		LandscapeLayerManager(DisplayRef _rDisplay);
		virtual ~LandscapeLayerManager();

		static void SetInstance(LandscapeLayerManagerPtr _pInstance);
		static LandscapeLayerManagerPtr GetInstance();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		void SetCurrentLayering(LandscapeLayeringPtr _pLayering);
		LandscapeLayeringPtr Get(const string& _strFileName);
		void UnloadAll();

		DisplayRef GetDisplay();

	protected:
		LandscapeLayeringPtr Load(const string& _strFileName);

	protected:
		static LandscapeLayerManagerPtr	s_pInstance;

		LandscapeLayeringPtrMap	m_mConfigs;
		DisplayRef				m_rDisplay;
		LandscapeLayeringPtr	m_pCurrentLayering;
		Key						m_uAtlasDiffuseKey;
		Key						m_uAtlasLUTKey;
		Key						m_uNoiseKey;
		Key						m_uAtlasDiffuseInfoKey;

	private:
	};
}

#endif // __LANDSCAPE_H__
