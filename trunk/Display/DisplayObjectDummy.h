#ifndef __DISPLAYOBJECTDUMMY_H__
#define __DISPLAYOBJECTDUMMY_H__

#include "../Display/DisplayTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayObjectDummy : public DisplayObject
	{
	public:
		DisplayObjectDummy();
		virtual ~DisplayObjectDummy();

		virtual void Release();

		virtual void SetWorldMatrix(MatrixRef _rWorld);
		virtual void RenderBegin();
		virtual void Render();
		virtual void RenderEnd();
		virtual bool RayIntersect(const Vector3& _f3RayBegin, const Vector3& _f3RayEnd, Vector3& _f3Intersect);

		void SetObject(DisplayObjectPtr	_pObject);

	protected:
		Matrix				m_m4WorldSave;
		DisplayObjectPtr	m_pActualObject;
	};
}

#endif // __DISPLAYOBJECTDUMMY_H__
