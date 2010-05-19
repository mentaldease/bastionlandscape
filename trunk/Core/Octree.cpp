#include "stdafx.h"
#include "../Core/Core.h"
#include "../Core/Octree.h"
#include "../Core/Util.h"
#include "../Core/Profiling.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Octree::Octree()
	:	CoreObject(),
		m_mTraverseModes(),
		m_vPool(),
		m_vInUse(),
		m_vAvailable(),
		m_pRoot(NULL),
		m_fLeafSize(0.0f),
		m_uDepth(0)
	{
	}

	Octree::~Octree()
	{

	}

	bool Octree::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo) && (0 < pInfo->m_uDepth) && (0.0f < pInfo->m_fLeafSize);

		Release();

		if (false != bResult)
		{
			m_fs3Center = pInfo->m_fs3Center;
			m_fLeafSize = pInfo->m_fLeafSize;
			m_uDepth = pInfo->m_uDepth;

			OctreeNode::CreateInfo oONCInfo;
			oONCInfo.m_fNodeSize = m_fLeafSize * pow(2.0f, int(m_uDepth));
			oONCInfo.m_uDepth = m_uDepth;
			oONCInfo.m_fs3Center = m_fs3Center;
			const UInt uIndex = NewNode();
			m_pRoot = GetNode(uIndex);
			bResult = m_pRoot->Create(boost::any(&oONCInfo));
		}

		return bResult;
	}

	void Octree::Update()
	{

	}

	void Octree::Release()
	{
		OctreeNodePtrVec::iterator iNode = m_vPool.begin();
		OctreeNodePtrVec::iterator iEnd = m_vPool.end();
		while (iEnd != iNode)
		{
			CoreObject::ReleaseDeleteReset(*iNode);
			++iNode;
		}

		m_mTraverseModes.clear();
		m_vAvailable.clear();
		m_vInUse.clear();
		m_vPool.clear();
		m_pRoot = NULL;
	}

	bool Octree::AddTraverseMode(Key _uModeNameKey, OctreeTraverseFunc _pFunc)
	{
		bool bResult = (m_mTraverseModes.end() == m_mTraverseModes.find(_uModeNameKey));
		if (false != bResult)
		{
			m_mTraverseModes[_uModeNameKey] = _pFunc;
		}
		return bResult;
	}

	void Octree::RemoveTraverseMode(Key _uModeNameKey)
	{
		OctreeTraverseFuncMap::iterator iPair = m_mTraverseModes.find(_uModeNameKey);
		bool bResult = (m_mTraverseModes.end() != iPair);
		if (false != bResult)
		{
			m_mTraverseModes.erase(iPair);
		}
	}

	void Octree::Traverse(Key _uModeNameKey, OctreeNodePtrVecRef _rvNodes, OctreeObjectPtrVecRef _rvObjects, OctreeNodePtr _pStartingNode)
	{
		PROFILING(__FUNCTION__);
		OctreeTraverseFuncMap::iterator iPair = m_mTraverseModes.find(_uModeNameKey);
		bool bResult = (m_mTraverseModes.end() != iPair);
		if (false != bResult)
		{
			OctreeTraverseFuncRef rFunc = iPair->second;
			_pStartingNode = (NULL == _pStartingNode) ? m_pRoot : _pStartingNode;
			//_pStartingNode->Traverse(rFunc, _rvNodes, _rvObjects);
			_pStartingNode->TraverseNoRecursion(rFunc, _rvNodes, _rvObjects);
		}
	}

	bool Octree::AddObject(OctreeObjectPtr _pObject)
	{
		return m_pRoot->AddObject(_pObject);
	}

	bool Octree::RemoveObject(OctreeObjectPtr _pObject)
	{
		return m_pRoot->RemoveObject(_pObject);
	}

	UInt Octree::NewNode()
	{
		UInt uResult = (false == m_vAvailable.empty()) ? UInt(m_vAvailable.back()) : NewNode_();
		m_vInUse.push_back(uResult);
		m_vAvailable.pop_back();
		return uResult;
	}

	void Octree::DeleteNode(UInt _uIndex)
	{
		UIntVec::iterator iNode = find(m_vInUse.begin(), m_vInUse.end(), _uIndex);
		if (m_vInUse.end() != iNode)
		{
			m_vInUse.erase(iNode);
			m_vAvailable.push_back(_uIndex);
		}
	}

	UInt Octree::NewNode_()
	{
		m_vPool.push_back(new OctreeNode(*this));
		m_vAvailable.push_back(m_vPool.size() - 1);
		return UInt(m_vAvailable.back());
	}
}
