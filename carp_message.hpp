#ifndef CARP_MESSAGE_INCLUDED
#define CARP_MESSAGE_INCLUDED

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

#define CARP_MESSAGE_DATA_OFFSET(data, offset) static_cast<void*>(static_cast<char*>(data) + offset)
#define CARP_MESSAGE_CONST_DATA_OFFSET(data, offset) static_cast<const void*>(static_cast<const char*>(data) + offset)

enum CarpMessage_DeserializeResult
{
	CARP_MESSAGE_DR_NO_DATA = 0,  // 当要反序列化时，没有数据了，这个主要是兼容协议向下兼容
	CARP_MESSAGE_DR_DATA_NOT_ENOUGH = -1,  // 实际的数据长度比需要的小
	CARP_MESSAGE_DR_FLAG_LEN_NOT_ENOUGH = -2,  // 前缀长度不足
};

typedef unsigned int CARP_MESSAGE_SIZE;
typedef int CARP_MESSAGE_ID;
typedef int CARP_MESSAGE_RPCID;

#define CARP_PROTOCOL_HEAD_SIZE (sizeof(CARP_MESSAGE_SIZE) + sizeof(CARP_MESSAGE_ID) + sizeof(CARP_MESSAGE_RPCID))

class CarpMessage
{
public:
	CarpMessage() : __rpc_id(0) {}
	virtual ~CarpMessage() = default;
	typedef CARP_MESSAGE_ID ID_TYPE;

public:
	virtual CARP_MESSAGE_ID GetID() const { return 0; }
	virtual int GetTotalSize() const { return 0; }
	virtual int Serialize(void* data) const { return 0; }
	virtual int Deserialize(const void* data, int len) { return 0; }

public:
	inline CARP_MESSAGE_RPCID GetRpcID() const { return __rpc_id; }
	inline virtual void SetRpcID(CARP_MESSAGE_RPCID rpc_id) { __rpc_id = rpc_id; }

#ifdef _WIN32
	static std::wstring UTF82Unicode(const std::string& utf8)
	{
		const int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
		std::wstring result;
		if (len >= 1) result.resize(len - 1);
		MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, const_cast<wchar_t*>(result.c_str()), len);
		return result;
	}
#endif
	
	// 传出的memory由外部接管释放，内部不做释放
	void* CreateMemoryForSend(int* size) const
	{
		// 获取协议大小
		CARP_MESSAGE_SIZE message_size = GetTotalSize();
		// 获取协议ID
		CARP_MESSAGE_ID message_id = GetID();
		// 获取RPCID
		CARP_MESSAGE_RPCID message_rpcid = GetRpcID();

		// 协议大小 = 协议体大小 + 协议头大小
		const int memory_size = message_size + CARP_PROTOCOL_HEAD_SIZE;

		// 申请内存
		void* memory = malloc(memory_size);
		char* body_memory = static_cast<char*>(memory);

		// 设置协议头信息
		memcpy(body_memory, &message_size, sizeof(CARP_MESSAGE_SIZE));
		body_memory += sizeof(CARP_MESSAGE_SIZE);
		memcpy(body_memory, &message_id, sizeof(CARP_MESSAGE_ID));
		body_memory += sizeof(CARP_MESSAGE_ID);
		memcpy(body_memory, &message_rpcid, sizeof(CARP_MESSAGE_RPCID));
		body_memory += sizeof(CARP_MESSAGE_RPCID);

		// 序列化消息包
		Serialize(body_memory);

		if (size) *size = memory_size;
		return memory;
	}

private:
	CARP_MESSAGE_RPCID __rpc_id;
};

template <typename T>
class CarpMessagePtrWapper : public CarpMessage
{
public:
	CarpMessagePtrWapper() : ptr(0) {}
	CarpMessagePtrWapper(T* o) : ptr(o) {}

public:
	CARP_MESSAGE_ID GetID() const override
	{
		if (ptr == nullptr) return 0;
		return ptr->GetID();
	}
	int GetTotalSize() const override
	{
		if (ptr == nullptr) return 0;
		return ptr->GetTotalSize();
	}
	int Serialize(void* data) const override
	{
		if (ptr == nullptr) return 0;
		return ptr->Serialize(data);
	}
	int Deserialize(const void* data, int len) override
	{
		if (ptr == nullptr) return 0;
		return ptr->Deserialize(data, len);
	}

public:
	T* ptr;
};

// ------------------------------------------------------------------------------------
class CarpMessageTemplate
{
public:
	template <typename T>
	static int GetTotalSize(const T& object)
	{
		return object.GetTotalSize() + sizeof(int);
	}
	template <typename T>
	static int GetTotalSize(const std::set<T>& object)
	{
		int total_size = sizeof(int);

		for (auto it = object.begin(); it != object.end(); ++it)
			total_size += GetTotalSize(*it);

		return total_size;
	}
	template <typename T>
	static int GetTotalSize(const std::list<T>& object)
	{
		int total_size = sizeof(int);

		for (auto it = object.begin(); it != object.end(); ++it)
			total_size += GetTotalSize(*it);

		return total_size;
	}
	template <typename T>
	static int GetTotalSize(const std::vector<T>& object)
	{
		int total_size = sizeof(int);

		for (size_t i = 0; i < object.size(); ++i)
			total_size += GetTotalSize(object[i]);

		return total_size;
	}
	template <typename K, typename V>
	static int GetTotalSize(const std::map<K, V>& object)
	{
		int total_size = sizeof(int);

		for (auto it = object.begin(); it != object.end(); ++it)
		{
			total_size += GetTotalSize(it->first);
			total_size += GetTotalSize(it->second);
		}

		return total_size;
	}

