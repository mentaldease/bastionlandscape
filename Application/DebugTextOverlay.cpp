#include "stdafx.h"
#include "../Application/Application.h"
#include "../Application/DebugTextOverlay.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DebugTextOverlay::DebugTextOverlay()
	:	CoreObject(),
		m_vDrawInfoPool(),
		m_vpDrawInfo()
	{

	}

	DebugTextOverlay::~DebugTextOverlay()
	{

	}

	bool DebugTextOverlay::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			Release();
		}

		return bResult;
	}

	void DebugTextOverlay::Update()
	{

	}

	void DebugTextOverlay::Release()
	{
		m_vDrawInfoPool.clear();
		m_vpDrawInfo.clear();
	}

	bool DebugTextOverlay::Draw(const float _fX, const float _fY, const Key& _uFontName, const wstring& _wstrText, const Vector4& _oColor)
	{
		bool bResult = false;
		return bResult;
	}
}
