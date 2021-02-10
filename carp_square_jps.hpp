#ifndef CARP_GRID_JPS_INCLUDED
#define CARP_GRID_JPS_INCLUDED

#include <cstdlib>
#include <cstring>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "carp_miniheap.hpp"

// 直线的代价
const int s_carp_square_straight_cost = 10;
// 斜线的代价
const int s_carp_square_hypotenuse_cost = 10;

// 跳点
struct CarpSquareJPSNode
{
	// v_x: 当前x坐标
	// v_y: 当前y坐标
	// v_cost: 从起点到当前坐标的总代价
	// end_x: 终点x坐标
	// end_y: 终点y坐标
	// v_parent: 父节点
	void Init(int v_x, int v_y, int v_cost, int end_x, int end_y, CarpSquareJPSNode* v_parent)
	{
		x = v_x;
		y = v_y;

		g = v_cost;
		const auto delta_x = std::abs(v_x - end_x);
		const auto delta_y = std::abs(v_y - end_y);
		if (delta_x < delta_y)
			f = g + delta_x * s_carp_square_hypotenuse_cost + delta_y * s_carp_square_straight_cost;
		else
			f = g + delta_y * s_carp_square_hypotenuse_cost + delta_x * s_carp_square_straight_cost;

		parent = v_parent;

		heap_index = -1;
	}

	// 堆下标
	int heap_index = -1;

	// 节点坐标
	int x = 0;
	int y = 0;

	// 代价
	int g = 0;
	int f = 0;

	// 父节点
	CarpSquareJPSNode* parent = nullptr;

    void SetHeapIndex(int index) { heap_index = index; }
    int GetHeapIndex() const { return heap_index; }
    void Reset() { heap_index = -1; }

    bool operator > (const CarpSquareJPSNode& node) const { return f > node.f; }
};

class CarpSquareJPS
{
public:
	CarpSquareJPS(int width, int height)
	{
		m_width = width;
		if (m_width <= 0) m_width = 1;

		m_height = height;
		if (m_height <= 0) m_height = 1;

		// 一个size_t有多少个bit
		const auto size_bit_count = 8 * sizeof(size_t);

		// 计算横向size_t数量
		m_x_size_t_count = (m_width + size_bit_count - 1) / size_bit_count;

		// 获取地图容量
		const auto byte_count = m_x_size_t_count * m_height * sizeof(size_t);
		m_map = static_cast<size_t*>(malloc(byte_count));
		memset(m_map, 0, byte_count);
	}

