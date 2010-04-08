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

		bool LoadBindings(const Key _uContextID, const string& _strFileName, const bool _bSetCurrentContext = false);
		bool SetCurrentContext(const Key _uContextID);

		bool TestAction(const UInt _uAction);
		void DisableAction(const UInt _uAction);
		void EnableAction(const UInt _uAction);

		static bool InitScripting();

	protected:
		typedef map<UInt, UInt> KeyActionMap;
		typedef map<UInt, bool> ActionMap;

		static const UInt s_uShiftModifier = (1 << 16);
		static const UInt s_uControlModifier = (1 << 17);
		static const UInt s_uAltModifier = (1 << 18);
		static const UInt s_uOnceModifier = (1 << 19);

		struct Context
		{
			void Clear();

			KeyActionMap	m_mKeyActions;
			ActionMap		m_mActions;
			ActionMap		m_mActionRights;
		};
		typedef Context* ContextPtr;
		typedef Context& ContextRef;
		typedef map<Key, ContextPtr> ContextPtrMap;

	protected:
		ContextPtrMap	m_mContextes;
		ContextPtr		m_pCurrentContext;
		BytePtr			m_pKeysInfo;
		BytePtr			m_pKeysInfoOld;
	};
}

#endif // __ACTIONKEYBINDING_H__
