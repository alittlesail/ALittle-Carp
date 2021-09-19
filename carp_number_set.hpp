
#ifndef CARP_NUMBER_SET_INCLUDED
#define CARP_NUMBER_SET_INCLUDED

#include <string>
#include <vector>
#include <memory>

// 用于管理只包含数字的字符串
// 对于普通的哈希表，可以节省更多的内存
// 对于200万的号码，unordered_set需要154M
// CarpNumberSet只需要68M
// 查找速度unordered_set是CarpNumberSet三倍快

class CarpNumberSet
{
public:
    struct CarpNumberNode
    {
        bool flag = false;  // 标记从根节点到当前是否有字符串
        unsigned char min_max = 0; // 当前节点范围
        CarpNumberNode* brother = nullptr;   // 下一个兄弟节点
        CarpNumberNode* child = nullptr;    // 子节点

        CarpNumberNode() {}
        ~CarpNumberNode() { delete child; delete brother; }
        CarpNumberNode(unsigned char min, unsigned char max, CarpNumberNode* b)
        {
            min_max = (min << 4) | max;
            brother = b;
        }
        void SetMinMax(unsigned char min, unsigned char max) { min_max = (min << 4) | max; }
        unsigned char Min() const { return min_max >> 4; }
        unsigned char Max() const { return min_max & 0x0F; }
        CarpNumberNode* Clone() const
        {
            auto node = new CarpNumberNode();
            node->min_max = min_max;
            if (brother != nullptr) node->brother = brother->Clone();
            if (child != nullptr) node->child = child->Clone();
            return node;
        }
    };
    
public:
    bool Find(const std::string& number)
    {
        // 初始化为根节点
        auto* node = &m_root;
        // 遍历所有字符
        for (size_t i = 0; i < number.size(); ++i)
        {
            // 获取整数
            unsigned char c = number[i] - '0';

            // 查找兄弟节点
            while (true)
            {
                // 如果是空节点，那么就查找失败
                if (node == nullptr)
                    return false;

                // 获取范围
                auto min = node->Min();
                auto max = node->Max();

                // 如果比第一个兄弟节点小，那么就查找失败
                if (c < min)
                    return false;

                // 如果满足条件就跳出
                if (c <= max) break;

                // 进入兄弟节点
                node = node->brother;
            }

            // 如果遍历到最后一个，直接查看这个节点的flag
            if (i + 1 >= number.size()) return node->flag;

            // 进入子节点
            node = node->child;
        }

        return false;
    }

    bool Erase(const std::string& number)
    {
        // 先查找，如果没有就直接返回
        if (!Find(number))
            return false;

        // 然后先执行插入，因为Insert会对合并过的节点进行拆分
        std::vector<CarpNumberNode*> list;
        list.reserve(number.size());
        Insert(number, &list);

        // 如果是空的，说明什么都没找到
        if (list.empty())
            return false;
        // 标记为没有字符串
        if (list.back()->flag) --m_size;
        list.back()->flag = false;

        // 这个循环不会对第一层节点进行处理
        for (int i = static_cast<int>(list.size()) - 1; i >= 0; --i)
        {
            // 获取当前节点
            auto node = list[i];

            // 标记为有字符串，那么说明不能删
            if (node->flag) break;

            // 判断当前节点是否已全部标记为空
            bool node_empty = true;
            // 遍历子节点
            auto* child_node = node->child;
            while (child_node)
            {
                if (child_node->flag || child_node->child)
                {
                    node_empty = false;
                    break;
                }
                child_node = child_node->brother;
            }

            // 如果不为空，那么说明不能删
            if (!node_empty) break;

            // 计算父节点
            CarpNumberNode* parent = i >= 1 ? list[i - 1] : nullptr;
            // 计算第一个兄弟节点
            auto* first_child = parent ? parent->child : &m_root;

            // 如果第一个节点就是node
            // 那么把node删除，然后把node的兄弟节点交给parent->child
            if (first_child == node)
            {
                if (parent)
                {
                    parent->child = node->brother;
                    node->brother = nullptr;
                    delete node;
                }
            }
            else
            {
                auto* prev_node = first_child;
                while (prev_node)
                {
                    if (prev_node->brother == node)
                    {
                        prev_node->brother = node->brother;
                        node->brother = nullptr;
                        delete node;
                        break;
                    }
                    prev_node = prev_node->brother;
                }
            }
        }

        return true;
    }