	virtual ~CarpSquareJPS()
	{
		free(m_map);
		for (auto& node : m_node_pool)
			delete node;
	}

public:
	bool SearchRoute(int start_x, int start_y, int end_x, int end_y, std::list<int>& x_list, std::list<int>& y_list)
	{
		// 调整参数
		if (start_x < 0) start_x = 0;
		else if (start_x >= m_width) start_x = m_width - 1;
		if (start_y < 0) start_y = 0;
		else if (start_y >= m_height) start_x = m_height - 1;

		if (end_x < 0) end_x = 0;
		else if (end_x >= m_width) end_x = m_width - 1;
		if (end_y < 0) end_y = 0;
		else if (end_y >= m_height) end_x = m_height - 1;

		// 如果起点和终点都有阻挡，那么就是找不到
		if (!IsEmptyImpl(start_x, start_y) || !IsEmptyImpl(end_x, end_y)) return false;

		// 跳点列表
        CarpMiniHeap<CarpSquareJPSNode> open_list;
		std::unordered_set<long long> open_set;
		// 关闭的跳点
		std::unordered_map<long long, CarpSquareJPSNode*> close_map;

		// 将起点添加到跳点列表
		{
			auto* node = new CarpSquareJPSNode();
			node->Init(start_x, start_y, 0, end_x, end_y, nullptr);
			open_list.Push(node);
			open_set.insert(static_cast<long long>(start_x) << 32 | start_y);
		}

		CarpSquareJPSNode* end_node = nullptr;
		while (true)
		{
		    // 取出cost最小的跳点
			auto* cur = open_list.Pop();
			if (cur == nullptr) break;
			// 计算key
			const auto cur_key = static_cast<long long>(cur->x) << 32 | cur->y;
			// 从跳点集合总删除，添加到关闭列表
			open_set.erase(cur_key);
			close_map[cur_key] = cur;

			// 如果是终点那么就跳出
			if (cur->x == end_x && cur->y == end_y)
			{
				end_node = cur;
				break;
			}

			// 如果没有父节点，那么就要搜索8个方向
			if (cur->parent == nullptr)
			{
				const auto dir_x = cur->x < end_x ? 1 : -1;
				// 水平正向搜索
				SearchHDir(cur, end_x, end_y, dir_x, open_list, open_set, close_map);
				// 水平负向搜索
				SearchHDir(cur, end_x, end_y, -dir_x, open_list, open_set, close_map);

			    const auto dir_y = cur->y < end_y ? 1 : -1;
				// 垂直正向搜索
				SearchVDir(cur, end_x, end_y, dir_y, open_list, open_set, close_map);
				// 垂直负向搜索
				SearchVDir(cur, end_x, end_y, -dir_y, open_list, open_set, close_map);

				// 正正斜
				SearchHVDir(cur, end_x, end_y, dir_x, dir_y, open_list, open_set, close_map);
				// 正负斜
				SearchHVDir(cur, end_x, end_y, dir_x, -dir_y, open_list, open_set, close_map);
				// 负正斜
				SearchHVDir(cur, end_x, end_y, -dir_x, dir_y, open_list, open_set, close_map);
				// 负负斜
				SearchHVDir(cur, end_x, end_y, -dir_x, -dir_y, open_list, open_set, close_map);
			}
			else
			{
				// 如果是水平直线方向
				if (cur->parent->y == cur->y)
				{
					const auto dir = cur->parent->x < cur->x ? 1 : -1;

					bool has_left_target = false;
					// cur左后方不可走 且 左方可走
					{
						const int target_x = cur->x - dir;
						const int target_y = cur->y - dir;
						if (target_x >= 0 && target_x < m_width && target_y >= 0 && target_y < m_height && !IsEmptyImpl(target_x, target_y) && IsEmptyImpl(cur->x, target_y))
							has_left_target = true;
					}

				    // cur右后方不可走 且 右方可走
					bool has_right_target = false;
					{
						const int target_x = cur->x - dir;
						const int target_y = cur->y + dir;
						if (target_x >= 0 && target_x < m_width && target_y >= 0 && target_y < m_height && !IsEmptyImpl(target_x, target_y) && IsEmptyImpl(cur->x, target_y))
							has_right_target = true;
					}

					// 当前方向
					SearchHDir(cur, end_x, end_y, dir, open_list, open_set, close_map);
					// 左方寻找
					if (has_left_target) SearchVDir(cur, end_x, end_y, -dir, open_list, open_set, close_map);
					// 右方寻找
					if (has_right_target) SearchVDir(cur, end_x, end_y, dir, open_list, open_set, close_map);
					// 左前方寻找
					if (has_left_target) SearchHVDir(cur, end_x, end_y, dir, -dir, open_list, open_set, close_map);
					// 右前方寻找
					if (has_right_target) SearchHVDir(cur, end_x, end_y, dir, dir, open_list, open_set, close_map);
				}
				// 如果是垂直方向
				else if (cur->parent->x == cur->x)
				{
					const auto dir = cur->parent->y < cur->y ? 1 : -1;

					// cur左后方不可走 且 左方可走
					bool has_left_target = false;
					{
						const int target_x = cur->x + dir;
						const int target_y = cur->y - dir;
						if (target_x >= 0 && target_x < m_width && target_y >= 0 && target_y < m_height && !IsEmptyImpl(target_x, target_y) && IsEmptyImpl(cur->x, target_y))
							has_left_target = true;
					}

					// cur右后方不可走 且 右方可走
					bool has_right_target = false;
					{
						const int target_x = cur->x - dir;
						const int target_y = cur->y - dir;
						if (target_x >= 0 && target_x < m_width && target_y >= 0 && target_y < m_height && !IsEmptyImpl(target_x, target_y) && IsEmptyImpl(cur->x, target_y))
							has_right_target = true;
					}

					// 当前方向
					SearchVDir(cur, end_x, end_y, dir, open_list, open_set, close_map);

					// 左方寻找
					if (has_left_target) SearchHDir(cur, end_x, end_y, dir, open_list, open_set, close_map);
					// 右方寻找
					if (has_right_target) SearchHDir(cur, end_x, end_y, -dir, open_list, open_set, close_map);
					// 左前方寻找
					if (has_left_target) SearchHVDir(cur, end_x, end_y, dir, dir, open_list, open_set, close_map);
					// 右前方寻找
					if (has_right_target) SearchHVDir(cur, end_x, end_y, -dir, dir, open_list, open_set, close_map);
				}
				// 如果是斜方向
				else
				{
					const auto dir_x = cur->parent->x < cur->x ? 1 : -1;
					const auto dir_y = cur->parent->y < cur->y ? 1 : -1;

					SearchHVDir(cur, end_x, end_y, dir_x, dir_y, open_list, open_set, close_map);
				}
			}

		}

		// 找到路径
		const bool result = end_node != nullptr;
		while (end_node != nullptr)
		{
			x_list.push_back(end_node->x);
			y_list.push_back(end_node->y);
			end_node = end_node->parent;
		}
		x_list.reverse();
		y_list.reverse();

		// 回收所有跳点
		for (unsigned int i = 0; i < open_list.GetSize(); ++i)
			ReleaseNode(open_list.GetElement(i));
		for (auto& pair : close_map)
			ReleaseNode(pair.second);

		// 返回搜索结果
		return result;
	}

private:
	void SearchHVDir(CarpSquareJPSNode* cur
		, int end_x, int end_y, int dir_x, int dir_y
		, CarpMiniHeap<CarpSquareJPSNode>& open_list
		, std::unordered_set<long long>& open_set
		, std::unordered_map<long long, CarpSquareJPSNode*> close_map)
	{
		int target_x = cur->x;
		int target_y = cur->y;
		while (true)
		{
			// 检查是否在地图内
			target_x += dir_x;
			if (target_x < 0 || target_x >= m_width) break;
			target_y += dir_y;
			if (target_y < 0 || target_y >= m_height) break;

			const auto key = static_cast<long long>(target_x) << 32 | target_y;
			// 既不在跳点列表中，也不在关闭列表中
			if (close_map.find(key) != close_map.end()) break;

			// 检查目标格子是否有阻挡
			if (!IsEmptyImpl(target_x, target_y)) break;
			// 检查水平相邻的格子是否有阻挡
			if (!IsEmptyImpl(target_x, target_y - dir_y)) break;
			// 检查垂直相邻的格子是否有阻挡
			if (!IsEmptyImpl(target_x - dir_x, target_y)) break;

			CarpSquareJPSNode* target_node = nullptr;

			// 如果发现是终点
			if (target_x == end_x && target_y == end_y)
			{
				target_node = CreateNode();
				target_node->Init(target_x, target_y, cur->g + s_carp_square_hypotenuse_cost, end_x, end_y, cur);
				open_list.Push(target_node);
				open_set.insert(key);
				break;
			}

			// 水平搜索
			int out_x = 0;
			if (SearchHDir(target_x, target_y, dir_x, end_x, end_y, out_x))
			{
				if (target_node == nullptr)
				{
					target_node = CreateNode();
					target_node->Init(target_x, target_y, cur->g + s_carp_square_hypotenuse_cost, end_x, end_y, cur);
					open_list.Push(target_node);
					open_set.insert(key);
				}

				TryAddHNode(target_node, end_x, end_y, out_x, open_list, open_set, close_map);
			}

			// 垂直搜索
			int out_y = 0;
			if (SearchVDir(target_x, target_y, dir_y, end_x, end_y, out_y))
			{
				if (target_node == nullptr)
				{
					target_node = CreateNode();
					target_node->Init(target_x, target_y, cur->g + s_carp_square_hypotenuse_cost, end_x, end_y, cur);
					open_list.Push(target_node);
					open_set.insert(key);
				}

				TryAddVNode(target_node, end_x, end_y, out_y, open_list, open_set, close_map);
			}

			if (target_node != nullptr) break;
		}
	}