	template <> static int GetTotalSize<std::string>(const std::string& object) /* include '\0' */ { return static_cast<int>(object.size()) + static_cast<int>(sizeof(int)) + 1; }
	template <> static int GetTotalSize<bool>(const bool& object) { return static_cast<int>(sizeof(bool)); }
	template <> static int GetTotalSize<char>(const char& object) { return static_cast<int>(sizeof(char)); }
	template <> static int GetTotalSize<unsigned char>(const unsigned char& object) { return static_cast<int>(sizeof(unsigned char)); }
	template <> static int GetTotalSize<short>(const short& object) { return static_cast<int>(sizeof(short)); }
	template <> static int GetTotalSize<unsigned short>(const unsigned short& object) { return static_cast<int>(sizeof(unsigned short)); }
	template <> static int GetTotalSize<int>(const int& object) { return static_cast<int>(sizeof(int)); }
	template <> static int GetTotalSize<unsigned int>(const unsigned int& object) { return static_cast<int>(sizeof(unsigned int)); }
	template <> static int GetTotalSize<long>(const long& object) { return static_cast<int>(sizeof(long)); }
	template <> static int GetTotalSize<unsigned long>(const unsigned long& object) { return static_cast<int>(sizeof(unsigned long)); }
	template <> static int GetTotalSize<long long>(const long long& object) { return static_cast<int>(sizeof(long long)); }
	template <> static int GetTotalSize<unsigned long long>(const unsigned long long& object) { return static_cast<int>(sizeof(unsigned long long)); }
	template <> static int GetTotalSize<float>(const float& object) { return static_cast<int>(sizeof(float)); }
	template <> static int GetTotalSize<double>(const double& object) { return static_cast<int>(sizeof(double)); }

	// ------------------------------------------------------------------------------------
	template <typename T>
	static inline int SerializePrimary(const T& object, void* data)
	{
		*static_cast<T*>(data) = object;
		return static_cast<int>(sizeof(T));
	}

	template <typename T>
	static int Serialize(const T& object, void* data)
	{
		const int result = object.Serialize(CARP_MESSAGE_DATA_OFFSET(data, sizeof(int)));
		*static_cast<int*>(data) = result;
		return result + sizeof(int);
	}
	template <typename T>
	static int Serialize(const std::set<T>& object, void* data)
	{
		// get size
		const int len = static_cast<int>(object.size());
		*static_cast<int*>(data) = len;
		// offset current data
		char* current_data = static_cast<char*>(data) + sizeof(int);
		for (auto it = object.begin(); it != object.end(); ++it)
			current_data += Serialize(*it, current_data);

		// return serialize size
		return static_cast<int>(static_cast<char*>(current_data) - static_cast<char*>(data));
	}
	template <typename T>
	static int Serialize(const std::list<T>& object, void* data)
	{
		// get size
		const int len = static_cast<int>(object.size());
		*static_cast<int*>(data) = len;
		// offset current data
		char* current_data = static_cast<char*>(data) + sizeof(int);
		for (auto it = object.begin(); it != object.end(); ++it)
			current_data += Serialize(*it, current_data);

		// return serialize size
		return static_cast<int>(static_cast<char*>(current_data) - static_cast<char*>(data));
	}
	template <typename T>
	static int Serialize(const std::vector<T>& object, void* data)
	{
		// get size
		const int len = static_cast<int>(object.size());
		*static_cast<int*>(data) = len;
		// offset current data
		char* current_data = static_cast<char*>(data) + sizeof(int);
		for (int i = 0; i < len; ++i)
			current_data += Serialize(object[i], current_data);

		// return serialize size
		return static_cast<int>(static_cast<char*>(current_data) - static_cast<char*>(data));
	}
	template <typename K, typename V>
	static int Serialize(const std::map<K, V>& object, void* data)
	{
		const int len = static_cast<int>(object.size());
		*static_cast<int*>(data) = len;

		char* current_data = static_cast<char*>(data) + sizeof(int);
		for (auto it = object.begin(); it != object.end(); ++it)
		{
			current_data += Serialize(it->first, current_data);
			current_data += Serialize(it->second, current_data);
		}

		return static_cast<int>(static_cast<char*>(current_data) - static_cast<char*>(data));
	}