    void Insert(const std::string& number, std::vector<CarpNumberNode*>* list=nullptr)
    {
        // 初始化为根节点
        auto* node = &m_root;
        // 初始化父节点
        CarpNumberNode* parent = nullptr;
        // 遍历所有字符
        for (size_t i = 0; i < number.size(); ++i)
        {
            // 获取整数
            unsigned char c = number[i] - '0';

            // 查找兄弟节点
            while (true)
            {
                // 获取范围
                auto min = node->Min();
                auto max = node->Max();

                // 如果小于当前节点
                if (c < min)
                {
                    // 创建一个新的节点，插入到node之前
                    auto* new_node = new CarpNumberNode(c, c, node);
                    // 从第一个node兄弟节点，找到在node前面的那个兄弟节点
                    auto* first_child = parent ? parent->child : &m_root;
                    if (first_child == node)
                    {
                        if (parent)
                            parent->child = new_node;
                    }
                    else
                    {
                        auto* prev_node = first_child;
                        while (prev_node)
                        {
                            if (prev_node->brother == node)
                            {
                                prev_node->brother = new_node;
                                break;
                            }
                            prev_node = prev_node->brother;
                        }
                    }

                    // 将新节点设置为node
                    node = new_node;
                }
                else if (c <= max)
                {
                    // 如果出现过合并，那么就拆开
                    if (min != max)
                    {
                        // 拆成 [min, c], [c+1, max]
                        if (c == min)
                        {
                            node->SetMinMax(min, c);
                            auto* new_node = new CarpNumberNode(c + 1, max, node->brother);
                            if (node->child) new_node->child = node->child->Clone();
                            node->brother = new_node;
                        }
                        // 拆成 [min, c-1], [c, max]
                        else if (c == max)
                        {
                            node->SetMinMax(min, c - 1);
                            auto* new_node = new CarpNumberNode(c, max, node->brother);
                            if (node->child) new_node->child = node->child->Clone();
                            node->brother = new_node;
                            node = new_node;
                        }
                        // 拆成 [min, c-1], [c, c], [c+1, max]
                        else
                        {
                            node->SetMinMax(min, c - 1);

                            auto* last_node = new CarpNumberNode(c + 1, max, node->brother);
                            if (node->child) last_node->child = node->child->Clone();
                            
                            auto* new_node = new CarpNumberNode(c, c, last_node);
                            if (node->child) new_node->child = node->child->Clone();

                            node->brother = new_node;
                            node = new_node;
                        }
                    }
                }
                else
                {
                    auto* new_node = node->brother;
                    // 如果没有兄弟节点了，那么就创建一个跳出
                    if (new_node == nullptr)
                    {
                        new_node = new CarpNumberNode(c, c, nullptr);
                        node->brother = new_node;
                        node = new_node;
                    }
                    else
                    {
                        // 进入兄弟节点
                        node = new_node;
                        // 进入下一个循环
                        continue;
                    }
                }

                // 能到这里，说明都找到了指定的节点
                break;
            }

            if (list != nullptr) list->push_back(node);

            // 如果遍历到最后一个，那么就跳出
            if (i + 1 >= number.size())
            {
                // 标记为有字符串
                if (!node->flag) ++m_size;
                node->flag = true;
                break;
            }

            // 如果没有子节点，那么就创建一个
            if (node->child == nullptr)
            {
                auto next_c = number[i + 1] - '0';
                node->child = new CarpNumberNode(next_c, next_c, nullptr);
            }

            // 将子节点作为下一个根节点
            parent = node;
            node = node->child;
        }
    }

    size_t Size() const { return m_size; }

private:
    size_t m_size = 0;
    CarpNumberNode m_root;
};

#endif