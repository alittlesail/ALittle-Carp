#ifndef CARP_JSON_INCLUDED
#define CARP_JSON_INCLUDED

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>

#include "rapidjson/include/rapidjson/prettywriter.h"
#include "rapidjson/include/rapidjson/stringbuffer.h"
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/rapidjson.h"

#include "carp_string.hpp"

typedef rapidjson::Value CarpJsonValue;
typedef rapidjson::Document::AllocatorType CarpJsonAlloc;
typedef std::map<std::string, std::string> CarpJsonValueMap;
typedef std::string  CARP_JSON_NAME;

class CarpJson
{
public:
	virtual ~CarpJson() { }
	typedef std::string ID_TYPE;
	typedef bool IS_JSON;

public:
	virtual const char* GetID() const { return "CarpJson"; }
	virtual const std::vector<std::string>& GetNameList() const { static std::vector<std::string> s_list; return s_list; }
	// 这里要注意，返回的类型字符串是最终填入宏参数的那个字符串
	// 比如定义一个 typdef unsigned int ACCOUNT_ID;
	// JSON_MACRO(Test, ACCOUNT_ID, account_id);
	// 这个时候返回的account_id类型是字符串"ACCOUNT_ID"
	virtual const std::vector<std::string>& GetTypeList() const { static std::vector<std::string> s_list; return s_list; }
	virtual void Reset() {}
public:
	virtual void Serialize(CarpJsonValue& v, CarpJsonAlloc &a) const { }
    virtual void Deserialize(const CarpJsonValue& v) { }
    virtual void Serialize(CarpJsonValueMap& v) const { }
	virtual void Deserialize(const CarpJsonValueMap& v) { }
	virtual void WriteToString(std::string& out) const
	{
		rapidjson::Document v(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType &alloc = v.GetAllocator();
		Serialize(v, alloc);
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		v.Accept(writer);
		out = buffer.GetString();
	}
	virtual bool ReadFromString(const std::string& in)
	{
		Reset();

        if (in.empty()) return true;

		rapidjson::Document doc;
		doc.Parse<0>(in.c_str());

		if (doc.HasParseError()) return false;
		Deserialize(doc);
		return true;
	}
	virtual std::string ToString() const
	{
		std::string out;
		WriteToString(out);
		return out;
	}
};

template <typename T>
class CarpVectorArrayJson : public CarpJson
{
public:
    CarpVectorArrayJson() {}
    virtual ~CarpVectorArrayJson() {}

public:
    void Serialize(CarpJsonValue& v, CarpJsonAlloc& a) const override;
    void Deserialize(const CarpJsonValue& v) override;
    void Reset() override { d.resize(0); }
    const std::vector<T>& Get() const { return d; }

public:
    T & operator[](size_t index) { return d[index]; }
    const T& operator[](size_t index) const { return d[index]; }
    size_t size() const { return d.size(); }
    void push_back(const T& v) { d.push_back(v); }
    void clear() { d.clear(); }
    void resize(size_t s) { d.resize(s); }

