
#ifndef CARP_NUMBER_SET_INCLUDED
#define CARP_NUMBER_SET_INCLUDED

#include <string>
#include <vector>
#include <memory>

// ���ڹ���ֻ�������ֵ��ַ���
// ������ͨ�Ĺ�ϣ�����Խ�ʡ������ڴ�
// ����200��ĺ��룬unordered_set��Ҫ154M
// CarpNumberSetֻ��Ҫ68M
// �����ٶ�unordered_set��CarpNumberSet������

class CarpNumberSet
{
public:
    struct CarpNumberNode
    {
        bool flag = false;  // ��ǴӸ��ڵ㵽��ǰ�Ƿ����ַ���
        unsigned char min_max = 0; // ��ǰ�ڵ㷶Χ
        CarpNumberNode* brother = nullptr;   // ��һ���ֵܽڵ�
        CarpNumberNode* child = nullptr;    // �ӽڵ�

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
        // ��ʼ��Ϊ���ڵ�
        auto* node = &m_root;
        // ���������ַ�
        for (size_t i = 0; i < number.size(); ++i)
        {
            // ��ȡ����
            unsigned char c = number[i] - '0';

            // �����ֵܽڵ�
            while (true)
            {
                // ����ǿսڵ㣬��ô�Ͳ���ʧ��
                if (node == nullptr)
                    return false;

                // ��ȡ��Χ
                auto min = node->Min();
                auto max = node->Max();

                // ����ȵ�һ���ֵܽڵ�С����ô�Ͳ���ʧ��
                if (c < min)
                    return false;

                // �����������������
                if (c <= max) break;

                // �����ֵܽڵ�
                node = node->brother;
            }

            // ������������һ����ֱ�Ӳ鿴����ڵ��flag
            if (i + 1 >= number.size()) return node->flag;

            // �����ӽڵ�
            node = node->child;
        }

        return false;
    }

    bool Erase(const std::string& number)
    {
        // �Ȳ��ң����û�о�ֱ�ӷ���
        if (!Find(number))
            return false;

        // Ȼ����ִ�в��룬��ΪInsert��Ժϲ����Ľڵ���в��
        std::vector<CarpNumberNode*> list;
        list.reserve(number.size());
        Insert(number, &list);

        // ����ǿյģ�˵��ʲô��û�ҵ�
        if (list.empty())
            return false;
        // ���Ϊû���ַ���
        if (list.back()->flag) --m_size;
        list.back()->flag = false;

        // ���ѭ������Ե�һ��ڵ���д���
        for (int i = static_cast<int>(list.size()) - 1; i >= 0; --i)
        {
            // ��ȡ��ǰ�ڵ�
            auto node = list[i];

            // ���Ϊ���ַ�������ô˵������ɾ
            if (node->flag) break;

            // �жϵ�ǰ�ڵ��Ƿ���ȫ�����Ϊ��
            bool node_empty = true;
            // �����ӽڵ�
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

            // �����Ϊ�գ���ô˵������ɾ
            if (!node_empty) break;

            // ���㸸�ڵ�
            CarpNumberNode* parent = i >= 1 ? list[i - 1] : nullptr;
            // �����һ���ֵܽڵ�
            auto* first_child = parent ? parent->child : &m_root;

            // �����һ���ڵ����node
            // ��ô��nodeɾ����Ȼ���node���ֵܽڵ㽻��parent->child
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
        // ��ʼ��Ϊ���ڵ�
        auto* node = &m_root;
        // ��ʼ�����ڵ�
        CarpNumberNode* parent = nullptr;
        // ���������ַ�
        for (size_t i = 0; i < number.size(); ++i)
        {
            // ��ȡ����
            unsigned char c = number[i] - '0';

            // �����ֵܽڵ�
            while (true)
            {
                // ��ȡ��Χ
                auto min = node->Min();
                auto max = node->Max();

                // ���С�ڵ�ǰ�ڵ�
                if (c < min)
                {
                    // ����һ���µĽڵ㣬���뵽node֮ǰ
                    auto* new_node = new CarpNumberNode(c, c, node);
                    // �ӵ�һ��node�ֵܽڵ㣬�ҵ���nodeǰ����Ǹ��ֵܽڵ�
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

                    // ���½ڵ�����Ϊnode
                    node = new_node;
                }
                else if (c <= max)
                {
                    // ������ֹ��ϲ�����ô�Ͳ�
                    if (min != max)
                    {
                        // ��� [min, c], [c+1, max]
                        if (c == min)
                        {
                            node->SetMinMax(min, c);
                            auto* new_node = new CarpNumberNode(c + 1, max, node->brother);
                            if (node->child) new_node->child = node->child->Clone();
                            node->brother = new_node;
                        }
                        // ��� [min, c-1], [c, max]
                        else if (c == max)
                        {
                            node->SetMinMax(min, c - 1);
                            auto* new_node = new CarpNumberNode(c, max, node->brother);
                            if (node->child) new_node->child = node->child->Clone();
                            node->brother = new_node;
                            node = new_node;
                        }
                        // ��� [min, c-1], [c, c], [c+1, max]
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
                    // ���û���ֵܽڵ��ˣ���ô�ʹ���һ������
                    if (new_node == nullptr)
                    {
                        new_node = new CarpNumberNode(c, c, nullptr);
                        node->brother = new_node;
                        node = new_node;
                    }
                    else
                    {
                        // �����ֵܽڵ�
                        node = new_node;
                        // ������һ��ѭ��
                        continue;
                    }
                }

                // �ܵ����˵�����ҵ���ָ���Ľڵ�
                break;
            }

            if (list != nullptr) list->push_back(node);

            // ������������һ������ô������
            if (i + 1 >= number.size())
            {
                // ���Ϊ���ַ���
                if (!node->flag) ++m_size;
                node->flag = true;
                break;
            }

            // ���û���ӽڵ㣬��ô�ʹ���һ��
            if (node->child == nullptr)
            {
                auto next_c = number[i + 1] - '0';
                node->child = new CarpNumberNode(next_c, next_c, nullptr);
            }

            // ���ӽڵ���Ϊ��һ�����ڵ�
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