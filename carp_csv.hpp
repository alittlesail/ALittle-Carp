
#ifndef CARP_CSV_INCLUDED
#define CARP_CSV_INCLUDED (1)

#include <string>
#include <vector>

#include "carp_string.hpp"

#define CARP_CSV_END_OF_FILE 0

class CarpCsv
{
public:
    CarpCsv() {}
    virtual ~CarpCsv() {}

    typedef size_t(*read_file)(void*, size_t, size_t, void*);

public:
	const char* Load(const char* file_path)
    {
        m_temp_string.clear();
        if (!ReadFromStdFile(file_path, &m_temp_string)) return m_temp_string.c_str();
        return nullptr;
    }
	
    bool ReadFromStdFile(const std::string& file_path, std::string* error = nullptr)
    {
        m_file_path = file_path;
        m_data.resize(0);

        FILE* file = nullptr;
#ifdef _WIN32
        _wfopen_s(&file, CarpString::UTF82Unicode(file_path).c_str(), L"rb");
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
            ReadFile((read_file)fread, file, -1);
            fclose(file);
        }
        catch (std::exception& e)
        {
            if (error != nullptr) *error = e.what();
            m_data.resize(0);
            if (file != nullptr) fclose(file);
            return false;
        }
    	
        return true;
    }

	bool ReadFromCustomFile(const std::string& file_path, read_file read_func, void* file, std::string* error = nullptr)
    {
        m_file_path = file_path;
        m_data.resize(0);

        try
        {
            ReadFile((read_file)fread, file, -1);
        }
        catch (std::exception& e)
        {
            if (error != nullptr) *error = e.what();
            m_data.resize(0);
            return false;
        }

        return true;
    }

    size_t GetColCount() const { return m_data.empty() ? 0 : m_data[0].size(); }
    size_t GetRowCount() const { return m_data.size(); }
    const std::vector<std::string>& GetRowData(size_t index) const { return m_data[index]; }

	// ����������±��1��ʼ
	const char* GetCell(size_t row, size_t col) const
	{
        if (row > m_data.size()) return nullptr;
        if (col > m_data[row - 1].size()) return nullptr;
        return m_data[row - 1][col - 1].c_str();
	}
    const char* GetPath() const { return m_file_path.c_str(); }
    void Clear() { m_data.clear(); }

