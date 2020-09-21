#ifndef CARP_STRINGHELPER_INCLUDED
#define CARP_STRINGHELPER_INCLUDED (1)

#include <list>
#include <string>
#include <vector>

// 数字和字符串互转，请使用std::to_string和std::atoi

class CarpStringHelper
{
public:
    // 删除左边的空格
    static void TrimLeft(std::string& target)
    {
        std::string::size_type pos = target.find_first_not_of(' ');
        if (pos == std::string::npos) return;
        if (pos == 0) return;

        target = target.substr(pos);
    }

    // 删除右边的空格
    static void TrimRight(std::string& target)
    {
        std::string::size_type pos = target.find_last_not_of(' ');
        if (pos == std::string::npos) return;
        if (pos + 1 == target.size()) return;

        target = target.substr(0, pos + 1);
    }

    // 改为大写
    static void UpperString(std::string& target)
    {
        for (size_t i = 0; i < target.size(); ++i)
        {
            char c = target[i];
            if (c >= 'a' && c <= 'z')
            {
                c -= 'a' - 'A';
                target[i] = c;
            }
        }
    }

    // 把整形IP转为字符串
    static std::string Ip2String(unsigned int ip)
    {
        char ip_dest[16] = { 0 };
#ifdef _WIN32
        sprintf_s(ip_dest, "%lu.%lu.%lu.%lu", ip >> 24, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
#else
        sprintf(ip_dest, "%lu.%lu.%lu.%lu", ip >> 24, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
#endif
        return std::string(ip_dest);
    }

    // 判断是否是数字
    static bool IsNumber(char value) { return value >= '0' && value <= '9'; }
    // 判断是否是字母
    static bool IsLetter(char value) { return (value >= 'A' && value <= 'Z') || (value >= 'a' && value <= 'z'); }

    // 双引号转移
    static std::string DoubleQouteEscape(const std::string& value)
    {
        std::string result;
        for (auto& c : value)
        {
            if (c == '\\')
            {
                result.push_back(c);
                result.push_back(c);
            }
            else if (c == '"')
            {
                result.push_back('\"');
                result.push_back(c);
            }
            else if (c == '\t')
            {
                result.push_back('\\');
                result.push_back('t');
            }
            else if (c == '\r')
            {
                result.push_back('\\');
                result.push_back('r');
            }
            else if (c == '\n')
            {
                result.push_back('\\');
                result.push_back('n');
            }
            else
            {
                result.push_back(c);
            }
        }
        return result;
    }

private:
    template <typename T>
    static void AddElement(T& list, const std::string& v) { list.push_back(v); }
    static void AddElement(std::list<int>& list, const std::string& v) { list.push_back(std::atoi(v.c_str())); }
    static void AddElement(std::vector<int>& list, const std::string& v) { list.push_back(std::atoi(v.c_str())); }

public:
    // 切割字符串
    template <typename T>
    static void Split(const std::string& content, const std::string& split, T& list)
    {
        list.resize(0);
        if (content.size() == 0) return;

        size_t start_index = 0;
        while (true)
        {
            size_t pos = content.find(split, start_index);
            if (pos != std::string::npos)
            {
                AddElement(list, content.substr(start_index, pos - start_index));
                start_index = pos + split.size();
                continue;
            }
            break;
        }

        if (start_index == 0)
            AddElement(list, content);
        else
            AddElement(list, content.substr(start_index));
    }

private:
    template <typename T>
    static void AddString(std::string& result, const T& v) { result.append(std::to_string(v)); }
    static void AddString(std::string& result, const std::string& v) { result.append(v); }

public:
    // 拼接字符串
    template <typename T>
    static std::string Join(const T& container, const std::string& split)
    {
        std::string result;
        size_t count = 0;
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            ++count;
            AddString(result, *it);
            if (count != container.size())
                result.append(split);

        }
        return result;
    }

};

#endif