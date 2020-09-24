#ifndef CARP_FILECACHE_INCLUDED
#define CARP_FILECACHE_INCLUDED (1)
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


#include "carp_string.hpp"
#include "carp_time.hpp"

class CarpFileChunk
{
public:
	CarpFileChunk(int unit_size)
	{
		if (unit_size <= 0)
			m_unit_size = 1024 - 4;
		else
			m_unit_size = unit_size;
	}
	~CarpFileChunk()
	{
		Clear();
	}

public:
	enum Status
	{
		FILECACHESTATUS_IDLE,           // ����
		FILECACHESTATUS_LOADING,        // ������
		FILECACHESTATUS_LOADED,         // �������
	};

public:
	// ��ȡ״̬
	Status GetStatus() const { return m_status; }
	// ����״̬
	void SetStatus(Status status) { m_status = status; }
	// ��ȡ��С
	int GetSize() const { return m_total_size; }

public:
	// ������д�뻺��
	void Write(void* memory, int size)
	{
		if (m_status == FILECACHESTATUS_LOADED) return;

		// �ѵ�ǰ���ݴ�С��ӵ��ܴ�С
		m_total_size += size;

		char* memory_tmp = static_cast<char*>(memory);
		while (size > 0)
		{
			// ������һ���黹δ��������ô�����
			if (m_last_empty_size > 0 && !m_memory_list.empty())
			{
				char* last_memory = static_cast<char*>(m_memory_list.back());
				if (m_last_empty_size >= size)
				{
					memcpy(last_memory + m_unit_size - m_last_empty_size, memory_tmp, size);
					m_last_empty_size -= size;
					memory_tmp += size;
					size = 0;
				}
				else
				{
					memcpy(last_memory + m_unit_size - m_last_empty_size, memory_tmp, m_last_empty_size);
					size -= m_last_empty_size;
					memory_tmp += m_last_empty_size;
					m_last_empty_size = 0;
				}
			}
			// ����һ���µ��ڴ�飬���������
			else
			{
				void* new_memory = malloc(m_unit_size);
				m_memory_list.push_back(new_memory);
				if (m_unit_size >= size)
				{
					memcpy(new_memory, memory_tmp, size);
					m_last_empty_size = m_unit_size - size;
					memory_tmp += size;
					size = 0;
				}
				else
				{
					memcpy(new_memory, memory_tmp, m_unit_size);
					size -= m_unit_size;
					memory_tmp += m_unit_size;
					m_last_empty_size = 0;
				}
			}
		}
	}
	// �ӻ����ܶ�ȡ����
	int Read(int offset, void* buffer, int size)
	{
		// ����ƫ��
		int index = offset / m_unit_size;
		int unit_offset = offset % m_unit_size;

		char* new_memory = static_cast<char*>(buffer);
		int last_buffer_size = size;

		int total_size = 0;
		while (true)
		{
			// ����Ƿ񳬳���Χ
			if (index >= static_cast<int>(m_memory_list.size()))
				break;

			// ����������Ѿ�û���㹻λ�ã���ô������
			if (last_buffer_size <= 0)
				break;

			// ��ȡ�ڴ�λ��
			char* file_memory = static_cast<char*>(m_memory_list[index]) + unit_offset;
			// ���㵱ǰ���ݿ�ʣ������ݴ�С
			int unit_last_size = m_unit_size - unit_offset;
			// �����ǰ�����һ�����ݿ飬����ʵ�ʴ�С���鿴���ݴ�С
			if (m_last_empty_size > 0 && index + 1 == static_cast<int>(m_memory_list.size()))
				unit_last_size = (m_unit_size - m_last_empty_size) - unit_offset;
			// ���ݴ�С����仺����
			if (unit_last_size > last_buffer_size)
			{
				memcpy(new_memory, file_memory, last_buffer_size);
				unit_offset += last_buffer_size;
				new_memory += last_buffer_size;
				total_size += last_buffer_size;
				last_buffer_size = 0;
			}
			else
			{
				memcpy(new_memory, file_memory, unit_last_size);
				unit_offset = 0;
				++index;
				new_memory += unit_last_size;
				total_size += unit_last_size;
				last_buffer_size -= unit_last_size;
			}
		}

		// ��������������
		return total_size;
	}