private:
    // ��ȡ�ļ������и�
    void ReadFile(read_file read_func, void* file, int max_row)
    {
        // ��ȡ��һ���ַ�
        char cur_char = CARP_CSV_END_OF_FILE;
        size_t size = read_func(&cur_char, 1, 1, file);
        if (size == 0) Throw("file is empty");
        if (cur_char == 0) Throw("row(" + std::to_string(GetRowCount() + 1) + ") has char is 0");

        // ��ȡ��һ���ַ�
        char next_char = 0;
        size = read_func(&next_char, 1, 1, file);
        if (size == 0)
        {
            m_data.resize(1);
            m_data[0].resize(1);
            m_data[0][0].push_back(cur_char);
            return;
        }
        if (next_char == 0) Throw("row(" + std::to_string(GetRowCount() + 1) + ") has char is 0");

        // ��ǵ�ǰ�Ƿ���������
        bool in_quote = false;

        // ��ǰ������
        std::vector<std::string> row;
        // ��ǰ������
        std::string cell;

        while (true)
        {
            // ����Ƿָ���
            if (cur_char == ',')
            {
                // ����������У���ô��Ҫ��ӵ���ǰ���ַ�
                if (in_quote)
                {
                    // ��ӵ���
                    cell.push_back(cur_char);

                    // �������û�������ˣ�˵��û�йر����ţ�����
                    if (next_char == CARP_CSV_END_OF_FILE) Throw("row(" + std::to_string(GetRowCount()) + ") have no close quote");
                }
                // ���û���������У���ô��Ҫ���и�
                else
                {
                    // ��ӵ���
                    row.push_back(std::move(cell));

                    // �������û��������
                    if (next_char == CARP_CSV_END_OF_FILE)
                    {
                        // ����һ������
                        row.emplace_back(std::string());
                        // ��ӵ���
                        AddRow(row);
                    }
                }
            }
            // ����ǻ���
            else if (cur_char == '\n')
            {
                // ����������У���ô��Ҫ��ӵ���ǰ���ַ�
                if (in_quote)
                {
                    // ��ӵ���
                    cell.push_back(cur_char);

                    // �������û�������ˣ�˵��û�йر����ţ�����
                    if (next_char == CARP_CSV_END_OF_FILE) Throw("row(" + std::to_string(GetRowCount()) + ") have no close quote");
                }
                // ���û���������У���ô��Ҫ���и�
                else
                {
                    // ��ӵ���
                    row.push_back(std::move(cell));
                    // ��ӵ���
                    AddRow(row);
                }
            }
            // ���������
            else if (cur_char == '"')
            {
                // �����������
                if (in_quote)
                {
                    // �����һ��Ҳ�����ţ���ô��ǰֱ��ת��Ϊ����
                    if (next_char == '"')
                    {
                        // ��ӵ���
                        cell.push_back('"');
                        // ֱ�Ӷ�ȡ��һ���ַ�
                        next_char = ReadNextChar(read_func, file);
                        // �������û�������ˣ�˵��û�йر����ţ�����
                        if (next_char == CARP_CSV_END_OF_FILE) Throw("row(" + std::to_string(GetRowCount()) + ") have no close quote");
                    }
                    // �����һ���������ţ���ô�͹ر�����
                    else
                    {
                        in_quote = false;

                        if (next_char == CARP_CSV_END_OF_FILE)
                        {
                            // ��ӵ���
                            row.push_back(std::move(cell));
                            // ��ӵ���
                            AddRow(row);
                        }
                    }
                }
                // ���������У�ֱ�ӱ��Ϊ��������
                else
                {
                    // �����ǰ���ǿյģ���ô�ͱ�����ſ�ʼ
                    if (cell.empty())
                        in_quote = true;
                    // �����ǰ�Ѿ��������ˣ���ôֱ�ӵ�����ͨ�ַ�
                    else
                        cell.push_back(cur_char);
                    
                    // �������û�������ˣ�˵��û�йر����ţ�����
                    if (next_char == CARP_CSV_END_OF_FILE) Throw("row(" + std::to_string(GetRowCount()) + ") have no close quote");
                }
            }
            // �����ַ�ֱ�����
            else
            {
                if (in_quote)
                {
                    // ����������ַ�
                    cell.push_back(cur_char);

                    // �������û�������ˣ�˵��û�йر����ţ�����
                    if (next_char == CARP_CSV_END_OF_FILE) Throw("row(" + std::to_string(GetRowCount()) + ") have no close quote");
                }
                else
                {
                    // ��ӵ���
                    if (!(cur_char == '\r' && next_char == '\n')) cell.push_back(cur_char);

                    // �������û��������
                    if (next_char == CARP_CSV_END_OF_FILE)
                    {
                        // ��ӵ���
                        row.push_back(std::move(cell));
                        // ��ӵ���
                        AddRow(row);
                    }
                }
            }

            // ���û���¸��ַ��ˣ�˵��������
            if (next_char == CARP_CSV_END_OF_FILE) break;

            // �������ָ����������ô������
            if (max_row >= 0 && GetRowCount() >= max_row) break;

            // ��ȡ��һ���ַ�
            cur_char = next_char;
            next_char = ReadNextChar(read_func, file);
        }
    }
	
    // ������ӵ���
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
	
    // ��ȡ�ļ�����һ���ַ�
    char ReadNextChar(read_file read_func, void* file) const
    {
        char next_char = CARP_CSV_END_OF_FILE;
        const size_t size = read_func(&next_char, 1, 1, file);
        if (size != 0 && next_char == 0) Throw("row(" + std::to_string(GetRowCount()) + ") has char is 0");
        return next_char;
    }
	
    // ���쳣
    static void Throw(const std::string& error) { throw std::exception(error.c_str()); }

private:
    std::string m_temp_string;
    std::string m_file_path;
    std::vector<std::vector<std::string>> m_data;
};

#endif