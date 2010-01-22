#include "stdafx.h"
#include "Landscape.h"
#include "../Display/Camera.h"
#include "../Display/Surface.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	#define SV2	sizeof(Vector2)
	#define SV3	sizeof(Vector3)
	#define SV4	sizeof(Vector4)

#if LANDSCAPE_USE_MORPHING
	VertexElement VertexDefault::s_VertexElement[7] =
	{
		{ 0,	0,						D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
		{ 0,	1 * SV3,				D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	1 },
		{ 0,	2 * SV3,				D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
		{ 0,	3 * SV3,				D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,		0 },
		{ 0,	3 * SV3 + SV4,			D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
		{ 0,	3 * SV3 + SV4 + SV2,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	1 },
		D3DDECL_END()
	};
#else // LANDSCAPE_USE_MORPHING
	VertexElement VertexDefault::s_VertexElement[6] =
	{
		{ 0,	0,						D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
		{ 0,	1 * SV3,				D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
		{ 0,	2 * SV3,				D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,		0 },
		{ 0,	2 * SV3 + SV4,			D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
		{ 0,	2 * SV3 + SV4 + SV2,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	1 },
		D3DDECL_END()
	};
#endif // LANDSCAPE_USE_MORPHING

	VertexElement VertexLiquid::s_VertexElement[6] =
	{
		{ 0,	0,						D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
		{ 0,	1 * SV3,				D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
		{ 0,	2 * SV3,				D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL,	0 },
		{ 0,	3 * SV3,				D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,	0 },
		{ 0,	4 * SV3,				D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
		D3DDECL_END()
	};

	#undef SV2
	#undef SV3
	#undef SV4

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Landscape::GlobalInfo::GlobalInfo()
	:	m_strName(),
		m_uQuadSize(0),
		m_uGridSize(0),
		m_uChunkCount(0),
		m_uVertexCount(0),
		m_uVertexPerRowCount(0),
		m_uRowCount(0),
		m_uStripSize(0),
		m_uLODCount(0),
		m_uTotalLODStripSize(0),
		m_pLODs(NULL),
		m_fPixelErrorMax(0.0f),
		m_fFloorScale(1.0f),
		m_fHeightScale(1.0f),
		m_fMinHeight(FLT_MAX),
		m_fMaxHeight(FLT_MIN),
		m_eFormat(ELandscapeVertexFormat_UNKNOWN)
	{

	}

	void Landscape::GlobalInfo::Reset()
	{
		m_strName.clear();
		m_uQuadSize = 0;
		m_uGridSize = 0;
		m_uChunkCount = 0;
		m_uVertexCount = 0;
		m_uVertexPerRowCount = 0;
		m_uRowCount = 0;
		m_uStripSize = 0;
		m_uLODCount = 0;
		m_uTotalLODStripSize = 0;
		m_pLODs = NULL;
		m_fPixelErrorMax = 0.0f;
		m_fFloorScale = 1.0f;
		m_fHeightScale = 1.0f;
		m_fMinHeight = FLT_MAX;
		m_fMaxHeight = FLT_MIN;
		m_eFormat = ELandscapeVertexFormat_UNKNOWN;
	}

	bool Landscape::GlobalInfo::Create(const Landscape::OpenInfo& _rOpenInfo)
	{
		Reset();
		m_strName = _rOpenInfo.m_strName;
		m_uQuadSize = _rOpenInfo.m_uQuadSize;
		m_uGridSize = _rOpenInfo.m_uGridSize;
		m_uVertexPerRowCount = _rOpenInfo.m_uQuadSize * _rOpenInfo.m_uGridSize + 1;
		m_uRowCount = _rOpenInfo.m_uQuadSize * _rOpenInfo.m_uGridSize + 1;
		m_uVertexCount = m_uVertexPerRowCount * m_uRowCount;
		m_fPixelErrorMax = _rOpenInfo.m_fPixelErrorMax;
		m_fFloorScale = _rOpenInfo.m_fFloorScale;
		m_fHeightScale = _rOpenInfo.m_fHeightScale;
		m_eFormat = _rOpenInfo.m_eFormat;
		const unsigned int uBandCount = m_uQuadSize;
		const unsigned int uVertexPerBand = (m_uQuadSize + 1) * 2;
		const unsigned int uBandJunctionVertexCount = 2 * (m_uQuadSize - 1);
		m_uStripSize = uBandCount * uVertexPerBand + uBandJunctionVertexCount;

		bool bResult = Display::IsPowerOf2(m_uGridSize, &m_uLODCount) && Display::IsPowerOf2(m_uQuadSize);

		if (false != bResult)
		{
			++m_uLODCount;
			m_uTotalLODStripSize = 0;
			m_pLODs = new LODInfo[m_uLODCount];
			LODInfoPtr pLODInfo = m_pLODs;
			for (unsigned int i = 0 ; m_uLODCount > i ; ++i)
			{
				pLODInfo->m_uStripSize = m_uStripSize;
				pLODInfo->m_uStartIndex = m_uTotalLODStripSize;
				pLODInfo->m_uGridSize = m_uGridSize >> i;
				pLODInfo->m_uQuadSize = m_uQuadSize << i;
				pLODInfo->m_uGeometricError = (0 != i) ? (0x00000001 << (i - 1)) : 0;
				//pLODInfo->m_uGeometricError = (0x00000001 << i );
				m_uTotalLODStripSize += m_uStripSize;
				pLODInfo++;
			}
		}

		return bResult;
	}

	void Landscape::GlobalInfo::Release()
	{
		if (NULL != m_pLODs)
		{
			delete[] m_pLODs;
			m_pLODs = NULL;
		}
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Landscape::Landscape(DisplayRef _rDisplay)
	:	DisplayObject(),
		m_oGlobalInfo(),
		m_vGrid(),
		m_vVertexBuffers(),
		m_vVertexes(),
		m_vVertexesIndependent(),
		m_pCurrentVertexBuffer(NULL),
		m_pIndexBuffer(NULL),
		m_pIndexes(NULL),
		m_pLayering(NULL)
	{

	}

	Landscape::~Landscape()
	{

	}

	ELandscapeVertexFormat Landscape::StringToVertexFormat(const string& _strFormat)
	{
		ELandscapeVertexFormat eFormat = ELandscapeVertexFormat_UNKNOWN;

		if ("liquid" == _strFormat)
		{
			eFormat = ELandscapeVertexFormat_LIQUID;
		}
		else if ("default" == _strFormat)
		{
			eFormat = ELandscapeVertexFormat_DEFAULT;
		}

		return eFormat;
	}

	bool Landscape::Create(const boost::any& _rConfig)
	{
		bool bResult = true;
		return bResult;
	}

	void Landscape::Update()
	{
		m_uOutOfFrustum = 0;
		const float fPixelSize = Display::GetInstance()->GetCurrentCamera()->GetPixelSize();
		const Vector3& rCamPos = Display::GetInstance()->GetCurrentCamera()->GetPosition();
		m_vGrid.back()->Traverse(m_vRenderList, rCamPos, fPixelSize);

		struct LODCompareFunction
		{
			bool operator() (LandscapeChunkPtr pLC1, LandscapeChunkPtr pLC2)
			{
				return (pLC1->GetLODID() < pLC2->GetLODID());
			}
		};
		sort(m_vRenderList.begin(), m_vRenderList.end(), LODCompareFunction());

		LandscapeChunkPtrVec::iterator iChunk = m_vRenderList.begin();
		LandscapeChunkPtrVec::iterator iEnd = m_vRenderList.end();
		DisplayPtr pDisplay = Display::GetInstance();
		const Key uPassNameKey = MakeKey(string("scene"));
		while (iEnd != iChunk)
		{
			LandscapeChunkPtr pChunk = *iChunk;
			*(pChunk->GetWorldMatrix()) = *(GetWorldMatrix());
			pChunk->SetMaterial(m_pMaterial);
			pDisplay->RenderRequest(uPassNameKey, pChunk);
			++iChunk;
		}
		m_vRenderList.clear();
	}

	void Landscape::Release()
	{
		Close();
	}

	void Landscape::Render()
	{
		m_pCurrentVertexBuffer = NULL;
		size_t uCount = m_vRenderList.size();
		if ((false == m_vRenderList.empty()) && (m_pIndexBuffer->Use()))
		{
			struct RenderFunction
			{
				void operator() (LandscapeChunkPtr pLandscapeChunk)
				{
					pLandscapeChunk->Render();
				}
			};
			for_each(m_vRenderList.begin(), m_vRenderList.end(), RenderFunction());
			m_vRenderList.clear();
		}
	}

	bool Landscape::Open(const OpenInfo& _rOpenInfo)
	{
		bool bResult = m_vGrid.empty() && m_oGlobalInfo.Create(_rOpenInfo);

		if (false != bResult)
		{
			bResult = CreateIndexBuffer();
		}

		if (false != bResult)
		{
			bResult = CreateVertexBufferIndependent();
		}

		if ((false != bResult) && (false == _rOpenInfo.m_strHeightmap.empty()))
		{
			bResult = LoadHeightmap(_rOpenInfo.m_strHeightmap);
		}

		if ((false != bResult) && (false == _rOpenInfo.m_strLayersConfig.empty()))
		{
			m_pLayering = LandscapeLayerManager::GetInstance()->Get(_rOpenInfo.m_strLayersConfig);
			bResult = (NULL != m_pLayering);
		}

		if (false != bResult)
		{
			switch (m_oGlobalInfo.m_eFormat)
			{
				case ELandscapeVertexFormat_DEFAULT:
				{
					bResult = CreateVertexBufferDefault();
					break;
				}
				case ELandscapeVertexFormat_LIQUID:
				{
					bResult = CreateVertexBufferLiquid();
					break;
				}
			}
		}

		if (false != bResult)
		{
			bResult = CreateChunks();
		}

		return bResult;
	}

	void Landscape::Close()
	{
		struct ReleaseAndDeleteFunction
		{
			void operator() (LandscapeChunkPtr pLandscapeChunk)
			{
				pLandscapeChunk->Release();
				delete pLandscapeChunk;
			}
		};
		for_each(m_vGrid.begin(), m_vGrid.end(), ReleaseAndDeleteFunction());
		m_vGrid.clear();

		while (false == m_vVertexBuffers.empty())
		{
			Display::GetInstance()->ReleaseVertexBuffer(m_vVertexBuffers.back());
			m_vVertexBuffers.pop_back();
		}

		while (false == m_vVertexesIndependent.empty())
		{
			delete[] m_vVertexesIndependent.back();
			m_vVertexesIndependent.pop_back();
		}

		while (false == m_vVertexes.empty())
		{
			delete[] m_vVertexes.back();
			m_vVertexes.pop_back();
		}

		if (NULL != m_pIndexBuffer)
		{
			Display::GetInstance()->ReleaseIndexBuffer(m_pIndexBuffer);
			m_pIndexBuffer = NULL;
		}

		if (NULL != m_pIndexes)
		{
			delete[] m_pIndexes;
			m_pIndexes = NULL;
		}

		m_oGlobalInfo.Release();
	}

	const Landscape::GlobalInfo& Landscape::GetGlobalInfo() const
	{
		return m_oGlobalInfo;
	}

	void Landscape::GetVertexPosition(const LODInfo& _rLODInfo, const unsigned int& _uIndexBufferIndex, const unsigned int& _uVertexStartIndex, Vector3& _rPosition)
	{
		const unsigned int uVertexIndex = _uVertexStartIndex + m_pIndexes[_rLODInfo.m_uStartIndex + _uIndexBufferIndex];
		switch (m_oGlobalInfo.m_eFormat)
		{
			case ELandscapeVertexFormat_DEFAULT:
			{
				VertexDefaultPtr pBuffer = static_cast<VertexDefaultPtr>(_rLODInfo.m_pVertexes);
				_rPosition = pBuffer[uVertexIndex].m_oPosition;
				break;
			}
			case ELandscapeVertexFormat_LIQUID:
			{
				VertexLiquidPtr pBuffer = static_cast<VertexLiquidPtr>(_rLODInfo.m_pVertexes);
				_rPosition = pBuffer[uVertexIndex].m_oPosition;
				break;
			}
		}
	}

	bool Landscape::SetIndices()
	{
		return m_pIndexBuffer->Use();
	}

	bool Landscape::UseLODVertexBuffer(const unsigned int& _uLOD)
	{
		bool bResult = (m_pCurrentVertexBuffer == m_oGlobalInfo.m_pLODs[_uLOD].m_pVertexBuffer);
		if (false == bResult)
		{
			m_pCurrentVertexBuffer = m_oGlobalInfo.m_pLODs[_uLOD].m_pVertexBuffer;
			bResult = m_pCurrentVertexBuffer->Use();
		}
		return bResult;
	}

	void Landscape::UseLayering()
	{
		LandscapeLayerManager::GetInstance()->SetCurrentLayering(m_pLayering);
	}

	bool Landscape::CreateIndexBuffer()
	{
		bool bResult = false;
		m_pIndexes = new unsigned int[m_oGlobalInfo.m_uTotalLODStripSize];
		bResult = (NULL != m_pIndexes);
		if (false != bResult)
		{
			unsigned int* pIndexes = m_pIndexes;

			for (unsigned int k = 0 ; m_oGlobalInfo.m_uLODCount > k ; ++k)
			{
				const unsigned int uLODIncrement = (0x00000001 << 0);
				const unsigned int uLODQuadSize = (m_oGlobalInfo.m_uQuadSize << 0);
				const unsigned int uVertexPerRowCount = (m_oGlobalInfo.m_uVertexPerRowCount >> k) | 0x00000001;
				for (unsigned int j = 0 ; uLODQuadSize > j ; j += uLODIncrement)
				{
					for (unsigned int i = 0 ; (uLODQuadSize + uLODIncrement) > i ; i += uLODIncrement)
					{
						*pIndexes = i + j * uVertexPerRowCount;
						++pIndexes;
						*pIndexes = i + (j + uLODIncrement) * uVertexPerRowCount;
						++pIndexes;
					}
					if ((uLODQuadSize - uLODIncrement) != j)
					{
						*pIndexes = uLODQuadSize + (j + uLODIncrement) * uVertexPerRowCount;
						++pIndexes;
						*pIndexes = 0 + (j + uLODIncrement) * uVertexPerRowCount;
						++pIndexes;
					}
				}
			}

			DisplayIndexBuffer::CreateInfo oIBCInfo = { m_oGlobalInfo.m_uTotalLODStripSize, false };
			m_pIndexBuffer = Display::GetInstance()->CreateIndexBuffer(oIBCInfo);
			bResult = (NULL != m_pIndexBuffer);
			if (false != bResult)
			{
				bResult = m_pIndexBuffer->Set(m_pIndexes);
			}
		}
		return bResult;
	}

	bool Landscape::CreateChunks()
	{
		// compute total chunks (all LODs)
		unsigned int uLOD = 0;
		unsigned int uGridSize = m_oGlobalInfo.m_uGridSize;
		m_oGlobalInfo.m_uChunkCount = 0;
		while (m_oGlobalInfo.m_uLODCount > uLOD)
		{
			m_oGlobalInfo.m_uChunkCount += uGridSize * uGridSize;
			uGridSize >>= 1;
			++uLOD;
		}
		bool bResult = (0 < m_oGlobalInfo.m_uChunkCount);

		// create chunks recursively
		if (false != bResult)
		{
			LandscapeChunkPtr pLandscapeChunk = new LandscapeChunk(*this, m_oGlobalInfo.m_uLODCount - 1);
			LandscapeChunk::CreateInfo oLCCInfo;
			oLCCInfo.m_uX = 0;
			oLCCInfo.m_uZ = 0;
			bResult = pLandscapeChunk->Create(boost::any(&oLCCInfo));
			if (false != bResult)
			{
				m_vGrid.push_back(pLandscapeChunk);
			}
		}

		return bResult;
	}

	bool Landscape::LoadHeightmap(const string& _strFileName)
	{
		bool bResult = Display::GetInstance()->GetSurfaceManager()->Load(_strFileName, _strFileName);
		DisplaySurfacePtr pSurface = Display::GetInstance()->GetSurfaceManager()->Get(_strFileName);

		// load heightmap through a surface
		if ((false != bResult) && (NULL != pSurface) && (pSurface->Lock(false)))
		{
			// get surface info
			ImageInfoRef rSurfaceInfo = pSurface->GetInfo();
			// foe each LOD
			for (unsigned int k = 0 ; m_oGlobalInfo.m_uLODCount > k ; ++k)
			{
				const unsigned int uLODVertexPerRowCount = m_oGlobalInfo.m_pLODs[k].m_uVertexPerRowCount;
				const unsigned int uLODRowCount = (m_oGlobalInfo.m_uRowCount >> k) | 0x00000001;
				const unsigned int uLODIncrement = 0x00000001 << k;
				const float fVStep = 1.0f / float(uLODRowCount);
				const float fUStep = 1.0f / float(uLODVertexPerRowCount);
				VertexIndependentPtr pVertexes = m_oGlobalInfo.m_pLODs[k].m_pVertexesIndependent;
				VertexIndependentPtr pVertex = pVertexes;
				DisplaySurface::UVInfo oUVInfo;
				Byte aRGBA[4];
				// DO NOT USE (1.0f > v) or (1.0f > u) as break conditions !!
				// In some cases round errors will set ++pVertex out of bound long before the double 'for' finishes.
				// for landscape height
				for (float v = 0.0f, vv = 0.0f ; float(uLODRowCount) > vv ; v += fVStep, ++vv)
				{
					// for landscape width
					for (float u = 0.0f, uu = 0.0f ; float(uLODVertexPerRowCount) > uu ; u += fUStep, ++uu)
					{
						// translate vertex (x, z) as texture coords (u, v)
						// get pixel data from surface GetDataUV(u, v, info)
						bResult = pSurface->GetDataUV(u, v, oUVInfo);
						if (false == bResult)
						{
							break;
						}
						// compute the interpolated pixel value based on data info and surface info
						switch (rSurfaceInfo.Format)
						{
							case D3DFMT_A8R8G8B8:
							{
								InterpolatePixelA8R8G8B8(oUVInfo, aRGBA[0], aRGBA[1], aRGBA[2], aRGBA[3]);
								break;
							}
						}
						// update vertex y with interpolated pixel value based on surface info
						//const float fRowHeight = float(aRGBA[0] + aRGBA[1] + aRGBA[2]) / 3.0f;
						const float fRowHeight = float(aRGBA[0]);
						const UInt uWaterLevel = aRGBA[3];
						pVertex->m_fNormalizedHeight = fRowHeight / 255.0f;
						pVertex->m_oPosition.y = fRowHeight * m_oGlobalInfo.m_fHeightScale;
						if (m_oGlobalInfo.m_fMinHeight > pVertex->m_oPosition.y)
						{
							m_oGlobalInfo.m_fMinHeight = pVertex->m_oPosition.y;
						}
						if (m_oGlobalInfo.m_fMaxHeight < pVertex->m_oPosition.y)
						{
							m_oGlobalInfo.m_fMaxHeight = pVertex->m_oPosition.y;
						}
						pVertex->m_uWaterLevel = uWaterLevel;
						++pVertex;
					}
					if (false == bResult)
					{
						break;
					}
				}
				if (false != bResult)
				{
#if LANDSCAPE_USE_MORPHING
					// update morph info
					ComputeVertexIndependentMorphs(m_oGlobalInfo.m_pLODs[k]);
#endif // LANDSCAPE_USE_MORPHING
					ComputeVertexIndependentNormals(m_oGlobalInfo.m_pLODs[k]);
				}
			}
			// release heightmap surface
			pSurface->Unlock();
			Display::GetInstance()->GetSurfaceManager()->Unload(_strFileName);
		}

		return bResult;
	}

	void Landscape::InterpolatePixelA8R8G8B8(const DisplaySurface::UVInfo& _rUVInfo, ByteRef _uRed, ByteRef _uGreen, ByteRef _uBlue, ByteRef _uAlpha)
	{
		BytePtr ppPixels[DisplaySurface::EUVInfoData_COUNT] =
		{
			static_cast<BytePtr>(_rUVInfo.m_aData[DisplaySurface::EUVInfoData_TOPLEFT]),
			static_cast<BytePtr>(_rUVInfo.m_aData[DisplaySurface::EUVInfoData_TOPRIGHT]),
			static_cast<BytePtr>(_rUVInfo.m_aData[DisplaySurface::EUVInfoData_BOTTOMLEFT]),
			static_cast<BytePtr>(_rUVInfo.m_aData[DisplaySurface::EUVInfoData_BOTTOMRIGHT])
		};

		Vector4 aPixels[4];
		//aPixels[DisplaySurface::EUVInfoData_TOPLEFT] = Vector4(ppPixels[DisplaySurface::EUVInfoData_TOPLEFT][1], ppPixels[DisplaySurface::EUVInfoData_TOPLEFT][2], ppPixels[DisplaySurface::EUVInfoData_TOPLEFT][3], ppPixels[DisplaySurface::EUVInfoData_TOPLEFT][0]);
		//aPixels[DisplaySurface::EUVInfoData_TOPRIGHT] = Vector4(ppPixels[DisplaySurface::EUVInfoData_TOPRIGHT][1], ppPixels[DisplaySurface::EUVInfoData_TOPRIGHT][2], ppPixels[DisplaySurface::EUVInfoData_TOPRIGHT][3], ppPixels[DisplaySurface::EUVInfoData_TOPRIGHT][0]);
		//aPixels[DisplaySurface::EUVInfoData_BOTTOMLEFT] = Vector4(ppPixels[DisplaySurface::EUVInfoData_BOTTOMLEFT][1], ppPixels[DisplaySurface::EUVInfoData_BOTTOMLEFT][2], ppPixels[DisplaySurface::EUVInfoData_BOTTOMLEFT][3], ppPixels[DisplaySurface::EUVInfoData_BOTTOMLEFT][0]);
		//aPixels[DisplaySurface::EUVInfoData_BOTTOMRIGHT] = Vector4(ppPixels[DisplaySurface::EUVInfoData_BOTTOMRIGHT][1], ppPixels[DisplaySurface::EUVInfoData_BOTTOMRIGHT][2], ppPixels[DisplaySurface::EUVInfoData_BOTTOMRIGHT][3], ppPixels[DisplaySurface::EUVInfoData_BOTTOMRIGHT][0]);
		#define VECTOR4_A8R8G8B8(Array) Vector4(Array[0], Array[1], Array[2], Array[3])
		aPixels[DisplaySurface::EUVInfoData_TOPLEFT] = VECTOR4_A8R8G8B8(ppPixels[DisplaySurface::EUVInfoData_TOPLEFT]);
		aPixels[DisplaySurface::EUVInfoData_TOPRIGHT] = VECTOR4_A8R8G8B8(ppPixels[DisplaySurface::EUVInfoData_TOPRIGHT]);
		aPixels[DisplaySurface::EUVInfoData_BOTTOMLEFT] = VECTOR4_A8R8G8B8(ppPixels[DisplaySurface::EUVInfoData_BOTTOMLEFT]);
		aPixels[DisplaySurface::EUVInfoData_BOTTOMRIGHT] = VECTOR4_A8R8G8B8(ppPixels[DisplaySurface::EUVInfoData_BOTTOMRIGHT]);
		#undef VECTOR4_A8R8G8B8

		const Vector4 oTop = aPixels[DisplaySurface::EUVInfoData_TOPLEFT] * (1.0f - _rUVInfo.m_fLocalU) + aPixels[DisplaySurface::EUVInfoData_TOPRIGHT] * _rUVInfo.m_fLocalU;
		const Vector4 oBottom = aPixels[DisplaySurface::EUVInfoData_BOTTOMLEFT] * (1.0f - _rUVInfo.m_fLocalU) + aPixels[DisplaySurface::EUVInfoData_BOTTOMRIGHT] * _rUVInfo.m_fLocalU;
		const Vector4 oFinal = oTop * (1.0f - _rUVInfo.m_fLocalV) + oBottom * _rUVInfo.m_fLocalV;

		_uRed = Byte(oFinal.x);
		_uGreen = Byte(oFinal.y);
		_uBlue = Byte(oFinal.z);
		_uAlpha = Byte(oFinal.w);
	}
}
