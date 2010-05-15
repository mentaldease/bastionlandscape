#ifndef __GEOMETRYHELPER_H__
#define __GEOMETRYHELPER_H__

#include "../Core/Core.h"
#include "../Core/Octree.h"
#include "../Display/Display.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct GeometryHelperVertex
	{
		static VertexElement s_VertexElement[4];

		Vector3	m_f3Position;
		Vector3	m_f3Normal;
		Vector2	m_f2UV;
	};

	struct GeometryHelperVertexColor
	{
		static VertexElement s_VertexElement[5];

		Vector3	m_f3Position;
		Vector3	m_f3Normal;
		Vector2	m_f2UV;
		Vector4	m_f4Color;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayGeometrySphere : public DisplayObject, public OctreeObject
	{
	public:
		struct CreateInfo
		{
			Vector4	m_f4Color;
			Vector3	m_f3Pos;
			Vector3	m_f3Rot;
			Vector3	m_f3Radius;
			UInt	m_uHorizSlices; // for one hemisphere
			UInt	m_uVertSlices;
			bool	m_bTopHemisphere;
			bool	m_bBottomHemisphere;
			bool	m_bViewFromInside;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

	public:
		DisplayGeometrySphere(OctreeRef _rOctree);
		virtual ~DisplayGeometrySphere();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual void RenderBegin();
		virtual void Render();
		virtual bool RayIntersect(const Vector3& _f3RayBegin, const Vector3& _f3RayEnd, Vector3& _f3Intersect);

	protected:
		bool CreateBuffers(CreateInfoRef _rInfo);
		bool FillVertexBuffer(CreateInfoRef _rInfo);
		bool FillIndexBuffer(CreateInfoRef _rInfo);
		bool CreateBoundingMesh(CreateInfoRef _rInfo);

	protected:
		Vector4					m_f4Color;
		WordPtr					m_pIndex16;
		UIntPtr					m_pIndex32;
		GeometryHelperVertexPtr	m_pVertex;
		Key						m_uVertexBuffer;
		Key						m_uIndexBuffer;
		UInt					m_uVertexCount;
		UInt					m_uIndexCount;
		UInt					m_uFanToStripSize;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayGeometryLineManager : public DisplayObject
	{
	public:
		struct CreateInfo
		{
			Vector3	m_f3Pos;
			Vector3	m_f3Rot;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

		struct LineStripInfo
		{
			GeometryHelperVertexColorVec	m_vVertexBuffer;
			UIntVec							m_vIndexBuffer;
		};
		typedef LineStripInfo* LineStripInfoPtr;
		typedef LineStripInfo& LineStripInfoRef;
		typedef vector<LineStripInfo> LineStripInfoVec;
		typedef vector<LineStripInfoPtr> LineStripInfoPtrVec;

	public:
		DisplayGeometryLineManager();
		virtual ~DisplayGeometryLineManager();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual void RenderBegin();
		virtual void Render();
		virtual void RenderEnd();

		LineStripInfoRef NewLineStrip();
		void NewTriangle(const Vector3& _f3Point0, const Vector3& _f3Point1, const Vector3& _f3Point2, const Vector4& _f4Color);
		void NewAABB(const Vector3& _f3TopRightFar, const Vector3& _f3BottomLeftNear, const Vector4& _f4Color);
		void NewBoundingMesh(DisplayObject::BoundingMeshRef _rBoundingMesh, const Vector4& _f4Color);

	protected:
		LineStripInfoPtrVec	m_vLineStrips;
		Key					m_uVertDecl;
		UInt				m_uCurrentLineStripIndex;
	};
}

#endif // __GEOMETRYHELPER_H__