    typedef typename std::vector<T>::iterator iterator;
    typedef typename std::vector<T>::const_iterator const_iterator;
    iterator begin() { return d.begin(); }
    iterator end() { return d.end(); }
    const_iterator begin() const { return d.begin(); }
    const_iterator end() const { return d.end(); }
    
private:
    std::vector<T> d;
};

template <typename J, char split>
class CarpSplitStringJson : public J
{
public:
    CarpSplitStringJson() {}
    virtual ~CarpSplitStringJson() {}

public:
    virtual void WriteToString(std::string& out) const override;
    virtual bool ReadFromString(const std::string& in) override;
};

template <typename J, char split>
class CarpSplitVectorJson : public CarpVectorArrayJson<J>
{
public:
    CarpSplitVectorJson() {}
    virtual ~CarpSplitVectorJson() {}

public:
    void WriteToString(std::string& out) const override;
    bool ReadFromString(const std::string& in) override;

private:
    std::vector<J> d;
};

template <typename J>
using CarpOneSplitStringJson = CarpSplitStringJson<J, ':'>;

template <typename J>
using CarpTwoSplitStringJson = CarpSplitVectorJson<CarpOneSplitStringJson<J>, '|'>;

template <typename J>
using CarpThreeSplitStringJson = CarpSplitVectorJson<CarpTwoSplitStringJson<J>, '#'>;

// ------------------------------------------------------------------------------------
class CarpJsonTemplate
{
public:
template <typename T> static void Serialize(const T &object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
	{
		object.Serialize(v, a);
		return;
	}
	CarpJsonValue obj(rapidjson::kObjectType);
	object.Serialize(obj, a);
	if (!v.IsObject()) v.SetObject();
	if (name_len >= 0)
		v.AddMember(rapidjson::StringRef(name, name_len), obj, a);
	else
	{
		CarpJsonValue key(name, a);
		v.AddMember(key, obj, a);
	}
}
template <typename T> static void Serialize(const std::set<T>& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
	{
		if (!v.IsArray()) v.SetArray();
		for (auto it = object.begin(); it != object.end(); ++it)
		{
			CarpJsonValue obj;
			Serialize(*it, nullptr, 0, obj, a);
			v.PushBack(obj, a);
		}
		return;
	}
	CarpJsonValue arr = CarpJsonValue(rapidjson::kArrayType);
	for (auto it = object.begin(); it != object.end(); ++it)
	{
		CarpJsonValue obj;
		Serialize(*it, nullptr, 0, obj, a);
		arr.PushBack(obj, a);
	}
	if (!v.IsObject()) v.SetObject();
	if (name_len >= 0)
		v.AddMember(rapidjson::StringRef(name, name_len), arr, a);
	else
	{
		CarpJsonValue key(name, a);
		v.AddMember(key, arr, a);
	}
}
template <typename T> static void Serialize(const std::list<T>& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
	{
		if (!v.IsArray()) v.SetArray();
		for (auto it = object.begin(); it != object.end(); ++it)
		{
			CarpJsonValue obj;
			Serialize(*it, nullptr, 0, obj, a);
			v.PushBack(obj, a);
		}
		return;
	}
	CarpJsonValue arr = CarpJsonValue(rapidjson::kArrayType);
	for (auto it = object.begin(); it != object.end(); ++it)
	{
		CarpJsonValue obj;
		Serialize(*it, nullptr, 0, obj, a);
		arr.PushBack(obj, a);
	}
	if (!v.IsObject()) v.SetObject();
	if (name_len >= 0)
		v.AddMember(rapidjson::StringRef(name, name_len), arr, a);
	else
	{
		CarpJsonValue key(name, a);
		v.AddMember(key, arr, a);
	}
}
template <typename T> static void Serialize(const std::vector<T>& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
	{
		if (!v.IsArray()) v.SetArray();
		for (unsigned int i = 0; i < object.size(); ++i)
		{
			CarpJsonValue obj;
			Serialize(object[i], nullptr, 0, obj, a);
			v.PushBack(obj, a);
		}
		return;
	}
	CarpJsonValue arr = CarpJsonValue(rapidjson::kArrayType);
	for (unsigned int i = 0; i < object.size(); ++i)
	{
		CarpJsonValue obj;
		Serialize(object[i], nullptr, 0, obj, a);
		arr.PushBack(obj, a);
	}
	if (!v.IsObject()) v.SetObject();
	if (name_len >= 0)
		v.AddMember(rapidjson::StringRef(name, name_len), arr, a);
	else
	{
		CarpJsonValue key(name, a);
		v.AddMember(key, arr, a);
	}
}
template <typename V> static void Serialize(const std::map<std::string, V>& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
	{
		if (!v.IsObject()) v.SetObject();
		for (auto it = object.begin(); it != object.end(); ++it)
			Serialize(it->second, it->first.c_str(), (int)it->first.size(), v, a);
		return;
	}
	CarpJsonValue obj = CarpJsonValue(rapidjson::kObjectType);
	for (auto it = object.begin(); it != object.end(); ++it)
		Serialize(it->second, it->first.c_str(), (int)it->first.size(), obj, a);
	if (!v.IsObject()) v.SetObject();
	if (name_len >= 0)
		v.AddMember(rapidjson::StringRef(name, name_len), obj, a);
	else
	{
		CarpJsonValue key(name, a);
		v.AddMember(key, obj, a);
	}
}
template <typename V> static void Serialize(const std::map<int, V>& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
	{
		if (!v.IsObject()) v.SetObject();
		char text[32] = { 0 };
		for (auto it = object.begin(); it != object.end(); ++it) {
#ifdef _WIN32
			sprintf_s(text, "%d", it->first);
#else
			sprintf(text, "%d", it->first);
#endif
			Serialize(it->second, text, -1, v, a);
		}
		return;
	}
	CarpJsonValue obj = CarpJsonValue(rapidjson::kObjectType);
	char text[32] = { 0 };
	for (auto it = object.begin(); it != object.end(); ++it) {
#ifdef _WIN32
		sprintf_s(text, "%d", it->first);
#else
		sprintf(text, "%d", it->first);
#endif
		Serialize(it->second, text, -1, obj, a);
	}
	if (!v.IsObject()) v.SetObject();
	if (name_len >= 0)
		v.AddMember(rapidjson::StringRef(name, name_len), obj, a);
	else
	{
		CarpJsonValue key(name, a);
		v.AddMember(key, obj, a);
	}
}
template <typename V> static void Serialize(const std::map<unsigned int, V>& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
	{
		if (!v.IsObject()) v.SetObject();
		char text[32] = { 0 };
		for (auto it = object.begin(); it != object.end(); ++it) {
#ifdef _WIN32
			sprintf_s(text, "%u", it->first);
#else
			sprintf(text, "%u", it->first);
#endif
			Serialize(it->second, text, -1, v, a);
		}
		return;
	}
	CarpJsonValue obj = CarpJsonValue(rapidjson::kObjectType);
	char text[32] = { 0 };
	for (auto it = object.begin(); it != object.end(); ++it) {
#ifdef _WIN32
		sprintf_s(text, "%u", it->first);
#else
		sprintf(text, "%u", it->first);
#endif
		Serialize(it->second, text, -1, obj, a);
	}
	if (!v.IsObject()) v.SetObject();
	if (name_len >= 0)
		v.AddMember(rapidjson::StringRef(name, name_len), obj, a);
	else
	{
		CarpJsonValue key(name, a);
		v.AddMember(key, obj, a);
	}
}
template <> static void Serialize<std::string>(const std::string& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v.SetString(rapidjson::StringRef(object.c_str(), object.size()), a);
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), rapidjson::StringRef(object.c_str(), object.size()), a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, rapidjson::StringRef(object.c_str(), object.size()), a);
		}
	}
}
template <> static void Serialize<bool>(const bool& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v = object;
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), object, a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, object, a);
		}
	}
}
template <> static void Serialize<char>(const char& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v = object;
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), object, a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, object, a);
		}
	}
}
template <> static void Serialize<unsigned char>(const unsigned char& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v = object;
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), object, a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, object, a);
		}
	}
}
template <> static void Serialize<short>(const short& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v = object;
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), object, a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, object, a);
		}
	}
}
template <> static void Serialize<unsigned short>(const unsigned short& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v = object;
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), object, a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, object, a);
		}
	}
}
template <> static void Serialize<int>(const int& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v = object;
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), object, a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, object, a);
		}
	}
}
template <> static void Serialize<unsigned int>(const unsigned int& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v = object;
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), object, a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, object, a);
		}
	}
}
template <> static void Serialize<long>(const long& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v = (int)(object);
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), (int)object, a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, (int)object, a);
		}
	}
}
template <> static void Serialize<unsigned long>(const unsigned long& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v = (unsigned int)object;
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), (unsigned int)object, a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, (unsigned int)object, a);
		}
	}
}
template <> static void Serialize<long long>(const long long& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v = object;
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), object, a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, object, a);
		}
	}
}
template <> static void Serialize<unsigned long long>(const unsigned long long& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v = object;
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), object, a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, object, a);
		}
	}
}
template <> static void Serialize<float>(const float& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v = object;
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), object, a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, object, a);
		}
	}
}
template <> static void Serialize<double>(const double& object, const char* name, int name_len, CarpJsonValue &v, CarpJsonAlloc &a)
{
	if (nullptr == name)
		v = object;
	else
	{
		if (!v.IsObject()) v.SetObject();
		if (name_len >= 0)
			v.AddMember(rapidjson::StringRef(name, name_len), object, a);
		else
		{
			CarpJsonValue key(name, a);
			v.AddMember(key, object, a);
		}
	}
}

