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

		void PreUpdate();

	protected:
		bool CreateLoadMaterials(Config& _rConfig);
		bool CreateLoadLandscapes(Config& _rConfig);
		bool CreateLoadLandscape(Config& _rConfig, ConfigShortcutPtr pShortcut);
		bool CreateLoadPostProcesses(Config& _rConfig);
		bool CreateLoadPostProcess(Config& _rConfig, ConfigShortcutPtr pShortcut);
		bool CreateLoadNormalProcesses(Config& _rConfig);
		bool CreateLoadNormalProcess(Config& _rConfig, ConfigShortcutPtr pShortcut);

	protected:
		ApplicationRef				m_rApplication;
		CoreObjectPtrMap			m_mAllObjects;
		LandscapePtrMap				m_mLandscapes;
		DisplayMaterialPtrMap		m_mMaterials;
		DisplayPostProcessPtrMap	m_mPostProcesses;
		DisplayPostProcessPtrVec	m_vPostProcesses;
		DisplayNormalProcessPtrMap	m_mNormalProcesses;
		DisplayNormalProcessPtrVec	m_vNormalProcesses;
		string						m_strName;
		float						m_fWaterLevel;
		Key							m_uWaterLevelKey;

	private:
	};
}

#endif // __SCENE_H__
