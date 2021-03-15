#ifndef CARP_TIMER_INCLUDED
#define CARP_TIMER_INCLUDED

#include <ctime>
#include <list>
#include <unordered_map>

#include "carp_miniheap.hpp"

struct CarpTimerNode
{
    time_t end_time = 0;
    int id = 0;
    int heap_index = -1;
    int loop = 0;
    long long interval_ms = 0;

    void SetHeapIndex(int index) { heap_index = index; }
    int GetHeapIndex() const { return heap_index; }
	void Reset()
    {
        end_time = 0;
        id = 0;
        heap_index = -1;
        loop = 0;
        interval_ms = 0;
    }

    bool operator > (const CarpTimerNode& node) const { return end_time > node.end_time; }
};

class CarpTimer
{
public:
	~CarpTimer() { Clear(); }
	
public:
    // ��Ӷ�ʱ��
    // delay_ms �ӳٵĺ���
    // loop С�ڻ����0����ʾ����ѭ��������0��ʾѭ������
    // interval_ms ѭ���������
    int Add(long long delay_ms, int loop, long long interval_ms)
    {
        CarpTimerNode* info = CreateNode();

        info->heap_index = -1;
        if (delay_ms < 0) delay_ms = 0;
        info->end_time = m_cur_time + delay_ms;
        if (loop <= 0) info->loop = -1;
        else info->loop = loop;
        info->interval_ms = interval_ms;
        if (info->interval_ms <= 0) info->interval_ms = 1;

        int find_count = 1000000;
        do
        {
            // ��ǰ��
            ++m_max_id;
            // ����������ֵ����ô�ص�1��ʼ
            if (m_max_id >= 0x7FFFFFFF) m_max_id = 1;

            // ���û�б�ʹ�ã���ô��������
        	if (m_map.find(m_max_id) == m_map.end())
            {
                info->id = m_max_id;
                break;
            }

            // ��������
            --find_count;
        } while (find_count > 0);

        if (info->id == 0 || !m_heap.Push(info))
        {
            ReleaseNode(info);
            return 0;
        }

        m_map[info->id] = info;
        return info->id;
    }
    // �Ƴ���ʱ��
    bool Remove(int id)
    {
        const auto it = m_map.find(id);
        if (it == m_map.end()) return false;

        m_heap.Erase(it->second);
        ReleaseNode(it->second);

        m_map.erase(it);
        return true;
    }
    // ����ʱ��
    void Update(long long frame_time)
    {
        m_cur_time += frame_time;
    }
    // ����Ϊ��ǰʱ��
    void UpdateCurTime()
    {
        m_cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
    // ȡ����ʱ�Ķ�ʱ��ID
    // �������0��˵��û�г�ʱ�Ķ�ʱ��
    // �������������˵����ʱ����ʱ�����ҵȴ��´�ѭ��
    // ������ظ�����˵����ʱ����ʱ�����Ҷ�ʱ���ѱ�ɾ��
    int Poll()
    {
        CarpTimerNode* info = m_heap.GetTop();
        if (info == nullptr) return 0;

        if (info->end_time > m_cur_time) return 0;
        m_heap.Pop();

        // ��������ѭ��
        if (info->loop < 0)
        {
            info->end_time = info->end_time + info->interval_ms;
            m_heap.Push(info);
            return info->id;
        }
        // ������д�������ô�Ϳ۳�����
        if (info->loop > 1)
        {
            --info->loop;
            info->end_time = info->end_time + info->interval_ms;
            m_heap.Push(info);
            return info->id;
        }
        // �����Ѿ����꣬��ô��ֱ��ɾ��
        const int id = info->id;
        const auto it = m_map.find(id);
    	if (it != m_map.end())
    	{
            ReleaseNode(it->second);
            m_map.erase(it);
    	}
        return -id;
    }
    // ����
    void Clear()
    {
        for (auto& pair : m_map)
            delete pair.second;
        m_map.clear();
        for (auto* node : m_pool)
            delete node;
        m_pool.clear();
    }

private:
	CarpTimerNode* CreateNode()
	{
        CarpTimerNode* node = nullptr;
        if (m_pool.empty())
        {
            node = new CarpTimerNode();
        }
        else
        {
            node = m_pool.back();
            m_pool.pop_back();
            node->Reset();
        }

        return node;
	}

	void ReleaseNode(CarpTimerNode* node)
	{
        if (m_pool.size() > 1000)
        {
            delete node;
            return;
        }

        m_pool.push_back(node);
	}
	
private:
    time_t m_cur_time = 0;
    int m_max_id = 0;
    CarpMiniHeap<CarpTimerNode> m_heap;
    std::vector<CarpTimerNode*> m_pool;
    std::unordered_map<int, CarpTimerNode*> m_map;
};

#endif