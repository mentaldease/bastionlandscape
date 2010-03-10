#include "stdafx.h"
#include "../Core/Core.h"
#include "../Core/Octree.h"
#include "../Core/Util.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	OctreeObject::OctreeObject(OctreeRef _rOctree)
	:	//CoreObject(),
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
		//vsoutput("aabb (%f %f %f) (%f %f %f)\n",
		//	_rf3BottomLeftNear.x(), _rf3BottomLeftNear.y(), _rf3BottomLeftNear.z(),
		//	_rf3TopRightFar.x(), _rf3TopRightFar.y(), _rf3TopRightFar.z());

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
		m_vChildren.resize(EOctreeAABB_COUNT, 0);
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
			OctreeNodePtr pNode = NULL;
			const fsVector3Vec& rvAABB = _pObject->GetAABB();
			//vsoutput("object (%f %f %f) (%f %f %f)\n",
			//	rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].x(), rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].y(), rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].z(),
			//	rvAABB[EOctreeAABB_TOPRIGHTTFAR].x(), rvAABB[EOctreeAABB_TOPRIGHTTFAR].y(), rvAABB[EOctreeAABB_TOPRIGHTTFAR].z());
			//vsoutput("node (%f %f %f) (%f %f %f)\n",
			//	m_vPoints[EOctreeAABB_BOTTOMLEFTTNEAR].x(), m_vPoints[EOctreeAABB_BOTTOMLEFTTNEAR].y(), m_vPoints[EOctreeAABB_BOTTOMLEFTTNEAR].z(),
			//	m_vPoints[EOctreeAABB_TOPRIGHTTFAR].x(), m_vPoints[EOctreeAABB_TOPRIGHTTFAR].y(), m_vPoints[EOctreeAABB_TOPRIGHTTFAR].z());

			for (UInt i = 0 ; EOctreeAABB_COUNT > i ; ++i)
			{
				const UInt uIndex = i * 2;

				//vsoutput("child #%d (%f %f %f) (%f %f %f)\n", i,
				//	m_vChildrenAABB[uIndex + 1].x(), m_vChildrenAABB[uIndex + 1].y(), m_vChildrenAABB[uIndex + 1].z(),
				//	m_vChildrenAABB[uIndex].x(), m_vChildrenAABB[uIndex].y(), m_vChildrenAABB[uIndex].z());

				if (m_vChildrenAABB[uIndex].x() < rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].x())
				{
					//vsoutput("child #%d reject x %f < %f \n", i, m_vChildrenAABB[uIndex].x(), rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].x());
					continue;
				}
				if (m_vChildrenAABB[uIndex].y() < rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].y())
				{
					//vsoutput("child #%d reject y %f < %f \n", i, m_vChildrenAABB[uIndex].y(), rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].y());
					continue;
				}
				if (m_vChildrenAABB[uIndex].z() < rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].z())
				{
					//vsoutput("child #%d reject z %f < %f \n", i, m_vChildrenAABB[uIndex].z(), rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].z());
					continue;
				}

				if (m_vChildrenAABB[uIndex + 1].x() > rvAABB[EOctreeAABB_TOPRIGHTTFAR].x())
				{
					//vsoutput("child #%d reject x %f > %f \n", i, m_vChildrenAABB[uIndex + 1].x(), rvAABB[EOctreeAABB_TOPRIGHTTFAR].x());
					continue;
				}
				if (m_vChildrenAABB[uIndex + 1].y() > rvAABB[EOctreeAABB_TOPRIGHTTFAR].y())
				{
					//vsoutput("child #%d reject y %f > %f \n", i, m_vChildrenAABB[uIndex + 1].y(), rvAABB[EOctreeAABB_TOPRIGHTTFAR].y());
					continue;
				}
				if (m_vChildrenAABB[uIndex + 1].z() > rvAABB[EOctreeAABB_TOPRIGHTTFAR].z())
				{
					//vsoutput("child #%d reject z %f > %f \n", i, m_vChildrenAABB[uIndex + 1].z(), rvAABB[EOctreeAABB_TOPRIGHTTFAR].z());
					continue;
				}

				bResult = (0 != m_vChildren[i]);
				if (false == bResult)
				{
					OctreeNode::CreateInfo oONCInfo;
					oONCInfo.m_fNodeSize = m_fNodeSize / 2.0f;
					oONCInfo.m_uDepth = m_uDepthLevel - 1;
					oONCInfo.m_fs3Center.x() = m_vChildrenAABB[uIndex].x() - oONCInfo.m_fNodeSize / 2.0f;
					oONCInfo.m_fs3Center.y() = m_vChildrenAABB[uIndex].y() - oONCInfo.m_fNodeSize / 2.0f;
					oONCInfo.m_fs3Center.z() = m_vChildrenAABB[uIndex].z() - oONCInfo.m_fNodeSize / 2.0f;

					m_vChildren[i] = m_rOctree.NewNode();
					pNode = m_rOctree.GetNode(m_vChildren[i]);
					bResult = (NULL != pNode) && pNode->Create(boost::any(&oONCInfo));
					if (false == bResult)
					{
						break;
					}
					//vsoutput("child #%d created\n");
				}

				if (false != bResult)
				{
					pNode = m_rOctree.GetNode(m_vChildren[i]);
					bResult = pNode->AddObject(_pObject);
				}

				if (false == bResult)
				{
					break;
				}
			}

			if (false == bResult)
			{
				UInt a = 0;
				++a;
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

					if (m_vChildrenAABB[uIndex].x() < rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].x())
					{
						continue;
					}
					if (m_vChildrenAABB[uIndex].y() < rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].y())
					{
						continue;
					}
					if (m_vChildrenAABB[uIndex].z() < rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR].z())
					{
						continue;
					}

					if (m_vChildrenAABB[uIndex + 1].x() > rvAABB[EOctreeAABB_TOPRIGHTTFAR].x())
					{
						continue;
					}
					if (m_vChildrenAABB[uIndex + 1].y() > rvAABB[EOctreeAABB_TOPRIGHTTFAR].y())
					{
						continue;
					}
					if (m_vChildrenAABB[uIndex + 1].z() > rvAABB[EOctreeAABB_TOPRIGHTTFAR].z())
					{
						continue;
					}

					OctreeNodePtr pNode = m_rOctree.GetNode(m_vChildren[i]);
					bResult = (NULL != pNode) && pNode->RemoveObject(_pObject);
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

	const fsVector3Vec& OctreeNode::GetAABB() const
	{
		return m_vPoints;
	}

	UInt OctreeNode::GetChildrenCount() const
	{
		return UInt(m_vObjects.size());
	}

	void OctreeNode::Traverse(OctreeTraverseFuncRef _rFunc, OctreeNodePtrVecRef _rvNodes, OctreeObjectPtrVecRef _rvObjects, const EOctreeTraverseResult _eOverride)
	{
		const EOctreeTraverseResult eResult = (EOctreeTraverseResult_UNKNOWN == _eOverride) ? _rFunc(*this) : _eOverride;
		if (0 == m_uDepthLevel)
		{
			if (EOctreeTraverseResult_NONE != eResult)
			{
				_rvNodes.push_back(this);
				for (UInt i = 0 ; m_vObjects.size() > i ; ++i)
				{
					OctreeObjectPtr pObject = m_vObjects[i];
					if (_rvObjects.end() == find(_rvObjects.begin(), _rvObjects.end(), pObject))
					{
						_rvObjects.push_back(pObject);
					}
				}
			}
		}
		else if (EOctreeTraverseResult_PARTIAL == eResult)
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
			OctreeNodePtr pNode = *iNode;
			pNode->Release();
			delete pNode;
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
		return m_vAvailable.back();
	}
}
