#ifndef __RENDERPASS_H__
#define __RENDERPASS_H__

#include "../Display/Display.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayRenderStage : public CoreObject
	{
	public:
		DisplayRenderStage(DisplayRef _rDisplay);
		virtual ~DisplayRenderStage();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		void RenderRequest(DisplayObjectPtr _pDisplayObject);
		DisplayObjectPtrVec& GetRenderList();

		const Key& GetNameKey() const;

	protected:
		bool CreatePostProcesses(LuaObjectRef _rLuaObject);
		bool CreatePostProcess(LuaObjectRef _rLuaObject);
		bool CreateNormalProcesses(LuaObjectRef _rLuaObject);
		bool CreateNormalProcess(LuaObjectRef _rLuaObject);

	protected:
		DisplayRef					m_rDisplay;
		Key							m_uPassName;
		Key							m_uCameraName;
		DisplayPostProcessPtrMap	m_mPostProcesses;
		DisplayPostProcessPtrVec	m_vPostProcesses;
		DisplayNormalProcessPtrMap	m_mNormalProcesses;
		DisplayNormalProcessPtrVec	m_vNormalProcesses;
		DisplayObjectPtrVec			m_vRenderList;
	};
}

#endif
