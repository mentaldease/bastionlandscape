#include "stdafx.h"
#include "../Core/Core.h"
#include "../Core/Octree.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	OctreeObject::OctreeObject(OctreeRef _rOctree)
	:	CoreObject(),
		m_vPoints(),
		m_rOctree(_rOctree)
	{
		m_vPoints.resize(EOctreeAABB_COUNT);
	}

	OctreeObject::~OctreeObject()
	{

	}

	void OctreeObject::SetAABB(const fsVector3& _rf3TopRightFar, const fsVector3& _rf3BottomLeftNear)
	{
		#define SETPOINT(ID, XVEC3, YVEC3, ZVEC3) \
		m_vPoints[ID].x() = XVEC3.x(); \
		m_vPoints[ID].y() = YVEC3.y(); \
		m_vPoints[ID].z() = ZVEC3.z();

		SETPOINT(EOctreeAABB_TOPLEFTFAR, _rf3TopRightFar, _rf3BottomLeftNear, _rf3TopRightFar);
		SETPOINT(EOctreeAABB_TOPRIGHTTFAR, _rf3TopRightFar, _rf3TopRightFar, _rf3TopRightFar);
		SETPOINT(EOctreeAABB_TOPRIGHTTNEAR, _rf3TopRightFar, _rf3TopRightFar, _rf3BottomLeftNear);
		SETPOINT(EOctreeAABB_TOPLEFTTNEAR, _rf3TopRightFar, _rf3BottomLeftNear, _rf3BottomLeftNear);

		SETPOINT(EOctreeAABB_BOTTOMLEFTFAR, _rf3BottomLeftNear, _rf3BottomLeftNear, _rf3TopRightFar);
		SETPOINT(EOctreeAABB_BOTTOMRIGHTTFAR, _rf3BottomLeftNear, _rf3TopRightFar, _rf3TopRightFar);
		SETPOINT(EOctreeAABB_BOTTOMRIGHTTNEAR, _rf3BottomLeftNear, _rf3TopRightFar, _rf3BottomLeftNear);
		SETPOINT(EOctreeAABB_BOTTOMLEFTTNEAR, _rf3BottomLeftNear, _rf3BottomLeftNear, _rf3BottomLeftNear);

		#undef SETPOINT
	}

	void OctreeObject::GetAABB(fsVector3Vec& _vPoints)
	{
		_vPoints = m_vPoints;
	}

	const fsVector3Vec& OctreeObject::GetAABB() const
	{
		return m_vPoints;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	OctreeNode::OctreeNode(OctreeRef _rOctree)
	:	CoreObject(),
		m_vPoints(),
		m_vChildrenAABB(),
		m_vChildren(),
		m_vObjects(),
		m_rOctree(_rOctree),
		m_fs3Center(0.0f, 0.0f, 0.0f),
		m_fNodeSize(0.0f),
		m_uDepthLevel(0)
	{
		m_vPoints.resize(EOctreeAABB_COUNT);
		m_vChildrenAABB.resize(EOctreeAABB_COUNT * 2);
	}
	
	OctreeNode::~OctreeNode()
	{

	}

	bool OctreeNode::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo);

		Release();

		if (false != bResult)
		{
			m_uDepthLevel = pInfo->m_uDepth;
			m_fs3Center = pInfo->m_fs3Center;
			m_fNodeSize = pInfo->m_fNodeSize;

			const fsVector3& rfs3Center = pInfo->m_fs3Center;
			const float fHalfSize = m_fNodeSize / 2.0f;
			const float fQuarterSize = m_fNodeSize / 4.0f;

			#define SETPOINT(ID, CENTER, XOFFSET, YOFFSET, ZOFFSET) \
				m_vPoints[ID].x() = CENTER.x() + (XOFFSET); \
				m_vPoints[ID].y() = CENTER.y() + (YOFFSET); \
				m_vPoints[ID].z() = CENTER.z() + (ZOFFSET);

			SETPOINT(EOctreeAABB_TOPLEFTFAR, rfs3Center, -fHalfSize, fHalfSize, fHalfSize);
			SETPOINT(EOctreeAABB_TOPRIGHTTFAR, rfs3Center, fHalfSize, fHalfSize, fHalfSize);
			SETPOINT(EOctreeAABB_TOPRIGHTTNEAR, rfs3Center, fHalfSize, fHalfSize, -fHalfSize);
			SETPOINT(EOctreeAABB_TOPLEFTTNEAR, rfs3Center, -fHalfSize, fHalfSize, -fHalfSize);

			SETPOINT(EOctreeAABB_BOTTOMLEFTFAR, rfs3Center, -fHalfSize, -fHalfSize, fHalfSize);
			SETPOINT(EOctreeAABB_BOTTOMRIGHTTFAR, rfs3Center, fHalfSize, -fHalfSize, fHalfSize);
			SETPOINT(EOctreeAABB_BOTTOMRIGHTTNEAR, rfs3Center, fHalfSize, -fHalfSize, -fHalfSize);
			SETPOINT(EOctreeAABB_BOTTOMLEFTTNEAR, rfs3Center, -fHalfSize, -fHalfSize, -fHalfSize);

			#undef SETPOINT

			#define SETAABB(ID, CENTER, OFFSET) \
				m_vChildrenAABB[(ID) + 0].x() = CENTER.x() + (OFFSET); \
				m_vChildrenAABB[(ID) + 0].y() = CENTER.y() + (OFFSET); \
				m_vChildrenAABB[(ID) + 0].z() = CENTER.z() + (OFFSET); \
				m_vChildrenAABB[(ID) + 1].x() = CENTER.x() - (OFFSET); \
				m_vChildrenAABB[(ID) + 1].y() = CENTER.y() - (OFFSET); \
				m_vChildrenAABB[(ID) + 1].z() = CENTER.z() - (OFFSET);

			const fsVector3 fs3TLF(rfs3Center.x() - fQuarterSize, rfs3Center.y() + fQuarterSize, rfs3Center.z() + fQuarterSize);
			SETAABB(EOctreeAABB_TOPLEFTFAR * 2, fs3TLF, fQuarterSize);
			const fsVector3 fs3TRF(rfs3Center.x() + fQuarterSize, rfs3Center.y() + fQuarterSize, rfs3Center.z() + fQuarterSize);
			SETAABB(EOctreeAABB_TOPRIGHTTFAR * 2, fs3TRF, fQuarterSize);
			const fsVector3 fs3TRN(rfs3Center.x() + fQuarterSize, rfs3Center.y() + fQuarterSize, rfs3Center.z() - fQuarterSize);
			SETAABB(EOctreeAABB_TOPRIGHTTNEAR * 2, fs3TRN, fQuarterSize);
			const fsVector3 fs3TLN(rfs3Center.x() - fQuarterSize, rfs3Center.y() + fQuarterSize, rfs3Center.z() - fQuarterSize);
			SETAABB(EOctreeAABB_TOPLEFTTNEAR * 2, fs3TLN, fQuarterSize);

			const fsVector3 fs3BLF(rfs3Center.x() - fQuarterSize, rfs3Center.y() - fQuarterSize, rfs3Center.z() + fQuarterSize);
			SETAABB(EOctreeAABB_BOTTOMLEFTFAR * 2, fs3BLF, fQuarterSize);
			const fsVector3 fs3BRF(rfs3Center.x() + fQuarterSize, rfs3Center.y() - fQuarterSize, rfs3Center.z() + fQuarterSize);
			SETAABB(EOctreeAABB_BOTTOMRIGHTTFAR * 2, fs3BRF, fQuarterSize);
			const fsVector3 fs3BRN(rfs3Center.x() + fQuarterSize, rfs3Center.y() - fQuarterSize, rfs3Center.z() - fQuarterSize);
			SETAABB(EOctreeAABB_BOTTOMRIGHTTNEAR * 2, fs3BRN, fQuarterSize);
			const fsVector3 fs3BLN(rfs3Center.x() - fQuarterSize, rfs3Center.y() - fQuarterSize, rfs3Center.z() - fQuarterSize);
			SETAABB(EOctreeAABB_BOTTOMLEFTTNEAR * 2, fs3BLN, fQuarterSize);

			#undef SETAABB

			m_vChildren.resize(EOctreeAABB_COUNT, 0);
		}

		return bResult;
	}

	void OctreeNode::Update()
	{

	}

	void OctreeNode::Release()
	{
		UIntVec::iterator iNode = m_vChildren.begin();
		UIntVec::iterator iEnd = m_vChildren.end();
		while (iEnd != iNode)
		{
			const UInt uIndex = *iNode;
			if (0 != uIndex)
			{
				m_rOctree.DeleteNode(uIndex);
			}
			++iNode;
		}
		m_vChildren.clear();
		m_vObjects.clear();
	}

	OctreeRef OctreeNode::GetOctree() const
	{
		return m_rOctree;
	}

	bool OctreeNode::AddObject(OctreeObjectPtr _pObject)
	{
		bool bResult = false;

		if (0 == m_uDepthLevel)
		{
			bResult = true;
			if (m_vObjects.end() == find(m_vObjects.begin(), m_vObjects.end(), _pObject))
			{
				m_vObjects.push_back(_pObject);
			}
		}
		else
		{
			const fsVector3Vec& rvAABB = _pObject->GetAABB();
			for (UInt i = 0 ; EOctreeAABB_COUNT > i ; ++i)
			{
				const UInt uIndex = i * 2;

				if (m_vChildrenAABB[i].x() < rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].x())
				{
					continue;
				}
				if (m_vChildrenAABB[i].y() < rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].y())
				{
					continue;
				}
				if (m_vChildrenAABB[i].z() < rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].z())
				{
					continue;
				}

				if (m_vChildrenAABB[i + 1].x() > rvAABB[EOctreeAABB_TOPRIGHTTFAR].x())
				{
					continue;
				}
				if (m_vChildrenAABB[i + 1].y() > rvAABB[EOctreeAABB_TOPRIGHTTFAR].y())
				{
					continue;
				}
				if (m_vChildrenAABB[i + 1].z() > rvAABB[EOctreeAABB_TOPRIGHTTFAR].z())
				{
					continue;
				}

				if (0 == m_vChildren[i])
				{
					OctreeNode::CreateInfo oONCInfo;
					oONCInfo.m_fNodeSize = m_fNodeSize / 2.0f;
					oONCInfo.m_uDepth = m_uDepthLevel - 1;
					oONCInfo.m_fs3Center.x() = m_vChildrenAABB[i].x() - oONCInfo.m_fNodeSize / 2.0f;
					oONCInfo.m_fs3Center.y() = m_vChildrenAABB[i].y() - oONCInfo.m_fNodeSize / 2.0f;
					oONCInfo.m_fs3Center.z() = m_vChildrenAABB[i].z() - oONCInfo.m_fNodeSize / 2.0f;

					m_vChildren[i] = m_rOctree.NewNode();
					OctreeNodePtr pNode = m_rOctree.GetNode(m_vChildren[i]);
					bResult = (NULL != pNode) && pNode->Create(boost::any(&oONCInfo));
					if (false == bResult)
					{
						break;
					}
				}

				if (false != bResult)
				{
					OctreeNodePtr pNode = m_rOctree.GetNode(m_vChildren[i]);
					bResult = pNode->AddObject(_pObject);
				}

				if (false == bResult)
				{
					break;
				}
			}

			if (false != bResult)
			{

			}
		}

		return bResult;
	}

	bool OctreeNode::RemoveObject(OctreeObjectPtr _pObject)
	{
		bool bResult = false;

		if (0 == m_uDepthLevel)
		{
			OctreeObjectPtrVec::iterator iObject = find(m_vObjects.begin(), m_vObjects.end(), _pObject);
			if (m_vObjects.end() != iObject)
			{
				m_vObjects.erase(iObject);
				bResult = true;
			}
		}
		else
		{
			const fsVector3Vec& rvAABB = _pObject->GetAABB();
			for (UInt i = 0 ; EOctreeAABB_COUNT > i ; ++i)
			{
				if (0 != m_vChildren[i])
				{
					const UInt uIndex = i * 2;

					if (m_vChildrenAABB[i].x() < rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].x())
					{
						continue;
					}
					if (m_vChildrenAABB[i].y() < rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].y())
					{
						continue;
					}
					if (m_vChildrenAABB[i].z() < rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].z())
					{
						continue;
					}

					if (m_vChildrenAABB[i + 1].x() > rvAABB[EOctreeAABB_TOPRIGHTTFAR].x())
					{
						continue;
					}
					if (m_vChildrenAABB[i + 1].y() > rvAABB[EOctreeAABB_TOPRIGHTTFAR].y())
					{
						continue;
					}
					if (m_vChildrenAABB[i + 1].z() > rvAABB[EOctreeAABB_TOPRIGHTTFAR].z())
					{
						continue;
					}

					OctreeNodePtr pNode = m_rOctree.GetNode(m_vChildren[i]);
					bResult = pNode->RemoveObject(_pObject);
					if (false == bResult)
					{
						break;
					}
					if (0 == pNode->GetChildrenCount())
					{
						pNode->Release();
						m_rOctree.DeleteNode(m_vChildren[i]);
						m_vChildren[i] = 0;
					}
				}
			}

			if (false != bResult)
			{

			}
		}

		return bResult;
	}

	void OctreeNode::GetAABB(fsVector3Vec& _vPoints)
	{
		_vPoints = m_vPoints;
	}

	UInt OctreeNode::GetChildrenCount() const
	{
		return UInt(m_vObjects.size());
	}

	void OctreeNode::Traverse(OctreeTraverseFuncRef _rFunc, OctreeNodePtrVecRef _rvNodes, OctreeObjectPtrVecRef _rvObjects, const EOctreeTraverseResult _eOverride)
	{
		const EOctreeTraverseResult eResult = (EOctreeTraverseResult_UNKNOWN == _eOverride) ? _rFunc(*this) : _eOverride;
		if (EOctreeTraverseResult_PARTIAL == eResult)
		{
			_rvNodes.push_back(this);
			for (UInt i = 0 ; EOctreeAABB_COUNT > i ; ++i)
			{
				const UInt uIndex = m_vChildren[i];
				if (0 < uIndex)
				{
					OctreeNodePtr pNode = m_rOctree.GetNode(uIndex);
					pNode->Traverse(_rFunc, _rvNodes, _rvObjects);
				}
			}
		}
		else if (EOctreeTraverseResult_FULL == eResult)
		{
			_rvNodes.push_back(this);
			if (0 == m_uDepthLevel)
			{
				for (UInt i = 0 ; m_vObjects.size() > i ; ++i)
				{
					if (_rvObjects.end() == find(_rvObjects.begin(), _rvObjects.end(), m_vObjects[i]))
					{
						_rvObjects.push_back(m_vObjects[i]);
					}
				}
			}
			else
			{
				for (UInt i = 0 ; EOctreeAABB_COUNT > i ; ++i)
				{
					const UInt uIndex = m_vChildren[i];
					if (0 < uIndex)
					{
						OctreeNodePtr pNode = m_rOctree.GetNode(uIndex);
						pNode->Traverse(_rFunc, _rvNodes, _rvObjects, eResult);
					}
				}
			}
		}
	}

	OctreeNode& OctreeNode::operator = (const OctreeNode& _rNode)
	{
		if (this != &_rNode)
		{
			CopyData oCopyData =
			{
				m_vPoints,
				m_vChildrenAABB,
				m_vChildren,
				m_vObjects,
				m_fs3Center,
				m_fNodeSize,
				m_uDepthLevel
			};
			_rNode.CopyTo(oCopyData);
			m_rOctree = _rNode.GetOctree();
		}
		return *this;
	}

	void OctreeNode::CopyTo(CopyDataRef _rCopyData) const
	{
		_rCopyData.m_vPoints = m_vPoints;
		_rCopyData.m_vChildrenAABB = m_vChildrenAABB;
		_rCopyData.m_vChildren = m_vChildren;
		_rCopyData.m_vObjects = m_vObjects;
		_rCopyData.m_fs3Center = m_fs3Center;
		_rCopyData.m_fNodeSize = m_fNodeSize;
		_rCopyData.m_uDepthLevel = m_uDepthLevel;
	}

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
			NewNode();
			m_pRoot = GetNode(0);
			bResult = m_pRoot->Create(boost::any(&oONCInfo));
		}

		return bResult;
	}

	void Octree::Update()
	{

	}

	void Octree::Release()
	{
		m_mTraverseModes.clear();
		m_vPool.clear();
		m_vAvailable.clear();
		m_vInUse.clear();
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
		OctreeTraverseFuncMap::iterator iPair = m_mTraverseModes.find(_uModeNameKey);
		bool bResult = (m_mTraverseModes.end() != iPair);
		if (false != bResult)
		{
			OctreeTraverseFuncRef rFunc = iPair->second;
			_pStartingNode = (NULL == _pStartingNode) ? m_pRoot : _pStartingNode;
			_pStartingNode->Traverse(rFunc, _rvNodes, _rvObjects);
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
		UInt uResult = (false == m_vAvailable.empty()) ? m_vAvailable.back() : NewNode_();
		if (NULL != uResult)
		{
			m_vInUse.push_back(uResult);
			m_vAvailable.pop_back();
		}
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

	OctreeNodePtr Octree::GetNode(const UInt _uIndex)
	{
		return &m_vPool[_uIndex];
	}

	UInt Octree::NewNode_()
	{
		m_vPool.push_back(OctreeNode(*this));
		m_vAvailable.push_back(m_vPool.size());
		return m_vAvailable.back();
	}
}
