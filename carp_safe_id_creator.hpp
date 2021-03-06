#ifndef CARP_SAFEIDCREATOR_INCLUDED
#define CARP_SAFEIDCREATOR_INCLUDED

#include <list>

template <typename T>
class CarpSafeIDCreator
{
public:
	CarpSafeIDCreator() { }
	~CarpSafeIDCreator() { }

public:
	T CreateID()
	{
		// 如果id列表中没有数据，那么就把最大ID加1，然后返回
		if (m_id_list.empty())
		{
			++ m_max_id;
			return m_max_id;
		}

		T id = m_id_list.front();
		m_id_list.pop_front();
		return id;
	}

	void ReleaseID(T id)
	{
		// 如果回收的id，正好是最大值，那么直接把最大值调小，然后返回即可
		if (id == m_max_id)
		{
			--m_max_id;
			return;
		}

		// 否则添加到列表中
		m_id_list.push_back(id);
	}

	size_t GetListSize() const
	{
		return m_id_list.size();
	}

	bool IsEmpty() const
	{
		if (!m_id_list.empty()) return false;
		return m_max_id >= std::numeric_limits<T>::max();
	}

	T GetMaxID() const
	{
		return m_max_id;
	}

private:
	std::list<T> m_id_list;
	T m_max_id = 0;
};

#endif
