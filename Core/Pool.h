#ifndef __POOL_H__
#define __POOL_H__

#include "../Core/CoreTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	template <typename T>
	class Pool
	{
	public:
		typedef T* TPtr;

	public:
		Pool(UIntRef _uSize);
		virtual ~Pool();

		TPtr Alloc(const UInt _uCount = 1);
		void Free(TPtr _pData);
		UInt Size();

	protected:
		void AddToAvailable(TPtr _pStart, const UInt _uSize);
		void RemoveFromAvailable(TPtr _pStart, const UInt _uSize);
		void AddToInUse(TPtr _pStart, const UInt _uSize);
		UInt RemoveFromInUse(TPtr _pStart);

	protected:
		typedef map<TPtr, UInt> BlockMap;

	protected:
		TPtr		m_pBuffer;
		BlockMap	m_mAvailable;
		BlockMap	m_mInUse;
		UInt		m_uSize;
	};

	template <typename T>
	Pool<T>::Pool(UIntRef _uSize)
	:	m_pBuffer(NULL),
		m_mAvailable(),
		m_mInUse(),
		m_uSize(0)

	{
		m_pBuffer = new T[_uSize];
		if ((0 < _uSize) && (NULL != m_pBuffer))
		{
			m_uSize = _uSize;
			AddToAvailable(m_pBuffer, _uSize);
		}
	}

	template <typename T>
	Pool<T>::~Pool()
	{
		if (NULL != m_pBuffer)
		{
			delete[] m_pBuffer;
		}
		m_mAvailable.clear();
		m_mInUse.clear();
	}

	template <typename T>
	T* Pool<T>::Alloc(const UInt _uCount)
	{
		TPtr pResult = NULL;
		UInt uBestSize = 0;

		BlockMap::iterator iPair = m_mAvailable.begin();
		BlockMap::iterator iEnd = m_mAvailable.end();
		while (iEnd != iPair)
		{
			TPtr pStart = iPair->first;
			UIntRef uSize = iPair->second;
			if (_uCount == uSize)
			{
				pResult = pStart;
				break;
			}
			else if ((_uCount <= uSize) && ((uBestSize > uSize) || (NULL == pResult)))
			{
				pResult = pStart;
				uBestSize = uSize;
			}
			++iPair;
		}

		if (NULL != pResult)
		{
			RemoveFromAvailable(pResult, _uCount);
			AddToInUse(pResult, _uCount);
		}

		return pResult;
	}

	template <typename T>
	void Pool<T>::Free(T* _pData)
	{
		const UInt uSize = RemoveFromInUse(_pData);
		if (0 != uSize)
		{
			AddToAvailable(_pData, uSize);
		}
	}

	template <typename T>
	UInt Pool<T>::Size()
	{
		return m_uSize;
	}

	template <typename T>
	void Pool<T>::AddToAvailable(TPtr _pStart, const UInt _uSize)
	{
		bool bMerge = false;

		BlockMap::iterator iPair = m_mAvailable.begin();
		BlockMap::iterator iEnd = m_mAvailable.end();
		while (iEnd != iPair)
		{
			TPtr pStart = iPair->first;
			UIntRef uSize = iPair->second;
			if (_pStart == &pStart[uSize])
			{
				uSize += _uSize;
				bMerge = true;
				break;
			}
			++iPair;
		}

		if (false == bMerge)
		{
			m_mAvailable[_pStart] = _uSize;
		}
	}

	template <typename T>
	void Pool<T>::RemoveFromAvailable(TPtr _pStart, const UInt _uSize)
	{
		BlockMap::iterator iPair = m_mAvailable.find(_pStart);
		if (m_mAvailable.end() != iPair)
		{
			const UInt uSize = iPair->second;
			m_mAvailable.erase(iPair);
			if (_uSize < uSize)
			{
				m_mAvailable[&_pStart[uSize]] = uSize - _uSize;
			}
		}
	}


	template <typename T>
	void Pool<T>::AddToInUse(TPtr _pStart, const UInt _uSize)
	{
		m_mInUse[_pStart] = _uSize;
	}

	template <typename T>
	UInt Pool<T>::RemoveFromInUse(TPtr _pStart)
	{
		UInt uSize = 0;

		BlockMap::iterator iPair = m_mInUse.find(_pStart);
		if (m_mInUse.end() != iPair)
		{
			uSize = iPair->second;
			m_mInUse.erase(iPair);
		}

		return uSize;
	}
}

#endif
