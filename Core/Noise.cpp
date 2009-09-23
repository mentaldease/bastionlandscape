#include <stdafx.h>
#include "../Core/Noise.h"

#include <noise/noise.h>
#include "../Core/noiseutils.h"
using namespace noise;

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct NoiseGenerator::Make2DInfo
	{
		Make2DInfo()
		:	m_pData(NULL),
			m_sStride(0)
		{

		}

		module::Perlin				m_oSrcModule;
		utils::NoiseMap				m_oHeightMap;
		utils::NoiseMapBuilderPlane m_oHeightMapBuilder;
		FloatPtr					m_pData;
		int							m_sStride;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	NoiseGenerator::NoiseGenerator()
	:	CoreObject(),
		m_pMake2DInfo(NULL)
	{

	}

	NoiseGenerator::~NoiseGenerator()
	{

	}

	bool NoiseGenerator::Create(const boost::any& _rConfig)
	{
		bool bResult = true;
		return bResult;
	}

	void NoiseGenerator::Update()
	{

	}

	void NoiseGenerator::Release()
	{
		if (NULL != m_pMake2DInfo)
		{
			delete m_pMake2DInfo;
			m_pMake2DInfo = NULL;
		}
	}

	void NoiseGenerator::Process(const int& _sWidth, const int& _sHeight, FloatPtr _pData)
	{
		if (NULL == m_pMake2DInfo)
		{
			m_pMake2DInfo = new Make2DInfo;
		}

		m_pMake2DInfo->m_oHeightMapBuilder.SetSourceModule(m_pMake2DInfo->m_oSrcModule);
		m_pMake2DInfo->m_oHeightMapBuilder.SetDestNoiseMap(m_pMake2DInfo->m_oHeightMap);
		m_pMake2DInfo->m_oHeightMapBuilder.SetDestSize(_sWidth, _sHeight);
		m_pMake2DInfo->m_oHeightMapBuilder.SetBounds(0.0, double(_sWidth) / 64.0, 0.0, double(_sHeight) / 64.0);
		m_pMake2DInfo->m_oHeightMapBuilder.Build();
		m_pMake2DInfo->m_pData = m_pMake2DInfo->m_oHeightMap.GetSlabPtr();
		m_pMake2DInfo->m_sStride = m_pMake2DInfo->m_oHeightMap.GetStride();

		if (NULL != _pData)
		{

		}
	}

	const FloatPtr NoiseGenerator::GetData() const
	{
		const FloatPtr pResult = (NULL != m_pMake2DInfo) ? m_pMake2DInfo->m_pData : NULL;
		return pResult;
	}

	int NoiseGenerator::GetStride() const
	{
		int sResult = (NULL != m_pMake2DInfo) ? m_pMake2DInfo->m_sStride : 0;
		return sResult;
	}
}