// ------------------------------------------------------------------------------------
template <typename T> static void Deserialize(T& object, const char* name, int name_len, const CarpJsonValue& v)
{
	if (nullptr == name)
	{
		object.Deserialize(v);
		return;
	}
	if (!v.IsObject()) return;
	auto it = v.FindMember(rapidjson::StringRef(name, name_len));
	if (it == v.MemberEnd()) return;
	object.Deserialize(it->value);
}
template <typename T> static void Deserialize(std::set<T>& object, const char* name, int name_len, const CarpJsonValue& v)
{
	object.clear();
	if (nullptr == name)
	{
		if (!v.IsArray()) return;
		for (unsigned int i = 0; i < v.Size(); ++i)
		{
			T data;
			Deserialize(data, nullptr, 0, v[i]);
			object.insert(data);
		}
		return;
	}
	if (!v.IsObject()) return;
	auto it = v.FindMember(rapidjson::StringRef(name, name_len));
	if (it == v.MemberEnd()) return;
	if (!it->value.IsArray()) return;

	const CarpJsonValue& arr = it->value;
	for (unsigned int i = 0; i < arr.Size(); ++i)
	{
		T data;
		Deserialize(data, nullptr, 0, arr[i]);
		object.insert(data);
	}
}
template <typename T> static void Deserialize(std::list<T>& object, const char* name, int name_len, const CarpJsonValue& v)
{
	object.clear();
	if (nullptr == name)
	{
		if (!v.IsArray()) return;
		for (unsigned int i = 0; i < v.Size(); ++i)
		{
			object.push_back(T());
			Deserialize(object.back(), nullptr, 0, v[i]);
		}
		return;
	}

	if (!v.IsObject()) return;
	auto it = v.FindMember(rapidjson::StringRef(name, name_len));
	if (it == v.MemberEnd()) return;
	if (!it->value.IsArray()) return;

	const CarpJsonValue& arr = it->value;
	for (unsigned int i = 0; i < arr.Size(); ++i)
	{
		object.push_back(T());
		Deserialize(object.back(), nullptr, 0, arr[i]);
	}
}
template <typename T> static void Deserialize(std::vector<T>& object, const char* name, int name_len, const CarpJsonValue& v)
{
	object.clear();
	if (nullptr == name)
	{
		if (!v.IsArray()) return;
		unsigned int len = v.Size();
		object.resize(len);
		for (unsigned int i = 0; i < len; ++i)
			Deserialize(object[i], nullptr, 0, v[i]);
		return;
	}

	if (!v.IsObject()) return;
	auto it = v.FindMember(rapidjson::StringRef(name, name_len));
	if (it == v.MemberEnd()) return;
	if (!it->value.IsArray()) return;

	const CarpJsonValue& arr = it->value;
	unsigned int len = arr.Size();
	object.resize(len);
	for (unsigned int i = 0; i < len; ++i)
		Deserialize(object[i], nullptr, 0, arr[i]);
}
template <typename K, typename V> static void Deserialize(std::map<K, V>& object, const char* name, int name_len, const CarpJsonValue& v)
{
	object.clear();
	if (nullptr == name)
	{
		if (!v.IsObject()) return;
		for (auto it = v.MemberBegin(); it != v.MemberEnd(); ++it)
		{
			V& data = object[it->name.GetString()] = V();
			Deserialize(data, it->name.GetString(), it->name.GetStringLength(), v);
		}
		return;
	}

	if (!v.IsObject()) return;
	auto it = v.FindMember(rapidjson::StringRef(name, name_len));
	if (it == v.MemberEnd()) return;
	if (!it->value.IsObject()) return;

	const CarpJsonValue& obj = it->value;
	for (auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it)
	{
		V& data = object[it->name.GetString()] = V();
		Deserialize(data, it->name.GetString(), it->name.GetStringLength(), obj);
	}
}
template <typename V> static void Deserialize(std::map<int, V>& object, const char* name, int name_len, const CarpJsonValue& v)
{
	object.clear();
	if (nullptr == name)
	{
		if (!v.IsObject()) return;
		for (auto it = v.MemberBegin(); it != v.MemberEnd(); ++it)
		{
			V& data = object[std::atoi(it->name.GetString())] = V();
			Deserialize(data, it->name.GetString(), it->name.GetStringLength(), v);
		}
		return;
	}

	if (!v.IsObject()) return;
	auto it = v.FindMember(rapidjson::StringRef(name, name_len));
	if (it == v.MemberEnd()) return;
	if (!it->value.IsObject()) return;

	const CarpJsonValue& obj = it->value;
	for (auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it)
	{
		V& data = object[std::atoi(it->name.GetString())] = V();
		Deserialize(data, it->name.GetString(), it->name.GetStringLength(), obj);
	}
}
template <typename V> static void Deserialize(std::map<unsigned int, V>& object, const char* name, int name_len, const CarpJsonValue& v)
{
	object.clear();
	if (nullptr == name)
	{
		if (!v.IsObject()) return;
		for (auto it = v.MemberBegin(); it != v.MemberEnd(); ++it)
		{
			V& data = object[std::atoi(it->name.GetString())] = V();
			Deserialize(data, it->name.GetString(), it->name.GetStringLength(), v);
		}
		return;
	}

	if (!v.IsObject()) return;
	auto it = v.FindMember(rapidjson::StringRef(name, name_len));
	if (it == v.MemberEnd()) return;
	if (!it->value.IsObject()) return;

	const CarpJsonValue& obj = it->value;
	for (auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it)
	{
		V& data = object[std::atoi(it->name.GetString())] = V();
		Deserialize(data, it->name.GetString(), it->name.GetStringLength(), obj);
	}
}
template <> static void Deserialize<std::string>(std::string& object, const char* name, int name_len, const CarpJsonValue& v)
{
	object.clear();
	if (nullptr == name)
	{
		if (!v.IsString()) return;
		object = v.GetString();
		return;
	}

	if (!v.IsObject()) return;
	auto it = v.FindMember(rapidjson::StringRef(name, name_len));
	if (it == v.MemberEnd()) return;
	if (!it->value.IsString()) return;
	object = it->value.GetString();
}
template <> static void Deserialize<bool>(bool& object, const char* name, int name_len, const CarpJsonValue& v)
{
	object = bool();
	if (nullptr == name)
	{
		if (!v.IsBool()) return;
		object = v.GetBool();
		return;
	}
	if (!v.IsObject()) return;
	auto it = v.FindMember(rapidjson::StringRef(name, name_len));
	if (it == v.MemberEnd()) return;
	if (!it->value.IsBool()) return;
	object = it->value.GetBool();
}
template <typename T> static void DeserializeNumber(T& object, const char* name, int name_len, const CarpJsonValue& v)
{
	object = T();
	if (nullptr == name)
	{
		if (v.IsInt()) object = (T)v.GetInt();
		else if (v.IsInt64()) object = (T)v.GetInt64();
		else if (v.IsUint()) object = (T)v.GetUint();
		else if (v.IsUint64()) object = (T)v.GetUint64();
		else if (v.IsDouble()) object = (T)v.GetDouble();
		return;
	}

	if (!v.IsObject()) return;
	auto it = v.FindMember(rapidjson::StringRef(name, name_len));
	if (it == v.MemberEnd()) return;

	if (it->value.IsInt()) object = (T)it->value.GetInt();
	else if (it->value.IsInt64()) object = (T)it->value.GetInt64();
	else if (it->value.IsUint()) object = (T)it->value.GetUint();
	else if (it->value.IsUint64()) object = (T)it->value.GetUint64();
	else if (it->value.IsDouble()) object = (T)it->value.GetDouble();
}

