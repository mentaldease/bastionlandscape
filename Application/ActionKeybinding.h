#ifndef __ACTIONKEYBINDING_H__
#define __ACTIONKEYBINDING_H__

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	#define DIM_SHIFT_MODIFIER	16
	#define DIM_EVENT(VALUE)	((1 << DIM_SHIFT_MODIFIER) + VALUE)
	#define DIM_MOVEX			DIM_EVENT(0)
	#define DIM_MOVEY			DIM_EVENT(1)
	#define DIM_MOVEZ			DIM_EVENT(2)
	#define DIM_BUTTONLEFT		DIM_EVENT(3)
	#define DIM_BUTTONRIGHT		DIM_EVENT(4)
	#define DIM_BUTTONMIDDLE	DIM_EVENT(5)
	#define DIM_BUTTON4			DIM_EVENT(6)
	#define DIM_BUTTON5			DIM_EVENT(7)
	#define DIM_BUTTON6			DIM_EVENT(8)
	#define DIM_BUTTON7			DIM_EVENT(9)
	#define DIM_BUTTON8			DIM_EVENT(10)

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

		static const UInt s_uShiftModifier = (1 << 8);
		static const UInt s_uControlModifier = (1 << 9);
		static const UInt s_uAltModifier = (1 << 10);
		static const UInt s_uOnceModifier = (1 << 11);
		static const UInt s_uMouseModifier = (1 << DIM_SHIFT_MODIFIER);

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
