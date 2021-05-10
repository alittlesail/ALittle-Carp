
#ifndef CARP_CSV_INCLUDED
#define CARP_CSV_INCLUDED

#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#define CARP_CSV_END_OF_FILE 0

class CarpCsv
{
public:
    class CarpCsvException : public std::exception
    {
    public:
        CarpCsvException(const std::string& msg) : m_msg(msg) {}
        const std::string& What() const { return m_msg; }

    private:
        std::string m_msg;
    };
public:
    virtual ~CarpCsv() = default;

    // 第一个参数是文件指针，第二个参数是内存地址，第三个参数是元素大小，第四个是元素个数
    typedef size_t(*READ_FILE)(void*, void*, size_t, size_t);

public:
	const char* Load(const char* file_path)
    {
        m_temp_string.clear();
        if (!ReadFromStdFile(file_path, &m_temp_string)) return m_temp_string.c_str();
        return nullptr;
    }

#ifdef _WIN32
    static std::wstring UTF82Unicode(const std::string& utf8)
    {
	    const int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
        std::wstring result;
        if (len >= 1) result.resize(len - 1);
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, const_cast<wchar_t*>(result.c_str()), len);
        return result;
    }
#endif

    static size_t WrapFRead(void* file, void* buffer, size_t size, size_t count) { return fread(buffer, size, count, static_cast<FILE*>(file)); }
	
    bool ReadFromStdFile(const std::string& file_path, std::string* error = nullptr)
    {
        m_file_path = file_path;
        m_data.resize(0);

        FILE* file = nullptr;
#ifdef _WIN32
        _wfopen_s(&file, UTF82Unicode(file_path).c_str(), L"rb");
#else
        file = fopen(file_path.c_str(), "rb");
#endif
        if (file == nullptr)
        {
            if (error) *error ="can't open file";
            return false;
        }

        try
        {
            ReadFile(WrapFRead, file, -1);
            fclose(file);
        }
        catch (CarpCsvException& e)
        {
            if (error != nullptr) *error = e.What();
            m_data.resize(0);
            if (file != nullptr) fclose(file);
            return false;
        }
    	
        return true;
    }

	bool ReadFromCustomFile(const std::string& file_path, READ_FILE read_func, void* file, std::string* error = nullptr)
    {
        m_file_path = file_path;
        m_data.resize(0);

        try
        {
            ReadFile(reinterpret_cast<READ_FILE>(read_func), file, -1);
        }
        catch (CarpCsvException& e)
        {
            if (error != nullptr) *error = e.What();
            m_data.resize(0);
            return false;
        }

        return true;
    }

    virtual size_t GetColCount() const { return m_data.empty() ? 0 : m_data[0].size(); }
    virtual size_t GetRowCount() const { return m_data.size(); }
    const std::vector<std::string>& GetRowData(size_t index) const { return m_data[index]; }

    virtual const char* ReadCell(size_t row, size_t col)
	{
        if (row >= m_data.size() || col >= m_data[row].size())
        {
            m_temp_string.clear();
            return m_temp_string.c_str();
        }
        return m_data[row][col].c_str();
	}
    virtual const char* GetPath() const { return m_file_path.c_str(); }
    virtual void Close() { m_data.clear(); }

public:
    // 以某一列为key，某一列为value，从某一行开始取键值
    const std::string& GetKeyValueString(const std::string& key, const std::string& default_value, size_t start_row = 0, size_t key_col = 0, size_t value_col = 1)
    {
        for (size_t row = start_row + 1; row < m_data.size(); ++row)
        {
            if (key_col >= m_data[row].size()) continue;
            if (value_col >= m_data[row].size()) continue;

            if (m_data[row][key_col] == key) return m_data[row][value_col];
        }

        m_temp_string = default_value;
        return m_temp_string;
    }

    // 以某一列为key，某一列为value，从某一行开始取键值
    int GetKeyValueInt(const std::string& key, int default_value, size_t start_row = 0, size_t key_col = 0, size_t value_col = 1)
    {
        for (size_t row = start_row + 1; row < m_data.size(); ++row)
        {
            if (key_col >= m_data[row].size()) continue;
            if (value_col >= m_data[row].size()) continue;

            if (m_data[row][key_col] == key) return std::atoi(m_data[row][value_col].c_str());
        }

        return default_value;
    }