template <> static void Deserialize<char>(char& object, const char* name, int name_len, const CarpJsonValue& v)
{
	DeserializeNumber(object, name, name_len, v);
}
template <> static void Deserialize<unsigned char>(unsigned char& object, const char* name, int name_len, const CarpJsonValue& v)
{
	DeserializeNumber(object, name, name_len, v);
}
template <> static void Deserialize<short>(short& object, const char* name, int name_len, const CarpJsonValue& v)
{
	DeserializeNumber(object, name, name_len, v);
}
template <> static void Deserialize<unsigned short>(unsigned short& object, const char* name, int name_len, const CarpJsonValue& v)
{
	DeserializeNumber(object, name, name_len, v);
}
template <> static void Deserialize<int>(int& object, const char* name, int name_len, const CarpJsonValue& v)
{
	DeserializeNumber(object, name, name_len, v);
}
template <> static void Deserialize<unsigned int>(unsigned int& object, const char* name, int name_len, const CarpJsonValue& v)
{
	DeserializeNumber(object, name, name_len, v);
}
template <> static void Deserialize<long>(long& object, const char* name, int name_len, const CarpJsonValue& v)
{
	DeserializeNumber(object, name, name_len, v);
}
template <> static void Deserialize<unsigned long>(unsigned long& object, const char* name, int name_len, const CarpJsonValue& v)
{
	DeserializeNumber(object, name, name_len, v);
}
template <> static void Deserialize<long long>(long long& object, const char* name, int name_len, const CarpJsonValue& v)
{
	DeserializeNumber(object, name, name_len, v);
}
template <> static void Deserialize<unsigned long long>(unsigned long long& object, const char* name, int name_len, const CarpJsonValue& v)
{
	DeserializeNumber(object, name, name_len, v);
}
template <> static void Deserialize<float>(float& object, const char* name, int name_len, const CarpJsonValue& v)
{
	object = float();
	if (nullptr == name)
	{
		if (!v.IsNumber()) return;
		object = (float)v.GetDouble();
		return;
	}
	if (!v.IsObject()) return;
	auto it = v.FindMember(rapidjson::StringRef(name, name_len));
	if (it == v.MemberEnd()) return;
	if (!it->value.IsNumber()) return;
	object = (float)it->value.GetDouble();
}
template <> static void Deserialize<double>(double& object, const char* name, int name_len, const CarpJsonValue& v)
{
	object = double();
	if (nullptr == name)
	{
		if (!v.IsNumber()) return;
		object = v.GetDouble();
		return;
	}
	if (!v.IsObject()) return;
	auto it = v.FindMember(rapidjson::StringRef(name, name_len));
	if (it == v.MemberEnd()) return;
	if (!it->value.IsNumber()) return;
	object = it->value.GetDouble();
}