	bool SearchHDir(int cur_x, int cur_y, int dir_x, int end_x, int end_y, int& out_x) const
	{
		auto x = cur_x;
		while (true)
		{
			// 向前进一步
			x += dir_x;

			// 遇到边界，跳出
			if (x < 0 || x >= m_width) break;

			// 遇到阻挡，跳出
			if (!IsEmptyImpl(x, cur_y)) break;

			// 如果是终点
			// 如果靠上后方有阻挡，并且靠上方没有阻挡，那么就找到跳点
			// 如果靠下后方有阻挡，并且靠下方没有阻挡，那么就找到跳点
			if (x == end_x && cur_y == end_y
				|| cur_y - 1 >= 0 && !IsEmptyImpl(x - dir_x, cur_y - 1) && IsEmptyImpl(x, cur_y - 1)
				|| cur_y + 1 < m_height && !IsEmptyImpl(x - dir_x, cur_y + 1) && IsEmptyImpl(x, cur_y + 1))
			{
				out_x = x;
				return true;
			}
		}

		return false;
	}

	void TryAddHNode(CarpSquareJPSNode* parent
		, int end_x, int end_y
		, int out_x
		, CarpMiniHeap<CarpSquareJPSNode>& open_list
		, std::unordered_set<long long>& open_set
		, std::unordered_map<long long, CarpSquareJPSNode*>& close_map)
	{
		const auto key = static_cast<long long>(out_x) << 32 | parent->y;
		// 既不在跳点列表中，也不在关闭列表中
		if (close_map.find(key) != close_map.end()) return;
		auto* node = CreateNode();
		node->Init(out_x, parent->y, parent->g + std::abs(out_x - parent->x), end_x, end_y, parent);
		open_list.Push(node);
		open_set.insert(key);
	}

