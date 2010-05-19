#include "stdafx.h"
#include "../Display/Display.h"
#include "../Display/DisplayComponent.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayComponent::DisplayComponent(EntityRef _rEntity)
	:	Component(_rEntity),
		m_pObject(NULL)
	{

	}

	DisplayComponent::~DisplayComponent()
	{

	}

	Key DisplayComponent::GetSignature()
	{
		static Key uSignature = MakeKey(string("DisplayComponent"));
		return uSignature;
	}

	void DisplayComponent::SetDisplayObject(DisplayObjectPtr _pObject)
	{
		m_pObject = _pObject;
	}

	DisplayObjectPtr DisplayComponent::GetDisplayObject()
	{
		return m_pObject;
	}
}