	template <> static int Serialize<std::string>(const std::string& object, void* data)
	{
		// get string length
		const int len = static_cast<int>(object.size());
		// add '\0' to length
		*static_cast<int*>(data) = len + 1;
		// offset to string position
		char* data_string = static_cast<char*>(data) + sizeof(int);
		// copy string
		for (int i = 0; i < len; ++i) data_string[i] = object.at(i);
		// set tail of string to 0
		data_string[len] = 0;
		// return length
		return len + static_cast<int>(sizeof(int)) + 1;
	}
	template <> static int Serialize<bool>(const bool& object, void* data) { return SerializePrimary(object, data); }
	template <> static int Serialize<char>(const char& object, void* data) { return SerializePrimary(object, data); }
	template <> static int Serialize<unsigned char>(const unsigned char& object, void* data) { return SerializePrimary(object, data); }
	template <> static int Serialize<short>(const short& object, void* data) { return SerializePrimary(object, data); }
	template <> static int Serialize<unsigned short>(const unsigned short& object, void* data) { return SerializePrimary(object, data); }
	template <> static int Serialize<int>(const int& object, void* data) { return SerializePrimary(object, data); }
	template <> static int Serialize<unsigned int>(const unsigned int& object, void* data) { return SerializePrimary(object, data); }
	template <> static int Serialize<long>(const long& object, void* data) { return SerializePrimary(object, data); }
	template <> static int Serialize<unsigned long>(const unsigned long& object, void* data) { return SerializePrimary(object, data); }
	template <> static int Serialize<long long>(const long long& object, void* data) { return SerializePrimary(object, data); }
	template <> static int Serialize<unsigned long long>(const unsigned long long& object, void* data) { return SerializePrimary(object, data); }
	template <> static int Serialize<float>(const float& object, void* data) { return SerializePrimary(object, data); }
	template <> static int Serialize<double>(const double& object, void* data) { return SerializePrimary(object, data); }


	// ------------------------------------------------------------------------------------
	template <typename T>
	static int DeserializePrimary(T& object, const void* data, int len)
	{
		if (len == 0)
		{
			object = 0;
			return CARP_MESSAGE_DR_NO_DATA;
		}
		if (len < static_cast<int>(sizeof(T))) return CARP_MESSAGE_DR_DATA_NOT_ENOUGH;
		object = *static_cast<const T*>(data);
		return static_cast<int>(sizeof(T));
	}

	template <typename T>
	static int Deserialize(T& object, const void* data, int len)
	{
		if (len == 0)
		{
			object = T();
			return CARP_MESSAGE_DR_NO_DATA;
		}

		// check size enough
		if (len < static_cast<int>(sizeof(int))) return CARP_MESSAGE_DR_DATA_NOT_ENOUGH;
		len -= static_cast<int>(sizeof(int)); // desc size of head

		int object_len = *static_cast<const int*>(data);
		if (object_len < 0) return CARP_MESSAGE_DR_FLAG_LEN_NOT_ENOUGH;

		// if length > remain length, then analysis error
		if (object_len > len) return CARP_MESSAGE_DR_DATA_NOT_ENOUGH;

		const int result = object.Deserialize(CARP_MESSAGE_CONST_DATA_OFFSET(data, sizeof(int)), object_len);
		if (result < CARP_MESSAGE_DR_NO_DATA) return result;

		return object_len + static_cast<int>(sizeof(int));
	}
	template <typename T>
	static int Deserialize(std::set<T>& object, const void* data, int len)
	{
		if (len == 0)
		{
			object.clear();
			return CARP_MESSAGE_DR_NO_DATA;
		}

		if (len < static_cast<int>(sizeof(int))) return CARP_MESSAGE_DR_DATA_NOT_ENOUGH;
		len -= static_cast<int>(sizeof(int));

		// clear self
		object.clear();

		// get array length and check
		const int array_len = *static_cast<const int*>(data);
		if (array_len < 0) return CARP_MESSAGE_DR_FLAG_LEN_NOT_ENOUGH;

		// offset target data position
		const char* current_data = static_cast<const char*>(data) + sizeof(int);
		for (int i = 0; i < array_len; ++i)
		{
			T t;
			const int result = Deserialize(t, current_data, len);
			if (result < CARP_MESSAGE_DR_NO_DATA) return result; current_data += result; len -= result;
			object.insert(t);
		}

		// calc final length
		return static_cast<int>(static_cast<const char*>(current_data) - static_cast<const char*>(data));
	}
	template <typename T>
	static int Deserialize(std::list<T>& object, const void* data, int len)
	{
		if (len == 0)
		{
			object.clear();
			return CARP_MESSAGE_DR_NO_DATA;
		}

		if (len < static_cast<int>(sizeof(int))) return CARP_MESSAGE_DR_DATA_NOT_ENOUGH;
		len -= static_cast<int>(sizeof(int));

		// clear self
		object.clear();

		// get array length and check
		const int array_len = *static_cast<const int*>(data);
		if (array_len < 0) return CARP_MESSAGE_DR_FLAG_LEN_NOT_ENOUGH;

		// offset target data position
		const char* current_data = static_cast<const char*>(data) + sizeof(int);
		for (int i = 0; i < array_len; ++i)
		{
			object.push_back(T());
			const int result = Deserialize(object.back(), current_data, len);
			if (result < CARP_MESSAGE_DR_NO_DATA) return result; current_data += result; len -= result;
		}

		// calc final length
		return static_cast<int>(static_cast<const char*>(current_data) - static_cast<const char*>(data));
	}
	template <typename T>
	static int Deserialize(std::vector<T>& object, const void* data, int len)
	{
		if (len == 0)
		{
			object.clear();
			return CARP_MESSAGE_DR_NO_DATA;
		}

		if (len < static_cast<int>(sizeof(int))) return CARP_MESSAGE_DR_DATA_NOT_ENOUGH;
		len -= static_cast<int>(sizeof(int));

		// clear self
		object.resize(0);

		// define max len
		const int len_max = 1024;

		// get array length and check
		int array_len = *static_cast<const int*>(data);
		if (array_len < 0) return CARP_MESSAGE_DR_FLAG_LEN_NOT_ENOUGH;
		if (array_len < len_max) object.reserve(array_len);

		// offset target data position
		const char* current_data = static_cast<const char*>(data) + sizeof(int);
		for (int i = 0; i < array_len; ++i)
		{
			object.push_back(T());
			const int result = Deserialize(object.back(), current_data, len);
			if (result < CARP_MESSAGE_DR_NO_DATA) return result; current_data += result; len -= result;
		}

		// calc final length
		return static_cast<int>(static_cast<const char*>(current_data) - static_cast<const char*>(data));
	}
	template <typename K, typename V>
	static int Deserialize(std::map<K, V>& object, const void* data, int len)
	{
		if (len == 0)
		{
			object.clear();
			return CARP_MESSAGE_DR_NO_DATA;
		}

		// check length last
		if (len < static_cast<int>(sizeof(int))) return CARP_MESSAGE_DR_DATA_NOT_ENOUGH;
		len -= static_cast<int>(sizeof(int));

		// clear self
		object.clear();

		// get length
		const int map_len = *static_cast<const int*>(data);
		if (map_len < 0) return CARP_MESSAGE_DR_FLAG_LEN_NOT_ENOUGH;

		// offset to data position
		const char* current_data = static_cast<const char*>(data) + sizeof(int);
		for (int i = 0; i < map_len; ++i)
		{
			// deserialize key
			K key;
			const int key_result = Deserialize(key, current_data, len);
			if (key_result < CARP_MESSAGE_DR_NO_DATA) return key_result; current_data += key_result; len -= key_result;
			// deserialize value
			V& value = object[key] = V();
			const int value_result = Deserialize(value, current_data, len);
			if (value_result < CARP_MESSAGE_DR_NO_DATA) return value_result; current_data += value_result; len -= value_result;
		}

		return static_cast<int>(static_cast<const char*>(current_data) - static_cast<const char*>(data));
	}
	
