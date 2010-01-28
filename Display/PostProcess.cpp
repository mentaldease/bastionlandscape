#include "stdafx.h"
#include "../Display/PostProcess.h"
#include "../Display/Display.h"
#include "../Display/Effect.h"
#include "../Display/RenderTarget.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayPostProcess::CreateInfo::CreateInfo()
	:	m_strName(),
		m_uMaterialNameKey(0),
		m_bImmediateWrite(false)
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayPostProcess::DisplayPostProcess(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_pRTChain(NULL),
		m_pMaterial(NULL),
		m_pDisplayObject(NULL),
		m_bImmediateWrite(false)
	{

	}

	DisplayPostProcess::~DisplayPostProcess()
	{

	}

	bool DisplayPostProcess::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		m_uMaterialNameKey = pInfo->m_uMaterialNameKey;
		m_pMaterial = m_rDisplay.GetMaterialManager()->GetMaterial(m_uMaterialNameKey);
		m_pDisplayObject = m_rDisplay.GetPostProcessGeometry();
		m_strName = pInfo->m_strName;
		m_bImmediateWrite = pInfo->m_bImmediateWrite;

		const bool bResult = (NULL != m_pMaterial);

		if (false != bResult)
		{
			m_pRTChain = m_rDisplay.GetRenderTargetChain();
		}

		return bResult;
	}

	void DisplayPostProcess::Update()
	{
		RenderObjectFunction oROF(m_pMaterial);
		oROF(m_pDisplayObject);
	}

	void DisplayPostProcess::Release()
	{
		if (NULL != m_pMaterial)
		{
			m_rDisplay.GetMaterialManager()->UnloadMaterial(m_uMaterialNameKey);
			m_pMaterial = NULL;
		}
	}

	void DisplayPostProcess::RenderBegin()
	{
		m_pRTChain->SetImmediateWrite(m_bImmediateWrite);
	}

	void DisplayPostProcess::RenderEnd()
	{

	}
}
