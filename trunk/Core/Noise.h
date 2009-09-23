#ifndef __NOISE_H__
#define __NOISE_H__

#include "../Core/Core.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class NoiseGenerator : public CoreObject
	{
	public:
		NoiseGenerator();
		virtual ~NoiseGenerator();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		void Process(const int& _sWidth, const int& _sHeight, FloatPtr _pData = NULL);
		const FloatPtr GetData() const;
		int GetStride() const;

	protected:
		struct Make2DInfo;
		typedef Make2DInfo*	Make2DInfoPtr;
		typedef Make2DInfo&	Make2DInfoRef;

		Make2DInfoPtr	m_pMake2DInfo;

	private:
	};
}

#endif // __NOISE_H__