	template <> static int Deserialize<std::string>(std::string& object, const void* data, int len)
	{
		if (len == 0)
		{
			object.clear();
			return CARP_MESSAGE_DR_NO_DATA;
		}

		// check size enough
		if (len <= static_cast<int>(sizeof(int))) return CARP_MESSAGE_DR_DATA_NOT_ENOUGH;
		len -= sizeof(int); // desc size of head

		// get string length(include '\0')
		const int str_len = *static_cast<const int*>(data);
		if (str_len <= 0) return CARP_MESSAGE_DR_FLAG_LEN_NOT_ENOUGH;

		// if length > remain length, then analysis error
		if (str_len > len) return CARP_MESSAGE_DR_DATA_NOT_ENOUGH;

		// offset to string position
		const char* str = static_cast<const char*>(data);
		str += sizeof(int);

		// check real length
		while (*str && len > 0) { ++str; --len; }
		if (len <= 0) return CARP_MESSAGE_DR_DATA_NOT_ENOUGH;

		// check completed and copy string
		object.assign(static_cast<const char*>(data) + sizeof(int));
		return str_len + static_cast<int>(sizeof(int));
	}
	
	template <> static int Deserialize<bool>(bool& object, const void* data, int len) { return DeserializePrimary(object, data, len); }
	template <> static int Deserialize<char>(char& object, const void* data, int len) { return DeserializePrimary(object, data, len); }
	template <> static int Deserialize<unsigned char>(unsigned char& object, const void* data, int len) { return DeserializePrimary(object, data, len); }
	template <> static int Deserialize<short>(short& object, const void* data, int len) { return DeserializePrimary(object, data, len); }
	template <> static int Deserialize<unsigned short>(unsigned short& object, const void* data, int len) { return DeserializePrimary(object, data, len); }
	template <> static int Deserialize<int>(int& object, const void* data, int len) { return DeserializePrimary(object, data, len); }
	template <> static int Deserialize<unsigned int>(unsigned int& object, const void* data, int len) { return DeserializePrimary(object, data, len); }
	template <> static int Deserialize<long>(long& object, const void* data, int len) { return DeserializePrimary(object, data, len); }
	template <> static int Deserialize<unsigned long>(unsigned long& object, const void* data, int len) { return DeserializePrimary(object, data, len); }
	template <> static int Deserialize<long long>(long long& object, const void* data, int len) { return DeserializePrimary(object, data, len); }
	template <> static int Deserialize<unsigned long long>(unsigned long long& object, const void* data, int len) { return DeserializePrimary(object, data, len); }
	template <> static int Deserialize<float>(float& object, const void* data, int len) { return DeserializePrimary(object, data, len); }
	template <> static int Deserialize<double>(double& object, const void* data, int len) { return DeserializePrimary(object, data, len); }

};

// ------------------------------------------------------------------------------------

// 边界条件
inline int Template_Message_GetTotalSize(int cur_size)
{
	return cur_size;
}

// 必须使用尾递归的写法，否则调用会产生非常大的代价
template<typename T1, typename ...T2>
inline int Template_Message_GetTotalSize(int cur_size, const T1& t0, const T2& ...args)
{
	const int result = CarpMessageTemplate::GetTotalSize(t0);
	return Template_Message_GetTotalSize(cur_size + result, args...);
}

// 边界条件
inline int Template_Message_Serialize(int cur_size, void* data)
{
	return cur_size;
}