// ------------------------------------------------------------------------------------

template <typename T> static void SerializeString(const T& object, std::string& s)
{
    rapidjson::Document v;
    rapidjson::Document::AllocatorType &alloc = v.GetAllocator();
    Serialize(object, nullptr, 0, v, alloc);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    v.Accept(writer);
    s = buffer.GetString();
}
template <typename T> static void SerializeString(const CarpOneSplitStringJson<T>& object, std::string& s)
{
    object.WriteToString(s);
}
template <typename T> static void SerializeString(const CarpTwoSplitStringJson<T>& object, std::string& s)
{
    object.WriteToString(s);
}
template <typename T> static void SerializeString(const CarpThreeSplitStringJson<T>& object, std::string& s)
{
    object.WriteToString(s);
}
template <> static void SerializeString<std::string>(const std::string& object, std::string& s)
{
    s = object;
}
template <> static void SerializeString<bool>(const bool& object, std::string& s)
{
    s = object ? "true" : "false";
}
template <> static void SerializeString<char>(const char& object, std::string& s)
{
    s = std::to_string(static_cast<int>(object));
}
template <> static void SerializeString<unsigned char>(const unsigned char& object, std::string& s)
{
    s = std::to_string(static_cast<unsigned int>(object));
}
template <> static void SerializeString<short>(const short& object, std::string& s)
{
    s = std::to_string(object);
}
template <> static void SerializeString<unsigned short>(const unsigned short& object, std::string& s)
{
    s = std::to_string(object);
}
template <> static void SerializeString<int>(const int& object, std::string& s)
{
    s = std::to_string(object);
}
template <> static void SerializeString<unsigned int>(const unsigned int& object, std::string& s)
{
    s = std::to_string(object);
}
template <> static void SerializeString<long>(const long& object, std::string& s)
{
    s = std::to_string(object);
}
template <> static void SerializeString<unsigned long>(const unsigned long& object, std::string& s)
{
    s = std::to_string(object);
}
template <> static void SerializeString<long long>(const long long& object, std::string& s)
{
    s = std::to_string(object);
}
template <> static void SerializeString<unsigned long long>(const unsigned long long& object, std::string& s)
{
    s = std::to_string(object);
}
template <> static void SerializeString<float>(const float& object, std::string& s)
{
    s = std::to_string(object);
}
template <> static void SerializeString<double>(const double& object, std::string& s)
{
    s = std::to_string(object);
}
template <typename T> static void SerializeValueMap(const T& object, const char* name, CarpJsonValueMap& value_map)
{
    std::string value;
    SerializeString(object, value);
    value_map[name] = value;
}

