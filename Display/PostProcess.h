#ifndef __POSTPROCESS_H__
#define __POSTPROCESS_H__

#include "../Core/Core.h"
#include "../Display/DisplayTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayPostProcess : public CoreObject
	{
	public:
		struct CreateInfo
		{
			string	m_strName;
			Key		m_uMaterialNameKey;
		};
		typedef CreateInfo* CreateInfoPtr;

	public:
		DisplayPostProcess(DisplayRef _rDisplay);
		virtual ~DisplayPostProcess();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		void Process();

	protected:
		string				m_strName;
		DisplayRef			m_rDisplay;
		DisplayMaterialPtr	m_pMaterial;
		DisplayObjectPtr	m_pDisplayObject;
		Key					m_uMaterialNameKey;

	private:
	};
}

#endif // __POSTPROCESS_H__
