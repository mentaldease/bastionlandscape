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
		m_vChildren(),
		m_vObjects(),
		m_rOctree(_rOctree),
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

			#define SETPOINT(ID, CENTER, XOFFSET, YOFFSET, ZOFFSET) \
				m_vPoints[ID].x() = CENTER.x() + (XOFFSET); \
				m_vPoints[ID].y() = CENTER.y() + (YOFFSET); \
				m_vPoints[ID].z() = CENTER.z() + (ZOFFSET);

			SETPOINT(EOctreeAABB_TOPLEFTFAR, rfs3Center, -m_fNodeSize, m_fNodeSize, m_fNodeSize);
			SETPOINT(EOctreeAABB_TOPRIGHTTFAR, rfs3Center, m_fNodeSize, m_fNodeSize, m_fNodeSize);
			SETPOINT(EOctreeAABB_TOPRIGHTTNEAR, rfs3Center, m_fNodeSize, m_fNodeSize, -m_fNodeSize);
			SETPOINT(EOctreeAABB_TOPLEFTTNEAR, rfs3Center, -m_fNodeSize, m_fNodeSize, -m_fNodeSize);

			SETPOINT(EOctreeAABB_BOTTOMLEFTFAR, rfs3Center, -m_fNodeSize, -m_fNodeSize, m_fNodeSize);
			SETPOINT(EOctreeAABB_BOTTOMRIGHTTFAR, rfs3Center, m_fNodeSize, -m_fNodeSize, m_fNodeSize);
			SETPOINT(EOctreeAABB_BOTTOMRIGHTTNEAR, rfs3Center, m_fNodeSize, -m_fNodeSize, -m_fNodeSize);
			SETPOINT(EOctreeAABB_BOTTOMLEFTTNEAR, rfs3Center, -m_fNodeSize, -m_fNodeSize, -m_fNodeSize);

			#undef SETPOINT

			#define SETAABB(ID, CENTER, OFFSET) \
				m_vChildrenAABB[(ID) + 0].x() = CENTER.x() + (OFFSET); \
				m_vChildrenAABB[(ID) + 0].y() = CENTER.y() + (OFFSET); \
				m_vChildrenAABB[(ID) + 0].z() = CENTER.z() + (OFFSET); \
				m_vChildrenAABB[(ID) + 1].x() = CENTER.x() - (OFFSET); \
				m_vChildrenAABB[(ID) + 1].y() = CENTER.y() - (OFFSET); \
				m_vChildrenAABB[(ID) + 1].z() = CENTER.z() - (OFFSET);

			const float fHalfSize = m_fNodeSize / 2.0f;
			const float fQuarterSize = m_fNodeSize / 4.0f;

			const fsVector3 fs3TLF(rfs3Center.x() - fHalfSize, rfs3Center.y() + fHalfSize, rfs3Center.z() + fHalfSize);
			SETAABB(EOctreeAABB_TOPLEFTFAR * 2, fs3TLF, fQuarterSize);
			const fsVector3 fs3TRF(rfs3Center.x() + fHalfSize, rfs3Center.y() + fHalfSize, rfs3Center.z() + fHalfSize);
			SETAABB(EOctreeAABB_TOPRIGHTTFAR * 2, fs3TRF, fQuarterSize);
			const fsVector3 fs3TRN(rfs3Center.x() + fHalfSize, rfs3Center.y() + fHalfSize, rfs3Center.z() - fHalfSize);
			SETAABB(EOctreeAABB_TOPRIGHTTNEAR * 2, fs3TRN, fQuarterSize);
			const fsVector3 fs3TLN(rfs3Center.x() - fHalfSize, rfs3Center.y() + fHalfSize, rfs3Center.z() - fHalfSize);
			SETAABB(EOctreeAABB_TOPLEFTTNEAR * 2, fs3TLN, fQuarterSize);

			const fsVector3 fs3BLF(rfs3Center.x() - fHalfSize, rfs3Center.y() - fHalfSize, rfs3Center.z() + fHalfSize);
			SETAABB(EOctreeAABB_BOTTOMLEFTFAR * 2, fs3BLF, fQuarterSize);
			const fsVector3 fs3BRF(rfs3Center.x() + fHalfSize, rfs3Center.y() - fHalfSize, rfs3Center.z() + fHalfSize);
			SETAABB(EOctreeAABB_BOTTOMRIGHTTFAR * 2, fs3BRF, fQuarterSize);
			const fsVector3 fs3BRN(rfs3Center.x() + fHalfSize, rfs3Center.y() - fHalfSize, rfs3Center.z() - fHalfSize);
			SETAABB(EOctreeAABB_BOTTOMRIGHTTNEAR * 2, fs3BRN, fQuarterSize);
			const fsVector3 fs3BLN(rfs3Center.x() - fHalfSize, rfs3Center.y() - fHalfSize, rfs3Center.z() - fHalfSize);
			SETAABB(EOctreeAABB_BOTTOMLEFTTNEAR * 2, fs3BLN, fQuarterSize);

			#undef SETAABB

			m_vChildren.resize(EOctreeAABB_COUNT, NULL);
		}

		return bResult;
	}

	void OctreeNode::Update()
	{

	}

	void OctreeNode::Release()
	{
		OctreeNodePtrVec::iterator iNode = m_vChildren.begin();
		OctreeNodePtrVec::iterator iEnd = m_vChildren.end();
		while (iEnd != iNode)
		{
			m_rOctree.DeleteNode(*iNode);
			++iNode;
		}
		m_vChildren.clear();
		m_vObjects.clear();
	}

	bool OctreeNode::AddObject(OctreeObjectPtr _pObject)
	{
		bool bResult = false;

		if (1 == m_uDepthLevel)
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

				if (NULL == m_vChildren[i])
				{
					OctreeNode::CreateInfo oONCInfo;
					oONCInfo.m_fNodeSize = m_fNodeSize / 2.0f;
					oONCInfo.m_uDepth = m_uDepthLevel - 1;
					oONCInfo.m_fs3Center.x() = m_vChildrenAABB[i].x() - oONCInfo.m_fNodeSize / 2.0f;
					oONCInfo.m_fs3Center.y() = m_vChildrenAABB[i].y() - oONCInfo.m_fNodeSize / 2.0f;
					oONCInfo.m_fs3Center.z() = m_vChildrenAABB[i].z() - oONCInfo.m_fNodeSize / 2.0f;

					m_vChildren[i] = m_rOctree.NewNode();
					bResult = m_vChildren[i]->Create(boost::any(&oONCInfo));
					if (false == bResult)
					{
						break;
					}
				}

				if (false != bResult)
				{
					bResult = m_vChildren[i]->AddObject(_pObject);
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

		if (1 == m_uDepthLevel)
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
				if (NULL != m_vChildren[i])
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

					bResult = m_vChildren[i]->RemoveObject(_pObject);
					if (false == bResult)
					{
						break;
					}
					if (0 == m_vChildren[i]->GetChildrenCount())
					{
						m_vChildren[i]->Release();
						m_rOctree.DeleteNode(m_vChildren[i]);
						m_vChildren[i] = NULL;
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
		return false;
	}

	void Octree::Update()
	{

	}

	void Octree::Release()
	{

	}

	bool Octree::AddTraverseMode(KeyRef _uModeNameKey, OctreeTraverseFunc _pFunc)
	{
		return false;
	}

	void Octree::RemoveTraverseMode(KeyRef _uModeNameKey)
	{

	}

	void Octree::Traverse(KeyRef _uModeNameKey, OctreeNodePtr _pStartingNode)
	{

	}

	bool Octree::AddObject(OctreeObjectPtr _pObject)
	{
		return false;
	}

	bool Octree::RemoveObject(OctreeObjectPtr _pObject)
	{
		return false;
	}

	OctreeNodePtr Octree::NewNode()
	{
		return NULL;
	}

	void Octree::DeleteNode(OctreeNodePtr _pNode)
	{

	}
}