// ------------------------------------------------------------------------------------

template <typename T> static void DeserializeString(T& object, const std::string& s)
{
    rapidjson::Document v;
    v.Parse<0>(s.c_str());
    Deserialize(object, nullptr, 0, v);
}
template <typename T> static void DeserializeString(CarpOneSplitStringJson<T>& object, const std::string& s)
{
    object.ReadFromString(s);
}
template <typename T> static void DeserializeString(CarpTwoSplitStringJson<T>& object, const std::string& s)
{
    object.ReadFromString(s);
}
template <typename T> static void DeserializeString(CarpThreeSplitStringJson<T>& object, const std::string& s)
{
    object.ReadFromString(s);
}
template <> static void DeserializeString<std::string>(std::string& object, const std::string& s)
{
	object = s;
}
template <> static void DeserializeString<bool>(bool& object, const std::string& s)
{
	object = (s == "true");
}
template <> static void DeserializeString<char>(char& object, const std::string& s)
{
	object = static_cast<char>(std::atoi(s.c_str()));
}
template <> static void DeserializeString<unsigned char>(unsigned char& object, const std::string& s)
{
    object = static_cast<unsigned char>(std::atoi(s.c_str()));
}
template <> static void DeserializeString<short>(short& object, const std::string& s)
{
    object = static_cast<short>(std::atoi(s.c_str()));
}
template <> static void DeserializeString<unsigned short>(unsigned short& object, const std::string& s)
{
    object = static_cast<unsigned short>(std::atoi(s.c_str()));
}
template <> static void DeserializeString<int>(int& object, const std::string& s)
{
    object = std::atoi(s.c_str());
}
template <> static void DeserializeString<unsigned int>(unsigned int& object, const std::string& s)
{
    object = static_cast<unsigned int>(std::atoi(s.c_str()));
}
template <> static void DeserializeString<long>(long& object, const std::string& s)
{
    object = std::atol(s.c_str());
}
template <> static void DeserializeString<unsigned long>(unsigned long& object, const std::string& s)
{
    object = static_cast<unsigned long>(std::atol(s.c_str()));
}
template <> static void DeserializeString<long long>(long long& object, const std::string& s)
{
    object = std::atoll(s.c_str());
}
template <> static void DeserializeString<unsigned long long>(unsigned long long& object, const std::string& s)
{
    object = static_cast<unsigned long long>(std::atoll(s.c_str()));
}
template <> static void DeserializeString<float>(float& object, const std::string& s)
{
    object = static_cast<float>(std::atof(s.c_str()));
}
template <> static void DeserializeString<double>(double& object, const std::string& s)
{
    object = std::atof(s.c_str());
}
template <typename T> static void DeserializeValueMap(T& object, const char* name, const CarpJsonValueMap& value_map)
{
	auto it = value_map.find(name);
	if (it == value_map.end())
		object = T();
	else
		DeserializeString(object, it->second);
}

};