	void SearchHDir(CarpSquareJPSNode* cur
		, int end_x, int end_y
		, int dir_x
		, CarpMiniHeap<CarpSquareJPSNode>& open_list
		, std::unordered_set<long long>& open_set
		, std::unordered_map<long long, CarpSquareJPSNode*>& close_map)
	{
		int out_x = 0;
		if (!SearchHDir(cur->x, cur->y, dir_x, end_x, end_y, out_x)) return;
		TryAddHNode(cur, end_x, end_y, out_x, open_list, open_set, close_map);
	}

	bool SearchVDir(int cur_x, int cur_y, int dir_y, int end_x, int end_y, int& out_y) const
	{
		auto y = cur_y;
		while (true)
		{
			// 向前进一步
			y += dir_y;

			// 遇到边界，跳出
			if (y < 0 || y >= m_height) break;

			// 遇到阻挡，跳出
			if (!IsEmptyImpl(cur_x, y)) break;

			// 如果靠左后方有阻挡，并且靠左方没有阻挡，那么就找到跳点
			// 如果靠右后方有阻挡，并且靠右方没有阻挡，那么就找到跳点
			if (cur_x == end_x && y == end_y
				|| cur_x - 1 >= 0 && !IsEmptyImpl(cur_x - 1, y - dir_y) && IsEmptyImpl(cur_x - 1, y)
				|| cur_x + 1 < m_width && !IsEmptyImpl(cur_x + 1, y - dir_y) && IsEmptyImpl(cur_x + 1, y))
			{
				out_y = y;
				return true;
			}
		}

		return false;
	}

