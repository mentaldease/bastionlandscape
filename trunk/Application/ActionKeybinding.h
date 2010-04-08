#ifndef __ACTIONKEYBINDING_H__
#define __ACTIONKEYBINDING_H__

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class ActionKeybindingManager : public CoreObject
	{
	public:
		struct CreateInfo
		{
			BytePtr	m_pKeysInfo;
			BytePtr	m_pKeysInfoOld;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

	public:
		ActionKeybindingManager();
		virtual ~ActionKeybindingManager();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		bool LoadBindings(const string& _strFileName);
		bool TestAction(const UInt _uAction);

		static bool InitScripting();

	protected:
		typedef map<UInt, UInt> KeyActionMap;
		typedef map<UInt, bool> ActionMap;

		static const UInt s_uShiftModifier = (1 << 16);
		static const UInt s_uControlModifier = (1 << 17);
		static const UInt s_uAltModifier = (1 << 18);
		static const UInt s_uOnceModifier = (1 << 19);

	protected:
		KeyActionMap	m_mKeyActions;
		ActionMap		m_mActions;
		BytePtr			m_pKeysInfo;
		BytePtr			m_pKeysInfoOld;
	};
}

#endif // __ACTIONKEYBINDING_H__
