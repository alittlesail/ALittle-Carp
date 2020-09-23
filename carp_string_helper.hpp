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
    static std::string DoubleQuoteEscape(const std::string& value)
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

public:
	// utf8转unicode
	static std::wstring UTF82Unicode(const std::string& utf8)
	{
        unsigned char* str = (unsigned char*)utf8.c_str();
        std::wstring buffer;
        unsigned int c;
        while (*str) {
            if (!(*str & 0x80))
                buffer.push_back(*str++);
            else if ((*str & 0xe0) == 0xc0) {
                if (*str < 0xc2) return buffer;
                c = (*str++ & 0x1f) << 6;
                if ((*str & 0xc0) != 0x80) return buffer;
                buffer.push_back(c + (*str++ & 0x3f));
            }
            else if ((*str & 0xf0) == 0xe0) {
                if (*str == 0xe0 && (str[1] < 0xa0 || str[1] > 0xbf)) return buffer;
                if (*str == 0xed && str[1] > 0x9f) return buffer; // str[1] < 0x80 is checked below
                c = (*str++ & 0x0f) << 12;
                if ((*str & 0xc0) != 0x80) return buffer;
                c += (*str++ & 0x3f) << 6;
                if ((*str & 0xc0) != 0x80) return buffer;
                buffer.push_back(c + (*str++ & 0x3f));
            }
            else if ((*str & 0xf8) == 0xf0) {
                if (*str > 0xf4) return buffer;
                if (*str == 0xf0 && (str[1] < 0x90 || str[1] > 0xbf)) return buffer;
                if (*str == 0xf4 && str[1] > 0x8f) return buffer; // str[1] < 0x80 is checked below
                c = (*str++ & 0x07) << 18;
                if ((*str & 0xc0) != 0x80) return buffer;
                c += (*str++ & 0x3f) << 12;
                if ((*str & 0xc0) != 0x80) return buffer;
                c += (*str++ & 0x3f) << 6;
                if ((*str & 0xc0) != 0x80) return buffer;
                c += (*str++ & 0x3f);
                // utf-8 encodings of values used in surrogate pairs are invalid
                if ((c & 0xFFFFF800) == 0xD800) return buffer;
                if (c >= 0x10000) {
                    c -= 0x10000;
                    buffer.push_back(0xD800 | (0x3ff & (c >> 10)));
                    buffer.push_back(0xDC00 | (0x3ff & (c)));
                }
            }
            else
                return buffer;
        }
		
        return buffer;
	}

	// unicode转utf8
	static std::string Unicode2UTF8(const std::wstring& unicode)
	{
        const wchar_t* str = unicode.c_str();
        std::string buffer;
        while (*str) {
            if (*str < 0x80) {
                buffer.push_back((char)*str++);
            }
            else if (*str < 0x800) {
                buffer.push_back(0xc0 + (*str >> 6));
                buffer.push_back(0x80 + (*str & 0x3f));
                str += 1;
            }
            else if (*str >= 0xd800 && *str < 0xdc00) {
                unsigned int c;
                c = ((str[0] - 0xd800) << 10) + ((str[1]) - 0xdc00) + 0x10000;
                buffer.push_back(0xf0 + (c >> 18));
                buffer.push_back(0x80 + ((c >> 12) & 0x3f));
                buffer.push_back(0x80 + ((c >> 6) & 0x3f));
                buffer.push_back(0x80 + ((c) & 0x3f));
                str += 2;
            }
            else if (*str >= 0xdc00 && *str < 0xe000) {
                return buffer;
            }
            else {
                buffer.push_back(0xe0 + (*str >> 12));
                buffer.push_back(0x80 + ((*str >> 6) & 0x3f));
                buffer.push_back(0x80 + ((*str) & 0x3f));
                str += 1;
            }
        }
        return buffer;
	}

#define UNKNOWN_UNICODE 0xFFFD
    static unsigned int GetOneUnicodeFromUTF8(const char* src, size_t srclen, int* increase)
    {
        const unsigned char* p = (const unsigned char*)src;
        size_t left = 0;
        size_t save_srclen = srclen;
        int overlong = 0;
        int underflow = 0;
        unsigned int ch = UNKNOWN_UNICODE;

        if (srclen == 0) {
            return UNKNOWN_UNICODE;
        }
        if (p[0] >= 0xFC) {
            if ((p[0] & 0xFE) == 0xFC) {
                if (p[0] == 0xFC && (p[1] & 0xFC) == 0x80) {
                    overlong = 1;
                }
                ch = (unsigned int)(p[0] & 0x01);
                left = 5;
            }
        }
        else if (p[0] >= 0xF8) {
            if ((p[0] & 0xFC) == 0xF8) {
                if (p[0] == 0xF8 && (p[1] & 0xF8) == 0x80) {
                    overlong = 1;
                }
                ch = (unsigned int)(p[0] & 0x03);
                left = 4;
            }
        }
        else if (p[0] >= 0xF0) {
            if ((p[0] & 0xF8) == 0xF0) {
                if (p[0] == 0xF0 && (p[1] & 0xF0) == 0x80) {
                    overlong = 1;
                }
                ch = (unsigned int)(p[0] & 0x07);
                left = 3;
            }
        }
        else if (p[0] >= 0xE0) {
            if ((p[0] & 0xF0) == 0xE0) {
                if (p[0] == 0xE0 && (p[1] & 0xE0) == 0x80) {
                    overlong = 1;
                }
                ch = (unsigned int)(p[0] & 0x0F);
                left = 2;
            }
        }
        else if (p[0] >= 0xC0) {
            if ((p[0] & 0xE0) == 0xC0) {
                if ((p[0] & 0xDE) == 0xC0) {
                    overlong = 1;
                }
                ch = (unsigned int)(p[0] & 0x1F);
                left = 1;
            }
        }
        else {
            if ((p[0] & 0x80) == 0x00) {
                ch = (unsigned int)p[0];
            }
        }
        --srclen;
        while (left > 0 && srclen > 0) {
            ++p;
            if ((p[0] & 0xC0) != 0x80) {
                ch = UNKNOWN_UNICODE;
                break;
            }
            ch <<= 6;
            ch |= (p[0] & 0x3F);
            --srclen;
            --left;
        }
        if (left > 0) {
            underflow = 1;
        }
        /* Technically overlong sequences are invalid and should not be interpreted.
           However, it doesn't cause a security risk here and I don't see any harm in
           displaying them. The application is responsible for any other side effects
           of allowing overlong sequences (e.g. string compares failing, etc.)
           See bug 1931 for sample input that triggers this.
        */
        /* if (overlong) return UNKNOWN_UNICODE; */

        (void)overlong;

        if (underflow ||
            (ch >= 0xD800 && ch <= 0xDFFF) ||
            (ch == 0xFFFE || ch == 0xFFFF) || ch > 0x10FFFF) {
            ch = UNKNOWN_UNICODE;
        }

        if (increase) *increase = (int)(save_srclen - srclen);

        return ch;
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