// 必须使用尾递归的写法，否则调用会产生非常大的代价
template<typename T1, typename ...T2>
inline int Template_Message_Serialize(int cur_size, void* data, const T1& t0, const T2& ...args)
{
	const int result = CarpMessageTemplate::Serialize(t0, data);
	return Template_Message_Serialize(cur_size + result, CARP_MESSAGE_DATA_OFFSET(data, result), args...);
}

// 边界条件
inline int Template_Message_Deserialize(int cur_size, const void* data, int len)
{
	return cur_size;
}

// 必须使用尾递归的写法，否则调用会产生非常大的代价
template<typename T1, typename ...T2>
inline int Template_Message_Deserialize(int cur_size, const void* data, int len, T1& t0, T2& ...args)
{
	const int result = CarpMessageTemplate::Deserialize(t0, static_cast<const char*>(data), len);
	if (result < CARP_MESSAGE_DR_NO_DATA) return result;

	return Template_Message_Deserialize(cur_size + result, static_cast<const char*>(data) + result, len - result, args...);
}

//===========================================================================================================

#define VA_CARP_MESSAGE_EXPAND(...)      __VA_ARGS__

#define VA_CARP_MESSAGE_FILTER_(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_N,...) _N
#define VA_CARP_MESSAGE_NUMBER_()        52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1
#define VA_CARP_MESSAGE_HELPER(...)      VA_CARP_MESSAGE_EXPAND(VA_CARP_MESSAGE_FILTER_(__VA_ARGS__))
#define VA_CARP_MESSAGE_COUNT(...)       VA_CARP_MESSAGE_HELPER(__VA_ARGS__, VA_CARP_MESSAGE_NUMBER_())

#define VA_CARP_MESSAGE_CAT(X, Y)        X##Y
#define VA_CARP_MESSAGE_JOIN(X, Y)       VA_CARP_MESSAGE_CAT(X, Y)
#define VA_CARP_MESSAGE_PROXY(F, ...)    VA_CARP_MESSAGE_EXPAND(F(__VA_ARGS__))