	void TryAddVNode(CarpSquareJPSNode* parent
		, int end_x, int end_y
		, int out_y
		, CarpMiniHeap<CarpSquareJPSNode>& open_list
		, std::unordered_set<long long>& open_set
		, std::unordered_map<long long, CarpSquareJPSNode*>& close_map)
	{
		const auto key = static_cast<long long>(parent->x) << 32 | out_y;
		// 既不在跳点列表中，也不在关闭列表中
		if (close_map.find(key) != close_map.end()) return;

		auto* node = CreateNode();
		node->Init(parent->x, out_y, parent->g + std::abs(out_y - parent->y), end_x, end_y, parent);
		open_list.Push(node);
		open_set.insert(key);
	}

	void SearchVDir(CarpSquareJPSNode* cur
		, int end_x, int end_y
		, int dir_y
		, CarpMiniHeap<CarpSquareJPSNode>& open_list
		, std::unordered_set<long long>& open_set
		, std::unordered_map<long long, CarpSquareJPSNode*>& close_map)
	{
		int out_y = 0;
		if (!SearchVDir(cur->x, cur->y, dir_y, end_x, end_y, out_y)) return;
		TryAddVNode(cur, end_x, end_y, out_y, open_list, open_set, close_map);
	}

public:
	virtual bool IsEmpty(int x, int y) const
	{
		if (x >= m_width || y >= m_height || x < 0 || y < 0) return false;
		return IsEmptyImpl(x, y);
	}

	virtual void SetEmpty(int x, int y, bool value)
	{
		if (x >= m_width || y >= m_height || x < 0 || y < 0) return;
		SetEmptyImpl(x, y, value);
	}

	virtual void SetAllEmpty()
	{
		memset(m_map, 0, m_x_size_t_count * m_height * sizeof(size_t));
	}

private:
	inline bool IsEmptyImpl(int x, int y) const
	{
		// 一个size_t有多少个bit
		const auto size_bit_count = 8 * sizeof(size_t);
		// 计算size_t的偏移
		const auto size_t_offset = y * m_x_size_t_count + x / size_bit_count;
		// 取出size_t
		const auto size_t_value = m_map[size_t_offset];
		// 计算bit偏移
		const auto bit_offset = x % size_bit_count;
		// 判断对应偏移位置是否是0
		return (size_t_value & (static_cast<size_t>(1) << (size_bit_count - 1 - bit_offset))) == 0;
	}

	inline void SetEmptyImpl(int x, int y, bool value)
    {
		// 一个size_t有多少个bit
		const auto size_bit_count = 8 * sizeof(size_t);
		// 计算size_t的偏移
		const auto size_t_offset = y * m_x_size_t_count + x / size_bit_count;
		// 取出size_t
		auto& size_t_value = m_map[size_t_offset];
		// 计算bit偏移
		const auto bit_offset = x % size_bit_count;
		// 计算bit值
		const auto bit_value = static_cast<size_t>(1) << (size_bit_count - 1 - bit_offset);
		if (value)
		    size_t_value |= bit_value;
		else
			size_t_value &= ~bit_value;
	}

	inline CarpSquareJPSNode* CreateNode()
	{
		if (m_node_pool.empty()) return new CarpSquareJPSNode();
		auto* node = m_node_pool.back();
		m_node_pool.pop_back();
		return node;
	}

	inline void ReleaseNode(CarpSquareJPSNode* node)
	{
		if (node == nullptr) return;
		if (m_node_pool.size() > 1000)
			delete node;
		else
			m_node_pool.emplace_back(node);
	}

private:
	// 地图的宽高
	int m_width = 0;
	int m_height = 0;

	// 地图数据，bit 1表示有阻挡，0表示没有阻挡
	size_t* m_map = nullptr;
	size_t m_x_size_t_count = 0;

	// 内存池
	std::vector<CarpSquareJPSNode*> m_node_pool;
};

#endif
