#ifndef CARP_STRING_INCLUDED
#define CARP_STRING_INCLUDED

#include <list>
#include <string>
#include <vector>
#include <time.h>

// Êý×ÖºÍ×Ö·û´®»¥×ª£¬ÇëÊ¹ÓÃstd::to_stringºÍstd::atoi

class CarpString
{
public:
    // É¾³ý×ó±ßµÄ¿Õ¸ñ
    static void TrimLeft(std::string& target)
    {
        const auto pos = target.find_first_not_of(' ');
        if (pos == std::string::npos) return;
        if (pos == 0) return;

        target = target.substr(pos);
    }

    // É¾³ýÓÒ±ßµÄ¿Õ¸ñ
    static void TrimRight(std::string& target)
    {
        const auto pos = target.find_last_not_of(' ');
        if (pos == std::string::npos) return;
        if (pos + 1 == target.size()) return;

        target = target.substr(0, pos + 1);
    }

    // ¸ÄÎª´óÐ´
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

    // Éú³ÉÎ¨Ò»×Ö·û´®
    static std::string GenerateID(const std::string& pre)
    {
        return pre + "_" + std::to_string(time(nullptr)) + "_" + std::to_string(rand()) + "_" + std::to_string(rand());
    }

    // ÅÐ¶ÏÊÇ·ñÊÇÊý×Ö
    static bool IsNumber(char value) { return value >= '0' && value <= '9'; }
    // ÅÐ¶ÏÊÇ·ñÊÇ×ÖÄ¸
    static bool IsLetter(char value) { return (value >= 'A' && value <= 'Z') || (value >= 'a' && value <= 'z'); }

    // Ë«ÒýºÅ×ªÒÆ
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
	// utf8×ªunicode
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

	// unicode×ªutf8
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

    static int UTF8GetByteCountOfOneWord(unsigned char first_char)
    {
        unsigned char temp = 0x80;
        int num = 0;

        const unsigned char char_value = first_char;

        if (char_value < 0x80) // ascii code.(0-127)
            return 1;
        while (temp & char_value)
        {
            ++num;
            temp = temp >> 1;
        }

        return num;
    }
	
    static int UTF8CalcWordCount(const char* str)
    {
        int count = 0;
        size_t index = 0;
        while (str[index] != 0)
        {
            index += UTF8GetByteCountOfOneWord(str[index]);
            ++count;
        }
        return count;
    }

    static int UTF8CalcByteCountByWordCount(const char* str, int offset, int word_count)
    {
        int count = 0;
        size_t index = offset;
        while (str[index] != 0)
        {
            index += UTF8GetByteCountOfOneWord(str[index]);
            ++count;
            if (count >= word_count) break;
        }
        return static_cast<int>(index) - offset;
    }

#define UNKNOWN_UNICODE 0xFFFD
    static unsigned int GetOneUnicodeFromUTF8(const char* src, size_t src_len, int* increase)
    {
        const auto* p = reinterpret_cast<const unsigned char*>(src);
        size_t left = 0;
        const size_t save_src_len = src_len;
        int overlong = 0;
        int underflow = 0;
        unsigned int ch = UNKNOWN_UNICODE;

        if (src_len == 0) {
            return UNKNOWN_UNICODE;
        }
        if (p[0] >= 0xFC) {
            if ((p[0] & 0xFE) == 0xFC) {
                if (p[0] == 0xFC && (p[1] & 0xFC) == 0x80) {
                    overlong = 1;
                }
                ch = static_cast<unsigned int>(p[0] & 0x01);
                left = 5;
            }
        }
        else if (p[0] >= 0xF8) {
            if ((p[0] & 0xFC) == 0xF8) {
                if (p[0] == 0xF8 && (p[1] & 0xF8) == 0x80) {
                    overlong = 1;
                }
                ch = static_cast<unsigned int>(p[0] & 0x03);
                left = 4;
            }
        }
        else if (p[0] >= 0xF0) {
            if ((p[0] & 0xF8) == 0xF0) {
                if (p[0] == 0xF0 && (p[1] & 0xF0) == 0x80) {
                    overlong = 1;
                }
                ch = static_cast<unsigned int>(p[0] & 0x07);
                left = 3;
            }
        }
        else if (p[0] >= 0xE0) {
            if ((p[0] & 0xF0) == 0xE0) {
                if (p[0] == 0xE0 && (p[1] & 0xE0) == 0x80) {
                    overlong = 1;
                }
                ch = static_cast<unsigned int>(p[0] & 0x0F);
                left = 2;
            }
        }
        else if (p[0] >= 0xC0) {
            if ((p[0] & 0xE0) == 0xC0) {
                if ((p[0] & 0xDE) == 0xC0) {
                    overlong = 1;
                }
                ch = static_cast<unsigned int>(p[0] & 0x1F);
                left = 1;
            }
        }
        else {
            if ((p[0] & 0x80) == 0x00) {
                ch = static_cast<unsigned int>(p[0]);
            }
        }
        --src_len;
        while (left > 0 && src_len > 0) {
            ++p;
            if ((p[0] & 0xC0) != 0x80) {
                ch = UNKNOWN_UNICODE;
                break;
            }
            ch <<= 6;
            ch |= (p[0] & 0x3F);
            --src_len;
            --left;
        }
        if (left > 0) {
            underflow = 1;
        }
        /* Technically overlong sequences are invalid and should not be interpreted.
           However, it doesn't cause a security risk here and I don't see any harm in
           displaying them. The application is responsible for any other side effects
           of allowing overlong sequences (e.g. string compares failing, etc.)
        */
        /* if (overlong) return UNKNOWN_UNICODE; */

        (void)overlong;

        if (underflow ||
            (ch >= 0xD800 && ch <= 0xDFFF) ||
            (ch == 0xFFFE || ch == 0xFFFF) || ch > 0x10FFFF) {
            ch = UNKNOWN_UNICODE;
        }

        if (increase) *increase = static_cast<int>(save_src_len - src_len);

        return ch;
    }

private:
    template <typename T>
    static void AddElement(T& list, const std::string& v, bool ignore_empty) { if (ignore_empty && v.empty()) return; list.push_back(v); }
    static void AddElement(std::list<int>& list, const std::string& v, bool ignore_empty) { if (ignore_empty && v.empty()) return; list.push_back(std::atoi(v.c_str())); }
    static void AddElement(std::vector<int>& list, const std::string& v, bool ignore_empty) { if (ignore_empty && v.empty()) return; list.push_back(std::atoi(v.c_str())); }

public:
    // ÇÐ¸î×Ö·û´®
    template <typename T>
    static void Split(const std::string& content, const std::string& split, bool ignore_empty, T& list)
    {
        list.resize(0);
        if (content.empty()) return;

        size_t start_index = 0;
        while (true)
        {
	        const size_t pos = content.find(split, start_index);
            if (pos != std::string::npos)
            {
                AddElement(list, content.substr(start_index, pos - start_index), ignore_empty);
                start_index = pos + split.size();
                continue;
            }
            break;
        }

        if (start_index == 0)
            AddElement(list, content, ignore_empty);
        else
            AddElement(list, content.substr(start_index), ignore_empty);
    }

private:
    template <typename T>
    static void AddString(std::string& result, const T& v) { result.append(std::to_string(v)); }
    static void AddString(std::string& result, const std::string& v) { result.append(v); }

public:
    // Æ´½Ó×Ö·û´®
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