#define CARP_MESSAGE_MACRO_2(Name, ID) \
class Name : public CarpMessage { \
public: \
	CARP_MESSAGE_ID GetID() const { return ID; } \
	inline static CARP_MESSAGE_ID GetStaticID() { return ID; } \
	inline static CarpMessage* Create() { return new Name; } \
}
#define CARP_MESSAGE_MACRO_4(Name, ID, t0, m0) \
class Name : public CarpMessage { \
public: \
	CARP_MESSAGE_ID GetID() const { return ID; } \
	inline static CARP_MESSAGE_ID GetStaticID() { return ID; } \
	inline static CarpMessage* Create() { return new Name; } \
public: \
	int GetTotalSize() const { return Template_Message_GetTotalSize(0, m0); } \
	int Serialize(void* _data) const { return Template_Message_Serialize(0, _data, m0); } \
	int Deserialize(const void* _data, int _len) { return Template_Message_Deserialize(0, _data, _len, m0); } \
	Name() : m0() {} \
	t0 m0; \
}
#define CARP_MESSAGE_MACRO_6(Name, ID, t0, m0, t1, m1) \
class Name : public CarpMessage { \
public: \
	CARP_MESSAGE_ID GetID() const { return ID; } \
	inline static CARP_MESSAGE_ID GetStaticID() { return ID; } \
	inline static CarpMessage* Create() { return new Name; } \
public: \
	int GetTotalSize() const { return Template_Message_GetTotalSize(0, m0, m1); } \
	int Serialize(void* _data) const { return Template_Message_Serialize(0, _data, m0, m1); } \
	int Deserialize(const void* _data, int _len) { return Template_Message_Deserialize(0, _data, _len, m0, m1); } \
    Name() : m0(), m1() {} \
	t0 m0; t1 m1; \
}
#define CARP_MESSAGE_MACRO_8(Name, ID, t0, m0, t1, m1, t2, m2) \
class Name : public CarpMessage { \
public: \
	CARP_MESSAGE_ID GetID() const { return ID; } \
	inline static CARP_MESSAGE_ID GetStaticID() { return ID; } \
	inline static CarpMessage* Create() { return new Name; } \
public: \
	int GetTotalSize() const { return Template_Message_GetTotalSize(0, m0, m1, m2); } \
	int Serialize(void* _data) const { return Template_Message_Serialize(0, _data, m0, m1, m2); } \
	int Deserialize(const void* _data, int _len) { return Template_Message_Deserialize(0, _data, _len, m0, m1, m2); } \
    Name() : m0(), m1(), m2() {} \
	t0 m0; t1 m1; t2 m2; \
}
#define CARP_MESSAGE_MACRO_10(Name, ID, t0, m0, t1, m1, t2, m2, t3, m3) \
class Name : public CarpMessage { \
public: \
	CARP_MESSAGE_ID GetID() const { return ID; } \
	inline static CARP_MESSAGE_ID GetStaticID() { return ID; } \
	inline static CarpMessage* Create() { return new Name; } \
public: \
	int GetTotalSize() const { return Template_Message_GetTotalSize(0, m0, m1, m2, m3); } \
	int Serialize(void* _data) const { return Template_Message_Serialize(0, _data, m0, m1, m2, m3); } \
	int Deserialize(const void* _data, int _len) { return Template_Message_Deserialize(0, _data, _len, m0, m1, m2, m3); } \
    Name() : m0(), m1(), m2(), m3() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; \
}
#define CARP_MESSAGE_MACRO_12(Name, ID, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4) \
class Name : public CarpMessage { \
public: \
	CARP_MESSAGE_ID GetID() const { return ID; } \
	inline static CARP_MESSAGE_ID GetStaticID() { return ID; } \
	inline static CarpMessage* Create() { return new Name; } \
public: \
	int GetTotalSize() const { return Template_Message_GetTotalSize(0, m0, m1, m2, m3, m4); } \
	int Serialize(void* _data) const { return Template_Message_Serialize(0, _data, m0, m1, m2, m3, m4); } \
	int Deserialize(const void* _data, int _len) { return Template_Message_Deserialize(0, _data, _len, m0, m1, m2, m3, m4); } \
    Name() : m0(), m1(), m2(), m3(), m4() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; \
}
#define CARP_MESSAGE_MACRO_14(Name, ID, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5) \
class Name : public CarpMessage { \
public: \
	CARP_MESSAGE_ID GetID() const { return ID; } \
	inline static CARP_MESSAGE_ID GetStaticID() { return ID; } \
	inline static CarpMessage* Create() { return new Name; } \
public: \
	int GetTotalSize() const { return Template_Message_GetTotalSize(0, m0, m1, m2, m3, m4, m5); } \
	int Serialize(void* _data) const { return Template_Message_Serialize(0, _data, m0, m1, m2, m3, m4, m5); } \
	int Deserialize(const void* _data, int _len) { return Template_Message_Deserialize(0, _data, _len, m0, m1, m2, m3, m4, m5); } \
    Name() : m0(), m1(), m2(), m3(), m4(), m5() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; \
}
#define CARP_MESSAGE_MACRO_16(Name, ID, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6) \
class Name : public CarpMessage { \
public: \
	CARP_MESSAGE_ID GetID() const { return ID; } \
	inline static CARP_MESSAGE_ID GetStaticID() { return ID; } \
	inline static CarpMessage* Create() { return new Name; } \
public: \
	int GetTotalSize() const { return Template_Message_GetTotalSize(0, m0, m1, m2, m3, m4, m5, m6); } \
	int Serialize(void* _data) const { return Template_Message_Serialize(0, _data, m0, m1, m2, m3, m4, m5, m6); } \
	int Deserialize(const void* _data, int _len) { return Template_Message_Deserialize(0, _data, _len, m0, m1, m2, m3, m4, m5, m6); } \
    Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; \
}
#define CARP_MESSAGE_MACRO_18(Name, ID, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7) \
class Name : public CarpMessage { \
public: \
	CARP_MESSAGE_ID GetID() const { return ID; } \
	inline static CARP_MESSAGE_ID GetStaticID() { return ID; } \
	inline static CarpMessage* Create() { return new Name; } \
public: \
	int GetTotalSize() const { return Template_Message_GetTotalSize(0, m0, m1, m2, m3, m4, m5, m6, m7); } \
	int Serialize(void* _data) const { return Template_Message_Serialize(0, _data, m0, m1, m2, m3, m4, m5, m6, m7); } \
	int Deserialize(const void* _data, int _len) { return Template_Message_Deserialize(0, _data, _len, m0, m1, m2, m3, m4, m5, m6, m7); } \
    Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; \
}
#define CARP_MESSAGE_MACRO_20(Name, ID, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8) \
class Name : public CarpMessage { \
public: \
	CARP_MESSAGE_ID GetID() const { return ID; } \
	inline static CARP_MESSAGE_ID GetStaticID() { return ID; } \
	inline static CarpMessage* Create() { return new Name; } \
public: \
	int GetTotalSize() const { return Template_Message_GetTotalSize(0, m0, m1, m2, m3, m4, m5, m6, m7, m8); } \
	int Serialize(void* _data) const { return Template_Message_Serialize(0, _data, m0, m1, m2, m3, m4, m5, m6, m7, m8); } \
	int Deserialize(const void* _data, int _len) { return Template_Message_Deserialize(0, _data, _len, m0, m1, m2, m3, m4, m5, m6, m7, m8); } \
    Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; \
}
#define CARP_MESSAGE_MACRO_22(Name, ID, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8, t9, m9) \
class Name : public CarpMessage { \
public: \
	CARP_MESSAGE_ID GetID() const { return ID; } \
	inline static CARP_MESSAGE_ID GetStaticID() { return ID; } \
	inline static CarpMessage* Create() { return new Name; } \
public: \
	int GetTotalSize() const { return Template_Message_GetTotalSize(0, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9); } \
	int Serialize(void* _data) const { return Template_Message_Serialize(0, _data, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9); } \
	int Deserialize(const void* _data, int _len) { return Template_Message_Deserialize(0, _data, _len, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9); } \
    Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8(), m9() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; \
}
#define CARP_MESSAGE_MACRO_24(Name, ID, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8, t9, m9, t10, m10) \
class Name : public CarpMessage { \
public: \
	CARP_MESSAGE_ID GetID() const { return ID; } \
	inline static CARP_MESSAGE_ID GetStaticID() { return ID; } \
	inline static CarpMessage* Create() { return new Name; } \
public: \
	int GetTotalSize() const { return Template_Message_GetTotalSize(0, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10); } \
	int Serialize(void* _data) const { return Template_Message_Serialize(0, _data, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10); } \
	int Deserialize(const void* _data, int _len) { return Template_Message_Deserialize(0, _data, _len, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10); } \
    Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8(), m9(), m10() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; \
}