private:
    // 读取文件，并切割
    void ReadFile(READ_FILE read_func, void* file, int max_row)
    {
        // 读取第一个字符
        char cur_char = CARP_CSV_END_OF_FILE;
        size_t size = read_func(file, &cur_char, 1, 1);
        if (size == 0) Throw("file is empty");
        if (cur_char == 0) Throw("row(" + std::to_string(GetRowCount() + 1) + ") has char is 0");

        // 读取下一个字符
        char next_char = 0;
        size = read_func(file, &next_char, 1, 1);
        if (size == 0)
        {
            m_data.resize(1);
            m_data[0].resize(1);
            m_data[0][0].push_back(cur_char);
            return;
        }
        if (next_char == 0) Throw("row(" + std::to_string(GetRowCount() + 1) + ") has char is 0");

        // 标记当前是否在引号中
        bool in_quote = false;

        // 当前行数据
        std::vector<std::string> row;
        // 当前列数据
        std::string cell;

        while (true)
        {
            // 如果是分隔符
            if (cur_char == ',')
            {
                // 如果在引号中，那么就要添加到当前的字符
                if (in_quote)
                {
                    // 添加到列
                    cell.push_back(cur_char);

                    // 如果后面没有数据了，说明没有关闭引号，报错
                    if (next_char == CARP_CSV_END_OF_FILE) Throw("row(" + std::to_string(GetRowCount()) + ") have no close quote");
                }
                // 如果没有在引号中，那么就要做切割
                else
                {
                    // 添加到行
                    row.push_back(std::move(cell));

                    // 如果后面没有数据了
                    if (next_char == CARP_CSV_END_OF_FILE)
                    {
                        // 再塞一个空列
                        row.emplace_back(std::string());
                        // 添加到表
                        AddRow(row);
                    }
                }
            }
            // 如果是换行
            else if (cur_char == '\n')
            {
                // 如果在引号中，那么就要添加到当前的字符
                if (in_quote)
                {
                    // 添加到列
                    cell.push_back(cur_char);

                    // 如果后面没有数据了，说明没有关闭引号，报错
                    if (next_char == CARP_CSV_END_OF_FILE) Throw("row(" + std::to_string(GetRowCount()) + ") have no close quote");
                }
                // 如果没有在引号中，那么就要做切割
                else
                {
                    // 添加到行
                    row.push_back(std::move(cell));
                    // 添加到表
                    AddRow(row);
                }
            }
            // 如果是引号
            else if (cur_char == '"')
            {
                // 如果在引号中
                if (in_quote)
                {
                    // 如果下一个也是引号，那么当前直接转义为引号
                    if (next_char == '"')
                    {
                        // 添加到列
                        cell.push_back('"');
                        // 直接读取下一个字符
                        next_char = ReadNextChar(read_func, file);
                        // 如果后面没有数据了，说明没有关闭引号，报错
                        if (next_char == CARP_CSV_END_OF_FILE) Throw("row(" + std::to_string(GetRowCount()) + ") have no close quote");
                    }
                    // 如果下一个不是引号，那么就关闭引号
                    else
                    {
                        in_quote = false;

                        if (next_char == CARP_CSV_END_OF_FILE)
                        {
                            // 添加到行
                            row.push_back(std::move(cell));
                            // 添加到表
                            AddRow(row);
                        }
                    }
                }
                // 不在引号中，直接标记为在引号中
                else
                {
                    // 如果当前列是空的，那么就标记引号开始
                    if (cell.empty())
                        in_quote = true;
                    // 如果当前已经有数据了，那么直接当做普通字符
                    else
                        cell.push_back(cur_char);
                    
                    // 如果后面没有数据了，说明没有关闭引号，报错
                    if (next_char == CARP_CSV_END_OF_FILE) Throw("row(" + std::to_string(GetRowCount()) + ") have no close quote");
                }
            }
            // 其他字符直接添加
            else
            {
                if (in_quote)
                {
                    // 无条件添加字符
                    cell.push_back(cur_char);

                    // 如果后面没有数据了，说明没有关闭引号，报错
                    if (next_char == CARP_CSV_END_OF_FILE) Throw("row(" + std::to_string(GetRowCount()) + ") have no close quote");
                }
                else
                {
                    // 添加到列
                    if (!(cur_char == '\r' && next_char == '\n')) cell.push_back(cur_char);

                    // 如果后面没有数据了
                    if (next_char == CARP_CSV_END_OF_FILE)
                    {
                        // 添加到行
                        row.push_back(std::move(cell));
                        // 添加到表
                        AddRow(row);
                    }
                }
            }

            // 如果没有下个字符了，说明结束了
            if (next_char == CARP_CSV_END_OF_FILE) break;

            // 如果到达指定行数，那么就跳出
            if (max_row >= 0 && static_cast<int>(GetRowCount()) >= max_row) break;

            // 读取下一个字符
            cur_char = next_char;
            next_char = ReadNextChar(read_func, file);
        }
    }
	
    // 把行添加到表
    void AddRow(std::vector<std::string>& row)
    {
        const size_t col_count = GetColCount();
        if (col_count == 0)
        {
            m_data.push_back(std::move(row));
            return;
        }

        if (row.size() != col_count) Throw("row(" + std::to_string(GetRowCount() + 1) + ") col: " + std::to_string(row.size()) + " != " + std::to_string(col_count));

        m_data.push_back(std::move(row));
    }
	
    // 读取文件的下一个字符
    char ReadNextChar(READ_FILE read_func, void* file) const
    {
        char next_char = CARP_CSV_END_OF_FILE;
        const size_t size = read_func(file, &next_char, 1, 1);
        if (size != 0 && next_char == 0) Throw("row(" + std::to_string(GetRowCount()) + ") has char is 0");
        return next_char;
    }
	
    // 抛异常
    static void Throw(const std::string& error) { throw CarpCsvException(error); }

private:
    std::string m_temp_string;
    std::string m_file_path;
    std::vector<std::vector<std::string>> m_data;
};

#endif