	// ������
	void Clear()
	{
		// �ͷ��ڴ�
		for (size_t i = 0; i < m_memory_list.size(); ++i)
			free(m_memory_list[i]);
		m_memory_list.clear();

		// ��������
		m_last_empty_size = 0;
		m_total_size = 0;
		m_status = FILECACHESTATUS_IDLE;
	}

private:
	Status m_status = FILECACHESTATUS_IDLE;					// ����״̬
	int m_unit_size = 0;					// �ڴ���С

	int m_last_empty_size = 0;				// ���һ������ڴ��С
	int m_total_size = 0;					// �ܴ�С
	std::vector<void*> m_memory_list;	// �ڴ���б�
};

typedef std::shared_ptr<CarpFileChunk> CarpFileChunkPtr;

class CarpFileCache
{
public:
	friend class CarpFileCacheGroup;
	~CarpFileCache() { Close(); }

private:
	CarpFileCache() {}

public:
	// �Ƿ��
	bool IsOpen() const { return m_native_file || m_cache; }

	// �ر��ļ�
	void Close()
	{
		if (m_cache && m_cache->GetStatus() != CarpFileChunk::FILECACHESTATUS_LOADED)
			m_cache->Clear();
		m_cache = CarpFileChunkPtr();
		m_offset = 0;

		if (m_native_file) fclose(m_native_file);
		m_native_file = nullptr;
		m_native_size = 0;
	}

public:
	// ��ȡ�ļ���С
	int GetFileSize() const
	{
		if (m_native_file) return m_native_size;

		if (!m_cache) return 0;
		return m_cache->GetSize();
	}
	
	// ����ƫ��
	void SetOffset(int offset)
	{
		// �����ԭʼ�ļ�����ô��ֱ�Ӳ���ԭʼ�ļ�
		if (m_native_file)
		{
			fseek(m_native_file, offset, SEEK_SET);
			m_offset = offset;

			// ���������ƫ�ƣ���ô�ͷ�������
			if (m_cache)
			{
				m_cache->Clear();
				m_cache = CarpFileChunkPtr();
			}
		}
		// �����cache����ôֱ������ƫ��
		else if (m_cache)
		{
			m_offset = offset;
		}
	}
	
	// ��ȡ�ļ�
	int Read(void* buffer, int size)
	{
		int read_size = 0;

		// �����ԭʼ�ļ�����ô�Ͷ�ԭʼ�ļ�
		if (m_native_file)
		{
			read_size = (int)fread(buffer, 1, size, m_native_file);
			m_offset += read_size;

			// �������cache��˵����ǰ�ǵ�һ��������ļ���Cache����ô��д��cache
			if (m_cache)
			{
				m_cache->Write(buffer, read_size);
				// ���ȫ��д��֮����ô��cache���Ϊ�Ѽ������
				if (m_offset >= m_native_size) m_cache->SetStatus(CarpFileChunk::FILECACHESTATUS_LOADED);
			}
		}
		// �������ȫ���ص�cache����ôֱ�Ӵ�cache�����ȡ����
		else if (m_cache)
		{
			read_size = m_cache->Read(m_offset, buffer, size);
			m_offset += read_size;
		}

		return read_size;
	}

private:
	CarpFileChunkPtr m_cache;  // �ļ�����
	int m_offset = 0;          // ƫ��

private:
	FILE* m_native_file = nullptr;  // ԭʼ�ļ���ͬһ���ļ�ָ��ֻ�ᱻһ��CarpFileCache���к�ʹ��
	int m_native_size = 0;          // ԭʼ�ļ���С
};

typedef std::shared_ptr<CarpFileCache> CarpFileCachePtr;

class CarpFileCacheGroup
{
public:
	CarpFileCacheGroup() {}
	~CarpFileCacheGroup() { ClearAll(); }

public:
	// �����ļ���������
	void SetMaxSize(int max_size)
	{
		m_max_size = max_size;
		ClearByMaxSize(m_max_size);
	}

public:
	// ������ʹ�õ�ʱ��С��time
	// ��ô�������
	void ClearByTime(int time)
	{
		auto it = m_cache_map.begin();
		while (it != m_cache_map.end())
		{
			if (it->second.update_time < time)
			{
				m_total_size -= it->second.native_size;
				it = m_cache_map.erase(it);
			}
			else
				++it;
		}
	}

	// ���ĳЩ�ļ�����size�Ĵ�С
	// ��ô�������
	void ClearBySize(int size)
	{
		auto it = m_cache_map.begin();
		while (it != m_cache_map.end())
		{
			if (it->second.chunk->GetSize() > size)
			{
				m_total_size -= it->second.native_size;
				it = m_cache_map.erase(it);
			}
			else
				++it;
		}
	}