// 后面可以按规律去增加
#define CARP_MESSAGE_MACRO(...)        VA_CARP_MESSAGE_PROXY(VA_CARP_MESSAGE_JOIN(CARP_MESSAGE_MACRO_, VA_CARP_MESSAGE_COUNT(__VA_ARGS__)), __VA_ARGS__)

CARP_MESSAGE_MACRO(HeartbeatMessage, 0);
CARP_MESSAGE_MACRO(RpcErrorMessage, 1, std::string, reason);


class CarpMessageReadFactory : public CarpMessage
{
public:
	CarpMessageReadFactory() : m_need_free(false), m_memory(nullptr), m_read_size(0), m_total_size(0), m_last_read_size(0), m_id(0) {}
	~CarpMessageReadFactory() { Clear(); }

public:
	/**
	 * set message ID
	 * @param id  message ID
	 */
	void SetID(CARP_MESSAGE_ID id) { m_id = id; }

	/**
	 * get message ID
	 * @return message ID
	 */
	CARP_MESSAGE_ID GetID() const override { return m_id; }

	/**
	 * get total size
	 * @return total size
	 */
	int GetDataSize() const { return m_total_size; }

	/**
	 * read factory not do serialize
	 */
	int Serialize(void* data) const override { return 0; }

	/**
	 * deserialize
	 * @param data message body
	 * @param len size of message body
	 */
	int Deserialize(const void* data, int len) override
	{
		Clear();
		m_memory = data;
		m_read_size = 0;
		m_total_size = len;
		m_last_read_size = 0;
		return 0;
	}

	// total message = message head + message body
	int DeserializeFromTotalMessage(const void* data)
	{
		const char* body = static_cast<const char*>(data);
		// get message size
		CARP_MESSAGE_SIZE len = 0;
		memcpy(&len, body, sizeof(CARP_MESSAGE_SIZE));
		body += sizeof(CARP_MESSAGE_SIZE);
		CARP_MESSAGE_ID head_id = 0;
		memcpy(&head_id, body, sizeof(CARP_MESSAGE_ID));
		body += sizeof(CARP_MESSAGE_ID);
		CARP_MESSAGE_RPCID head_rpc_id = 0;
		memcpy(&head_rpc_id, body, sizeof(CARP_MESSAGE_RPCID));
		body += sizeof(CARP_MESSAGE_RPCID);

		SetID(head_id);
		SetRpcID(head_rpc_id);
		Deserialize(body, len);
		return 0;
	}

	// 加载文件
	static bool LoadStdFile(const std::string& file_path, std::vector<char>& out)
	{
#ifdef _WIN32
		FILE* file = nullptr;
		_wfopen_s(&file, UTF82Unicode(file_path).c_str(), L"rb");
#else
		FILE* file = fopen(file_path.c_str(), "rb");
#endif
		if (file == nullptr) return false;

		char buffer[1024];
		while (true)
		{
			const size_t read_size = fread(buffer, 1, sizeof(buffer), file);
			if (read_size == 0) break;
			for (size_t i = 0; i < read_size; ++i)
				out.push_back(buffer[i]);
		}
		fclose(file);

		return true;
	}
	
	/**
	 * read from file
	 * @param file_path
	 */
	bool ReadFromStdFile(const char* file_path)
	{
		std::vector<char> data;
		if (!LoadStdFile(file_path, data))
			return false;

		Clear();

		m_need_free = true;
		m_total_size = 0;
		if (data.size() > 0)
		{
			m_total_size = static_cast<int>(data.size());
			m_memory = malloc(data.size());
			memcpy(const_cast<void*>(m_memory), data.data(), data.size());
		}

		m_read_size = 0;
		m_last_read_size = 0;
		return true;
	}

public:
	template<typename T>
	inline T ReadPrimaryType()
	{
		if (m_read_size >= m_total_size || m_read_size < 0)
		{
			m_last_read_size = -1;
			return 0;
		}
		T value = 0;
		memcpy(&value, CARP_MESSAGE_CONST_DATA_OFFSET(m_memory, m_read_size), sizeof(T));
		m_read_size += static_cast<int>(sizeof(T));
		m_last_read_size = static_cast<int>(sizeof(T));
		return value;
	}

public:
	bool ReadBool() { return ReadPrimaryType<bool>(); }
	char ReadChar() { return ReadPrimaryType<char>(); }
	unsigned char ReadUChar() { return ReadPrimaryType<unsigned char>(); }
	short ReadShort() { return ReadPrimaryType<short>(); }
	unsigned short ReadUShort() { return ReadPrimaryType<unsigned short>(); }
	int ReadInt() { return ReadPrimaryType<int>(); }
	unsigned int ReadUInt() { return ReadPrimaryType<unsigned int>(); }
	long ReadLong() { return ReadPrimaryType<long>(); }
	unsigned long ReadULong() { return ReadPrimaryType<unsigned long>(); }
	long long ReadLongLong() { return ReadPrimaryType<long long>(); }
	unsigned long long ReadULongLong() { return ReadPrimaryType<unsigned long long>(); }
	float ReadFloat() { return ReadPrimaryType<float>(); }
	double ReadDouble() { return ReadPrimaryType<double>(); }
	const char* ReadString()
	{
		if (m_read_size >= m_total_size || m_read_size < 0)
		{
			m_last_read_size = -1;
			return "";
		}
		int len = 0; // include '\0'
		memcpy(&len, CARP_MESSAGE_CONST_DATA_OFFSET(m_memory, m_read_size), sizeof(len));
		if (len <= 0)
		{
			m_last_read_size = -1;
			return "";
		}
		const char* value = static_cast<const char*>(m_memory) + m_read_size + sizeof(int);
		m_read_size += len + static_cast<int>(sizeof(int));
		m_last_read_size = len + static_cast<int>(sizeof(int));
		return value;
	}
	
