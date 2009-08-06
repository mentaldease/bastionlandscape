#ifndef __SCENE_H__
#define __SCENE_H__

#include "../Application/ApplicationIncludes.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Scene : public CoreObject
	{
	public:
		struct CreateInfo
		{
			string	m_strPath;
		};

	public:
		Scene(ApplicationRef _rApplication);
		virtual ~Scene();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

	protected:
		bool CreateLoadMaterials(Config& _rConfig);
		bool CreateLoadLandscapes(Config& _rConfig);
		bool CreateLoadLandscape(Config& _rConfig, ConfigShortcutPtr pShortcut);

	protected:
		ApplicationRef			m_rApplication;
		CoreObjectPtrMap		m_mAllObjects;
		LandscapePtrMap			m_mLandscapes;
		DisplayMaterialPtrMap	m_mMaterials;
		string					m_strName;

	private:
	};
}

#endif // __SCENE_H__
