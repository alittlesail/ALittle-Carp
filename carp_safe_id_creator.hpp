#ifndef CARP_SAFEIDCREATOR_INCLUDED
#define CARP_SAFEIDCREATOR_INCLUDED (1)

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
		// ���id�б���û�����ݣ���ô�Ͱ����ID��1��Ȼ�󷵻�
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
		// ������յ�id�����������ֵ����ôֱ�Ӱ����ֵ��С��Ȼ�󷵻ؼ���
		if (id == m_max_id)
		{
			--m_max_id;
			return;
		}

		// ������ӵ��б���
		m_id_list.push_back(id);
	}

	size_t GetListSize() const
	{
		return m_id_list.size();
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
