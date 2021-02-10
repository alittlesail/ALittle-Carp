#ifndef CARP_GRID_JPS_INCLUDED
#define CARP_GRID_JPS_INCLUDED

#include <cstdlib>
#include <cstring>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "carp_miniheap.hpp"

// ֱ�ߵĴ���
const int s_carp_square_straight_cost = 10;
// б�ߵĴ���
const int s_carp_square_hypotenuse_cost = 10;

// ����
struct CarpSquareJPSNode
{
	// v_x: ��ǰx����
	// v_y: ��ǰy����
	// v_cost: ����㵽��ǰ������ܴ���
	// end_x: �յ�x����
	// end_y: �յ�y����
	// v_parent: ���ڵ�
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

	// ���±�
	int heap_index = -1;

	// �ڵ�����
	int x = 0;
	int y = 0;

	// ����
	int g = 0;
	int f = 0;

	// ���ڵ�
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

		// һ��size_t�ж��ٸ�bit
		const auto size_bit_count = 8 * sizeof(size_t);

		// �������size_t����
		m_x_size_t_count = (m_width + size_bit_count - 1) / size_bit_count;

		// ��ȡ��ͼ����
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
		// ��������
		if (start_x < 0) start_x = 0;
		else if (start_x >= m_width) start_x = m_width - 1;
		if (start_y < 0) start_y = 0;
		else if (start_y >= m_height) start_x = m_height - 1;

		if (end_x < 0) end_x = 0;
		else if (end_x >= m_width) end_x = m_width - 1;
		if (end_y < 0) end_y = 0;
		else if (end_y >= m_height) end_x = m_height - 1;

		// ��������յ㶼���赲����ô�����Ҳ���
		if (!IsEmptyImpl(start_x, start_y) || !IsEmptyImpl(end_x, end_y)) return false;

		// �����б�
        CarpMiniHeap<CarpSquareJPSNode> open_list;
		std::unordered_set<long long> open_set;
		// �رյ�����
		std::unordered_map<long long, CarpSquareJPSNode*> close_map;

		// �������ӵ������б�
		{
			auto* node = new CarpSquareJPSNode();
			node->Init(start_x, start_y, 0, end_x, end_y, nullptr);
			open_list.Push(node);
			open_set.insert(static_cast<long long>(start_x) << 32 | start_y);
		}

