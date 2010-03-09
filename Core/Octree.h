#ifndef __OCTREE_H__
#define __OCTREE_H__

#include "../Core/Core.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	// use this enum to access :
	// * points retrieved from GetAABB methods
	// * OctreeNode::m_vChildren nodes 
	enum EOctreeAABB
	{
		EOctreeAABB_TOPLEFTFAR,
		EOctreeAABB_TOPRIGHTTFAR,
		EOctreeAABB_TOPRIGHTTNEAR,
		EOctreeAABB_TOPLEFTTNEAR,
		EOctreeAABB_BOTTOMLEFTFAR,
		EOctreeAABB_BOTTOMRIGHTTFAR,
		EOctreeAABB_BOTTOMRIGHTTNEAR,
		EOctreeAABB_BOTTOMLEFTTNEAR,
		EOctreeAABB_COUNT // always last enum member
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class OctreeObject : public CoreObject
	{
	public:
		OctreeObject(OctreeRef _rOctree);
		virtual ~OctreeObject();

		void SetAABB(const fsVector3& _rf3TopRightFar, const fsVector3& _rf3BottomLeftNear);
		void GetAABB(fsVector3Vec& _vPoints);
		const fsVector3Vec& GetAABB() const;

	protected:
		fsVector3Vec	m_vPoints;
		OctreeRef		m_rOctree;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class OctreeNode : public CoreObject
	{
	public:
		struct CreateInfo
		{
			fsVector3	m_fs3Center;
			float		m_fNodeSize;
			UInt		m_uDepth;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

	public:
		OctreeNode(OctreeRef _rOctree);
		virtual ~OctreeNode();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		OctreeRef GetOctree() const;

		bool AddObject(OctreeObjectPtr _pObject);
		bool RemoveObject(OctreeObjectPtr _pObject);

		void GetAABB(fsVector3Vec& _vPoints);
		UInt GetChildrenCount() const;

		void Traverse(OctreeTraverseFuncRef _rFunc, OctreeNodePtrVecRef _rvNodes, OctreeObjectPtrVecRef _rvObjects, const EOctreeTraverseResult _eOverride = EOctreeTraverseResult_UNKNOWN);

	protected:
		fsVector3Vec		m_vPoints;
		fsVector3Vec		m_vChildrenAABB;
		UIntVec				m_vChildren;
		OctreeObjectPtrVec	m_vObjects;
		OctreeRef			m_rOctree;
		fsVector3			m_fs3Center;
		float				m_fNodeSize;
		UInt				m_uDepthLevel;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Octree : public CoreObject
	{
	public:
		struct CreateInfo
		{
			fsVector3	m_fs3Center;
			float		m_fLeafSize;
			UInt		m_uDepth;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

	public:
		Octree();
		virtual ~Octree();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		bool AddTraverseMode(Key _uModeNameKey, OctreeTraverseFunc _pFunc);
		void RemoveTraverseMode(Key _uModeNameKey);
		void Traverse(Key _uModeNameKey, OctreeNodePtrVecRef _rvNodes, OctreeObjectPtrVecRef _rvObjects, OctreeNodePtr _pStartingNode = NULL);

		bool AddObject(OctreeObjectPtr _pObject);
		bool RemoveObject(OctreeObjectPtr _pObject);

		UInt NewNode();
		void DeleteNode(UInt _uIndex);
		inline OctreeNodePtr GetNode(const UInt _uIndex)
		{
			return m_vPool[_uIndex];
		}

	protected:
		UInt NewNode_();

	protected:
		OctreeTraverseFuncMap	m_mTraverseModes;
		OctreeNodePtrVec		m_vPool;
		UIntVec					m_vInUse;
		UIntVec					m_vAvailable;
		OctreeNodePtr			m_pRoot;
		fsVector3				m_fs3Center;
		float					m_fLeafSize;
		UInt					m_uDepth;
	};
}

#endif // __OCTREE_H__
