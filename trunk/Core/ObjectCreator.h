#ifndef __OBJECTCREATOR_H__
#define __OBJECTCREATOR_H__

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	template<typename T>
	class ObjectCreator : public CoreObject
	{
	public:
		typedef T* TPtr;
		typedef map<Key, TPtr> TPtrMap;

		virtual void Release();

	public:
		ObjectCreator();
		virtual ~ObjectCreator();

		inline Key New(const boost::any& _rConfig);
		inline void Delete(Key _uID);
		inline TPtr Get(Key _uID);

	protected:
		TPtrMap	m_mAvailable;
		TPtrMap	m_mInUse;
		Key		m_uNexID;
	};

	template<typename T>
	ObjectCreator<T>::ObjectCreator()
	:	CoreObject(),
		m_mAvailable(),
		m_mInUse(),
		m_uNexID(0)
	{

	}

	template<typename T>
	ObjectCreator<T>::~ObjectCreator()
	{

	}

	template<typename T>
	void ObjectCreator<T>::Release()
	{
		while (false == m_mAvailable.empty())
		{
			TPtr pObject = m_mAvailable.begin()->second;
			T::DeleteInstance(pObject);
			m_mAvailable.erase(m_mAvailable.begin());
		}
		while (false == m_mInUse.empty())
		{
			TPtr pObject = m_mInUse.begin()->second;
			pObject->Release();
			T::DeleteInstance(pObject);
			m_mAvailable.erase(m_mInUse.begin());
		}
		m_uNexID = 0;
	}

	template<typename T>
	Key ObjectCreator<T>::New(const boost::any& _rConfig)
	{
		Key uResult = 0;
		if (false == m_mAvailable.empty())
		{
			TPtrMap::iterator iPair = m_mAvailable.begin();
			if (false != iPair->second->Create(_rConfig))
			{
				uResult = iPair->first;
				m_mInUse[uResult] = iPair->second;
				m_mAvailable.erase(iPair);
			}
		}
		else
		{
			TPtr pObject = T::NewInstance();
			if (false != pObject->Create(_rConfig))
			{
				uResult = ++m_uNexID;
				m_mInUse[uResult] = pObject;
			}
			else
			{
				pObject->Release();
				T::DeleteInstance(pObject);
			}
		}
		return uResult;
	}

	template<typename T>
	void ObjectCreator<T>::Delete(Key _uID)
	{
		TPtrMap::iterator iPair = m_mInUse.find(_uID);
		if (m_mInUse.end() != iPair)
		{
			m_mAvailable[_uID] = iPair->second;
			m_mAvailable[_uID]->Release();
			m_mInUse.erase(iPair);
		}
	}

	template<typename T>
	T* ObjectCreator<T>::Get(Key _uID)
	{
		return (m_mInUse.end() != m_mInUse.find(_uID)) ? m_mInUse[_uID] : NULL;
	}
}

#endif // __OBJECTCREATOR_H__