		CarpSquareJPSNode* end_node = nullptr;
		while (true)
		{
		    // ȡ��cost��С������
			auto* cur = open_list.Pop();
			if (cur == nullptr) break;
			// ����key
			const auto cur_key = static_cast<long long>(cur->x) << 32 | cur->y;
			// �����㼯����ɾ������ӵ��ر��б�
			open_set.erase(cur_key);
			close_map[cur_key] = cur;

			// ������յ���ô������
			if (cur->x == end_x && cur->y == end_y)
			{
				end_node = cur;
				break;
			}

			// ���û�и��ڵ㣬��ô��Ҫ����8������
			if (cur->parent == nullptr)
			{
				const auto dir_x = cur->x < end_x ? 1 : -1;
				// ˮƽ��������
				SearchHDir(cur, end_x, end_y, dir_x, open_list, open_set, close_map);
				// ˮƽ��������
				SearchHDir(cur, end_x, end_y, -dir_x, open_list, open_set, close_map);

			    const auto dir_y = cur->y < end_y ? 1 : -1;
				// ��ֱ��������
				SearchVDir(cur, end_x, end_y, dir_y, open_list, open_set, close_map);
				// ��ֱ��������
				SearchVDir(cur, end_x, end_y, -dir_y, open_list, open_set, close_map);

				// ����б
				SearchHVDir(cur, end_x, end_y, dir_x, dir_y, open_list, open_set, close_map);
				// ����б
				SearchHVDir(cur, end_x, end_y, dir_x, -dir_y, open_list, open_set, close_map);
				// ����б
				SearchHVDir(cur, end_x, end_y, -dir_x, dir_y, open_list, open_set, close_map);
				// ����б
				SearchHVDir(cur, end_x, end_y, -dir_x, -dir_y, open_list, open_set, close_map);
			}
			else
			{
				// �����ˮƽֱ�߷���
				if (cur->parent->y == cur->y)
				{
					const auto dir = cur->parent->x < cur->x ? 1 : -1;

					bool has_left_target = false;
					// cur��󷽲����� �� �󷽿���
					{
						const int target_x = cur->x - dir;
						const int target_y = cur->y - dir;
						if (target_x >= 0 && target_x < m_width && target_y >= 0 && target_y < m_height && !IsEmptyImpl(target_x, target_y) && IsEmptyImpl(cur->x, target_y))
							has_left_target = true;
					}

				    // cur�Һ󷽲����� �� �ҷ�����
					bool has_right_target = false;
					{
						const int target_x = cur->x - dir;
						const int target_y = cur->y + dir;
						if (target_x >= 0 && target_x < m_width && target_y >= 0 && target_y < m_height && !IsEmptyImpl(target_x, target_y) && IsEmptyImpl(cur->x, target_y))
							has_right_target = true;
					}

					// ��ǰ����
					SearchHDir(cur, end_x, end_y, dir, open_list, open_set, close_map);
					// ��Ѱ��
					if (has_left_target) SearchVDir(cur, end_x, end_y, -dir, open_list, open_set, close_map);
					// �ҷ�Ѱ��
					if (has_right_target) SearchVDir(cur, end_x, end_y, dir, open_list, open_set, close_map);
					// ��ǰ��Ѱ��
					if (has_left_target) SearchHVDir(cur, end_x, end_y, dir, -dir, open_list, open_set, close_map);
					// ��ǰ��Ѱ��
					if (has_right_target) SearchHVDir(cur, end_x, end_y, dir, dir, open_list, open_set, close_map);
				}
				// ����Ǵ�ֱ����
				else if (cur->parent->x == cur->x)
				{
					const auto dir = cur->parent->y < cur->y ? 1 : -1;

					// cur��󷽲����� �� �󷽿���
					bool has_left_target = false;
					{
						const int target_x = cur->x + dir;
						const int target_y = cur->y - dir;
						if (target_x >= 0 && target_x < m_width && target_y >= 0 && target_y < m_height && !IsEmptyImpl(target_x, target_y) && IsEmptyImpl(cur->x, target_y))
							has_left_target = true;
					}

					// cur�Һ󷽲����� �� �ҷ�����
					bool has_right_target = false;
					{
						const int target_x = cur->x - dir;
						const int target_y = cur->y - dir;
						if (target_x >= 0 && target_x < m_width && target_y >= 0 && target_y < m_height && !IsEmptyImpl(target_x, target_y) && IsEmptyImpl(cur->x, target_y))
							has_right_target = true;
					}

					// ��ǰ����
					SearchVDir(cur, end_x, end_y, dir, open_list, open_set, close_map);

					// ��Ѱ��
					if (has_left_target) SearchHDir(cur, end_x, end_y, dir, open_list, open_set, close_map);
					// �ҷ�Ѱ��
					if (has_right_target) SearchHDir(cur, end_x, end_y, -dir, open_list, open_set, close_map);
					// ��ǰ��Ѱ��
					if (has_left_target) SearchHVDir(cur, end_x, end_y, dir, dir, open_list, open_set, close_map);
					// ��ǰ��Ѱ��
					if (has_right_target) SearchHVDir(cur, end_x, end_y, -dir, dir, open_list, open_set, close_map);
				}
				// �����б����
				else
				{
					const auto dir_x = cur->parent->x < cur->x ? 1 : -1;
					const auto dir_y = cur->parent->y < cur->y ? 1 : -1;

					SearchHVDir(cur, end_x, end_y, dir_x, dir_y, open_list, open_set, close_map);
				}
			}

		}

		// �ҵ�·��
		const bool result = end_node != nullptr;
		while (end_node != nullptr)
		{
			x_list.push_back(end_node->x);
			y_list.push_back(end_node->y);
			end_node = end_node->parent;
		}
		x_list.reverse();
		y_list.reverse();

