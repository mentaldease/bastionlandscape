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
		EOctreeAABB_TOPLEFTNEAR,
		EOctreeAABB_BOTTOMLEFTFAR,
		EOctreeAABB_BOTTOMRIGHTTFAR,
		EOctreeAABB_BOTTOMRIGHTTNEAR,
		EOctreeAABB_BOTTOMLEFTTNEAR,
		EOctreeAABB_COUNT // always last enum member
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class OctreeObject// : public CoreObject
	{
	public:
		OctreeObject(OctreeRef _rOctree);
		virtual ~OctreeObject();

		virtual void SetAABB(const fsVector3& _rf3TopRightFar, const fsVector3& _rf3BottomLeftNear);
		virtual void GetAABB(fsVector3Vec& _vPoints);

		const fsVector3Vec& GetAABB() const;
		const fsVector3& GetCenter() const;
		const float GetRadius() const;

		virtual bool RayIntersect(const fsVector3& _f3RayBegin, const fsVector3& _f3RayEnd, fsVector3& _f3Intersect1, fsVector3& _f3Intersect2);

	protected:
		bool ClipSegment(float min, float max, float a, float b, float d, float& t0, float& t1);
		bool ClipSegment(fsVector3& A, fsVector3& B, const fsVector3& Min, const fsVector3& Max);

	protected:
		fsVector3Vec	m_vPoints;
		fsVector3		m_fs3Center;
		OctreeRef		m_rOctree;
		float			m_fRadius;
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
		inline OctreeObjectPtrVecRef GetObjects();

		inline void GetAABB(fsVector3Vec& _vPoints);
		inline const fsVector3Vec& GetAABB() const;

		inline UInt GetChildrenCount() const;
		inline UIntVec& GetChildren();

		inline UInt GetDepth() const;

		inline const fsVector3& GetCenter() const;
		inline const float GetRadius() const;

		void Traverse(OctreeTraverseFuncRef _rFunc, OctreeNodePtrVecRef _rvNodes, OctreeObjectPtrVecRef _rvObjects, const EOctreeTraverseResult _eOverride = EOctreeTraverseResult_UNKNOWN);
		void TraverseNoRecursion(OctreeTraverseFuncRef _rFunc, OctreeNodePtrVecRef _rvNodes, OctreeObjectPtrVecRef _rvObjects, const EOctreeTraverseResult _eOverride = EOctreeTraverseResult_UNKNOWN);

	protected:
		fsVector3Vec		m_vPoints;
		fsVector3Vec		m_vChildrenAABB;
		UIntVec				m_vChildren;
		OctreeObjectPtrVec	m_vObjects;
		OctreeRef			m_rOctree;
		fsVector3			m_fs3Center;
		float				m_fNodeSize;
		float				m_fRadius;
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

	inline OctreeObjectPtrVecRef OctreeNode::GetObjects()
	{
		return m_vObjects;
	}

	inline void OctreeNode::GetAABB(fsVector3Vec& _vPoints)
	{
		_vPoints = m_vPoints;
	}

	inline const fsVector3Vec& OctreeNode::GetAABB() const
	{
		return m_vPoints;
	}

	inline UInt OctreeNode::GetChildrenCount() const
	{
		return UInt(m_vObjects.size());
	}

	inline UIntVec& OctreeNode::GetChildren()
	{
		return m_vChildren;
	}

	inline UInt OctreeNode::GetDepth() const
	{
		return m_uDepthLevel;
	}

	inline const fsVector3& OctreeNode::GetCenter() const
	{
		return m_fs3Center;
	}

	inline const float OctreeNode::GetRadius() const
	{
		return m_fRadius;
	}
}

#endif // __OCTREE_H__
