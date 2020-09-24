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
		FILECACHESTATUS_IDLE,           // 空闲
		FILECACHESTATUS_LOADING,        // 加载中
		FILECACHESTATUS_LOADED,         // 加载完成
	};

public:
	// 获取状态
	Status GetStatus() const { return m_status; }
	// 设置状态
	void SetStatus(Status status) { m_status = status; }
	// 获取大小
	int GetSize() const { return m_total_size; }

public:
	// 将数据写入缓存
	void Write(void* memory, int size)
	{
		if (m_status == FILECACHESTATUS_LOADED) return;

		// 把当前数据大小添加到总大小
		m_total_size += size;

		char* memory_tmp = static_cast<char*>(memory);
		while (size > 0)
		{
			// 如果最后一个块还未填满，那么就填充
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
			// 创建一个新的内存块，并填充数据
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
	// 从缓存总读取数据
	int Read(int offset, void* buffer, int size)
	{
		// 计算偏移
		int index = offset / m_unit_size;
		int unit_offset = offset % m_unit_size;

		char* new_memory = static_cast<char*>(buffer);
		int last_buffer_size = size;

		int total_size = 0;
		while (true)
		{
			// 检查是否超出范围
			if (index >= static_cast<int>(m_memory_list.size()))
				break;

			// 如果缓冲区已经没有足够位置，那么就跳出
			if (last_buffer_size <= 0)
				break;

			// 获取内存位置
			char* file_memory = static_cast<char*>(m_memory_list[index]) + unit_offset;
			// 计算当前数据块剩余的数据大小
			int unit_last_size = m_unit_size - unit_offset;
			// 如果当前是最后一个数据块，根据实际大小来查看数据大小
			if (m_last_empty_size > 0 && index + 1 == static_cast<int>(m_memory_list.size()))
				unit_last_size = (m_unit_size - m_last_empty_size) - unit_offset;
			// 根据大小来填充缓冲区
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

		// 返回填充的数据量
		return total_size;
	}

	// 清理缓存
	void Clear()
	{
		// 释放内存
		for (size_t i = 0; i < m_memory_list.size(); ++i)
			free(m_memory_list[i]);
		m_memory_list.clear();

		// 数据重置
		m_last_empty_size = 0;
		m_total_size = 0;
		m_status = FILECACHESTATUS_IDLE;
	}

private:
	Status m_status = FILECACHESTATUS_IDLE;					// 缓存状态
	int m_unit_size = 0;					// 内存块大小

	int m_last_empty_size = 0;				// 最后一个块的内存大小
	int m_total_size = 0;					// 总大小
	std::vector<void*> m_memory_list;	// 内存块列表
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
	// 是否打开
	bool IsOpen() const { return m_native_file || m_cache; }

	// 关闭文件
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
	// 获取文件大小
	int GetFileSize() const
	{
		if (m_native_file) return m_native_size;

		if (!m_cache) return 0;
		return m_cache->GetSize();
	}
	
	// 设置偏移
	void SetOffset(int offset)
	{
		// 如果有原始文件，那么就直接操作原始文件
		if (m_native_file)
		{
			fseek(m_native_file, offset, SEEK_SET);
			m_offset = offset;

			// 如果设置了偏移，那么就放弃缓存
			if (m_cache)
			{
				m_cache->Clear();
				m_cache = CarpFileChunkPtr();
			}
		}
		// 如果有cache，那么直接设置偏移
		else if (m_cache)
		{
			m_offset = offset;
		}
	}
	
	// 读取文件
	int Read(void* buffer, int size)
	{
		int read_size = 0;

		// 如果有原始文件，那么就读原始文件
		if (m_native_file)
		{
			read_size = (int)fread(buffer, 1, size, m_native_file);
			m_offset += read_size;

			// 如果持有cache，说明当前是第一个打开这个文件的Cache，那么就写入cache
			if (m_cache)
			{
				m_cache->Write(buffer, read_size);
				// 如果全部写入之后，那么把cache标记为已加载完成
				if (m_offset >= m_native_size) m_cache->SetStatus(CarpFileChunk::FILECACHESTATUS_LOADED);
			}
		}
		// 如果有完全加载的cache，那么直接从cache里面读取数据
		else if (m_cache)
		{
			read_size = m_cache->Read(m_offset, buffer, size);
			m_offset += read_size;
		}

		return read_size;
	}

private:
	CarpFileChunkPtr m_cache;  // 文件缓存
	int m_offset = 0;          // 偏移

private:
	FILE* m_native_file = nullptr;  // 原始文件，同一个文件指针只会被一个CarpFileCache持有和使用
	int m_native_size = 0;          // 原始文件大小
};

typedef std::shared_ptr<CarpFileCache> CarpFileCachePtr;

class CarpFileCacheGroup
{
public:
	CarpFileCacheGroup() {}
	~CarpFileCacheGroup() { ClearAll(); }

public:
	// 设置文件缓存上限
	void SetMaxSize(int max_size)
	{
		m_max_size = max_size;
		ClearByMaxSize(m_max_size);
	}

public:
	// 如果最后使用的时间小于time
	// 那么就清理掉
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

	// 如果某些文件超过size的大小
	// 那么就清理掉
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

	// 如果当前所有文件的大小总和超过max_size
	// 那么就清理到max_size为止
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

	// 根据文件路径来清理
	void ClearByPath(const char* file_path)
	{
		auto it = m_cache_map.find(file_path);
		if (it == m_cache_map.end()) return;

		m_total_size -= it->second.native_size;
		m_cache_map.erase(it);
	}

	// 清理全部
	void ClearAll()
	{
		m_cache_map.clear();
		m_total_size = 0;
	}

public:
	CarpFileCachePtr Create(const std::string& file_path, bool use_cache)
	{
		CarpFileCachePtr cache = CarpFileCachePtr(new CarpFileCache());

		// 如果使用缓存，那么从缓存中创建
		if (use_cache)
		{
			if (CreateFromCache(file_path, cache->m_cache, cache->m_native_file, cache->m_native_size))
				return cache;
			return nullptr;
		}

		// 不使用缓存，那么就直接打开原始文件
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
			// 只有处于完全加载状态才可以被多个引用
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
		CarpFileChunkPtr chunk;			// 缓存
		time_t create_time = 0;         // 创建时间
		time_t update_time = 0;         // 更新时间
		int native_size = 0;            // 文件大小
	};

	std::unordered_map<std::string, FileCacheInfo> m_cache_map;

	int m_total_size = 0;
	int m_max_size = 1024 * 1024 * 100;
};
#endif