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
    int interval_ms = 0;

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
	~CarpTimer()
	{
		for (auto& pair : m_map)
            delete pair.second;
        for (auto* node : m_pool)
            delete node;
	}
	
public:
    // 添加定时器
    int Add(int delay_ms, int loop, int interval_ms)
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
            // 往前加
            ++m_max_id;
            // 如果到达最大值，那么回到1开始
            if (m_max_id >= 0x7FFFFFFF) m_max_id = 1;

            // 如果没有被使用，那么可以跳出
        	if (m_map.find(m_max_id) == m_map.end())
            {
                info->id = m_max_id;
                break;
            }

            // 继续查找
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
    // 移除定时器
    bool Remove(int id)
    {
        const auto it = m_map.find(id);
        if (it == m_map.end()) return false;

        m_heap.Erase(it->second);
        ReleaseNode(it->second);

        m_map.erase(it);
        return true;
    }
    // 更新时间
    void Update(int frame_ms)
    {
        m_cur_time += frame_ms;
    }
    // 取出超时的定时器ID
    // 如果返回0，说明没有超时的定时器
    // 如果返回正数，说明定时器超时，并且等待下次循环
    // 如果返回负数，说明定时器超时，并且定时器已被删除
    int Poll()
    {
        CarpTimerNode* info = m_heap.GetTop();
        if (info == nullptr) return 0;

        if (info->end_time > m_cur_time) return 0;
        m_heap.Pop();

        // 处理无限循环
        if (info->loop < 0)
        {
            info->end_time = info->end_time + info->interval_ms;
            m_heap.Push(info);
            return info->id;
        }
        // 如果还有次数，那么就扣除次数
        
        if (info->loop > 1)
        {
            --info->loop;
            info->end_time = info->end_time + info->interval_ms;
            m_heap.Push(info);
            return info->id;
        }
        // 次数已经用完，那么就直接删除
        
        const int id = info->id;
        const auto it = m_map.find(id);
    	if (it != m_map.end())
    	{
            ReleaseNode(it->second);
            m_map.erase(it);
    	}
        return -id;
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