	int GetReadSize() const { return m_last_read_size; }

private:
	void Clear()
	{
		if (m_memory && m_need_free)
			free(const_cast<void*>(m_memory));
		m_need_free = false;
		m_memory = nullptr;
	}

public:
	bool m_need_free;
	const void* m_memory;	// memory
	int m_read_size;		// read offset
	int m_total_size;		// total size
	int m_last_read_size;   // last read size
	CARP_MESSAGE_ID m_id;		// ID
};

class CarpMessageWriteFactory : public CarpMessage
{
public:
	/**
	 * get message ID
	 * @return message ID
	 */
	CARP_MESSAGE_ID GetID() const override { return m_id; }
	/**
	 * set message ID
	 * @param id message ID
	 */
	void SetID(CARP_MESSAGE_ID id) { m_id = id; }
	void SetRpcID(CARP_MESSAGE_RPCID rpc_id) override { CarpMessage::SetRpcID(rpc_id); }

	/**
	 * get total size
	 * @return total size
	 */
	int GetTotalSize() const override { return m_size; }

	/**
	 * Serialize
	 * @param data
	 * @return -1:failed, otherwise is serialize size
	 */
	int Serialize(void* data) const override { memcpy(data, m_memory.data(), m_size); return m_size; }

	/**
	 * write factory not do deserialize
	 */
	int Deserialize(const void* data, int len) override { return -1; }

	static bool WriteMemoryToStdFile(const std::string& file_path, const char* memory, size_t size)
	{
#ifdef _WIN32
		FILE* file = nullptr;
		_wfopen_s(&file, UTF82Unicode(file_path).c_str(), L"wb");
#else
		FILE* file = fopen(file_path.c_str(), "wb");
#endif
		if (file == nullptr) return false;

		if (size > 0) fwrite(memory, 1, size, file);

		fclose(file);
		return true;
	}
	
	/**
	 * write to file
	 * @param file_path
	 */
	bool WriteToStdFile(const char* file_path) const
	{
		return WriteMemoryToStdFile(file_path, m_memory.data(), m_memory.size());
	}

	/**
	 * reset offset
	 */
	void ResetOffset() { m_size = 0; }
	CARP_MESSAGE_SIZE GetOffset() const { return m_size; }

public:
	template<typename T>
	inline int WritePrimaryType(T value)
	{
		ResizeMemory(sizeof(value));
		m_size += CarpMessageTemplate::Serialize(value, m_memory.data() + m_size);
		return static_cast<int>(sizeof(value));
	}

public:
	int WriteBool(bool value) { return WritePrimaryType(value); }
	int WriteChar(char value) { return WritePrimaryType(value); }
	int WriteUChar(unsigned char value) { return WritePrimaryType(value); }
	int WriteShort(short value) { return WritePrimaryType(value); }
	int WriteUShort(unsigned short value) { return WritePrimaryType(value); }
	int WriteInt(int value) { return WritePrimaryType(value); }
	int WriteUInt(unsigned int value) { return WritePrimaryType(value); }
	int WriteLong(long value) { return WritePrimaryType(value); }
	int WriteULong(unsigned long value) { return WritePrimaryType(value); }
	int WriteLongLong(long long value) { return WritePrimaryType(value); }
	int WriteULongLong(unsigned long long value) { return WritePrimaryType(value); }
	int WriteFloat(float value) { return WritePrimaryType(value); }
	int WriteDouble(double value) { return WritePrimaryType(value); }
	void SetInt(int offset, int value) { CarpMessageTemplate::Serialize(value, m_memory.data() + offset); }
	int WriteString(const char* value)
	{
		const int len = static_cast<int>(strlen(value));
		const int total_size = len + static_cast<int>(sizeof(int)) + 1;
		ResizeMemory(total_size);

		void* memory = m_memory.data() + m_size;

		int str_len = len + 1; // include '\0'
		memcpy(memory, &str_len, sizeof(str_len));
		char* data_string = static_cast<char*>(memory) + sizeof(int);
		for (int i = 0; i < len; ++i)
			data_string[i] = value[i];
		data_string[len] = 0;

		m_size += total_size;

		return total_size;
	}

private:
	/**
	 * add memory size
	 * @param add_size
	 */
	inline void ResizeMemory(size_t add_size)
	{
		if (m_memory.size() >= m_size + add_size) return;
		m_memory.resize(m_size + add_size, 0);
	}

public:
	std::vector<char> m_memory;

	CARP_MESSAGE_ID m_id = 0;		// id
	CARP_MESSAGE_SIZE m_size = 0;	// size
};

#endif