
#ifndef CARP_NUMBER_SET_INCLUDED
#define CARP_NUMBER_SET_INCLUDED

#include <string>
#include <vector>

class CarpNumberSet
{
public:
    struct CarpNumberNode
    {
        unsigned char min_max = 0; // 当前节点范围
        CarpNumberNode* brother = nullptr;   // 下一个兄弟节点
        CarpNumberNode* child = nullptr;    // 子节点

        CarpNumberNode() {}
        ~CarpNumberNode() { delete brother; delete child; }
        CarpNumberNode(unsigned char min, unsigned char max, CarpNumberNode* brother)
        {
            min_max = (min << 4) | max;
            brother = brother;
            child = nullptr;
        }
        void SetMinMax(unsigned char min, unsigned char max) { min_max = (min << 4) | max; }
        unsigned char Min() const { return min_max >> 4; }
        unsigned char Max() const { return min_max & 0x0F; }
        CarpNumberNode* Clone() const
        {
            CarpNumberNode* node = new CarpNumberNode();
            node->min_max = min_max;
            if (brother != nullptr) node->brother = brother->Clone();
            if (child != nullptr) node->child = child->Clone();
            return node;
        }
    };

public:
    bool Find(const std::string& number)
    {
        if (number.size() == 1 && number[0] == '0') return true;

        CarpNumberNode* node = &m_root;
        for (size_t i = 0; i < number.size(); ++i)
        {
            unsigned char c = number[i] - '0';

            // 查找兄弟节点
            CarpNumberNode* level_node = node;
            while (true)
            {
                // 如果满足条件就跳出
                if (c >= level_node->Min() && c <= level_node->Max()) break;

                CarpNumberNode* brother = level_node->brother;
                // 如果没有兄弟节点，那么就跳出
                if (brother == nullptr) return false;

                // 如果比第一个兄弟节点小，那么就跳出
                if (c < brother->Min()) return false;

                // 进入下一个循环
                level_node = brother;
            }

            // 如果遍历到最后一个，那么就跳出，说明查找成功
            if (i + 1 >= number.size()) return true;

            // 如果没有子节点，那么就创建一个
            if (level_node->child == nullptr) return false;

            // 将子节点作为下一个根节点
            node = level_node->child;
        }

        return true;
    }

    bool Erase(const std::string& number)
    {
        // 先查找，如果没有就直接返回
        if (!Find(number)) return false;

        if (number.size() == 1 && number[0] == '0')
        {
            m_contain_zero = false;
            return true;
        }

        // 然后先执行插入，因为Insert会对合并过的节点进行拆分
        std::vector<CarpNumberNode*> list;
        list.reserve(number.size());
        Insert(number, &list);

        if (list.empty()) return false;

        // 因为i==0时就是m_root，所以不做处理
        for (size_t i = list.size() - 1; i > 0; --i)
        {
            auto* node = list[i];
            auto* parent = list[i - 1];

            // 如果没有子节点，兄弟节点，并且父节点的child就是自己，那么自己就可以删掉了
            if (node->child == nullptr && node->brother == nullptr && parent->child == node)
            {
                delete node;
                parent->child = nullptr;
            }
        }

        return true;
    }

    void Insert(const std::string& number, std::vector<CarpNumberNode*>* list)
    {
        if (number.size() == 1 && number[0] == '0')
        {
            m_contain_zero = true;
            if (list) list->push_back(&m_root);
            return;
        }

        CarpNumberNode* node = &m_root;
        for (size_t i = 0; i < number.size(); ++i)
        {
            unsigned char c = number[i] - '0';

            // 查找兄弟节点
            CarpNumberNode* level_node = node;
            while (true)
            {
                auto min = level_node->Min();
                auto max = level_node->Max();

                // 如果满足条件就跳出
                if (c >= min && c <= max)
                {
                    // 如果出现过合并，那么就拆开
                    if (min != max)
                    {
                        if (c == min)
                        {
                            level_node->SetMinMax(min, c);
                            auto* brother = new CarpNumberNode(c + 1, max, level_node->brother);
                            if (level_node->child) brother->child = level_node->child->Clone();
                            level_node->brother = brother;
                            level_node = brother;
                        }
                        else if (c == max)
                        {
                            level_node->SetMinMax(min, c - 1);
                            auto* brother = new CarpNumberNode(c, max, level_node->brother);
                            if (level_node->child) brother->child = level_node->child->Clone();
                            level_node->brother = brother;
                            level_node = brother;
                        }
                        else
                        {
                            level_node->SetMinMax(min, c - 1);
                            {
                                auto* brother = new CarpNumberNode(c, c, level_node->brother);
                                if (level_node->child) brother->child = level_node->child->Clone();
                                level_node->brother = brother;
                                level_node = brother;
                            }

                            {
                                auto* brother = new CarpNumberNode(c + 1, max, level_node->brother);
                                if (level_node->child) brother->child = level_node->child->Clone();
                                level_node->brother = brother;
                                level_node = brother;
                            }
                        }
                    }

                    break;
                }

                CarpNumberNode* brother = level_node->brother;
                // 如果没有兄弟节点了，那么就创建一个跳出
                if (brother == nullptr)
                {
                    brother = new CarpNumberNode(c, c, nullptr);
                    level_node->brother = brother;
                    break;
                }
                
                // 如果比兄弟节点小，那么就创建一个插入到中间，然后跳出
                if (c < brother->Min())
                {
                    auto* new_brother = new CarpNumberNode(c, c, brother);
                    level_node->brother = new_brother;
                    level_node = new_brother;
                    break;
                }
                
                // 进入下一个循环
                level_node = brother;
            }

            if (list != nullptr) list->push_back(level_node);

            // 如果遍历到最后一个，那么就跳出
            if (i + 1 >= number.size())
            {
                // 获取最后一个节点
                node = level_node;
                break;
            }

            // 如果没有子节点，那么就创建一个
            if (level_node->child == nullptr)
                level_node->child = new CarpNumberNode(number[i + 1], number[i + 1], nullptr);

            // 将子节点作为下一个根节点
            node = level_node->child;
        }
    }

private:
    bool m_contain_zero = false;    // 是否包含"0"字符串
    CarpNumberNode m_root;
};

#endif