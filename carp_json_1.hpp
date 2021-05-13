
#ifndef CARP_JSON_1_INCLUDED
#define CARP_JSON_1_INCLUDED

#include "carp_json.hpp"

#define CARP_JSON_MACRO_3(Name, t0, m0) \
class Name : public CarpJson { \
public: \
	virtual const char* GetID() const { return #Name; } \
	inline static const char* GetStaticID() { return #Name; } \
public: \
	virtual void Serialize(CarpJsonValue& v, CarpJsonAlloc& a) const { \
		CarpJsonTemplate::Serialize(m0, #m0, (int)sizeof(#m0)-1, v, a); \
	} \
	virtual void Deserialize(const CarpJsonValue& v) { \
		CarpJsonTemplate::Deserialize(m0, #m0, (int)sizeof(#m0)-1, v); \
	} \
	virtual void Serialize(CarpJsonValueMap& v) const { \
		CarpJsonTemplate::SerializeValueMap(m0, #m0, v); \
	} \
	virtual void Deserialize(const CarpJsonValueMap& v) { \
		CarpJsonTemplate::DeserializeValueMap(m0, #m0, v); \
	} \
	virtual const std::vector<std::string>& GetNameList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#m0); \
		} return s_list; \
	} \
	virtual const std::vector<std::string>& GetTypeList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#t0); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0() {} \
	t0 m0; \
}
#define CARP_JSON_MACRO_5(Name, t0, m0, t1, m1) \
class Name : public CarpJson { \
public: \
	virtual const char* GetID() const { return #Name; } \
	inline static const char* GetStaticID() { return #Name; } \
public: \
	virtual void Serialize(CarpJsonValue& v, CarpJsonAlloc& a) const { \
		CarpJsonTemplate::Serialize(m0, #m0, (int)sizeof(#m0)-1, v, a); \
		CarpJsonTemplate::Serialize(m1, #m1, (int)sizeof(#m1)-1, v, a); \
	} \
	virtual void Deserialize(const CarpJsonValue& v) { \
		CarpJsonTemplate::Deserialize(m0, #m0, (int)sizeof(#m0)-1, v); \
		CarpJsonTemplate::Deserialize(m1, #m1, (int)sizeof(#m1)-1, v); \
	} \
	virtual void Serialize(CarpJsonValueMap& v) const { \
		CarpJsonTemplate::SerializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::SerializeValueMap(m1, #m1, v); \
	} \
	virtual void Deserialize(const CarpJsonValueMap& v) { \
		CarpJsonTemplate::DeserializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::DeserializeValueMap(m1, #m1, v); \
	} \
	virtual const std::vector<std::string>& GetNameList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#m0); \
			s_list.push_back(#m1); \
		} return s_list; \
	} \
	virtual const std::vector<std::string>& GetTypeList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#t0); \
			s_list.push_back(#t1); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1() {} \
	t0 m0; t1 m1; \
}
#define CARP_JSON_MACRO_7(Name, t0, m0, t1, m1, t2, m2) \
class Name : public CarpJson { \
public: \
	virtual const char* GetID() const { return #Name; } \
	inline static const char* GetStaticID() { return #Name; } \
public: \
	virtual void Serialize(CarpJsonValue& v, CarpJsonAlloc& a) const { \
		CarpJsonTemplate::Serialize(m0, #m0, (int)sizeof(#m0)-1, v, a); \
		CarpJsonTemplate::Serialize(m1, #m1, (int)sizeof(#m1)-1, v, a); \
		CarpJsonTemplate::Serialize(m2, #m2, (int)sizeof(#m2)-1, v, a); \
	} \
	virtual void Deserialize(const CarpJsonValue& v) { \
		CarpJsonTemplate::Deserialize(m0, #m0, (int)sizeof(#m0)-1, v); \
		CarpJsonTemplate::Deserialize(m1, #m1, (int)sizeof(#m1)-1, v); \
		CarpJsonTemplate::Deserialize(m2, #m2, (int)sizeof(#m2)-1, v); \
	} \
	virtual void Serialize(CarpJsonValueMap& v) const { \
		CarpJsonTemplate::SerializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::SerializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::SerializeValueMap(m2, #m2, v); \
	} \
	virtual void Deserialize(const CarpJsonValueMap& v) { \
		CarpJsonTemplate::DeserializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::DeserializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::DeserializeValueMap(m2, #m2, v); \
	} \
	virtual const std::vector<std::string>& GetNameList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#m0); \
			s_list.push_back(#m1); \
			s_list.push_back(#m2); \
		} return s_list; \
	} \
	virtual const std::vector<std::string>& GetTypeList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#t0); \
			s_list.push_back(#t1); \
			s_list.push_back(#t2); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2() {} \
	t0 m0; t1 m1; t2 m2; \
}
#define CARP_JSON_MACRO_9(Name, t0, m0, t1, m1, t2, m2, t3, m3) \
class Name : public CarpJson { \
public: \
	virtual const char* GetID() const { return #Name; } \
	inline static const char* GetStaticID() { return #Name; } \
public: \
	virtual void Serialize(CarpJsonValue& v, CarpJsonAlloc& a) const { \
		CarpJsonTemplate::Serialize(m0, #m0, (int)sizeof(#m0)-1, v, a); \
		CarpJsonTemplate::Serialize(m1, #m1, (int)sizeof(#m1)-1, v, a); \
		CarpJsonTemplate::Serialize(m2, #m2, (int)sizeof(#m2)-1, v, a); \
		CarpJsonTemplate::Serialize(m3, #m3, (int)sizeof(#m3)-1, v, a); \
	} \
	virtual void Deserialize(const CarpJsonValue& v) { \
		CarpJsonTemplate::Deserialize(m0, #m0, (int)sizeof(#m0)-1, v); \
		CarpJsonTemplate::Deserialize(m1, #m1, (int)sizeof(#m1)-1, v); \
		CarpJsonTemplate::Deserialize(m2, #m2, (int)sizeof(#m2)-1, v); \
		CarpJsonTemplate::Deserialize(m3, #m3, (int)sizeof(#m3)-1, v); \
	} \
	virtual void Serialize(CarpJsonValueMap& v) const { \
		CarpJsonTemplate::SerializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::SerializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::SerializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::SerializeValueMap(m3, #m3, v); \
	} \
	virtual void Deserialize(const CarpJsonValueMap& v) { \
		CarpJsonTemplate::DeserializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::DeserializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::DeserializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::DeserializeValueMap(m3, #m3, v); \
	} \
	virtual const std::vector<std::string>& GetNameList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#m0); \
			s_list.push_back(#m1); \
			s_list.push_back(#m2); \
			s_list.push_back(#m3); \
		} return s_list; \
	} \
	virtual const std::vector<std::string>& GetTypeList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#t0); \
			s_list.push_back(#t1); \
			s_list.push_back(#t2); \
			s_list.push_back(#t3); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; \
}
#define CARP_JSON_MACRO_11(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4) \
class Name : public CarpJson { \
public: \
	virtual const char* GetID() const { return #Name; } \
	inline static const char* GetStaticID() { return #Name; } \
public: \
	virtual void Serialize(CarpJsonValue& v, CarpJsonAlloc& a) const { \
		CarpJsonTemplate::Serialize(m0, #m0, (int)sizeof(#m0)-1, v, a); \
		CarpJsonTemplate::Serialize(m1, #m1, (int)sizeof(#m1)-1, v, a); \
		CarpJsonTemplate::Serialize(m2, #m2, (int)sizeof(#m2)-1, v, a); \
		CarpJsonTemplate::Serialize(m3, #m3, (int)sizeof(#m3)-1, v, a); \
		CarpJsonTemplate::Serialize(m4, #m4, (int)sizeof(#m4)-1, v, a); \
	} \
	virtual void Deserialize(const CarpJsonValue& v) { \
		CarpJsonTemplate::Deserialize(m0, #m0, (int)sizeof(#m0)-1, v); \
		CarpJsonTemplate::Deserialize(m1, #m1, (int)sizeof(#m1)-1, v); \
		CarpJsonTemplate::Deserialize(m2, #m2, (int)sizeof(#m2)-1, v); \
		CarpJsonTemplate::Deserialize(m3, #m3, (int)sizeof(#m3)-1, v); \
		CarpJsonTemplate::Deserialize(m4, #m4, (int)sizeof(#m4)-1, v); \
	} \
	virtual void Serialize(CarpJsonValueMap& v) const { \
		CarpJsonTemplate::SerializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::SerializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::SerializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::SerializeValueMap(m3, #m3, v); \
		CarpJsonTemplate::SerializeValueMap(m4, #m4, v); \
	} \
	virtual void Deserialize(const CarpJsonValueMap& v) { \
		CarpJsonTemplate::DeserializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::DeserializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::DeserializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::DeserializeValueMap(m3, #m3, v); \
		CarpJsonTemplate::DeserializeValueMap(m4, #m4, v); \
	} \
	virtual const std::vector<std::string>& GetNameList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#m0); \
			s_list.push_back(#m1); \
			s_list.push_back(#m2); \
			s_list.push_back(#m3); \
			s_list.push_back(#m4); \
		} return s_list; \
	} \
	virtual const std::vector<std::string>& GetTypeList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#t0); \
			s_list.push_back(#t1); \
			s_list.push_back(#t2); \
			s_list.push_back(#t3); \
			s_list.push_back(#t4); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; \
}
#define CARP_JSON_MACRO_13(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5) \
class Name : public CarpJson { \
public: \
	virtual const char* GetID() const { return #Name; } \
	inline static const char* GetStaticID() { return #Name; } \
public: \
	virtual void Serialize(CarpJsonValue& v, CarpJsonAlloc& a) const { \
		CarpJsonTemplate::Serialize(m0, #m0, (int)sizeof(#m0)-1, v, a); \
		CarpJsonTemplate::Serialize(m1, #m1, (int)sizeof(#m1)-1, v, a); \
		CarpJsonTemplate::Serialize(m2, #m2, (int)sizeof(#m2)-1, v, a); \
		CarpJsonTemplate::Serialize(m3, #m3, (int)sizeof(#m3)-1, v, a); \
		CarpJsonTemplate::Serialize(m4, #m4, (int)sizeof(#m4)-1, v, a); \
		CarpJsonTemplate::Serialize(m5, #m5, (int)sizeof(#m5)-1, v, a); \
	} \
	virtual void Deserialize(const CarpJsonValue& v) { \
		CarpJsonTemplate::Deserialize(m0, #m0, (int)sizeof(#m0)-1, v); \
		CarpJsonTemplate::Deserialize(m1, #m1, (int)sizeof(#m1)-1, v); \
		CarpJsonTemplate::Deserialize(m2, #m2, (int)sizeof(#m2)-1, v); \
		CarpJsonTemplate::Deserialize(m3, #m3, (int)sizeof(#m3)-1, v); \
		CarpJsonTemplate::Deserialize(m4, #m4, (int)sizeof(#m4)-1, v); \
		CarpJsonTemplate::Deserialize(m5, #m5, (int)sizeof(#m5)-1, v); \
	} \
	virtual void Serialize(CarpJsonValueMap& v) const { \
		CarpJsonTemplate::SerializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::SerializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::SerializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::SerializeValueMap(m3, #m3, v); \
		CarpJsonTemplate::SerializeValueMap(m4, #m4, v); \
		CarpJsonTemplate::SerializeValueMap(m5, #m5, v); \
	} \
	virtual void Deserialize(const CarpJsonValueMap& v) { \
		CarpJsonTemplate::DeserializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::DeserializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::DeserializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::DeserializeValueMap(m3, #m3, v); \
		CarpJsonTemplate::DeserializeValueMap(m4, #m4, v); \
		CarpJsonTemplate::DeserializeValueMap(m5, #m5, v); \
	} \
	virtual const std::vector<std::string>& GetNameList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#m0); \
			s_list.push_back(#m1); \
			s_list.push_back(#m2); \
			s_list.push_back(#m3); \
			s_list.push_back(#m4); \
			s_list.push_back(#m5); \
		} return s_list; \
	} \
	virtual const std::vector<std::string>& GetTypeList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#t0); \
			s_list.push_back(#t1); \
			s_list.push_back(#t2); \
			s_list.push_back(#t3); \
			s_list.push_back(#t4); \
			s_list.push_back(#t5); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; \
}
#define CARP_JSON_MACRO_15(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6) \
class Name : public CarpJson { \
public: \
	virtual const char* GetID() const { return #Name; } \
	inline static const char* GetStaticID() { return #Name; } \
public: \
	virtual void Serialize(CarpJsonValue& v, CarpJsonAlloc& a) const { \
		CarpJsonTemplate::Serialize(m0, #m0, (int)sizeof(#m0)-1, v, a); \
		CarpJsonTemplate::Serialize(m1, #m1, (int)sizeof(#m1)-1, v, a); \
		CarpJsonTemplate::Serialize(m2, #m2, (int)sizeof(#m2)-1, v, a); \
		CarpJsonTemplate::Serialize(m3, #m3, (int)sizeof(#m3)-1, v, a); \
		CarpJsonTemplate::Serialize(m4, #m4, (int)sizeof(#m4)-1, v, a); \
		CarpJsonTemplate::Serialize(m5, #m5, (int)sizeof(#m5)-1, v, a); \
		CarpJsonTemplate::Serialize(m6, #m6, (int)sizeof(#m6)-1, v, a); \
	} \
	virtual void Deserialize(const CarpJsonValue& v) { \
		CarpJsonTemplate::Deserialize(m0, #m0, (int)sizeof(#m0)-1, v); \
		CarpJsonTemplate::Deserialize(m1, #m1, (int)sizeof(#m1)-1, v); \
		CarpJsonTemplate::Deserialize(m2, #m2, (int)sizeof(#m2)-1, v); \
		CarpJsonTemplate::Deserialize(m3, #m3, (int)sizeof(#m3)-1, v); \
		CarpJsonTemplate::Deserialize(m4, #m4, (int)sizeof(#m4)-1, v); \
		CarpJsonTemplate::Deserialize(m5, #m5, (int)sizeof(#m5)-1, v); \
		CarpJsonTemplate::Deserialize(m6, #m6, (int)sizeof(#m6)-1, v); \
	} \
	virtual void Serialize(CarpJsonValueMap& v) const { \
		CarpJsonTemplate::SerializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::SerializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::SerializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::SerializeValueMap(m3, #m3, v); \
		CarpJsonTemplate::SerializeValueMap(m4, #m4, v); \
		CarpJsonTemplate::SerializeValueMap(m5, #m5, v); \
		CarpJsonTemplate::SerializeValueMap(m6, #m6, v); \
	} \
	virtual void Deserialize(const CarpJsonValueMap& v) { \
		CarpJsonTemplate::DeserializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::DeserializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::DeserializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::DeserializeValueMap(m3, #m3, v); \
		CarpJsonTemplate::DeserializeValueMap(m4, #m4, v); \
		CarpJsonTemplate::DeserializeValueMap(m5, #m5, v); \
		CarpJsonTemplate::DeserializeValueMap(m6, #m6, v); \
	} \
	virtual const std::vector<std::string>& GetNameList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#m0); \
			s_list.push_back(#m1); \
			s_list.push_back(#m2); \
			s_list.push_back(#m3); \
			s_list.push_back(#m4); \
			s_list.push_back(#m5); \
			s_list.push_back(#m6); \
		} return s_list; \
	} \
	virtual const std::vector<std::string>& GetTypeList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#t0); \
			s_list.push_back(#t1); \
			s_list.push_back(#t2); \
			s_list.push_back(#t3); \
			s_list.push_back(#t4); \
			s_list.push_back(#t5); \
			s_list.push_back(#t6); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; \
}
#define CARP_JSON_MACRO_17(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7) \
class Name : public CarpJson { \
public: \
	virtual const char* GetID() const { return #Name; } \
	inline static const char* GetStaticID() { return #Name; } \
public: \
	virtual void Serialize(CarpJsonValue& v, CarpJsonAlloc& a) const { \
		CarpJsonTemplate::Serialize(m0, #m0, (int)sizeof(#m0)-1, v, a); \
		CarpJsonTemplate::Serialize(m1, #m1, (int)sizeof(#m1)-1, v, a); \
		CarpJsonTemplate::Serialize(m2, #m2, (int)sizeof(#m2)-1, v, a); \
		CarpJsonTemplate::Serialize(m3, #m3, (int)sizeof(#m3)-1, v, a); \
		CarpJsonTemplate::Serialize(m4, #m4, (int)sizeof(#m4)-1, v, a); \
		CarpJsonTemplate::Serialize(m5, #m5, (int)sizeof(#m5)-1, v, a); \
		CarpJsonTemplate::Serialize(m6, #m6, (int)sizeof(#m6)-1, v, a); \
		CarpJsonTemplate::Serialize(m7, #m7, (int)sizeof(#m7)-1, v, a); \
	} \
	virtual void Deserialize(const CarpJsonValue& v) { \
		CarpJsonTemplate::Deserialize(m0, #m0, (int)sizeof(#m0)-1, v); \
		CarpJsonTemplate::Deserialize(m1, #m1, (int)sizeof(#m1)-1, v); \
		CarpJsonTemplate::Deserialize(m2, #m2, (int)sizeof(#m2)-1, v); \
		CarpJsonTemplate::Deserialize(m3, #m3, (int)sizeof(#m3)-1, v); \
		CarpJsonTemplate::Deserialize(m4, #m4, (int)sizeof(#m4)-1, v); \
		CarpJsonTemplate::Deserialize(m5, #m5, (int)sizeof(#m5)-1, v); \
		CarpJsonTemplate::Deserialize(m6, #m6, (int)sizeof(#m6)-1, v); \
		CarpJsonTemplate::Deserialize(m7, #m7, (int)sizeof(#m7)-1, v); \
	} \
	virtual void Serialize(CarpJsonValueMap& v) const { \
		CarpJsonTemplate::SerializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::SerializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::SerializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::SerializeValueMap(m3, #m3, v); \
		CarpJsonTemplate::SerializeValueMap(m4, #m4, v); \
		CarpJsonTemplate::SerializeValueMap(m5, #m5, v); \
		CarpJsonTemplate::SerializeValueMap(m6, #m6, v); \
		CarpJsonTemplate::SerializeValueMap(m7, #m7, v); \
	} \
	virtual void Deserialize(const CarpJsonValueMap& v) { \
		CarpJsonTemplate::DeserializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::DeserializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::DeserializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::DeserializeValueMap(m3, #m3, v); \
		CarpJsonTemplate::DeserializeValueMap(m4, #m4, v); \
		CarpJsonTemplate::DeserializeValueMap(m5, #m5, v); \
		CarpJsonTemplate::DeserializeValueMap(m6, #m6, v); \
		CarpJsonTemplate::DeserializeValueMap(m7, #m7, v); \
	} \
	virtual const std::vector<std::string>& GetNameList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#m0); \
			s_list.push_back(#m1); \
			s_list.push_back(#m2); \
			s_list.push_back(#m3); \
			s_list.push_back(#m4); \
			s_list.push_back(#m5); \
			s_list.push_back(#m6); \
			s_list.push_back(#m7); \
		} return s_list; \
	} \
	virtual const std::vector<std::string>& GetTypeList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#t0); \
			s_list.push_back(#t1); \
			s_list.push_back(#t2); \
			s_list.push_back(#t3); \
			s_list.push_back(#t4); \
			s_list.push_back(#t5); \
			s_list.push_back(#t6); \
			s_list.push_back(#t7); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; \
}
#define CARP_JSON_MACRO_19(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8) \
class Name : public CarpJson { \
public: \
	virtual const char* GetID() const { return #Name; } \
	inline static const char* GetStaticID() { return #Name; } \
public: \
	virtual void Serialize(CarpJsonValue& v, CarpJsonAlloc& a) const { \
		CarpJsonTemplate::Serialize(m0, #m0, (int)sizeof(#m0)-1, v, a); \
		CarpJsonTemplate::Serialize(m1, #m1, (int)sizeof(#m1)-1, v, a); \
		CarpJsonTemplate::Serialize(m2, #m2, (int)sizeof(#m2)-1, v, a); \
		CarpJsonTemplate::Serialize(m3, #m3, (int)sizeof(#m3)-1, v, a); \
		CarpJsonTemplate::Serialize(m4, #m4, (int)sizeof(#m4)-1, v, a); \
		CarpJsonTemplate::Serialize(m5, #m5, (int)sizeof(#m5)-1, v, a); \
		CarpJsonTemplate::Serialize(m6, #m6, (int)sizeof(#m6)-1, v, a); \
		CarpJsonTemplate::Serialize(m7, #m7, (int)sizeof(#m7)-1, v, a); \
		CarpJsonTemplate::Serialize(m8, #m8, (int)sizeof(#m8)-1, v, a); \
	} \
	virtual void Deserialize(const CarpJsonValue& v) { \
		CarpJsonTemplate::Deserialize(m0, #m0, (int)sizeof(#m0)-1, v); \
		CarpJsonTemplate::Deserialize(m1, #m1, (int)sizeof(#m1)-1, v); \
		CarpJsonTemplate::Deserialize(m2, #m2, (int)sizeof(#m2)-1, v); \
		CarpJsonTemplate::Deserialize(m3, #m3, (int)sizeof(#m3)-1, v); \
		CarpJsonTemplate::Deserialize(m4, #m4, (int)sizeof(#m4)-1, v); \
		CarpJsonTemplate::Deserialize(m5, #m5, (int)sizeof(#m5)-1, v); \
		CarpJsonTemplate::Deserialize(m6, #m6, (int)sizeof(#m6)-1, v); \
		CarpJsonTemplate::Deserialize(m7, #m7, (int)sizeof(#m7)-1, v); \
		CarpJsonTemplate::Deserialize(m8, #m8, (int)sizeof(#m8)-1, v); \
	} \
	virtual void Serialize(CarpJsonValueMap& v) const { \
		CarpJsonTemplate::SerializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::SerializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::SerializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::SerializeValueMap(m3, #m3, v); \
		CarpJsonTemplate::SerializeValueMap(m4, #m4, v); \
		CarpJsonTemplate::SerializeValueMap(m5, #m5, v); \
		CarpJsonTemplate::SerializeValueMap(m6, #m6, v); \
		CarpJsonTemplate::SerializeValueMap(m7, #m7, v); \
		CarpJsonTemplate::SerializeValueMap(m8, #m8, v); \
	} \
	virtual void Deserialize(const CarpJsonValueMap& v) { \
		CarpJsonTemplate::DeserializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::DeserializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::DeserializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::DeserializeValueMap(m3, #m3, v); \
		CarpJsonTemplate::DeserializeValueMap(m4, #m4, v); \
		CarpJsonTemplate::DeserializeValueMap(m5, #m5, v); \
		CarpJsonTemplate::DeserializeValueMap(m6, #m6, v); \
		CarpJsonTemplate::DeserializeValueMap(m7, #m7, v); \
		CarpJsonTemplate::DeserializeValueMap(m8, #m8, v); \
	} \
	virtual const std::vector<std::string>& GetNameList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#m0); \
			s_list.push_back(#m1); \
			s_list.push_back(#m2); \
			s_list.push_back(#m3); \
			s_list.push_back(#m4); \
			s_list.push_back(#m5); \
			s_list.push_back(#m6); \
			s_list.push_back(#m7); \
			s_list.push_back(#m8); \
		} return s_list; \
	} \
	virtual const std::vector<std::string>& GetTypeList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#t0); \
			s_list.push_back(#t1); \
			s_list.push_back(#t2); \
			s_list.push_back(#t3); \
			s_list.push_back(#t4); \
			s_list.push_back(#t5); \
			s_list.push_back(#t6); \
			s_list.push_back(#t7); \
			s_list.push_back(#t8); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; \
}
#define CARP_JSON_MACRO_21(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8, t9, m9) \
class Name : public CarpJson { \
public: \
	virtual const char* GetID() const { return #Name; } \
	inline static const char* GetStaticID() { return #Name; } \
public: \
	virtual void Serialize(CarpJsonValue& v, CarpJsonAlloc& a) const { \
		CarpJsonTemplate::Serialize(m0, #m0, (int)sizeof(#m0)-1, v, a); \
		CarpJsonTemplate::Serialize(m1, #m1, (int)sizeof(#m1)-1, v, a); \
		CarpJsonTemplate::Serialize(m2, #m2, (int)sizeof(#m2)-1, v, a); \
		CarpJsonTemplate::Serialize(m3, #m3, (int)sizeof(#m3)-1, v, a); \
		CarpJsonTemplate::Serialize(m4, #m4, (int)sizeof(#m4)-1, v, a); \
		CarpJsonTemplate::Serialize(m5, #m5, (int)sizeof(#m5)-1, v, a); \
		CarpJsonTemplate::Serialize(m6, #m6, (int)sizeof(#m6)-1, v, a); \
		CarpJsonTemplate::Serialize(m7, #m7, (int)sizeof(#m7)-1, v, a); \
		CarpJsonTemplate::Serialize(m8, #m8, (int)sizeof(#m8)-1, v, a); \
		CarpJsonTemplate::Serialize(m9, #m9, (int)sizeof(#m9)-1, v, a); \
	} \
	virtual void Deserialize(const CarpJsonValue& v) { \
		CarpJsonTemplate::Deserialize(m0, #m0, (int)sizeof(#m0)-1, v); \
		CarpJsonTemplate::Deserialize(m1, #m1, (int)sizeof(#m1)-1, v); \
		CarpJsonTemplate::Deserialize(m2, #m2, (int)sizeof(#m2)-1, v); \
		CarpJsonTemplate::Deserialize(m3, #m3, (int)sizeof(#m3)-1, v); \
		CarpJsonTemplate::Deserialize(m4, #m4, (int)sizeof(#m4)-1, v); \
		CarpJsonTemplate::Deserialize(m5, #m5, (int)sizeof(#m5)-1, v); \
		CarpJsonTemplate::Deserialize(m6, #m6, (int)sizeof(#m6)-1, v); \
		CarpJsonTemplate::Deserialize(m7, #m7, (int)sizeof(#m7)-1, v); \
		CarpJsonTemplate::Deserialize(m8, #m8, (int)sizeof(#m8)-1, v); \
		CarpJsonTemplate::Deserialize(m9, #m9, (int)sizeof(#m9)-1, v); \
	} \
	virtual void Serialize(CarpJsonValueMap& v) const { \
		CarpJsonTemplate::SerializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::SerializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::SerializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::SerializeValueMap(m3, #m3, v); \
		CarpJsonTemplate::SerializeValueMap(m4, #m4, v); \
		CarpJsonTemplate::SerializeValueMap(m5, #m5, v); \
		CarpJsonTemplate::SerializeValueMap(m6, #m6, v); \
		CarpJsonTemplate::SerializeValueMap(m7, #m7, v); \
		CarpJsonTemplate::SerializeValueMap(m8, #m8, v); \
		CarpJsonTemplate::SerializeValueMap(m9, #m9, v); \
	} \
	virtual void Deserialize(const CarpJsonValueMap& v) { \
		CarpJsonTemplate::DeserializeValueMap(m0, #m0, v); \
		CarpJsonTemplate::DeserializeValueMap(m1, #m1, v); \
		CarpJsonTemplate::DeserializeValueMap(m2, #m2, v); \
		CarpJsonTemplate::DeserializeValueMap(m3, #m3, v); \
		CarpJsonTemplate::DeserializeValueMap(m4, #m4, v); \
		CarpJsonTemplate::DeserializeValueMap(m5, #m5, v); \
		CarpJsonTemplate::DeserializeValueMap(m6, #m6, v); \
		CarpJsonTemplate::DeserializeValueMap(m7, #m7, v); \
		CarpJsonTemplate::DeserializeValueMap(m8, #m8, v); \
		CarpJsonTemplate::DeserializeValueMap(m9, #m9, v); \
	} \
	virtual const std::vector<std::string>& GetNameList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#m0); \
			s_list.push_back(#m1); \
			s_list.push_back(#m2); \
			s_list.push_back(#m3); \
			s_list.push_back(#m4); \
			s_list.push_back(#m5); \
			s_list.push_back(#m6); \
			s_list.push_back(#m7); \
			s_list.push_back(#m8); \
			s_list.push_back(#m9); \
		} return s_list; \
	} \
	virtual const std::vector<std::string>& GetTypeList() const { \
		static std::vector<std::string> s_list; \
		if (s_list.empty()) { \
			s_list.push_back(#t0); \
			s_list.push_back(#t1); \
			s_list.push_back(#t2); \
			s_list.push_back(#t3); \
			s_list.push_back(#t4); \
			s_list.push_back(#t5); \
			s_list.push_back(#t6); \
			s_list.push_back(#t7); \
			s_list.push_back(#t8); \
			s_list.push_back(#t9); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8(), m9() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; \
}

#endif // _ALITTLE_JSON_1_H_
