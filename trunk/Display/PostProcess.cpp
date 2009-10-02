#include "stdafx.h"
#include "../Display/PostProcess.h"
#include "../Display/Display.h"
#include "../Display/Effect.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayPostProcess::DisplayPostProcess(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
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
		bool bResult = (NULL != m_pMaterial);

		return bResult;
	}

	void DisplayPostProcess::Update()
	{

	}

	void DisplayPostProcess::Release()
	{
		if (NULL != m_pMaterial)
		{
			m_rDisplay.GetMaterialManager()->UnloadMaterial(m_uMaterialNameKey);
			m_pMaterial = NULL;
		}
	}

	void DisplayPostProcess::Process()
	{
		RenderObjectFunction oROF(m_pMaterial);
		oROF(m_pDisplayObject);
	}
}