template <typename T>
void CarpVectorArrayJson<T>::Serialize(CarpJsonValue& v, CarpJsonAlloc& a) const
{
	if (!v.IsArray()) v.SetArray();
	for (size_t i = 0; i < d.size(); ++i)
	{
		CarpJsonValue obj;
		CarpJsonTemplate::Serialize(d[i], nullptr, 0, obj, a);
		v.PushBack(obj, a);
	}
}

template <typename T>
void CarpVectorArrayJson<T>::Deserialize(const CarpJsonValue& v)
{
    Reset();
    if (!v.IsArray()) return;
    rapidjson::SizeType len = v.Size();
    d.resize(len);
    for (rapidjson::SizeType i = 0; i < len; ++i)
        CarpJsonTemplate::Deserialize(d[i], nullptr, 0, v[i]);
}

template <typename J, char split>
void CarpSplitStringJson<J, split>::WriteToString(std::string& out) const
{
    // 序列化得出键值对映射表
    CarpJsonValueMap value_map;
    this->Serialize(value_map);
    // 遍历把值取出来，然后添加到列表中
    std::vector<std::string> split_list;
    for (auto it = value_map.begin(); it != value_map.end(); ++it)
        split_list.emplace_back(it->second);
    // 最后拼接得到结果
    std::string split_string;
    split_string.push_back(split);
    out = CarpString::Join(split_list, split_string);
}

