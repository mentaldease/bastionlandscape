#ifndef __GEOMETRYHELPER_H__
#define __GEOMETRYHELPER_H__

#include "../Core/Core.h"
#include "../Display/DisplayTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct GeometryHelperVertex
	{
		static VertexElement s_VertexElement[5];

		GeometryHelperVertexRef operator = (GeometryHelperVertexRef _rVertexIndependent);

		Vector3	m_oPosition;
		Vector3	m_oNormal;
		Vector2	m_oUV;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayGeometrySphere : public DisplayObject
	{
	public:
		struct CreateInfo
		{
			Vector4	m_f4Color;
			Vector3	m_oPos;
			Vector3	m_oRot;
			Vector3	m_oRadius;
			UInt	m_uHorizSlices; // for one hemisphere
			UInt	m_uVertSlices;
			bool	m_bTopHemisphere;
			bool	m_bBottomHemisphere;
			bool	m_bViewFromInside;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

	public:
		DisplayGeometrySphere();
		virtual ~DisplayGeometrySphere();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual void RenderBegin();
		virtual void Render();

	protected:
		bool CreateBuffers(CreateInfoRef _rInfo);
		bool FillVertexBuffer(CreateInfoRef _rInfo);
		bool FillIndexBuffer(CreateInfoRef _rInfo);

	protected:
		DisplayVertexBufferPtr	m_pVertexBuffer;
		DisplayIndexBufferPtr	m_pIndexBuffer;
		UInt					m_uVertexCount;
		UInt					m_uIndexCount;
		UInt					m_uFanToStripSize;
		Vector4					m_f4Color;
	};
}

#endif // __GEOMETRYHELPER_H__