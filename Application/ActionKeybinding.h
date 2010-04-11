#ifndef __ACTIONKEYBINDING_H__
#define __ACTIONKEYBINDING_H__

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	#define DIM_MOVEX			0x00010000
	#define DIM_MOVEY			0x00020000
	#define DIM_MOVEZ			0x00030000
	#define DIM_BUTTONLEFT		0x00040000
	#define DIM_BUTTONRIGHT		0x00050000
	#define DIM_BUTTONMIDDLE	0x00060000
	#define DIM_BUTTON4			0x00070000
	#define DIM_BUTTON5			0x00080000
	#define DIM_BUTTON6			0x00090000
	#define DIM_BUTTON7			0x000a0000
	#define DIM_BUTTON8			0x000b0000

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class ActionKeybindingManager : public CoreObject
	{
	public:
		struct CreateInfo
		{
			BytePtr			m_pKeysInfo;
			BytePtr			m_pKeysInfoOld;
			DIMouseStatePtr	m_pMouseInfo;
			DIMouseStatePtr	m_pMouseInfoOld;
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

		const UIntVec& GetActiveActions() const;

		static bool InitScripting();

	protected:
		typedef map<UInt, UInt> KeyActionMap;
		typedef map<UInt, bool> ActionMap;

		static const UInt s_uShiftModifier = (1 << 24);
		static const UInt s_uControlModifier = (1 << 25);
		static const UInt s_uAltModifier = (1 << 26);
		static const UInt s_uOnceModifier = (1 << 27);
		static const UInt s_uMouseModifier = (1 << 16);

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
		UIntVec			m_vActions;
		ContextPtr		m_pCurrentContext;
		BytePtr			m_pKeysInfo;
		BytePtr			m_pKeysInfoOld;
		DIMouseStatePtr	m_pMouseInfo;
		DIMouseStatePtr	m_pMouseInfoOld;
	};
}

#endif // __ACTIONKEYBINDING_H__