template <typename J, char split>
bool CarpSplitStringJson<J, split>::ReadFromString(const std::string& in)
{
	this->Reset();

    // 根据分隔符，切割出值
    std::string split_string;
    split_string.push_back(split);
    std::vector<std::string> split_list;
    CarpString::Split(in, split_string, false, split_list);
	CarpJsonValueMap value_map;
    // 根据名称列表，拼接出映射表
    const std::vector<std::string>& name_list = this->GetNameList();
    for (size_t i = 0; i < name_list.size(); ++i)
    {
        if (i >= split_list.size()) break;
        value_map[name_list[i]] = split_list[i];
    }
    // 最后执行反序列化
	this->Deserialize(value_map);
    return true;
}

template <typename J, char split>
void CarpSplitVectorJson<J, split>::WriteToString(std::string& out) const
{
    // 定义结果列表
    std::vector<std::string> split_list;
    for (size_t i = 0; i < d.size(); ++i)
    {
        split_list.emplace_back(std::string());
        d[i].WriteToString(split_list.back());
    }
    // 把结果拼接起来
    std::string split_string;
    split_string.push_back(split);
    out = CarpString::Join(split_list, split_string);
}

template <typename J, char split>
bool CarpSplitVectorJson<J, split>::ReadFromString(const std::string& in)
{
	this->Reset();

    // 根据分隔符，切割出值
    std::string split_string;
    split_string.push_back(split);
    std::vector<std::string> split_list;
    CarpString::Split(in, split_string, false, split_list);
    // 遍历反序列化所有数据
    for (size_t i = 0; i < split_list.size(); ++i)
    {
        d.emplace_back(J());
        d.back().ReadFromString(split_list[i]);
    }

    return true;
}

//===========================================================================================================

#define CARP_JSON_MACRO_1(Name) \
class Name : public CarpJson { \
public: \
	const char* GetID() const { return #Name; } \
	inline static const char* GetStaticID() { return #Name; } \
}

#endif // _ALITTLE_JSON_H_
