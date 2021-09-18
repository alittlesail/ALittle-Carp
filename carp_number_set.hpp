
#ifndef CARP_NUMBER_SET_INCLUDED
#define CARP_NUMBER_SET_INCLUDED

#include <string>
#include <vector>

class CarpNumberSet
{
public:
    struct CarpNumberNode
    {
        unsigned char min_max = 0; // ��ǰ�ڵ㷶Χ
        CarpNumberNode* brother = nullptr;   // ��һ���ֵܽڵ�
        CarpNumberNode* child = nullptr;    // �ӽڵ�

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

            // �����ֵܽڵ�
            CarpNumberNode* level_node = node;
            while (true)
            {
                // �����������������
                if (c >= level_node->Min() && c <= level_node->Max()) break;

                CarpNumberNode* brother = level_node->brother;
                // ���û���ֵܽڵ㣬��ô������
                if (brother == nullptr) return false;

                // ����ȵ�һ���ֵܽڵ�С����ô������
                if (c < brother->Min()) return false;

                // ������һ��ѭ��
                level_node = brother;
            }

            // ������������һ������ô��������˵�����ҳɹ�
            if (i + 1 >= number.size()) return true;

            // ���û���ӽڵ㣬��ô�ʹ���һ��
            if (level_node->child == nullptr) return false;

            // ���ӽڵ���Ϊ��һ�����ڵ�
            node = level_node->child;
        }

        return true;
    }

    bool Erase(const std::string& number)
    {
        // �Ȳ��ң����û�о�ֱ�ӷ���
        if (!Find(number)) return false;

        if (number.size() == 1 && number[0] == '0')
        {
            m_contain_zero = false;
            return true;
        }

        // Ȼ����ִ�в��룬��ΪInsert��Ժϲ����Ľڵ���в��
        std::vector<CarpNumberNode*> list;
        list.reserve(number.size());
        Insert(number, &list);

        if (list.empty()) return false;

        // ��Ϊi==0ʱ����m_root�����Բ�������
        for (size_t i = list.size() - 1; i > 0; --i)
        {
            auto* node = list[i];
            auto* parent = list[i - 1];

            // ���û���ӽڵ㣬�ֵܽڵ㣬���Ҹ��ڵ��child�����Լ�����ô�Լ��Ϳ���ɾ����
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

            // �����ֵܽڵ�
            CarpNumberNode* level_node = node;
            while (true)
            {
                auto min = level_node->Min();
                auto max = level_node->Max();

                // �����������������
                if (c >= min && c <= max)
                {
                    // ������ֹ��ϲ�����ô�Ͳ�
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
                // ���û���ֵܽڵ��ˣ���ô�ʹ���һ������
                if (brother == nullptr)
                {
                    brother = new CarpNumberNode(c, c, nullptr);
                    level_node->brother = brother;
                    break;
                }
                
                // ������ֵܽڵ�С����ô�ʹ���һ�����뵽�м䣬Ȼ������
                if (c < brother->Min())
                {
                    auto* new_brother = new CarpNumberNode(c, c, brother);
                    level_node->brother = new_brother;
                    level_node = new_brother;
                    break;
                }
                
                // ������һ��ѭ��
                level_node = brother;
            }

            if (list != nullptr) list->push_back(level_node);

            // ������������һ������ô������
            if (i + 1 >= number.size())
            {
                // ��ȡ���һ���ڵ�
                node = level_node;
                break;
            }

            // ���û���ӽڵ㣬��ô�ʹ���һ��
            if (level_node->child == nullptr)
                level_node->child = new CarpNumberNode(number[i + 1], number[i + 1], nullptr);

            // ���ӽڵ���Ϊ��һ�����ڵ�
            node = level_node->child;
        }
    }

private:
    bool m_contain_zero = false;    // �Ƿ����"0"�ַ���
    CarpNumberNode m_root;
};

#endif