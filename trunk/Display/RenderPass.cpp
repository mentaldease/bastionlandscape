#include "stdafx.h"
#include "../Display/RenderPass.h"
#include "../Display/PostProcess.h"
#include "../Display/NormalProcess.h"
#include "../Core/Scripting.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayRenderPass::DisplayRenderPass(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_uPassName(0),
		m_uCameraName(0),
		m_mPostProcesses(),
		m_vPostProcesses(),
		m_mNormalProcesses(),
		m_vNormalProcesses(),
		m_vRenderList()
	{

	}

	DisplayRenderPass::~DisplayRenderPass()
	{

	}

	bool DisplayRenderPass::Create(const boost::any& _rConfig)
	{
		LuaObjectPtr pRoot = boost::any_cast<LuaObjectPtr>(_rConfig);
		string strPassName;
		string strCameraName;
		bool bResult = (NULL != pRoot);

		if (false != bResult)
		{
			Release();
			LuaObjectRef oRoot = *pRoot;
			bResult = Scripting::Lua::Get(oRoot, "camera", string(""), strCameraName)
				&& Scripting::Lua::Get(oRoot, "name", string(""), strPassName)
				&& CreatePostProcesses(oRoot)
				&& CreateNormalProcesses(oRoot);
		}

		if (false != bResult)
		{
			m_uCameraName = MakeKey(strCameraName);
			m_uPassName = MakeKey(strPassName);
		}

		return bResult;
	}

	void DisplayRenderPass::Update()
	{
		if (false == m_vPostProcesses.empty())
		{
			m_rDisplay.SetPostProcessesList(&m_vPostProcesses);
		}
		if (false == m_vNormalProcesses.empty())
		{
			m_rDisplay.SetNormalProcessesList(&m_vNormalProcesses);
		}
		m_rDisplay.SetCurrentCamera(m_rDisplay.GetCamera(m_uCameraName));
	}

	void DisplayRenderPass::Release()
	{
		// render post processes
		while (m_mPostProcesses.end() != m_mPostProcesses.begin())
		{
			DisplayPostProcessPtr pPostProcess = m_mPostProcesses.begin()->second;
			pPostProcess->Release();
			delete pPostProcess;
			m_mPostProcesses.erase(m_mPostProcesses.begin());
		}
		m_vPostProcesses.clear();

		// render normal processes
		while (m_mNormalProcesses.end() != m_mNormalProcesses.begin())
		{
			DisplayNormalProcessPtr pNormalProcess = m_mNormalProcesses.begin()->second;
			pNormalProcess->Release();
			delete pNormalProcess;
			m_mNormalProcesses.erase(m_mNormalProcesses.begin());
		}
		m_vNormalProcesses.clear();
	}

	void DisplayRenderPass::RenderRequest(DisplayObjectPtr _pDisplayObject)
	{
		if (m_vRenderList.end() == find(m_vRenderList.begin(), m_vRenderList.end(), _pDisplayObject))
		{
			m_vRenderList.push_back(_pDisplayObject);
		}
	}

	DisplayObjectPtrVec& DisplayRenderPass::GetRenderList()
	{
		return m_vRenderList;
	}

	const Key& DisplayRenderPass::GetNameKey() const
	{
		return m_uPassName;
	}

	bool DisplayRenderPass::CreatePostProcesses(LuaObjectRef _rLuaObject)
	{
		LuaObject oPostProcesses = _rLuaObject["postprocesses"];
		bool bResult = true;

		if (false == oPostProcesses.IsNil())
		{
			const int sCount = oPostProcesses.GetCount();
			for (int i = 0 ; sCount > i ; ++i)
			{
				bResult = CreatePostProcess(oPostProcesses[i + 1]);
				if (false == bResult)
				{
					break;
				}
			}
		}

		return bResult;
	}

	bool DisplayRenderPass::CreatePostProcess(LuaObjectRef _rLuaObject)
	{
		DisplayPostProcessPtr pPostProcess = new DisplayPostProcess(m_rDisplay);
		DisplayPostProcess::CreateInfo oPPCInfo = { &_rLuaObject };
		string strName;
		Scripting::Lua::Get(_rLuaObject, "name", strName, strName);
		const Key uNameKey = MakeKey(strName);
		bool bResult = (m_mPostProcesses.end() == m_mPostProcesses.find(uNameKey)) // <== check that there is NOT another post process with the same name
			&& pPostProcess->Create(boost::any(&oPPCInfo));

		if (false != bResult)
		{
			m_mPostProcesses[uNameKey] = pPostProcess;
			m_vPostProcesses.push_back(pPostProcess);
		}
		else
		{
			pPostProcess->Release();
			delete pPostProcess;
		}

		return bResult;
	}

	bool DisplayRenderPass::CreateNormalProcesses(LuaObjectRef _rLuaObject)
	{
		LuaObject oNormalProcesses = _rLuaObject["normalprocesses"];
		bool bResult = true;

		if (false == oNormalProcesses.IsNil())
		{
			const int sCount = oNormalProcesses.GetCount();
			for (int i = 0 ; sCount > i ; ++i)
			{
				bResult = CreateNormalProcess(oNormalProcesses[i + 1]);
				if (false == bResult)
				{
					break;
				}
			}
		}

		return bResult;
	}

	bool DisplayRenderPass::CreateNormalProcess(LuaObjectRef _rLuaObject)
	{
		DisplayNormalProcessPtr pNormalProcess = new DisplayNormalProcess(m_rDisplay);
		DisplayNormalProcess::CreateInfo oNPCInfo = { &_rLuaObject };
		string strName;
		Scripting::Lua::Get(_rLuaObject, "name", strName, strName);
		const Key uNameKey = MakeKey(strName);
		bool bResult = (m_mNormalProcesses.end() == m_mNormalProcesses.find(uNameKey)) // <== check that there is NOT another normal process with the same name
			&& pNormalProcess->Create(boost::any(&oNPCInfo));

		if (false != bResult)
		{
			m_mNormalProcesses[uNameKey] = pNormalProcess;
			m_vNormalProcesses.push_back(pNormalProcess);
		}
		else
		{
			pNormalProcess->Release();
			delete pNormalProcess;
		}

		return bResult;
	}
}