		// ������������
		for (unsigned int i = 0; i < open_list.GetSize(); ++i)
			ReleaseNode(open_list.GetElement(i));
		for (auto& pair : close_map)
			ReleaseNode(pair.second);

		// �����������
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
			// ����Ƿ��ڵ�ͼ��
			target_x += dir_x;
			if (target_x < 0 || target_x >= m_width) break;
			target_y += dir_y;
			if (target_y < 0 || target_y >= m_height) break;

			const auto key = static_cast<long long>(target_x) << 32 | target_y;
			// �Ȳ��������б��У�Ҳ���ڹر��б���
			if (close_map.find(key) != close_map.end()) break;

			// ���Ŀ������Ƿ����赲
			if (!IsEmptyImpl(target_x, target_y)) break;
			// ���ˮƽ���ڵĸ����Ƿ����赲
			if (!IsEmptyImpl(target_x, target_y - dir_y)) break;
			// ��鴹ֱ���ڵĸ����Ƿ����赲
			if (!IsEmptyImpl(target_x - dir_x, target_y)) break;

			CarpSquareJPSNode* target_node = nullptr;

			// ����������յ�
			if (target_x == end_x && target_y == end_y)
			{
				target_node = CreateNode();
				target_node->Init(target_x, target_y, cur->g + s_carp_square_hypotenuse_cost, end_x, end_y, cur);
				open_list.Push(target_node);
				open_set.insert(key);
				break;
			}

			// ˮƽ����
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

			// ��ֱ����
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
			// ��ǰ��һ��
			x += dir_x;

			// �����߽磬����
			if (x < 0 || x >= m_width) break;

			// �����赲������
			if (!IsEmptyImpl(x, cur_y)) break;

			// ������յ�
			// ������Ϻ����赲�����ҿ��Ϸ�û���赲����ô���ҵ�����
			// ������º����赲�����ҿ��·�û���赲����ô���ҵ�����
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
		// �Ȳ��������б��У�Ҳ���ڹر��б���
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
			// ��ǰ��һ��
			y += dir_y;

			// �����߽磬����
			if (y < 0 || y >= m_height) break;

			// �����赲������
			if (!IsEmptyImpl(cur_x, y)) break;

			// �����������赲�����ҿ���û���赲����ô���ҵ�����
			// ������Һ����赲�����ҿ��ҷ�û���赲����ô���ҵ�����
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
		// �Ȳ��������б��У�Ҳ���ڹر��б���
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
		// һ��size_t�ж��ٸ�bit
		const auto size_bit_count = 8 * sizeof(size_t);
		// ����size_t��ƫ��
		const auto size_t_offset = y * m_x_size_t_count + x / size_bit_count;
		// ȡ��size_t
		const auto size_t_value = m_map[size_t_offset];
		// ����bitƫ��
		const auto bit_offset = x % size_bit_count;
		// �ж϶�Ӧƫ��λ���Ƿ���0
		return (size_t_value & (static_cast<size_t>(1) << (size_bit_count - 1 - bit_offset))) == 0;
	}

	inline void SetEmptyImpl(int x, int y, bool value)
    {
		// һ��size_t�ж��ٸ�bit
		const auto size_bit_count = 8 * sizeof(size_t);
		// ����size_t��ƫ��
		const auto size_t_offset = y * m_x_size_t_count + x / size_bit_count;
		// ȡ��size_t
		auto& size_t_value = m_map[size_t_offset];
		// ����bitƫ��
		const auto bit_offset = x % size_bit_count;
		// ����bitֵ
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
	// ��ͼ�Ŀ��
	int m_width = 0;
	int m_height = 0;

	// ��ͼ���ݣ�bit 1��ʾ���赲��0��ʾû���赲
	size_t* m_map = nullptr;
	size_t m_x_size_t_count = 0;

	// �ڴ��
	std::vector<CarpSquareJPSNode*> m_node_pool;
};

#endif
