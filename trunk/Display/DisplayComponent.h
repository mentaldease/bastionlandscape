#ifndef __DISPLAYCOMPONENT_H__
#define __DISPLAYCOMPONENT_H__

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayComponent : public Component
	{
	public:
		DisplayComponent(EntityRef _rEntity);
		virtual ~DisplayComponent();

		virtual Key GetSignature();

		void SetDisplayObject(DisplayObjectPtr _pObject);
		DisplayObjectPtr GetDisplayObject();

	protected:
		DisplayObjectPtr	m_pObject;
	};
}

#endif // __DISPLAYCOMPONENT_H__