	// �����ǰ�����ļ��Ĵ�С�ܺͳ���max_size
	// ��ô������max_sizeΪֹ
	void ClearByMaxSize(int max_size)
	{
		if (m_total_size <= max_size) return;

		auto it = m_cache_map.begin();
		while (it != m_cache_map.end())
		{
			if (m_total_size <= max_size) return;

			m_total_size -= it->second.native_size;
			it = m_cache_map.erase(it);
		}
	}

	// �����ļ�·��������
	void ClearByPath(const char* file_path)
	{
		auto it = m_cache_map.find(file_path);
		if (it == m_cache_map.end()) return;

		m_total_size -= it->second.native_size;
		m_cache_map.erase(it);
	}

	// ����ȫ��
	void ClearAll()
	{
		m_cache_map.clear();
		m_total_size = 0;
	}

public:
	CarpFileCachePtr Create(const std::string& file_path, bool use_cache)
	{
		CarpFileCachePtr cache = CarpFileCachePtr(new CarpFileCache());

		// ���ʹ�û��棬��ô�ӻ����д���
		if (use_cache)
		{
			if (CreateFromCache(file_path, cache->m_cache, cache->m_native_file, cache->m_native_size))
				return cache;
			return nullptr;
		}

		// ��ʹ�û��棬��ô��ֱ�Ӵ�ԭʼ�ļ�
		if (!OpenNativeFile(file_path, cache->m_native_file, cache->m_native_size))
			return cache;

		return nullptr;
	}

private:
	static bool OpenNativeFile(const std::string& file_path, FILE*& native_file, int& native_size)
	{
		native_file = nullptr;
		native_size = 0;
		
		// open native file
#ifdef _WIN32
		_wfopen_s(&native_file, CarpString::UTF82Unicode(file_path).c_str(), L"rb");
#else
		native_file = fopen(file_path.c_str(), "rb");
#endif
		if (native_file == nullptr) return false;

		fseek(native_file, 0, SEEK_END);
		native_size = (int)ftell(native_file);
		fseek(native_file, 0, SEEK_SET);

		return true;
	}
	
	bool CreateFromCache(const std::string& file_path, CarpFileChunkPtr& file_cache, FILE*& native_file, int& native_size)
	{
		// init result
		file_cache = CarpFileChunkPtr();
		native_file = 0;
		native_size = 0;

		// find file cache
		auto it = m_cache_map.find(file_path);
		if (it != m_cache_map.end())
		{
			if (it->second.chunk->GetStatus() == CarpFileChunk::FILECACHESTATUS_IDLE)
			{
				// open native file
				if (!OpenNativeFile(file_path, native_file, native_size))
				{
					m_total_size -= it->second.native_size;
					m_cache_map.erase(it);
					return false;
				}
				
				it->second.chunk->SetStatus(CarpFileChunk::FILECACHESTATUS_LOADING);
				file_cache = it->second.chunk;
				it->second.update_time = CarpTime::GetCurTime();
			}
			else if (it->second.chunk->GetStatus() == CarpFileChunk::FILECACHESTATUS_LOADING)
			{
				// open native file
				if (!OpenNativeFile(file_path, native_file, native_size)) return false;
			}
			// ֻ�д�����ȫ����״̬�ſ��Ա��������
			else
			{
				it->second.update_time = CarpTime::GetCurTime();
				file_cache = it->second.chunk;
			}
		}
		else
		{
			// create info and set status
			FileCacheInfo info;
			info.create_time = CarpTime::GetCurTime();
			info.update_time = info.create_time;
			info.chunk = CarpFileChunkPtr(new CarpFileChunk(-1)); // use default unit size
			info.chunk->SetStatus(CarpFileChunk::FILECACHESTATUS_LOADING);

			// open native file
			if (!OpenNativeFile(file_path, native_file, native_size)) return false;
			info.native_size = native_size;

			// save in system
			file_cache = info.chunk;
			m_cache_map[file_path] = info;
			m_total_size += native_size;

			ClearByMaxSize(m_max_size);
		}

		return true;
	}

private:
	struct FileCacheInfo
	{
		CarpFileChunkPtr chunk;			// ����
		time_t create_time = 0;         // ����ʱ��
		time_t update_time = 0;         // ����ʱ��
		int native_size = 0;            // �ļ���С
	};

	std::unordered_map<std::string, FileCacheInfo> m_cache_map;

	int m_total_size = 0;
	int m_max_size = 1024 * 1024 * 100;
};
#endif