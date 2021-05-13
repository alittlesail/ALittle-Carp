
#ifndef CARP_JSON_6_INCLUDED
#define CARP_JSON_6_INCLUDED

#include "carp_json.hpp"

#define CARP_JSON_MACRO_103(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8, t9, m9, t10, m10, t11, m11, t12, m12, t13, m13, t14, m14, t15, m15, t16, m16, t17, m17, t18, m18, t19, m19, t20, m20, t21, m21, t22, m22, t23, m23, t24, m24, t25, m25, t26, m26, t27, m27, t28, m28, t29, m29, t30, m30, t31, m31, t32, m32, t33, m33, t34, m34, t35, m35, t36, m36, t37, m37, t38, m38, t39, m39, t40, m40, t41, m41, t42, m42, t43, m43, t44, m44, t45, m45, t46, m46, t47, m47, t48, m48, t49, m49, t50, m50) \
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
		CarpJsonTemplate::Serialize(m10, #m10, (int)sizeof(#m10)-1, v, a); \
		CarpJsonTemplate::Serialize(m11, #m11, (int)sizeof(#m11)-1, v, a); \
		CarpJsonTemplate::Serialize(m12, #m12, (int)sizeof(#m12)-1, v, a); \
		CarpJsonTemplate::Serialize(m13, #m13, (int)sizeof(#m13)-1, v, a); \
		CarpJsonTemplate::Serialize(m14, #m14, (int)sizeof(#m14)-1, v, a); \
		CarpJsonTemplate::Serialize(m15, #m15, (int)sizeof(#m15)-1, v, a); \
		CarpJsonTemplate::Serialize(m16, #m16, (int)sizeof(#m16)-1, v, a); \
		CarpJsonTemplate::Serialize(m17, #m17, (int)sizeof(#m17)-1, v, a); \
		CarpJsonTemplate::Serialize(m18, #m18, (int)sizeof(#m18)-1, v, a); \
		CarpJsonTemplate::Serialize(m19, #m19, (int)sizeof(#m19)-1, v, a); \
		CarpJsonTemplate::Serialize(m20, #m20, (int)sizeof(#m20)-1, v, a); \
		CarpJsonTemplate::Serialize(m21, #m21, (int)sizeof(#m21)-1, v, a); \
		CarpJsonTemplate::Serialize(m22, #m22, (int)sizeof(#m22)-1, v, a); \
		CarpJsonTemplate::Serialize(m23, #m23, (int)sizeof(#m23)-1, v, a); \
		CarpJsonTemplate::Serialize(m24, #m24, (int)sizeof(#m24)-1, v, a); \
		CarpJsonTemplate::Serialize(m25, #m25, (int)sizeof(#m25)-1, v, a); \
		CarpJsonTemplate::Serialize(m26, #m26, (int)sizeof(#m26)-1, v, a); \
		CarpJsonTemplate::Serialize(m27, #m27, (int)sizeof(#m27)-1, v, a); \
		CarpJsonTemplate::Serialize(m28, #m28, (int)sizeof(#m28)-1, v, a); \
		CarpJsonTemplate::Serialize(m29, #m29, (int)sizeof(#m29)-1, v, a); \
		CarpJsonTemplate::Serialize(m30, #m30, (int)sizeof(#m30)-1, v, a); \
		CarpJsonTemplate::Serialize(m31, #m31, (int)sizeof(#m31)-1, v, a); \
		CarpJsonTemplate::Serialize(m32, #m32, (int)sizeof(#m32)-1, v, a); \
		CarpJsonTemplate::Serialize(m33, #m33, (int)sizeof(#m33)-1, v, a); \
		CarpJsonTemplate::Serialize(m34, #m34, (int)sizeof(#m34)-1, v, a); \
		CarpJsonTemplate::Serialize(m35, #m35, (int)sizeof(#m35)-1, v, a); \
		CarpJsonTemplate::Serialize(m36, #m36, (int)sizeof(#m36)-1, v, a); \
		CarpJsonTemplate::Serialize(m37, #m37, (int)sizeof(#m37)-1, v, a); \
		CarpJsonTemplate::Serialize(m38, #m38, (int)sizeof(#m38)-1, v, a); \
		CarpJsonTemplate::Serialize(m39, #m39, (int)sizeof(#m39)-1, v, a); \
		CarpJsonTemplate::Serialize(m40, #m40, (int)sizeof(#m40)-1, v, a); \
		CarpJsonTemplate::Serialize(m41, #m41, (int)sizeof(#m41)-1, v, a); \
		CarpJsonTemplate::Serialize(m42, #m42, (int)sizeof(#m42)-1, v, a); \
		CarpJsonTemplate::Serialize(m43, #m43, (int)sizeof(#m43)-1, v, a); \
		CarpJsonTemplate::Serialize(m44, #m44, (int)sizeof(#m44)-1, v, a); \
		CarpJsonTemplate::Serialize(m45, #m45, (int)sizeof(#m45)-1, v, a); \
		CarpJsonTemplate::Serialize(m46, #m46, (int)sizeof(#m46)-1, v, a); \
		CarpJsonTemplate::Serialize(m47, #m47, (int)sizeof(#m47)-1, v, a); \
		CarpJsonTemplate::Serialize(m48, #m48, (int)sizeof(#m48)-1, v, a); \
		CarpJsonTemplate::Serialize(m49, #m49, (int)sizeof(#m49)-1, v, a); \
		CarpJsonTemplate::Serialize(m50, #m50, (int)sizeof(#m50)-1, v, a); \
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
		CarpJsonTemplate::Deserialize(m10, #m10, (int)sizeof(#m10)-1, v); \
		CarpJsonTemplate::Deserialize(m11, #m11, (int)sizeof(#m11)-1, v); \
		CarpJsonTemplate::Deserialize(m12, #m12, (int)sizeof(#m12)-1, v); \
		CarpJsonTemplate::Deserialize(m13, #m13, (int)sizeof(#m13)-1, v); \
		CarpJsonTemplate::Deserialize(m14, #m14, (int)sizeof(#m14)-1, v); \
		CarpJsonTemplate::Deserialize(m15, #m15, (int)sizeof(#m15)-1, v); \
		CarpJsonTemplate::Deserialize(m16, #m16, (int)sizeof(#m16)-1, v); \
		CarpJsonTemplate::Deserialize(m17, #m17, (int)sizeof(#m17)-1, v); \
		CarpJsonTemplate::Deserialize(m18, #m18, (int)sizeof(#m18)-1, v); \
		CarpJsonTemplate::Deserialize(m19, #m19, (int)sizeof(#m19)-1, v); \
		CarpJsonTemplate::Deserialize(m20, #m20, (int)sizeof(#m20)-1, v); \
		CarpJsonTemplate::Deserialize(m21, #m21, (int)sizeof(#m21)-1, v); \
		CarpJsonTemplate::Deserialize(m22, #m22, (int)sizeof(#m22)-1, v); \
		CarpJsonTemplate::Deserialize(m23, #m23, (int)sizeof(#m23)-1, v); \
		CarpJsonTemplate::Deserialize(m24, #m24, (int)sizeof(#m24)-1, v); \
		CarpJsonTemplate::Deserialize(m25, #m25, (int)sizeof(#m25)-1, v); \
		CarpJsonTemplate::Deserialize(m26, #m26, (int)sizeof(#m26)-1, v); \
		CarpJsonTemplate::Deserialize(m27, #m27, (int)sizeof(#m27)-1, v); \
		CarpJsonTemplate::Deserialize(m28, #m28, (int)sizeof(#m28)-1, v); \
		CarpJsonTemplate::Deserialize(m29, #m29, (int)sizeof(#m29)-1, v); \
		CarpJsonTemplate::Deserialize(m30, #m30, (int)sizeof(#m30)-1, v); \
		CarpJsonTemplate::Deserialize(m31, #m31, (int)sizeof(#m31)-1, v); \
		CarpJsonTemplate::Deserialize(m32, #m32, (int)sizeof(#m32)-1, v); \
		CarpJsonTemplate::Deserialize(m33, #m33, (int)sizeof(#m33)-1, v); \
		CarpJsonTemplate::Deserialize(m34, #m34, (int)sizeof(#m34)-1, v); \
		CarpJsonTemplate::Deserialize(m35, #m35, (int)sizeof(#m35)-1, v); \
		CarpJsonTemplate::Deserialize(m36, #m36, (int)sizeof(#m36)-1, v); \
		CarpJsonTemplate::Deserialize(m37, #m37, (int)sizeof(#m37)-1, v); \
		CarpJsonTemplate::Deserialize(m38, #m38, (int)sizeof(#m38)-1, v); \
		CarpJsonTemplate::Deserialize(m39, #m39, (int)sizeof(#m39)-1, v); \
		CarpJsonTemplate::Deserialize(m40, #m40, (int)sizeof(#m40)-1, v); \
		CarpJsonTemplate::Deserialize(m41, #m41, (int)sizeof(#m41)-1, v); \
		CarpJsonTemplate::Deserialize(m42, #m42, (int)sizeof(#m42)-1, v); \
		CarpJsonTemplate::Deserialize(m43, #m43, (int)sizeof(#m43)-1, v); \
		CarpJsonTemplate::Deserialize(m44, #m44, (int)sizeof(#m44)-1, v); \
		CarpJsonTemplate::Deserialize(m45, #m45, (int)sizeof(#m45)-1, v); \
		CarpJsonTemplate::Deserialize(m46, #m46, (int)sizeof(#m46)-1, v); \
		CarpJsonTemplate::Deserialize(m47, #m47, (int)sizeof(#m47)-1, v); \
		CarpJsonTemplate::Deserialize(m48, #m48, (int)sizeof(#m48)-1, v); \
		CarpJsonTemplate::Deserialize(m49, #m49, (int)sizeof(#m49)-1, v); \
		CarpJsonTemplate::Deserialize(m50, #m50, (int)sizeof(#m50)-1, v); \
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
		CarpJsonTemplate::SerializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::SerializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::SerializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::SerializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::SerializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::SerializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::SerializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::SerializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::SerializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::SerializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::SerializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::SerializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::SerializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::SerializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::SerializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::SerializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::SerializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::SerializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::SerializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::SerializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::SerializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::SerializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::SerializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::SerializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::SerializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::SerializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::SerializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::SerializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::SerializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::SerializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::SerializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::SerializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::SerializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::SerializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::SerializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::SerializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::SerializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::SerializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::SerializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::SerializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::SerializeValueMap(m50, #m50, v); \
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
		CarpJsonTemplate::DeserializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::DeserializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::DeserializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::DeserializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::DeserializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::DeserializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::DeserializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::DeserializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::DeserializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::DeserializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::DeserializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::DeserializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::DeserializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::DeserializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::DeserializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::DeserializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::DeserializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::DeserializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::DeserializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::DeserializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::DeserializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::DeserializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::DeserializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::DeserializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::DeserializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::DeserializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::DeserializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::DeserializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::DeserializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::DeserializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::DeserializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::DeserializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::DeserializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::DeserializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::DeserializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::DeserializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::DeserializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::DeserializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::DeserializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::DeserializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::DeserializeValueMap(m50, #m50, v); \
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
			s_list.push_back(#m10); \
			s_list.push_back(#m11); \
			s_list.push_back(#m12); \
			s_list.push_back(#m13); \
			s_list.push_back(#m14); \
			s_list.push_back(#m15); \
			s_list.push_back(#m16); \
			s_list.push_back(#m17); \
			s_list.push_back(#m18); \
			s_list.push_back(#m19); \
			s_list.push_back(#m20); \
			s_list.push_back(#m21); \
			s_list.push_back(#m22); \
			s_list.push_back(#m23); \
			s_list.push_back(#m24); \
			s_list.push_back(#m25); \
			s_list.push_back(#m26); \
			s_list.push_back(#m27); \
			s_list.push_back(#m28); \
			s_list.push_back(#m29); \
			s_list.push_back(#m30); \
			s_list.push_back(#m31); \
			s_list.push_back(#m32); \
			s_list.push_back(#m33); \
			s_list.push_back(#m34); \
			s_list.push_back(#m35); \
			s_list.push_back(#m36); \
			s_list.push_back(#m37); \
			s_list.push_back(#m38); \
			s_list.push_back(#m39); \
			s_list.push_back(#m40); \
			s_list.push_back(#m41); \
			s_list.push_back(#m42); \
			s_list.push_back(#m43); \
			s_list.push_back(#m44); \
			s_list.push_back(#m45); \
			s_list.push_back(#m46); \
			s_list.push_back(#m47); \
			s_list.push_back(#m48); \
			s_list.push_back(#m49); \
			s_list.push_back(#m50); \
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
			s_list.push_back(#t10); \
			s_list.push_back(#t11); \
			s_list.push_back(#t12); \
			s_list.push_back(#t13); \
			s_list.push_back(#t14); \
			s_list.push_back(#t15); \
			s_list.push_back(#t16); \
			s_list.push_back(#t17); \
			s_list.push_back(#t18); \
			s_list.push_back(#t19); \
			s_list.push_back(#t20); \
			s_list.push_back(#t21); \
			s_list.push_back(#t22); \
			s_list.push_back(#t23); \
			s_list.push_back(#t24); \
			s_list.push_back(#t25); \
			s_list.push_back(#t26); \
			s_list.push_back(#t27); \
			s_list.push_back(#t28); \
			s_list.push_back(#t29); \
			s_list.push_back(#t30); \
			s_list.push_back(#t31); \
			s_list.push_back(#t32); \
			s_list.push_back(#t33); \
			s_list.push_back(#t34); \
			s_list.push_back(#t35); \
			s_list.push_back(#t36); \
			s_list.push_back(#t37); \
			s_list.push_back(#t38); \
			s_list.push_back(#t39); \
			s_list.push_back(#t40); \
			s_list.push_back(#t41); \
			s_list.push_back(#t42); \
			s_list.push_back(#t43); \
			s_list.push_back(#t44); \
			s_list.push_back(#t45); \
			s_list.push_back(#t46); \
			s_list.push_back(#t47); \
			s_list.push_back(#t48); \
			s_list.push_back(#t49); \
			s_list.push_back(#t50); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8(), m9(), m10(), m11(), m12(), m13(), m14(), m15(), m16(), m17(), m18(), m19(), m20(), m21(), m22(), m23(), m24(), m25(), m26(), m27(), m28(), m29(), m30(), m31(), m32(), m33(), m34(), m35(), m36(), m37(), m38(), m39(), m40(), m41(), m42(), m43(), m44(), m45(), m46(), m47(), m48(), m49(), m50() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24; t25 m25; t26 m26; t27 m27; t28 m28; t29 m29; t30 m30; t31 m31; t32 m32; t33 m33; t34 m34; t35 m35; t36 m36; t37 m37; t38 m38; t39 m39; t40 m40; t41 m41; t42 m42; t43 m43; t44 m44; t45 m45; t46 m46; t47 m47; t48 m48; t49 m49; t50 m50; \
}
#define CARP_JSON_MACRO_105(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8, t9, m9, t10, m10, t11, m11, t12, m12, t13, m13, t14, m14, t15, m15, t16, m16, t17, m17, t18, m18, t19, m19, t20, m20, t21, m21, t22, m22, t23, m23, t24, m24, t25, m25, t26, m26, t27, m27, t28, m28, t29, m29, t30, m30, t31, m31, t32, m32, t33, m33, t34, m34, t35, m35, t36, m36, t37, m37, t38, m38, t39, m39, t40, m40, t41, m41, t42, m42, t43, m43, t44, m44, t45, m45, t46, m46, t47, m47, t48, m48, t49, m49, t50, m50, t51, m51) \
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
		CarpJsonTemplate::Serialize(m10, #m10, (int)sizeof(#m10)-1, v, a); \
		CarpJsonTemplate::Serialize(m11, #m11, (int)sizeof(#m11)-1, v, a); \
		CarpJsonTemplate::Serialize(m12, #m12, (int)sizeof(#m12)-1, v, a); \
		CarpJsonTemplate::Serialize(m13, #m13, (int)sizeof(#m13)-1, v, a); \
		CarpJsonTemplate::Serialize(m14, #m14, (int)sizeof(#m14)-1, v, a); \
		CarpJsonTemplate::Serialize(m15, #m15, (int)sizeof(#m15)-1, v, a); \
		CarpJsonTemplate::Serialize(m16, #m16, (int)sizeof(#m16)-1, v, a); \
		CarpJsonTemplate::Serialize(m17, #m17, (int)sizeof(#m17)-1, v, a); \
		CarpJsonTemplate::Serialize(m18, #m18, (int)sizeof(#m18)-1, v, a); \
		CarpJsonTemplate::Serialize(m19, #m19, (int)sizeof(#m19)-1, v, a); \
		CarpJsonTemplate::Serialize(m20, #m20, (int)sizeof(#m20)-1, v, a); \
		CarpJsonTemplate::Serialize(m21, #m21, (int)sizeof(#m21)-1, v, a); \
		CarpJsonTemplate::Serialize(m22, #m22, (int)sizeof(#m22)-1, v, a); \
		CarpJsonTemplate::Serialize(m23, #m23, (int)sizeof(#m23)-1, v, a); \
		CarpJsonTemplate::Serialize(m24, #m24, (int)sizeof(#m24)-1, v, a); \
		CarpJsonTemplate::Serialize(m25, #m25, (int)sizeof(#m25)-1, v, a); \
		CarpJsonTemplate::Serialize(m26, #m26, (int)sizeof(#m26)-1, v, a); \
		CarpJsonTemplate::Serialize(m27, #m27, (int)sizeof(#m27)-1, v, a); \
		CarpJsonTemplate::Serialize(m28, #m28, (int)sizeof(#m28)-1, v, a); \
		CarpJsonTemplate::Serialize(m29, #m29, (int)sizeof(#m29)-1, v, a); \
		CarpJsonTemplate::Serialize(m30, #m30, (int)sizeof(#m30)-1, v, a); \
		CarpJsonTemplate::Serialize(m31, #m31, (int)sizeof(#m31)-1, v, a); \
		CarpJsonTemplate::Serialize(m32, #m32, (int)sizeof(#m32)-1, v, a); \
		CarpJsonTemplate::Serialize(m33, #m33, (int)sizeof(#m33)-1, v, a); \
		CarpJsonTemplate::Serialize(m34, #m34, (int)sizeof(#m34)-1, v, a); \
		CarpJsonTemplate::Serialize(m35, #m35, (int)sizeof(#m35)-1, v, a); \
		CarpJsonTemplate::Serialize(m36, #m36, (int)sizeof(#m36)-1, v, a); \
		CarpJsonTemplate::Serialize(m37, #m37, (int)sizeof(#m37)-1, v, a); \
		CarpJsonTemplate::Serialize(m38, #m38, (int)sizeof(#m38)-1, v, a); \
		CarpJsonTemplate::Serialize(m39, #m39, (int)sizeof(#m39)-1, v, a); \
		CarpJsonTemplate::Serialize(m40, #m40, (int)sizeof(#m40)-1, v, a); \
		CarpJsonTemplate::Serialize(m41, #m41, (int)sizeof(#m41)-1, v, a); \
		CarpJsonTemplate::Serialize(m42, #m42, (int)sizeof(#m42)-1, v, a); \
		CarpJsonTemplate::Serialize(m43, #m43, (int)sizeof(#m43)-1, v, a); \
		CarpJsonTemplate::Serialize(m44, #m44, (int)sizeof(#m44)-1, v, a); \
		CarpJsonTemplate::Serialize(m45, #m45, (int)sizeof(#m45)-1, v, a); \
		CarpJsonTemplate::Serialize(m46, #m46, (int)sizeof(#m46)-1, v, a); \
		CarpJsonTemplate::Serialize(m47, #m47, (int)sizeof(#m47)-1, v, a); \
		CarpJsonTemplate::Serialize(m48, #m48, (int)sizeof(#m48)-1, v, a); \
		CarpJsonTemplate::Serialize(m49, #m49, (int)sizeof(#m49)-1, v, a); \
		CarpJsonTemplate::Serialize(m50, #m50, (int)sizeof(#m50)-1, v, a); \
		CarpJsonTemplate::Serialize(m51, #m51, (int)sizeof(#m51)-1, v, a); \
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
		CarpJsonTemplate::Deserialize(m10, #m10, (int)sizeof(#m10)-1, v); \
		CarpJsonTemplate::Deserialize(m11, #m11, (int)sizeof(#m11)-1, v); \
		CarpJsonTemplate::Deserialize(m12, #m12, (int)sizeof(#m12)-1, v); \
		CarpJsonTemplate::Deserialize(m13, #m13, (int)sizeof(#m13)-1, v); \
		CarpJsonTemplate::Deserialize(m14, #m14, (int)sizeof(#m14)-1, v); \
		CarpJsonTemplate::Deserialize(m15, #m15, (int)sizeof(#m15)-1, v); \
		CarpJsonTemplate::Deserialize(m16, #m16, (int)sizeof(#m16)-1, v); \
		CarpJsonTemplate::Deserialize(m17, #m17, (int)sizeof(#m17)-1, v); \
		CarpJsonTemplate::Deserialize(m18, #m18, (int)sizeof(#m18)-1, v); \
		CarpJsonTemplate::Deserialize(m19, #m19, (int)sizeof(#m19)-1, v); \
		CarpJsonTemplate::Deserialize(m20, #m20, (int)sizeof(#m20)-1, v); \
		CarpJsonTemplate::Deserialize(m21, #m21, (int)sizeof(#m21)-1, v); \
		CarpJsonTemplate::Deserialize(m22, #m22, (int)sizeof(#m22)-1, v); \
		CarpJsonTemplate::Deserialize(m23, #m23, (int)sizeof(#m23)-1, v); \
		CarpJsonTemplate::Deserialize(m24, #m24, (int)sizeof(#m24)-1, v); \
		CarpJsonTemplate::Deserialize(m25, #m25, (int)sizeof(#m25)-1, v); \
		CarpJsonTemplate::Deserialize(m26, #m26, (int)sizeof(#m26)-1, v); \
		CarpJsonTemplate::Deserialize(m27, #m27, (int)sizeof(#m27)-1, v); \
		CarpJsonTemplate::Deserialize(m28, #m28, (int)sizeof(#m28)-1, v); \
		CarpJsonTemplate::Deserialize(m29, #m29, (int)sizeof(#m29)-1, v); \
		CarpJsonTemplate::Deserialize(m30, #m30, (int)sizeof(#m30)-1, v); \
		CarpJsonTemplate::Deserialize(m31, #m31, (int)sizeof(#m31)-1, v); \
		CarpJsonTemplate::Deserialize(m32, #m32, (int)sizeof(#m32)-1, v); \
		CarpJsonTemplate::Deserialize(m33, #m33, (int)sizeof(#m33)-1, v); \
		CarpJsonTemplate::Deserialize(m34, #m34, (int)sizeof(#m34)-1, v); \
		CarpJsonTemplate::Deserialize(m35, #m35, (int)sizeof(#m35)-1, v); \
		CarpJsonTemplate::Deserialize(m36, #m36, (int)sizeof(#m36)-1, v); \
		CarpJsonTemplate::Deserialize(m37, #m37, (int)sizeof(#m37)-1, v); \
		CarpJsonTemplate::Deserialize(m38, #m38, (int)sizeof(#m38)-1, v); \
		CarpJsonTemplate::Deserialize(m39, #m39, (int)sizeof(#m39)-1, v); \
		CarpJsonTemplate::Deserialize(m40, #m40, (int)sizeof(#m40)-1, v); \
		CarpJsonTemplate::Deserialize(m41, #m41, (int)sizeof(#m41)-1, v); \
		CarpJsonTemplate::Deserialize(m42, #m42, (int)sizeof(#m42)-1, v); \
		CarpJsonTemplate::Deserialize(m43, #m43, (int)sizeof(#m43)-1, v); \
		CarpJsonTemplate::Deserialize(m44, #m44, (int)sizeof(#m44)-1, v); \
		CarpJsonTemplate::Deserialize(m45, #m45, (int)sizeof(#m45)-1, v); \
		CarpJsonTemplate::Deserialize(m46, #m46, (int)sizeof(#m46)-1, v); \
		CarpJsonTemplate::Deserialize(m47, #m47, (int)sizeof(#m47)-1, v); \
		CarpJsonTemplate::Deserialize(m48, #m48, (int)sizeof(#m48)-1, v); \
		CarpJsonTemplate::Deserialize(m49, #m49, (int)sizeof(#m49)-1, v); \
		CarpJsonTemplate::Deserialize(m50, #m50, (int)sizeof(#m50)-1, v); \
		CarpJsonTemplate::Deserialize(m51, #m51, (int)sizeof(#m51)-1, v); \
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
		CarpJsonTemplate::SerializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::SerializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::SerializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::SerializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::SerializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::SerializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::SerializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::SerializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::SerializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::SerializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::SerializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::SerializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::SerializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::SerializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::SerializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::SerializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::SerializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::SerializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::SerializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::SerializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::SerializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::SerializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::SerializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::SerializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::SerializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::SerializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::SerializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::SerializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::SerializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::SerializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::SerializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::SerializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::SerializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::SerializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::SerializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::SerializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::SerializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::SerializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::SerializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::SerializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::SerializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::SerializeValueMap(m51, #m51, v); \
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
		CarpJsonTemplate::DeserializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::DeserializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::DeserializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::DeserializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::DeserializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::DeserializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::DeserializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::DeserializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::DeserializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::DeserializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::DeserializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::DeserializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::DeserializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::DeserializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::DeserializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::DeserializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::DeserializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::DeserializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::DeserializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::DeserializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::DeserializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::DeserializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::DeserializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::DeserializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::DeserializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::DeserializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::DeserializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::DeserializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::DeserializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::DeserializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::DeserializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::DeserializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::DeserializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::DeserializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::DeserializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::DeserializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::DeserializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::DeserializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::DeserializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::DeserializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::DeserializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::DeserializeValueMap(m51, #m51, v); \
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
			s_list.push_back(#m10); \
			s_list.push_back(#m11); \
			s_list.push_back(#m12); \
			s_list.push_back(#m13); \
			s_list.push_back(#m14); \
			s_list.push_back(#m15); \
			s_list.push_back(#m16); \
			s_list.push_back(#m17); \
			s_list.push_back(#m18); \
			s_list.push_back(#m19); \
			s_list.push_back(#m20); \
			s_list.push_back(#m21); \
			s_list.push_back(#m22); \
			s_list.push_back(#m23); \
			s_list.push_back(#m24); \
			s_list.push_back(#m25); \
			s_list.push_back(#m26); \
			s_list.push_back(#m27); \
			s_list.push_back(#m28); \
			s_list.push_back(#m29); \
			s_list.push_back(#m30); \
			s_list.push_back(#m31); \
			s_list.push_back(#m32); \
			s_list.push_back(#m33); \
			s_list.push_back(#m34); \
			s_list.push_back(#m35); \
			s_list.push_back(#m36); \
			s_list.push_back(#m37); \
			s_list.push_back(#m38); \
			s_list.push_back(#m39); \
			s_list.push_back(#m40); \
			s_list.push_back(#m41); \
			s_list.push_back(#m42); \
			s_list.push_back(#m43); \
			s_list.push_back(#m44); \
			s_list.push_back(#m45); \
			s_list.push_back(#m46); \
			s_list.push_back(#m47); \
			s_list.push_back(#m48); \
			s_list.push_back(#m49); \
			s_list.push_back(#m50); \
			s_list.push_back(#m51); \
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
			s_list.push_back(#t10); \
			s_list.push_back(#t11); \
			s_list.push_back(#t12); \
			s_list.push_back(#t13); \
			s_list.push_back(#t14); \
			s_list.push_back(#t15); \
			s_list.push_back(#t16); \
			s_list.push_back(#t17); \
			s_list.push_back(#t18); \
			s_list.push_back(#t19); \
			s_list.push_back(#t20); \
			s_list.push_back(#t21); \
			s_list.push_back(#t22); \
			s_list.push_back(#t23); \
			s_list.push_back(#t24); \
			s_list.push_back(#t25); \
			s_list.push_back(#t26); \
			s_list.push_back(#t27); \
			s_list.push_back(#t28); \
			s_list.push_back(#t29); \
			s_list.push_back(#t30); \
			s_list.push_back(#t31); \
			s_list.push_back(#t32); \
			s_list.push_back(#t33); \
			s_list.push_back(#t34); \
			s_list.push_back(#t35); \
			s_list.push_back(#t36); \
			s_list.push_back(#t37); \
			s_list.push_back(#t38); \
			s_list.push_back(#t39); \
			s_list.push_back(#t40); \
			s_list.push_back(#t41); \
			s_list.push_back(#t42); \
			s_list.push_back(#t43); \
			s_list.push_back(#t44); \
			s_list.push_back(#t45); \
			s_list.push_back(#t46); \
			s_list.push_back(#t47); \
			s_list.push_back(#t48); \
			s_list.push_back(#t49); \
			s_list.push_back(#t50); \
			s_list.push_back(#t51); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8(), m9(), m10(), m11(), m12(), m13(), m14(), m15(), m16(), m17(), m18(), m19(), m20(), m21(), m22(), m23(), m24(), m25(), m26(), m27(), m28(), m29(), m30(), m31(), m32(), m33(), m34(), m35(), m36(), m37(), m38(), m39(), m40(), m41(), m42(), m43(), m44(), m45(), m46(), m47(), m48(), m49(), m50(), m51() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24; t25 m25; t26 m26; t27 m27; t28 m28; t29 m29; t30 m30; t31 m31; t32 m32; t33 m33; t34 m34; t35 m35; t36 m36; t37 m37; t38 m38; t39 m39; t40 m40; t41 m41; t42 m42; t43 m43; t44 m44; t45 m45; t46 m46; t47 m47; t48 m48; t49 m49; t50 m50; t51 m51; \
}
#define CARP_JSON_MACRO_107(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8, t9, m9, t10, m10, t11, m11, t12, m12, t13, m13, t14, m14, t15, m15, t16, m16, t17, m17, t18, m18, t19, m19, t20, m20, t21, m21, t22, m22, t23, m23, t24, m24, t25, m25, t26, m26, t27, m27, t28, m28, t29, m29, t30, m30, t31, m31, t32, m32, t33, m33, t34, m34, t35, m35, t36, m36, t37, m37, t38, m38, t39, m39, t40, m40, t41, m41, t42, m42, t43, m43, t44, m44, t45, m45, t46, m46, t47, m47, t48, m48, t49, m49, t50, m50, t51, m51, t52, m52) \
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
		CarpJsonTemplate::Serialize(m10, #m10, (int)sizeof(#m10)-1, v, a); \
		CarpJsonTemplate::Serialize(m11, #m11, (int)sizeof(#m11)-1, v, a); \
		CarpJsonTemplate::Serialize(m12, #m12, (int)sizeof(#m12)-1, v, a); \
		CarpJsonTemplate::Serialize(m13, #m13, (int)sizeof(#m13)-1, v, a); \
		CarpJsonTemplate::Serialize(m14, #m14, (int)sizeof(#m14)-1, v, a); \
		CarpJsonTemplate::Serialize(m15, #m15, (int)sizeof(#m15)-1, v, a); \
		CarpJsonTemplate::Serialize(m16, #m16, (int)sizeof(#m16)-1, v, a); \
		CarpJsonTemplate::Serialize(m17, #m17, (int)sizeof(#m17)-1, v, a); \
		CarpJsonTemplate::Serialize(m18, #m18, (int)sizeof(#m18)-1, v, a); \
		CarpJsonTemplate::Serialize(m19, #m19, (int)sizeof(#m19)-1, v, a); \
		CarpJsonTemplate::Serialize(m20, #m20, (int)sizeof(#m20)-1, v, a); \
		CarpJsonTemplate::Serialize(m21, #m21, (int)sizeof(#m21)-1, v, a); \
		CarpJsonTemplate::Serialize(m22, #m22, (int)sizeof(#m22)-1, v, a); \
		CarpJsonTemplate::Serialize(m23, #m23, (int)sizeof(#m23)-1, v, a); \
		CarpJsonTemplate::Serialize(m24, #m24, (int)sizeof(#m24)-1, v, a); \
		CarpJsonTemplate::Serialize(m25, #m25, (int)sizeof(#m25)-1, v, a); \
		CarpJsonTemplate::Serialize(m26, #m26, (int)sizeof(#m26)-1, v, a); \
		CarpJsonTemplate::Serialize(m27, #m27, (int)sizeof(#m27)-1, v, a); \
		CarpJsonTemplate::Serialize(m28, #m28, (int)sizeof(#m28)-1, v, a); \
		CarpJsonTemplate::Serialize(m29, #m29, (int)sizeof(#m29)-1, v, a); \
		CarpJsonTemplate::Serialize(m30, #m30, (int)sizeof(#m30)-1, v, a); \
		CarpJsonTemplate::Serialize(m31, #m31, (int)sizeof(#m31)-1, v, a); \
		CarpJsonTemplate::Serialize(m32, #m32, (int)sizeof(#m32)-1, v, a); \
		CarpJsonTemplate::Serialize(m33, #m33, (int)sizeof(#m33)-1, v, a); \
		CarpJsonTemplate::Serialize(m34, #m34, (int)sizeof(#m34)-1, v, a); \
		CarpJsonTemplate::Serialize(m35, #m35, (int)sizeof(#m35)-1, v, a); \
		CarpJsonTemplate::Serialize(m36, #m36, (int)sizeof(#m36)-1, v, a); \
		CarpJsonTemplate::Serialize(m37, #m37, (int)sizeof(#m37)-1, v, a); \
		CarpJsonTemplate::Serialize(m38, #m38, (int)sizeof(#m38)-1, v, a); \
		CarpJsonTemplate::Serialize(m39, #m39, (int)sizeof(#m39)-1, v, a); \
		CarpJsonTemplate::Serialize(m40, #m40, (int)sizeof(#m40)-1, v, a); \
		CarpJsonTemplate::Serialize(m41, #m41, (int)sizeof(#m41)-1, v, a); \
		CarpJsonTemplate::Serialize(m42, #m42, (int)sizeof(#m42)-1, v, a); \
		CarpJsonTemplate::Serialize(m43, #m43, (int)sizeof(#m43)-1, v, a); \
		CarpJsonTemplate::Serialize(m44, #m44, (int)sizeof(#m44)-1, v, a); \
		CarpJsonTemplate::Serialize(m45, #m45, (int)sizeof(#m45)-1, v, a); \
		CarpJsonTemplate::Serialize(m46, #m46, (int)sizeof(#m46)-1, v, a); \
		CarpJsonTemplate::Serialize(m47, #m47, (int)sizeof(#m47)-1, v, a); \
		CarpJsonTemplate::Serialize(m48, #m48, (int)sizeof(#m48)-1, v, a); \
		CarpJsonTemplate::Serialize(m49, #m49, (int)sizeof(#m49)-1, v, a); \
		CarpJsonTemplate::Serialize(m50, #m50, (int)sizeof(#m50)-1, v, a); \
		CarpJsonTemplate::Serialize(m51, #m51, (int)sizeof(#m51)-1, v, a); \
		CarpJsonTemplate::Serialize(m52, #m52, (int)sizeof(#m52)-1, v, a); \
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
		CarpJsonTemplate::Deserialize(m10, #m10, (int)sizeof(#m10)-1, v); \
		CarpJsonTemplate::Deserialize(m11, #m11, (int)sizeof(#m11)-1, v); \
		CarpJsonTemplate::Deserialize(m12, #m12, (int)sizeof(#m12)-1, v); \
		CarpJsonTemplate::Deserialize(m13, #m13, (int)sizeof(#m13)-1, v); \
		CarpJsonTemplate::Deserialize(m14, #m14, (int)sizeof(#m14)-1, v); \
		CarpJsonTemplate::Deserialize(m15, #m15, (int)sizeof(#m15)-1, v); \
		CarpJsonTemplate::Deserialize(m16, #m16, (int)sizeof(#m16)-1, v); \
		CarpJsonTemplate::Deserialize(m17, #m17, (int)sizeof(#m17)-1, v); \
		CarpJsonTemplate::Deserialize(m18, #m18, (int)sizeof(#m18)-1, v); \
		CarpJsonTemplate::Deserialize(m19, #m19, (int)sizeof(#m19)-1, v); \
		CarpJsonTemplate::Deserialize(m20, #m20, (int)sizeof(#m20)-1, v); \
		CarpJsonTemplate::Deserialize(m21, #m21, (int)sizeof(#m21)-1, v); \
		CarpJsonTemplate::Deserialize(m22, #m22, (int)sizeof(#m22)-1, v); \
		CarpJsonTemplate::Deserialize(m23, #m23, (int)sizeof(#m23)-1, v); \
		CarpJsonTemplate::Deserialize(m24, #m24, (int)sizeof(#m24)-1, v); \
		CarpJsonTemplate::Deserialize(m25, #m25, (int)sizeof(#m25)-1, v); \
		CarpJsonTemplate::Deserialize(m26, #m26, (int)sizeof(#m26)-1, v); \
		CarpJsonTemplate::Deserialize(m27, #m27, (int)sizeof(#m27)-1, v); \
		CarpJsonTemplate::Deserialize(m28, #m28, (int)sizeof(#m28)-1, v); \
		CarpJsonTemplate::Deserialize(m29, #m29, (int)sizeof(#m29)-1, v); \
		CarpJsonTemplate::Deserialize(m30, #m30, (int)sizeof(#m30)-1, v); \
		CarpJsonTemplate::Deserialize(m31, #m31, (int)sizeof(#m31)-1, v); \
		CarpJsonTemplate::Deserialize(m32, #m32, (int)sizeof(#m32)-1, v); \
		CarpJsonTemplate::Deserialize(m33, #m33, (int)sizeof(#m33)-1, v); \
		CarpJsonTemplate::Deserialize(m34, #m34, (int)sizeof(#m34)-1, v); \
		CarpJsonTemplate::Deserialize(m35, #m35, (int)sizeof(#m35)-1, v); \
		CarpJsonTemplate::Deserialize(m36, #m36, (int)sizeof(#m36)-1, v); \
		CarpJsonTemplate::Deserialize(m37, #m37, (int)sizeof(#m37)-1, v); \
		CarpJsonTemplate::Deserialize(m38, #m38, (int)sizeof(#m38)-1, v); \
		CarpJsonTemplate::Deserialize(m39, #m39, (int)sizeof(#m39)-1, v); \
		CarpJsonTemplate::Deserialize(m40, #m40, (int)sizeof(#m40)-1, v); \
		CarpJsonTemplate::Deserialize(m41, #m41, (int)sizeof(#m41)-1, v); \
		CarpJsonTemplate::Deserialize(m42, #m42, (int)sizeof(#m42)-1, v); \
		CarpJsonTemplate::Deserialize(m43, #m43, (int)sizeof(#m43)-1, v); \
		CarpJsonTemplate::Deserialize(m44, #m44, (int)sizeof(#m44)-1, v); \
		CarpJsonTemplate::Deserialize(m45, #m45, (int)sizeof(#m45)-1, v); \
		CarpJsonTemplate::Deserialize(m46, #m46, (int)sizeof(#m46)-1, v); \
		CarpJsonTemplate::Deserialize(m47, #m47, (int)sizeof(#m47)-1, v); \
		CarpJsonTemplate::Deserialize(m48, #m48, (int)sizeof(#m48)-1, v); \
		CarpJsonTemplate::Deserialize(m49, #m49, (int)sizeof(#m49)-1, v); \
		CarpJsonTemplate::Deserialize(m50, #m50, (int)sizeof(#m50)-1, v); \
		CarpJsonTemplate::Deserialize(m51, #m51, (int)sizeof(#m51)-1, v); \
		CarpJsonTemplate::Deserialize(m52, #m52, (int)sizeof(#m52)-1, v); \
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
		CarpJsonTemplate::SerializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::SerializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::SerializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::SerializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::SerializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::SerializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::SerializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::SerializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::SerializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::SerializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::SerializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::SerializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::SerializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::SerializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::SerializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::SerializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::SerializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::SerializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::SerializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::SerializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::SerializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::SerializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::SerializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::SerializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::SerializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::SerializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::SerializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::SerializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::SerializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::SerializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::SerializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::SerializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::SerializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::SerializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::SerializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::SerializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::SerializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::SerializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::SerializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::SerializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::SerializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::SerializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::SerializeValueMap(m52, #m52, v); \
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
		CarpJsonTemplate::DeserializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::DeserializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::DeserializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::DeserializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::DeserializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::DeserializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::DeserializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::DeserializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::DeserializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::DeserializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::DeserializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::DeserializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::DeserializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::DeserializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::DeserializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::DeserializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::DeserializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::DeserializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::DeserializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::DeserializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::DeserializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::DeserializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::DeserializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::DeserializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::DeserializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::DeserializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::DeserializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::DeserializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::DeserializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::DeserializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::DeserializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::DeserializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::DeserializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::DeserializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::DeserializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::DeserializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::DeserializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::DeserializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::DeserializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::DeserializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::DeserializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::DeserializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::DeserializeValueMap(m52, #m52, v); \
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
			s_list.push_back(#m10); \
			s_list.push_back(#m11); \
			s_list.push_back(#m12); \
			s_list.push_back(#m13); \
			s_list.push_back(#m14); \
			s_list.push_back(#m15); \
			s_list.push_back(#m16); \
			s_list.push_back(#m17); \
			s_list.push_back(#m18); \
			s_list.push_back(#m19); \
			s_list.push_back(#m20); \
			s_list.push_back(#m21); \
			s_list.push_back(#m22); \
			s_list.push_back(#m23); \
			s_list.push_back(#m24); \
			s_list.push_back(#m25); \
			s_list.push_back(#m26); \
			s_list.push_back(#m27); \
			s_list.push_back(#m28); \
			s_list.push_back(#m29); \
			s_list.push_back(#m30); \
			s_list.push_back(#m31); \
			s_list.push_back(#m32); \
			s_list.push_back(#m33); \
			s_list.push_back(#m34); \
			s_list.push_back(#m35); \
			s_list.push_back(#m36); \
			s_list.push_back(#m37); \
			s_list.push_back(#m38); \
			s_list.push_back(#m39); \
			s_list.push_back(#m40); \
			s_list.push_back(#m41); \
			s_list.push_back(#m42); \
			s_list.push_back(#m43); \
			s_list.push_back(#m44); \
			s_list.push_back(#m45); \
			s_list.push_back(#m46); \
			s_list.push_back(#m47); \
			s_list.push_back(#m48); \
			s_list.push_back(#m49); \
			s_list.push_back(#m50); \
			s_list.push_back(#m51); \
			s_list.push_back(#m52); \
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
			s_list.push_back(#t10); \
			s_list.push_back(#t11); \
			s_list.push_back(#t12); \
			s_list.push_back(#t13); \
			s_list.push_back(#t14); \
			s_list.push_back(#t15); \
			s_list.push_back(#t16); \
			s_list.push_back(#t17); \
			s_list.push_back(#t18); \
			s_list.push_back(#t19); \
			s_list.push_back(#t20); \
			s_list.push_back(#t21); \
			s_list.push_back(#t22); \
			s_list.push_back(#t23); \
			s_list.push_back(#t24); \
			s_list.push_back(#t25); \
			s_list.push_back(#t26); \
			s_list.push_back(#t27); \
			s_list.push_back(#t28); \
			s_list.push_back(#t29); \
			s_list.push_back(#t30); \
			s_list.push_back(#t31); \
			s_list.push_back(#t32); \
			s_list.push_back(#t33); \
			s_list.push_back(#t34); \
			s_list.push_back(#t35); \
			s_list.push_back(#t36); \
			s_list.push_back(#t37); \
			s_list.push_back(#t38); \
			s_list.push_back(#t39); \
			s_list.push_back(#t40); \
			s_list.push_back(#t41); \
			s_list.push_back(#t42); \
			s_list.push_back(#t43); \
			s_list.push_back(#t44); \
			s_list.push_back(#t45); \
			s_list.push_back(#t46); \
			s_list.push_back(#t47); \
			s_list.push_back(#t48); \
			s_list.push_back(#t49); \
			s_list.push_back(#t50); \
			s_list.push_back(#t51); \
			s_list.push_back(#t52); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8(), m9(), m10(), m11(), m12(), m13(), m14(), m15(), m16(), m17(), m18(), m19(), m20(), m21(), m22(), m23(), m24(), m25(), m26(), m27(), m28(), m29(), m30(), m31(), m32(), m33(), m34(), m35(), m36(), m37(), m38(), m39(), m40(), m41(), m42(), m43(), m44(), m45(), m46(), m47(), m48(), m49(), m50(), m51(), m52() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24; t25 m25; t26 m26; t27 m27; t28 m28; t29 m29; t30 m30; t31 m31; t32 m32; t33 m33; t34 m34; t35 m35; t36 m36; t37 m37; t38 m38; t39 m39; t40 m40; t41 m41; t42 m42; t43 m43; t44 m44; t45 m45; t46 m46; t47 m47; t48 m48; t49 m49; t50 m50; t51 m51; t52 m52; \
}
#define CARP_JSON_MACRO_109(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8, t9, m9, t10, m10, t11, m11, t12, m12, t13, m13, t14, m14, t15, m15, t16, m16, t17, m17, t18, m18, t19, m19, t20, m20, t21, m21, t22, m22, t23, m23, t24, m24, t25, m25, t26, m26, t27, m27, t28, m28, t29, m29, t30, m30, t31, m31, t32, m32, t33, m33, t34, m34, t35, m35, t36, m36, t37, m37, t38, m38, t39, m39, t40, m40, t41, m41, t42, m42, t43, m43, t44, m44, t45, m45, t46, m46, t47, m47, t48, m48, t49, m49, t50, m50, t51, m51, t52, m52, t53, m53) \
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
		CarpJsonTemplate::Serialize(m10, #m10, (int)sizeof(#m10)-1, v, a); \
		CarpJsonTemplate::Serialize(m11, #m11, (int)sizeof(#m11)-1, v, a); \
		CarpJsonTemplate::Serialize(m12, #m12, (int)sizeof(#m12)-1, v, a); \
		CarpJsonTemplate::Serialize(m13, #m13, (int)sizeof(#m13)-1, v, a); \
		CarpJsonTemplate::Serialize(m14, #m14, (int)sizeof(#m14)-1, v, a); \
		CarpJsonTemplate::Serialize(m15, #m15, (int)sizeof(#m15)-1, v, a); \
		CarpJsonTemplate::Serialize(m16, #m16, (int)sizeof(#m16)-1, v, a); \
		CarpJsonTemplate::Serialize(m17, #m17, (int)sizeof(#m17)-1, v, a); \
		CarpJsonTemplate::Serialize(m18, #m18, (int)sizeof(#m18)-1, v, a); \
		CarpJsonTemplate::Serialize(m19, #m19, (int)sizeof(#m19)-1, v, a); \
		CarpJsonTemplate::Serialize(m20, #m20, (int)sizeof(#m20)-1, v, a); \
		CarpJsonTemplate::Serialize(m21, #m21, (int)sizeof(#m21)-1, v, a); \
		CarpJsonTemplate::Serialize(m22, #m22, (int)sizeof(#m22)-1, v, a); \
		CarpJsonTemplate::Serialize(m23, #m23, (int)sizeof(#m23)-1, v, a); \
		CarpJsonTemplate::Serialize(m24, #m24, (int)sizeof(#m24)-1, v, a); \
		CarpJsonTemplate::Serialize(m25, #m25, (int)sizeof(#m25)-1, v, a); \
		CarpJsonTemplate::Serialize(m26, #m26, (int)sizeof(#m26)-1, v, a); \
		CarpJsonTemplate::Serialize(m27, #m27, (int)sizeof(#m27)-1, v, a); \
		CarpJsonTemplate::Serialize(m28, #m28, (int)sizeof(#m28)-1, v, a); \
		CarpJsonTemplate::Serialize(m29, #m29, (int)sizeof(#m29)-1, v, a); \
		CarpJsonTemplate::Serialize(m30, #m30, (int)sizeof(#m30)-1, v, a); \
		CarpJsonTemplate::Serialize(m31, #m31, (int)sizeof(#m31)-1, v, a); \
		CarpJsonTemplate::Serialize(m32, #m32, (int)sizeof(#m32)-1, v, a); \
		CarpJsonTemplate::Serialize(m33, #m33, (int)sizeof(#m33)-1, v, a); \
		CarpJsonTemplate::Serialize(m34, #m34, (int)sizeof(#m34)-1, v, a); \
		CarpJsonTemplate::Serialize(m35, #m35, (int)sizeof(#m35)-1, v, a); \
		CarpJsonTemplate::Serialize(m36, #m36, (int)sizeof(#m36)-1, v, a); \
		CarpJsonTemplate::Serialize(m37, #m37, (int)sizeof(#m37)-1, v, a); \
		CarpJsonTemplate::Serialize(m38, #m38, (int)sizeof(#m38)-1, v, a); \
		CarpJsonTemplate::Serialize(m39, #m39, (int)sizeof(#m39)-1, v, a); \
		CarpJsonTemplate::Serialize(m40, #m40, (int)sizeof(#m40)-1, v, a); \
		CarpJsonTemplate::Serialize(m41, #m41, (int)sizeof(#m41)-1, v, a); \
		CarpJsonTemplate::Serialize(m42, #m42, (int)sizeof(#m42)-1, v, a); \
		CarpJsonTemplate::Serialize(m43, #m43, (int)sizeof(#m43)-1, v, a); \
		CarpJsonTemplate::Serialize(m44, #m44, (int)sizeof(#m44)-1, v, a); \
		CarpJsonTemplate::Serialize(m45, #m45, (int)sizeof(#m45)-1, v, a); \
		CarpJsonTemplate::Serialize(m46, #m46, (int)sizeof(#m46)-1, v, a); \
		CarpJsonTemplate::Serialize(m47, #m47, (int)sizeof(#m47)-1, v, a); \
		CarpJsonTemplate::Serialize(m48, #m48, (int)sizeof(#m48)-1, v, a); \
		CarpJsonTemplate::Serialize(m49, #m49, (int)sizeof(#m49)-1, v, a); \
		CarpJsonTemplate::Serialize(m50, #m50, (int)sizeof(#m50)-1, v, a); \
		CarpJsonTemplate::Serialize(m51, #m51, (int)sizeof(#m51)-1, v, a); \
		CarpJsonTemplate::Serialize(m52, #m52, (int)sizeof(#m52)-1, v, a); \
		CarpJsonTemplate::Serialize(m53, #m53, (int)sizeof(#m53)-1, v, a); \
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
		CarpJsonTemplate::Deserialize(m10, #m10, (int)sizeof(#m10)-1, v); \
		CarpJsonTemplate::Deserialize(m11, #m11, (int)sizeof(#m11)-1, v); \
		CarpJsonTemplate::Deserialize(m12, #m12, (int)sizeof(#m12)-1, v); \
		CarpJsonTemplate::Deserialize(m13, #m13, (int)sizeof(#m13)-1, v); \
		CarpJsonTemplate::Deserialize(m14, #m14, (int)sizeof(#m14)-1, v); \
		CarpJsonTemplate::Deserialize(m15, #m15, (int)sizeof(#m15)-1, v); \
		CarpJsonTemplate::Deserialize(m16, #m16, (int)sizeof(#m16)-1, v); \
		CarpJsonTemplate::Deserialize(m17, #m17, (int)sizeof(#m17)-1, v); \
		CarpJsonTemplate::Deserialize(m18, #m18, (int)sizeof(#m18)-1, v); \
		CarpJsonTemplate::Deserialize(m19, #m19, (int)sizeof(#m19)-1, v); \
		CarpJsonTemplate::Deserialize(m20, #m20, (int)sizeof(#m20)-1, v); \
		CarpJsonTemplate::Deserialize(m21, #m21, (int)sizeof(#m21)-1, v); \
		CarpJsonTemplate::Deserialize(m22, #m22, (int)sizeof(#m22)-1, v); \
		CarpJsonTemplate::Deserialize(m23, #m23, (int)sizeof(#m23)-1, v); \
		CarpJsonTemplate::Deserialize(m24, #m24, (int)sizeof(#m24)-1, v); \
		CarpJsonTemplate::Deserialize(m25, #m25, (int)sizeof(#m25)-1, v); \
		CarpJsonTemplate::Deserialize(m26, #m26, (int)sizeof(#m26)-1, v); \
		CarpJsonTemplate::Deserialize(m27, #m27, (int)sizeof(#m27)-1, v); \
		CarpJsonTemplate::Deserialize(m28, #m28, (int)sizeof(#m28)-1, v); \
		CarpJsonTemplate::Deserialize(m29, #m29, (int)sizeof(#m29)-1, v); \
		CarpJsonTemplate::Deserialize(m30, #m30, (int)sizeof(#m30)-1, v); \
		CarpJsonTemplate::Deserialize(m31, #m31, (int)sizeof(#m31)-1, v); \
		CarpJsonTemplate::Deserialize(m32, #m32, (int)sizeof(#m32)-1, v); \
		CarpJsonTemplate::Deserialize(m33, #m33, (int)sizeof(#m33)-1, v); \
		CarpJsonTemplate::Deserialize(m34, #m34, (int)sizeof(#m34)-1, v); \
		CarpJsonTemplate::Deserialize(m35, #m35, (int)sizeof(#m35)-1, v); \
		CarpJsonTemplate::Deserialize(m36, #m36, (int)sizeof(#m36)-1, v); \
		CarpJsonTemplate::Deserialize(m37, #m37, (int)sizeof(#m37)-1, v); \
		CarpJsonTemplate::Deserialize(m38, #m38, (int)sizeof(#m38)-1, v); \
		CarpJsonTemplate::Deserialize(m39, #m39, (int)sizeof(#m39)-1, v); \
		CarpJsonTemplate::Deserialize(m40, #m40, (int)sizeof(#m40)-1, v); \
		CarpJsonTemplate::Deserialize(m41, #m41, (int)sizeof(#m41)-1, v); \
		CarpJsonTemplate::Deserialize(m42, #m42, (int)sizeof(#m42)-1, v); \
		CarpJsonTemplate::Deserialize(m43, #m43, (int)sizeof(#m43)-1, v); \
		CarpJsonTemplate::Deserialize(m44, #m44, (int)sizeof(#m44)-1, v); \
		CarpJsonTemplate::Deserialize(m45, #m45, (int)sizeof(#m45)-1, v); \
		CarpJsonTemplate::Deserialize(m46, #m46, (int)sizeof(#m46)-1, v); \
		CarpJsonTemplate::Deserialize(m47, #m47, (int)sizeof(#m47)-1, v); \
		CarpJsonTemplate::Deserialize(m48, #m48, (int)sizeof(#m48)-1, v); \
		CarpJsonTemplate::Deserialize(m49, #m49, (int)sizeof(#m49)-1, v); \
		CarpJsonTemplate::Deserialize(m50, #m50, (int)sizeof(#m50)-1, v); \
		CarpJsonTemplate::Deserialize(m51, #m51, (int)sizeof(#m51)-1, v); \
		CarpJsonTemplate::Deserialize(m52, #m52, (int)sizeof(#m52)-1, v); \
		CarpJsonTemplate::Deserialize(m53, #m53, (int)sizeof(#m53)-1, v); \
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
		CarpJsonTemplate::SerializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::SerializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::SerializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::SerializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::SerializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::SerializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::SerializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::SerializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::SerializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::SerializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::SerializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::SerializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::SerializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::SerializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::SerializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::SerializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::SerializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::SerializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::SerializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::SerializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::SerializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::SerializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::SerializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::SerializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::SerializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::SerializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::SerializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::SerializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::SerializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::SerializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::SerializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::SerializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::SerializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::SerializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::SerializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::SerializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::SerializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::SerializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::SerializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::SerializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::SerializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::SerializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::SerializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::SerializeValueMap(m53, #m53, v); \
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
		CarpJsonTemplate::DeserializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::DeserializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::DeserializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::DeserializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::DeserializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::DeserializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::DeserializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::DeserializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::DeserializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::DeserializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::DeserializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::DeserializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::DeserializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::DeserializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::DeserializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::DeserializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::DeserializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::DeserializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::DeserializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::DeserializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::DeserializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::DeserializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::DeserializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::DeserializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::DeserializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::DeserializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::DeserializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::DeserializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::DeserializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::DeserializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::DeserializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::DeserializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::DeserializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::DeserializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::DeserializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::DeserializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::DeserializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::DeserializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::DeserializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::DeserializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::DeserializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::DeserializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::DeserializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::DeserializeValueMap(m53, #m53, v); \
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
			s_list.push_back(#m10); \
			s_list.push_back(#m11); \
			s_list.push_back(#m12); \
			s_list.push_back(#m13); \
			s_list.push_back(#m14); \
			s_list.push_back(#m15); \
			s_list.push_back(#m16); \
			s_list.push_back(#m17); \
			s_list.push_back(#m18); \
			s_list.push_back(#m19); \
			s_list.push_back(#m20); \
			s_list.push_back(#m21); \
			s_list.push_back(#m22); \
			s_list.push_back(#m23); \
			s_list.push_back(#m24); \
			s_list.push_back(#m25); \
			s_list.push_back(#m26); \
			s_list.push_back(#m27); \
			s_list.push_back(#m28); \
			s_list.push_back(#m29); \
			s_list.push_back(#m30); \
			s_list.push_back(#m31); \
			s_list.push_back(#m32); \
			s_list.push_back(#m33); \
			s_list.push_back(#m34); \
			s_list.push_back(#m35); \
			s_list.push_back(#m36); \
			s_list.push_back(#m37); \
			s_list.push_back(#m38); \
			s_list.push_back(#m39); \
			s_list.push_back(#m40); \
			s_list.push_back(#m41); \
			s_list.push_back(#m42); \
			s_list.push_back(#m43); \
			s_list.push_back(#m44); \
			s_list.push_back(#m45); \
			s_list.push_back(#m46); \
			s_list.push_back(#m47); \
			s_list.push_back(#m48); \
			s_list.push_back(#m49); \
			s_list.push_back(#m50); \
			s_list.push_back(#m51); \
			s_list.push_back(#m52); \
			s_list.push_back(#m53); \
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
			s_list.push_back(#t10); \
			s_list.push_back(#t11); \
			s_list.push_back(#t12); \
			s_list.push_back(#t13); \
			s_list.push_back(#t14); \
			s_list.push_back(#t15); \
			s_list.push_back(#t16); \
			s_list.push_back(#t17); \
			s_list.push_back(#t18); \
			s_list.push_back(#t19); \
			s_list.push_back(#t20); \
			s_list.push_back(#t21); \
			s_list.push_back(#t22); \
			s_list.push_back(#t23); \
			s_list.push_back(#t24); \
			s_list.push_back(#t25); \
			s_list.push_back(#t26); \
			s_list.push_back(#t27); \
			s_list.push_back(#t28); \
			s_list.push_back(#t29); \
			s_list.push_back(#t30); \
			s_list.push_back(#t31); \
			s_list.push_back(#t32); \
			s_list.push_back(#t33); \
			s_list.push_back(#t34); \
			s_list.push_back(#t35); \
			s_list.push_back(#t36); \
			s_list.push_back(#t37); \
			s_list.push_back(#t38); \
			s_list.push_back(#t39); \
			s_list.push_back(#t40); \
			s_list.push_back(#t41); \
			s_list.push_back(#t42); \
			s_list.push_back(#t43); \
			s_list.push_back(#t44); \
			s_list.push_back(#t45); \
			s_list.push_back(#t46); \
			s_list.push_back(#t47); \
			s_list.push_back(#t48); \
			s_list.push_back(#t49); \
			s_list.push_back(#t50); \
			s_list.push_back(#t51); \
			s_list.push_back(#t52); \
			s_list.push_back(#t53); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8(), m9(), m10(), m11(), m12(), m13(), m14(), m15(), m16(), m17(), m18(), m19(), m20(), m21(), m22(), m23(), m24(), m25(), m26(), m27(), m28(), m29(), m30(), m31(), m32(), m33(), m34(), m35(), m36(), m37(), m38(), m39(), m40(), m41(), m42(), m43(), m44(), m45(), m46(), m47(), m48(), m49(), m50(), m51(), m52(), m53() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24; t25 m25; t26 m26; t27 m27; t28 m28; t29 m29; t30 m30; t31 m31; t32 m32; t33 m33; t34 m34; t35 m35; t36 m36; t37 m37; t38 m38; t39 m39; t40 m40; t41 m41; t42 m42; t43 m43; t44 m44; t45 m45; t46 m46; t47 m47; t48 m48; t49 m49; t50 m50; t51 m51; t52 m52; t53 m53; \
}
#define CARP_JSON_MACRO_111(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8, t9, m9, t10, m10, t11, m11, t12, m12, t13, m13, t14, m14, t15, m15, t16, m16, t17, m17, t18, m18, t19, m19, t20, m20, t21, m21, t22, m22, t23, m23, t24, m24, t25, m25, t26, m26, t27, m27, t28, m28, t29, m29, t30, m30, t31, m31, t32, m32, t33, m33, t34, m34, t35, m35, t36, m36, t37, m37, t38, m38, t39, m39, t40, m40, t41, m41, t42, m42, t43, m43, t44, m44, t45, m45, t46, m46, t47, m47, t48, m48, t49, m49, t50, m50, t51, m51, t52, m52, t53, m53, t54, m54) \
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
		CarpJsonTemplate::Serialize(m10, #m10, (int)sizeof(#m10)-1, v, a); \
		CarpJsonTemplate::Serialize(m11, #m11, (int)sizeof(#m11)-1, v, a); \
		CarpJsonTemplate::Serialize(m12, #m12, (int)sizeof(#m12)-1, v, a); \
		CarpJsonTemplate::Serialize(m13, #m13, (int)sizeof(#m13)-1, v, a); \
		CarpJsonTemplate::Serialize(m14, #m14, (int)sizeof(#m14)-1, v, a); \
		CarpJsonTemplate::Serialize(m15, #m15, (int)sizeof(#m15)-1, v, a); \
		CarpJsonTemplate::Serialize(m16, #m16, (int)sizeof(#m16)-1, v, a); \
		CarpJsonTemplate::Serialize(m17, #m17, (int)sizeof(#m17)-1, v, a); \
		CarpJsonTemplate::Serialize(m18, #m18, (int)sizeof(#m18)-1, v, a); \
		CarpJsonTemplate::Serialize(m19, #m19, (int)sizeof(#m19)-1, v, a); \
		CarpJsonTemplate::Serialize(m20, #m20, (int)sizeof(#m20)-1, v, a); \
		CarpJsonTemplate::Serialize(m21, #m21, (int)sizeof(#m21)-1, v, a); \
		CarpJsonTemplate::Serialize(m22, #m22, (int)sizeof(#m22)-1, v, a); \
		CarpJsonTemplate::Serialize(m23, #m23, (int)sizeof(#m23)-1, v, a); \
		CarpJsonTemplate::Serialize(m24, #m24, (int)sizeof(#m24)-1, v, a); \
		CarpJsonTemplate::Serialize(m25, #m25, (int)sizeof(#m25)-1, v, a); \
		CarpJsonTemplate::Serialize(m26, #m26, (int)sizeof(#m26)-1, v, a); \
		CarpJsonTemplate::Serialize(m27, #m27, (int)sizeof(#m27)-1, v, a); \
		CarpJsonTemplate::Serialize(m28, #m28, (int)sizeof(#m28)-1, v, a); \
		CarpJsonTemplate::Serialize(m29, #m29, (int)sizeof(#m29)-1, v, a); \
		CarpJsonTemplate::Serialize(m30, #m30, (int)sizeof(#m30)-1, v, a); \
		CarpJsonTemplate::Serialize(m31, #m31, (int)sizeof(#m31)-1, v, a); \
		CarpJsonTemplate::Serialize(m32, #m32, (int)sizeof(#m32)-1, v, a); \
		CarpJsonTemplate::Serialize(m33, #m33, (int)sizeof(#m33)-1, v, a); \
		CarpJsonTemplate::Serialize(m34, #m34, (int)sizeof(#m34)-1, v, a); \
		CarpJsonTemplate::Serialize(m35, #m35, (int)sizeof(#m35)-1, v, a); \
		CarpJsonTemplate::Serialize(m36, #m36, (int)sizeof(#m36)-1, v, a); \
		CarpJsonTemplate::Serialize(m37, #m37, (int)sizeof(#m37)-1, v, a); \
		CarpJsonTemplate::Serialize(m38, #m38, (int)sizeof(#m38)-1, v, a); \
		CarpJsonTemplate::Serialize(m39, #m39, (int)sizeof(#m39)-1, v, a); \
		CarpJsonTemplate::Serialize(m40, #m40, (int)sizeof(#m40)-1, v, a); \
		CarpJsonTemplate::Serialize(m41, #m41, (int)sizeof(#m41)-1, v, a); \
		CarpJsonTemplate::Serialize(m42, #m42, (int)sizeof(#m42)-1, v, a); \
		CarpJsonTemplate::Serialize(m43, #m43, (int)sizeof(#m43)-1, v, a); \
		CarpJsonTemplate::Serialize(m44, #m44, (int)sizeof(#m44)-1, v, a); \
		CarpJsonTemplate::Serialize(m45, #m45, (int)sizeof(#m45)-1, v, a); \
		CarpJsonTemplate::Serialize(m46, #m46, (int)sizeof(#m46)-1, v, a); \
		CarpJsonTemplate::Serialize(m47, #m47, (int)sizeof(#m47)-1, v, a); \
		CarpJsonTemplate::Serialize(m48, #m48, (int)sizeof(#m48)-1, v, a); \
		CarpJsonTemplate::Serialize(m49, #m49, (int)sizeof(#m49)-1, v, a); \
		CarpJsonTemplate::Serialize(m50, #m50, (int)sizeof(#m50)-1, v, a); \
		CarpJsonTemplate::Serialize(m51, #m51, (int)sizeof(#m51)-1, v, a); \
		CarpJsonTemplate::Serialize(m52, #m52, (int)sizeof(#m52)-1, v, a); \
		CarpJsonTemplate::Serialize(m53, #m53, (int)sizeof(#m53)-1, v, a); \
		CarpJsonTemplate::Serialize(m54, #m54, (int)sizeof(#m54)-1, v, a); \
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
		CarpJsonTemplate::Deserialize(m10, #m10, (int)sizeof(#m10)-1, v); \
		CarpJsonTemplate::Deserialize(m11, #m11, (int)sizeof(#m11)-1, v); \
		CarpJsonTemplate::Deserialize(m12, #m12, (int)sizeof(#m12)-1, v); \
		CarpJsonTemplate::Deserialize(m13, #m13, (int)sizeof(#m13)-1, v); \
		CarpJsonTemplate::Deserialize(m14, #m14, (int)sizeof(#m14)-1, v); \
		CarpJsonTemplate::Deserialize(m15, #m15, (int)sizeof(#m15)-1, v); \
		CarpJsonTemplate::Deserialize(m16, #m16, (int)sizeof(#m16)-1, v); \
		CarpJsonTemplate::Deserialize(m17, #m17, (int)sizeof(#m17)-1, v); \
		CarpJsonTemplate::Deserialize(m18, #m18, (int)sizeof(#m18)-1, v); \
		CarpJsonTemplate::Deserialize(m19, #m19, (int)sizeof(#m19)-1, v); \
		CarpJsonTemplate::Deserialize(m20, #m20, (int)sizeof(#m20)-1, v); \
		CarpJsonTemplate::Deserialize(m21, #m21, (int)sizeof(#m21)-1, v); \
		CarpJsonTemplate::Deserialize(m22, #m22, (int)sizeof(#m22)-1, v); \
		CarpJsonTemplate::Deserialize(m23, #m23, (int)sizeof(#m23)-1, v); \
		CarpJsonTemplate::Deserialize(m24, #m24, (int)sizeof(#m24)-1, v); \
		CarpJsonTemplate::Deserialize(m25, #m25, (int)sizeof(#m25)-1, v); \
		CarpJsonTemplate::Deserialize(m26, #m26, (int)sizeof(#m26)-1, v); \
		CarpJsonTemplate::Deserialize(m27, #m27, (int)sizeof(#m27)-1, v); \
		CarpJsonTemplate::Deserialize(m28, #m28, (int)sizeof(#m28)-1, v); \
		CarpJsonTemplate::Deserialize(m29, #m29, (int)sizeof(#m29)-1, v); \
		CarpJsonTemplate::Deserialize(m30, #m30, (int)sizeof(#m30)-1, v); \
		CarpJsonTemplate::Deserialize(m31, #m31, (int)sizeof(#m31)-1, v); \
		CarpJsonTemplate::Deserialize(m32, #m32, (int)sizeof(#m32)-1, v); \
		CarpJsonTemplate::Deserialize(m33, #m33, (int)sizeof(#m33)-1, v); \
		CarpJsonTemplate::Deserialize(m34, #m34, (int)sizeof(#m34)-1, v); \
		CarpJsonTemplate::Deserialize(m35, #m35, (int)sizeof(#m35)-1, v); \
		CarpJsonTemplate::Deserialize(m36, #m36, (int)sizeof(#m36)-1, v); \
		CarpJsonTemplate::Deserialize(m37, #m37, (int)sizeof(#m37)-1, v); \
		CarpJsonTemplate::Deserialize(m38, #m38, (int)sizeof(#m38)-1, v); \
		CarpJsonTemplate::Deserialize(m39, #m39, (int)sizeof(#m39)-1, v); \
		CarpJsonTemplate::Deserialize(m40, #m40, (int)sizeof(#m40)-1, v); \
		CarpJsonTemplate::Deserialize(m41, #m41, (int)sizeof(#m41)-1, v); \
		CarpJsonTemplate::Deserialize(m42, #m42, (int)sizeof(#m42)-1, v); \
		CarpJsonTemplate::Deserialize(m43, #m43, (int)sizeof(#m43)-1, v); \
		CarpJsonTemplate::Deserialize(m44, #m44, (int)sizeof(#m44)-1, v); \
		CarpJsonTemplate::Deserialize(m45, #m45, (int)sizeof(#m45)-1, v); \
		CarpJsonTemplate::Deserialize(m46, #m46, (int)sizeof(#m46)-1, v); \
		CarpJsonTemplate::Deserialize(m47, #m47, (int)sizeof(#m47)-1, v); \
		CarpJsonTemplate::Deserialize(m48, #m48, (int)sizeof(#m48)-1, v); \
		CarpJsonTemplate::Deserialize(m49, #m49, (int)sizeof(#m49)-1, v); \
		CarpJsonTemplate::Deserialize(m50, #m50, (int)sizeof(#m50)-1, v); \
		CarpJsonTemplate::Deserialize(m51, #m51, (int)sizeof(#m51)-1, v); \
		CarpJsonTemplate::Deserialize(m52, #m52, (int)sizeof(#m52)-1, v); \
		CarpJsonTemplate::Deserialize(m53, #m53, (int)sizeof(#m53)-1, v); \
		CarpJsonTemplate::Deserialize(m54, #m54, (int)sizeof(#m54)-1, v); \
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
		CarpJsonTemplate::SerializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::SerializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::SerializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::SerializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::SerializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::SerializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::SerializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::SerializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::SerializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::SerializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::SerializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::SerializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::SerializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::SerializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::SerializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::SerializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::SerializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::SerializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::SerializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::SerializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::SerializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::SerializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::SerializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::SerializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::SerializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::SerializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::SerializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::SerializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::SerializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::SerializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::SerializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::SerializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::SerializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::SerializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::SerializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::SerializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::SerializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::SerializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::SerializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::SerializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::SerializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::SerializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::SerializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::SerializeValueMap(m53, #m53, v); \
		CarpJsonTemplate::SerializeValueMap(m54, #m54, v); \
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
		CarpJsonTemplate::DeserializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::DeserializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::DeserializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::DeserializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::DeserializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::DeserializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::DeserializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::DeserializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::DeserializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::DeserializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::DeserializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::DeserializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::DeserializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::DeserializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::DeserializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::DeserializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::DeserializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::DeserializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::DeserializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::DeserializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::DeserializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::DeserializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::DeserializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::DeserializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::DeserializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::DeserializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::DeserializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::DeserializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::DeserializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::DeserializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::DeserializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::DeserializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::DeserializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::DeserializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::DeserializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::DeserializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::DeserializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::DeserializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::DeserializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::DeserializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::DeserializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::DeserializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::DeserializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::DeserializeValueMap(m53, #m53, v); \
		CarpJsonTemplate::DeserializeValueMap(m54, #m54, v); \
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
			s_list.push_back(#m10); \
			s_list.push_back(#m11); \
			s_list.push_back(#m12); \
			s_list.push_back(#m13); \
			s_list.push_back(#m14); \
			s_list.push_back(#m15); \
			s_list.push_back(#m16); \
			s_list.push_back(#m17); \
			s_list.push_back(#m18); \
			s_list.push_back(#m19); \
			s_list.push_back(#m20); \
			s_list.push_back(#m21); \
			s_list.push_back(#m22); \
			s_list.push_back(#m23); \
			s_list.push_back(#m24); \
			s_list.push_back(#m25); \
			s_list.push_back(#m26); \
			s_list.push_back(#m27); \
			s_list.push_back(#m28); \
			s_list.push_back(#m29); \
			s_list.push_back(#m30); \
			s_list.push_back(#m31); \
			s_list.push_back(#m32); \
			s_list.push_back(#m33); \
			s_list.push_back(#m34); \
			s_list.push_back(#m35); \
			s_list.push_back(#m36); \
			s_list.push_back(#m37); \
			s_list.push_back(#m38); \
			s_list.push_back(#m39); \
			s_list.push_back(#m40); \
			s_list.push_back(#m41); \
			s_list.push_back(#m42); \
			s_list.push_back(#m43); \
			s_list.push_back(#m44); \
			s_list.push_back(#m45); \
			s_list.push_back(#m46); \
			s_list.push_back(#m47); \
			s_list.push_back(#m48); \
			s_list.push_back(#m49); \
			s_list.push_back(#m50); \
			s_list.push_back(#m51); \
			s_list.push_back(#m52); \
			s_list.push_back(#m53); \
			s_list.push_back(#m54); \
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
			s_list.push_back(#t10); \
			s_list.push_back(#t11); \
			s_list.push_back(#t12); \
			s_list.push_back(#t13); \
			s_list.push_back(#t14); \
			s_list.push_back(#t15); \
			s_list.push_back(#t16); \
			s_list.push_back(#t17); \
			s_list.push_back(#t18); \
			s_list.push_back(#t19); \
			s_list.push_back(#t20); \
			s_list.push_back(#t21); \
			s_list.push_back(#t22); \
			s_list.push_back(#t23); \
			s_list.push_back(#t24); \
			s_list.push_back(#t25); \
			s_list.push_back(#t26); \
			s_list.push_back(#t27); \
			s_list.push_back(#t28); \
			s_list.push_back(#t29); \
			s_list.push_back(#t30); \
			s_list.push_back(#t31); \
			s_list.push_back(#t32); \
			s_list.push_back(#t33); \
			s_list.push_back(#t34); \
			s_list.push_back(#t35); \
			s_list.push_back(#t36); \
			s_list.push_back(#t37); \
			s_list.push_back(#t38); \
			s_list.push_back(#t39); \
			s_list.push_back(#t40); \
			s_list.push_back(#t41); \
			s_list.push_back(#t42); \
			s_list.push_back(#t43); \
			s_list.push_back(#t44); \
			s_list.push_back(#t45); \
			s_list.push_back(#t46); \
			s_list.push_back(#t47); \
			s_list.push_back(#t48); \
			s_list.push_back(#t49); \
			s_list.push_back(#t50); \
			s_list.push_back(#t51); \
			s_list.push_back(#t52); \
			s_list.push_back(#t53); \
			s_list.push_back(#t54); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8(), m9(), m10(), m11(), m12(), m13(), m14(), m15(), m16(), m17(), m18(), m19(), m20(), m21(), m22(), m23(), m24(), m25(), m26(), m27(), m28(), m29(), m30(), m31(), m32(), m33(), m34(), m35(), m36(), m37(), m38(), m39(), m40(), m41(), m42(), m43(), m44(), m45(), m46(), m47(), m48(), m49(), m50(), m51(), m52(), m53(), m54() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24; t25 m25; t26 m26; t27 m27; t28 m28; t29 m29; t30 m30; t31 m31; t32 m32; t33 m33; t34 m34; t35 m35; t36 m36; t37 m37; t38 m38; t39 m39; t40 m40; t41 m41; t42 m42; t43 m43; t44 m44; t45 m45; t46 m46; t47 m47; t48 m48; t49 m49; t50 m50; t51 m51; t52 m52; t53 m53; t54 m54; \
}
#define CARP_JSON_MACRO_113(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8, t9, m9, t10, m10, t11, m11, t12, m12, t13, m13, t14, m14, t15, m15, t16, m16, t17, m17, t18, m18, t19, m19, t20, m20, t21, m21, t22, m22, t23, m23, t24, m24, t25, m25, t26, m26, t27, m27, t28, m28, t29, m29, t30, m30, t31, m31, t32, m32, t33, m33, t34, m34, t35, m35, t36, m36, t37, m37, t38, m38, t39, m39, t40, m40, t41, m41, t42, m42, t43, m43, t44, m44, t45, m45, t46, m46, t47, m47, t48, m48, t49, m49, t50, m50, t51, m51, t52, m52, t53, m53, t54, m54, t55, m55) \
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
		CarpJsonTemplate::Serialize(m10, #m10, (int)sizeof(#m10)-1, v, a); \
		CarpJsonTemplate::Serialize(m11, #m11, (int)sizeof(#m11)-1, v, a); \
		CarpJsonTemplate::Serialize(m12, #m12, (int)sizeof(#m12)-1, v, a); \
		CarpJsonTemplate::Serialize(m13, #m13, (int)sizeof(#m13)-1, v, a); \
		CarpJsonTemplate::Serialize(m14, #m14, (int)sizeof(#m14)-1, v, a); \
		CarpJsonTemplate::Serialize(m15, #m15, (int)sizeof(#m15)-1, v, a); \
		CarpJsonTemplate::Serialize(m16, #m16, (int)sizeof(#m16)-1, v, a); \
		CarpJsonTemplate::Serialize(m17, #m17, (int)sizeof(#m17)-1, v, a); \
		CarpJsonTemplate::Serialize(m18, #m18, (int)sizeof(#m18)-1, v, a); \
		CarpJsonTemplate::Serialize(m19, #m19, (int)sizeof(#m19)-1, v, a); \
		CarpJsonTemplate::Serialize(m20, #m20, (int)sizeof(#m20)-1, v, a); \
		CarpJsonTemplate::Serialize(m21, #m21, (int)sizeof(#m21)-1, v, a); \
		CarpJsonTemplate::Serialize(m22, #m22, (int)sizeof(#m22)-1, v, a); \
		CarpJsonTemplate::Serialize(m23, #m23, (int)sizeof(#m23)-1, v, a); \
		CarpJsonTemplate::Serialize(m24, #m24, (int)sizeof(#m24)-1, v, a); \
		CarpJsonTemplate::Serialize(m25, #m25, (int)sizeof(#m25)-1, v, a); \
		CarpJsonTemplate::Serialize(m26, #m26, (int)sizeof(#m26)-1, v, a); \
		CarpJsonTemplate::Serialize(m27, #m27, (int)sizeof(#m27)-1, v, a); \
		CarpJsonTemplate::Serialize(m28, #m28, (int)sizeof(#m28)-1, v, a); \
		CarpJsonTemplate::Serialize(m29, #m29, (int)sizeof(#m29)-1, v, a); \
		CarpJsonTemplate::Serialize(m30, #m30, (int)sizeof(#m30)-1, v, a); \
		CarpJsonTemplate::Serialize(m31, #m31, (int)sizeof(#m31)-1, v, a); \
		CarpJsonTemplate::Serialize(m32, #m32, (int)sizeof(#m32)-1, v, a); \
		CarpJsonTemplate::Serialize(m33, #m33, (int)sizeof(#m33)-1, v, a); \
		CarpJsonTemplate::Serialize(m34, #m34, (int)sizeof(#m34)-1, v, a); \
		CarpJsonTemplate::Serialize(m35, #m35, (int)sizeof(#m35)-1, v, a); \
		CarpJsonTemplate::Serialize(m36, #m36, (int)sizeof(#m36)-1, v, a); \
		CarpJsonTemplate::Serialize(m37, #m37, (int)sizeof(#m37)-1, v, a); \
		CarpJsonTemplate::Serialize(m38, #m38, (int)sizeof(#m38)-1, v, a); \
		CarpJsonTemplate::Serialize(m39, #m39, (int)sizeof(#m39)-1, v, a); \
		CarpJsonTemplate::Serialize(m40, #m40, (int)sizeof(#m40)-1, v, a); \
		CarpJsonTemplate::Serialize(m41, #m41, (int)sizeof(#m41)-1, v, a); \
		CarpJsonTemplate::Serialize(m42, #m42, (int)sizeof(#m42)-1, v, a); \
		CarpJsonTemplate::Serialize(m43, #m43, (int)sizeof(#m43)-1, v, a); \
		CarpJsonTemplate::Serialize(m44, #m44, (int)sizeof(#m44)-1, v, a); \
		CarpJsonTemplate::Serialize(m45, #m45, (int)sizeof(#m45)-1, v, a); \
		CarpJsonTemplate::Serialize(m46, #m46, (int)sizeof(#m46)-1, v, a); \
		CarpJsonTemplate::Serialize(m47, #m47, (int)sizeof(#m47)-1, v, a); \
		CarpJsonTemplate::Serialize(m48, #m48, (int)sizeof(#m48)-1, v, a); \
		CarpJsonTemplate::Serialize(m49, #m49, (int)sizeof(#m49)-1, v, a); \
		CarpJsonTemplate::Serialize(m50, #m50, (int)sizeof(#m50)-1, v, a); \
		CarpJsonTemplate::Serialize(m51, #m51, (int)sizeof(#m51)-1, v, a); \
		CarpJsonTemplate::Serialize(m52, #m52, (int)sizeof(#m52)-1, v, a); \
		CarpJsonTemplate::Serialize(m53, #m53, (int)sizeof(#m53)-1, v, a); \
		CarpJsonTemplate::Serialize(m54, #m54, (int)sizeof(#m54)-1, v, a); \
		CarpJsonTemplate::Serialize(m55, #m55, (int)sizeof(#m55)-1, v, a); \
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
		CarpJsonTemplate::Deserialize(m10, #m10, (int)sizeof(#m10)-1, v); \
		CarpJsonTemplate::Deserialize(m11, #m11, (int)sizeof(#m11)-1, v); \
		CarpJsonTemplate::Deserialize(m12, #m12, (int)sizeof(#m12)-1, v); \
		CarpJsonTemplate::Deserialize(m13, #m13, (int)sizeof(#m13)-1, v); \
		CarpJsonTemplate::Deserialize(m14, #m14, (int)sizeof(#m14)-1, v); \
		CarpJsonTemplate::Deserialize(m15, #m15, (int)sizeof(#m15)-1, v); \
		CarpJsonTemplate::Deserialize(m16, #m16, (int)sizeof(#m16)-1, v); \
		CarpJsonTemplate::Deserialize(m17, #m17, (int)sizeof(#m17)-1, v); \
		CarpJsonTemplate::Deserialize(m18, #m18, (int)sizeof(#m18)-1, v); \
		CarpJsonTemplate::Deserialize(m19, #m19, (int)sizeof(#m19)-1, v); \
		CarpJsonTemplate::Deserialize(m20, #m20, (int)sizeof(#m20)-1, v); \
		CarpJsonTemplate::Deserialize(m21, #m21, (int)sizeof(#m21)-1, v); \
		CarpJsonTemplate::Deserialize(m22, #m22, (int)sizeof(#m22)-1, v); \
		CarpJsonTemplate::Deserialize(m23, #m23, (int)sizeof(#m23)-1, v); \
		CarpJsonTemplate::Deserialize(m24, #m24, (int)sizeof(#m24)-1, v); \
		CarpJsonTemplate::Deserialize(m25, #m25, (int)sizeof(#m25)-1, v); \
		CarpJsonTemplate::Deserialize(m26, #m26, (int)sizeof(#m26)-1, v); \
		CarpJsonTemplate::Deserialize(m27, #m27, (int)sizeof(#m27)-1, v); \
		CarpJsonTemplate::Deserialize(m28, #m28, (int)sizeof(#m28)-1, v); \
		CarpJsonTemplate::Deserialize(m29, #m29, (int)sizeof(#m29)-1, v); \
		CarpJsonTemplate::Deserialize(m30, #m30, (int)sizeof(#m30)-1, v); \
		CarpJsonTemplate::Deserialize(m31, #m31, (int)sizeof(#m31)-1, v); \
		CarpJsonTemplate::Deserialize(m32, #m32, (int)sizeof(#m32)-1, v); \
		CarpJsonTemplate::Deserialize(m33, #m33, (int)sizeof(#m33)-1, v); \
		CarpJsonTemplate::Deserialize(m34, #m34, (int)sizeof(#m34)-1, v); \
		CarpJsonTemplate::Deserialize(m35, #m35, (int)sizeof(#m35)-1, v); \
		CarpJsonTemplate::Deserialize(m36, #m36, (int)sizeof(#m36)-1, v); \
		CarpJsonTemplate::Deserialize(m37, #m37, (int)sizeof(#m37)-1, v); \
		CarpJsonTemplate::Deserialize(m38, #m38, (int)sizeof(#m38)-1, v); \
		CarpJsonTemplate::Deserialize(m39, #m39, (int)sizeof(#m39)-1, v); \
		CarpJsonTemplate::Deserialize(m40, #m40, (int)sizeof(#m40)-1, v); \
		CarpJsonTemplate::Deserialize(m41, #m41, (int)sizeof(#m41)-1, v); \
		CarpJsonTemplate::Deserialize(m42, #m42, (int)sizeof(#m42)-1, v); \
		CarpJsonTemplate::Deserialize(m43, #m43, (int)sizeof(#m43)-1, v); \
		CarpJsonTemplate::Deserialize(m44, #m44, (int)sizeof(#m44)-1, v); \
		CarpJsonTemplate::Deserialize(m45, #m45, (int)sizeof(#m45)-1, v); \
		CarpJsonTemplate::Deserialize(m46, #m46, (int)sizeof(#m46)-1, v); \
		CarpJsonTemplate::Deserialize(m47, #m47, (int)sizeof(#m47)-1, v); \
		CarpJsonTemplate::Deserialize(m48, #m48, (int)sizeof(#m48)-1, v); \
		CarpJsonTemplate::Deserialize(m49, #m49, (int)sizeof(#m49)-1, v); \
		CarpJsonTemplate::Deserialize(m50, #m50, (int)sizeof(#m50)-1, v); \
		CarpJsonTemplate::Deserialize(m51, #m51, (int)sizeof(#m51)-1, v); \
		CarpJsonTemplate::Deserialize(m52, #m52, (int)sizeof(#m52)-1, v); \
		CarpJsonTemplate::Deserialize(m53, #m53, (int)sizeof(#m53)-1, v); \
		CarpJsonTemplate::Deserialize(m54, #m54, (int)sizeof(#m54)-1, v); \
		CarpJsonTemplate::Deserialize(m55, #m55, (int)sizeof(#m55)-1, v); \
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
		CarpJsonTemplate::SerializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::SerializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::SerializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::SerializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::SerializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::SerializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::SerializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::SerializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::SerializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::SerializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::SerializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::SerializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::SerializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::SerializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::SerializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::SerializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::SerializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::SerializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::SerializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::SerializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::SerializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::SerializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::SerializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::SerializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::SerializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::SerializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::SerializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::SerializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::SerializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::SerializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::SerializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::SerializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::SerializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::SerializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::SerializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::SerializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::SerializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::SerializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::SerializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::SerializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::SerializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::SerializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::SerializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::SerializeValueMap(m53, #m53, v); \
		CarpJsonTemplate::SerializeValueMap(m54, #m54, v); \
		CarpJsonTemplate::SerializeValueMap(m55, #m55, v); \
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
		CarpJsonTemplate::DeserializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::DeserializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::DeserializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::DeserializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::DeserializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::DeserializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::DeserializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::DeserializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::DeserializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::DeserializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::DeserializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::DeserializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::DeserializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::DeserializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::DeserializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::DeserializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::DeserializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::DeserializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::DeserializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::DeserializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::DeserializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::DeserializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::DeserializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::DeserializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::DeserializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::DeserializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::DeserializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::DeserializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::DeserializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::DeserializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::DeserializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::DeserializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::DeserializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::DeserializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::DeserializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::DeserializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::DeserializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::DeserializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::DeserializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::DeserializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::DeserializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::DeserializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::DeserializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::DeserializeValueMap(m53, #m53, v); \
		CarpJsonTemplate::DeserializeValueMap(m54, #m54, v); \
		CarpJsonTemplate::DeserializeValueMap(m55, #m55, v); \
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
			s_list.push_back(#m10); \
			s_list.push_back(#m11); \
			s_list.push_back(#m12); \
			s_list.push_back(#m13); \
			s_list.push_back(#m14); \
			s_list.push_back(#m15); \
			s_list.push_back(#m16); \
			s_list.push_back(#m17); \
			s_list.push_back(#m18); \
			s_list.push_back(#m19); \
			s_list.push_back(#m20); \
			s_list.push_back(#m21); \
			s_list.push_back(#m22); \
			s_list.push_back(#m23); \
			s_list.push_back(#m24); \
			s_list.push_back(#m25); \
			s_list.push_back(#m26); \
			s_list.push_back(#m27); \
			s_list.push_back(#m28); \
			s_list.push_back(#m29); \
			s_list.push_back(#m30); \
			s_list.push_back(#m31); \
			s_list.push_back(#m32); \
			s_list.push_back(#m33); \
			s_list.push_back(#m34); \
			s_list.push_back(#m35); \
			s_list.push_back(#m36); \
			s_list.push_back(#m37); \
			s_list.push_back(#m38); \
			s_list.push_back(#m39); \
			s_list.push_back(#m40); \
			s_list.push_back(#m41); \
			s_list.push_back(#m42); \
			s_list.push_back(#m43); \
			s_list.push_back(#m44); \
			s_list.push_back(#m45); \
			s_list.push_back(#m46); \
			s_list.push_back(#m47); \
			s_list.push_back(#m48); \
			s_list.push_back(#m49); \
			s_list.push_back(#m50); \
			s_list.push_back(#m51); \
			s_list.push_back(#m52); \
			s_list.push_back(#m53); \
			s_list.push_back(#m54); \
			s_list.push_back(#m55); \
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
			s_list.push_back(#t10); \
			s_list.push_back(#t11); \
			s_list.push_back(#t12); \
			s_list.push_back(#t13); \
			s_list.push_back(#t14); \
			s_list.push_back(#t15); \
			s_list.push_back(#t16); \
			s_list.push_back(#t17); \
			s_list.push_back(#t18); \
			s_list.push_back(#t19); \
			s_list.push_back(#t20); \
			s_list.push_back(#t21); \
			s_list.push_back(#t22); \
			s_list.push_back(#t23); \
			s_list.push_back(#t24); \
			s_list.push_back(#t25); \
			s_list.push_back(#t26); \
			s_list.push_back(#t27); \
			s_list.push_back(#t28); \
			s_list.push_back(#t29); \
			s_list.push_back(#t30); \
			s_list.push_back(#t31); \
			s_list.push_back(#t32); \
			s_list.push_back(#t33); \
			s_list.push_back(#t34); \
			s_list.push_back(#t35); \
			s_list.push_back(#t36); \
			s_list.push_back(#t37); \
			s_list.push_back(#t38); \
			s_list.push_back(#t39); \
			s_list.push_back(#t40); \
			s_list.push_back(#t41); \
			s_list.push_back(#t42); \
			s_list.push_back(#t43); \
			s_list.push_back(#t44); \
			s_list.push_back(#t45); \
			s_list.push_back(#t46); \
			s_list.push_back(#t47); \
			s_list.push_back(#t48); \
			s_list.push_back(#t49); \
			s_list.push_back(#t50); \
			s_list.push_back(#t51); \
			s_list.push_back(#t52); \
			s_list.push_back(#t53); \
			s_list.push_back(#t54); \
			s_list.push_back(#t55); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8(), m9(), m10(), m11(), m12(), m13(), m14(), m15(), m16(), m17(), m18(), m19(), m20(), m21(), m22(), m23(), m24(), m25(), m26(), m27(), m28(), m29(), m30(), m31(), m32(), m33(), m34(), m35(), m36(), m37(), m38(), m39(), m40(), m41(), m42(), m43(), m44(), m45(), m46(), m47(), m48(), m49(), m50(), m51(), m52(), m53(), m54(), m55() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24; t25 m25; t26 m26; t27 m27; t28 m28; t29 m29; t30 m30; t31 m31; t32 m32; t33 m33; t34 m34; t35 m35; t36 m36; t37 m37; t38 m38; t39 m39; t40 m40; t41 m41; t42 m42; t43 m43; t44 m44; t45 m45; t46 m46; t47 m47; t48 m48; t49 m49; t50 m50; t51 m51; t52 m52; t53 m53; t54 m54; t55 m55; \
}
#define CARP_JSON_MACRO_115(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8, t9, m9, t10, m10, t11, m11, t12, m12, t13, m13, t14, m14, t15, m15, t16, m16, t17, m17, t18, m18, t19, m19, t20, m20, t21, m21, t22, m22, t23, m23, t24, m24, t25, m25, t26, m26, t27, m27, t28, m28, t29, m29, t30, m30, t31, m31, t32, m32, t33, m33, t34, m34, t35, m35, t36, m36, t37, m37, t38, m38, t39, m39, t40, m40, t41, m41, t42, m42, t43, m43, t44, m44, t45, m45, t46, m46, t47, m47, t48, m48, t49, m49, t50, m50, t51, m51, t52, m52, t53, m53, t54, m54, t55, m55, t56, m56) \
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
		CarpJsonTemplate::Serialize(m10, #m10, (int)sizeof(#m10)-1, v, a); \
		CarpJsonTemplate::Serialize(m11, #m11, (int)sizeof(#m11)-1, v, a); \
		CarpJsonTemplate::Serialize(m12, #m12, (int)sizeof(#m12)-1, v, a); \
		CarpJsonTemplate::Serialize(m13, #m13, (int)sizeof(#m13)-1, v, a); \
		CarpJsonTemplate::Serialize(m14, #m14, (int)sizeof(#m14)-1, v, a); \
		CarpJsonTemplate::Serialize(m15, #m15, (int)sizeof(#m15)-1, v, a); \
		CarpJsonTemplate::Serialize(m16, #m16, (int)sizeof(#m16)-1, v, a); \
		CarpJsonTemplate::Serialize(m17, #m17, (int)sizeof(#m17)-1, v, a); \
		CarpJsonTemplate::Serialize(m18, #m18, (int)sizeof(#m18)-1, v, a); \
		CarpJsonTemplate::Serialize(m19, #m19, (int)sizeof(#m19)-1, v, a); \
		CarpJsonTemplate::Serialize(m20, #m20, (int)sizeof(#m20)-1, v, a); \
		CarpJsonTemplate::Serialize(m21, #m21, (int)sizeof(#m21)-1, v, a); \
		CarpJsonTemplate::Serialize(m22, #m22, (int)sizeof(#m22)-1, v, a); \
		CarpJsonTemplate::Serialize(m23, #m23, (int)sizeof(#m23)-1, v, a); \
		CarpJsonTemplate::Serialize(m24, #m24, (int)sizeof(#m24)-1, v, a); \
		CarpJsonTemplate::Serialize(m25, #m25, (int)sizeof(#m25)-1, v, a); \
		CarpJsonTemplate::Serialize(m26, #m26, (int)sizeof(#m26)-1, v, a); \
		CarpJsonTemplate::Serialize(m27, #m27, (int)sizeof(#m27)-1, v, a); \
		CarpJsonTemplate::Serialize(m28, #m28, (int)sizeof(#m28)-1, v, a); \
		CarpJsonTemplate::Serialize(m29, #m29, (int)sizeof(#m29)-1, v, a); \
		CarpJsonTemplate::Serialize(m30, #m30, (int)sizeof(#m30)-1, v, a); \
		CarpJsonTemplate::Serialize(m31, #m31, (int)sizeof(#m31)-1, v, a); \
		CarpJsonTemplate::Serialize(m32, #m32, (int)sizeof(#m32)-1, v, a); \
		CarpJsonTemplate::Serialize(m33, #m33, (int)sizeof(#m33)-1, v, a); \
		CarpJsonTemplate::Serialize(m34, #m34, (int)sizeof(#m34)-1, v, a); \
		CarpJsonTemplate::Serialize(m35, #m35, (int)sizeof(#m35)-1, v, a); \
		CarpJsonTemplate::Serialize(m36, #m36, (int)sizeof(#m36)-1, v, a); \
		CarpJsonTemplate::Serialize(m37, #m37, (int)sizeof(#m37)-1, v, a); \
		CarpJsonTemplate::Serialize(m38, #m38, (int)sizeof(#m38)-1, v, a); \
		CarpJsonTemplate::Serialize(m39, #m39, (int)sizeof(#m39)-1, v, a); \
		CarpJsonTemplate::Serialize(m40, #m40, (int)sizeof(#m40)-1, v, a); \
		CarpJsonTemplate::Serialize(m41, #m41, (int)sizeof(#m41)-1, v, a); \
		CarpJsonTemplate::Serialize(m42, #m42, (int)sizeof(#m42)-1, v, a); \
		CarpJsonTemplate::Serialize(m43, #m43, (int)sizeof(#m43)-1, v, a); \
		CarpJsonTemplate::Serialize(m44, #m44, (int)sizeof(#m44)-1, v, a); \
		CarpJsonTemplate::Serialize(m45, #m45, (int)sizeof(#m45)-1, v, a); \
		CarpJsonTemplate::Serialize(m46, #m46, (int)sizeof(#m46)-1, v, a); \
		CarpJsonTemplate::Serialize(m47, #m47, (int)sizeof(#m47)-1, v, a); \
		CarpJsonTemplate::Serialize(m48, #m48, (int)sizeof(#m48)-1, v, a); \
		CarpJsonTemplate::Serialize(m49, #m49, (int)sizeof(#m49)-1, v, a); \
		CarpJsonTemplate::Serialize(m50, #m50, (int)sizeof(#m50)-1, v, a); \
		CarpJsonTemplate::Serialize(m51, #m51, (int)sizeof(#m51)-1, v, a); \
		CarpJsonTemplate::Serialize(m52, #m52, (int)sizeof(#m52)-1, v, a); \
		CarpJsonTemplate::Serialize(m53, #m53, (int)sizeof(#m53)-1, v, a); \
		CarpJsonTemplate::Serialize(m54, #m54, (int)sizeof(#m54)-1, v, a); \
		CarpJsonTemplate::Serialize(m55, #m55, (int)sizeof(#m55)-1, v, a); \
		CarpJsonTemplate::Serialize(m56, #m56, (int)sizeof(#m56)-1, v, a); \
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
		CarpJsonTemplate::Deserialize(m10, #m10, (int)sizeof(#m10)-1, v); \
		CarpJsonTemplate::Deserialize(m11, #m11, (int)sizeof(#m11)-1, v); \
		CarpJsonTemplate::Deserialize(m12, #m12, (int)sizeof(#m12)-1, v); \
		CarpJsonTemplate::Deserialize(m13, #m13, (int)sizeof(#m13)-1, v); \
		CarpJsonTemplate::Deserialize(m14, #m14, (int)sizeof(#m14)-1, v); \
		CarpJsonTemplate::Deserialize(m15, #m15, (int)sizeof(#m15)-1, v); \
		CarpJsonTemplate::Deserialize(m16, #m16, (int)sizeof(#m16)-1, v); \
		CarpJsonTemplate::Deserialize(m17, #m17, (int)sizeof(#m17)-1, v); \
		CarpJsonTemplate::Deserialize(m18, #m18, (int)sizeof(#m18)-1, v); \
		CarpJsonTemplate::Deserialize(m19, #m19, (int)sizeof(#m19)-1, v); \
		CarpJsonTemplate::Deserialize(m20, #m20, (int)sizeof(#m20)-1, v); \
		CarpJsonTemplate::Deserialize(m21, #m21, (int)sizeof(#m21)-1, v); \
		CarpJsonTemplate::Deserialize(m22, #m22, (int)sizeof(#m22)-1, v); \
		CarpJsonTemplate::Deserialize(m23, #m23, (int)sizeof(#m23)-1, v); \
		CarpJsonTemplate::Deserialize(m24, #m24, (int)sizeof(#m24)-1, v); \
		CarpJsonTemplate::Deserialize(m25, #m25, (int)sizeof(#m25)-1, v); \
		CarpJsonTemplate::Deserialize(m26, #m26, (int)sizeof(#m26)-1, v); \
		CarpJsonTemplate::Deserialize(m27, #m27, (int)sizeof(#m27)-1, v); \
		CarpJsonTemplate::Deserialize(m28, #m28, (int)sizeof(#m28)-1, v); \
		CarpJsonTemplate::Deserialize(m29, #m29, (int)sizeof(#m29)-1, v); \
		CarpJsonTemplate::Deserialize(m30, #m30, (int)sizeof(#m30)-1, v); \
		CarpJsonTemplate::Deserialize(m31, #m31, (int)sizeof(#m31)-1, v); \
		CarpJsonTemplate::Deserialize(m32, #m32, (int)sizeof(#m32)-1, v); \
		CarpJsonTemplate::Deserialize(m33, #m33, (int)sizeof(#m33)-1, v); \
		CarpJsonTemplate::Deserialize(m34, #m34, (int)sizeof(#m34)-1, v); \
		CarpJsonTemplate::Deserialize(m35, #m35, (int)sizeof(#m35)-1, v); \
		CarpJsonTemplate::Deserialize(m36, #m36, (int)sizeof(#m36)-1, v); \
		CarpJsonTemplate::Deserialize(m37, #m37, (int)sizeof(#m37)-1, v); \
		CarpJsonTemplate::Deserialize(m38, #m38, (int)sizeof(#m38)-1, v); \
		CarpJsonTemplate::Deserialize(m39, #m39, (int)sizeof(#m39)-1, v); \
		CarpJsonTemplate::Deserialize(m40, #m40, (int)sizeof(#m40)-1, v); \
		CarpJsonTemplate::Deserialize(m41, #m41, (int)sizeof(#m41)-1, v); \
		CarpJsonTemplate::Deserialize(m42, #m42, (int)sizeof(#m42)-1, v); \
		CarpJsonTemplate::Deserialize(m43, #m43, (int)sizeof(#m43)-1, v); \
		CarpJsonTemplate::Deserialize(m44, #m44, (int)sizeof(#m44)-1, v); \
		CarpJsonTemplate::Deserialize(m45, #m45, (int)sizeof(#m45)-1, v); \
		CarpJsonTemplate::Deserialize(m46, #m46, (int)sizeof(#m46)-1, v); \
		CarpJsonTemplate::Deserialize(m47, #m47, (int)sizeof(#m47)-1, v); \
		CarpJsonTemplate::Deserialize(m48, #m48, (int)sizeof(#m48)-1, v); \
		CarpJsonTemplate::Deserialize(m49, #m49, (int)sizeof(#m49)-1, v); \
		CarpJsonTemplate::Deserialize(m50, #m50, (int)sizeof(#m50)-1, v); \
		CarpJsonTemplate::Deserialize(m51, #m51, (int)sizeof(#m51)-1, v); \
		CarpJsonTemplate::Deserialize(m52, #m52, (int)sizeof(#m52)-1, v); \
		CarpJsonTemplate::Deserialize(m53, #m53, (int)sizeof(#m53)-1, v); \
		CarpJsonTemplate::Deserialize(m54, #m54, (int)sizeof(#m54)-1, v); \
		CarpJsonTemplate::Deserialize(m55, #m55, (int)sizeof(#m55)-1, v); \
		CarpJsonTemplate::Deserialize(m56, #m56, (int)sizeof(#m56)-1, v); \
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
		CarpJsonTemplate::SerializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::SerializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::SerializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::SerializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::SerializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::SerializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::SerializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::SerializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::SerializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::SerializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::SerializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::SerializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::SerializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::SerializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::SerializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::SerializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::SerializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::SerializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::SerializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::SerializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::SerializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::SerializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::SerializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::SerializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::SerializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::SerializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::SerializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::SerializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::SerializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::SerializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::SerializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::SerializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::SerializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::SerializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::SerializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::SerializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::SerializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::SerializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::SerializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::SerializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::SerializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::SerializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::SerializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::SerializeValueMap(m53, #m53, v); \
		CarpJsonTemplate::SerializeValueMap(m54, #m54, v); \
		CarpJsonTemplate::SerializeValueMap(m55, #m55, v); \
		CarpJsonTemplate::SerializeValueMap(m56, #m56, v); \
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
		CarpJsonTemplate::DeserializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::DeserializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::DeserializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::DeserializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::DeserializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::DeserializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::DeserializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::DeserializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::DeserializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::DeserializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::DeserializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::DeserializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::DeserializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::DeserializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::DeserializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::DeserializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::DeserializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::DeserializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::DeserializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::DeserializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::DeserializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::DeserializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::DeserializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::DeserializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::DeserializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::DeserializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::DeserializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::DeserializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::DeserializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::DeserializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::DeserializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::DeserializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::DeserializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::DeserializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::DeserializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::DeserializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::DeserializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::DeserializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::DeserializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::DeserializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::DeserializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::DeserializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::DeserializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::DeserializeValueMap(m53, #m53, v); \
		CarpJsonTemplate::DeserializeValueMap(m54, #m54, v); \
		CarpJsonTemplate::DeserializeValueMap(m55, #m55, v); \
		CarpJsonTemplate::DeserializeValueMap(m56, #m56, v); \
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
			s_list.push_back(#m10); \
			s_list.push_back(#m11); \
			s_list.push_back(#m12); \
			s_list.push_back(#m13); \
			s_list.push_back(#m14); \
			s_list.push_back(#m15); \
			s_list.push_back(#m16); \
			s_list.push_back(#m17); \
			s_list.push_back(#m18); \
			s_list.push_back(#m19); \
			s_list.push_back(#m20); \
			s_list.push_back(#m21); \
			s_list.push_back(#m22); \
			s_list.push_back(#m23); \
			s_list.push_back(#m24); \
			s_list.push_back(#m25); \
			s_list.push_back(#m26); \
			s_list.push_back(#m27); \
			s_list.push_back(#m28); \
			s_list.push_back(#m29); \
			s_list.push_back(#m30); \
			s_list.push_back(#m31); \
			s_list.push_back(#m32); \
			s_list.push_back(#m33); \
			s_list.push_back(#m34); \
			s_list.push_back(#m35); \
			s_list.push_back(#m36); \
			s_list.push_back(#m37); \
			s_list.push_back(#m38); \
			s_list.push_back(#m39); \
			s_list.push_back(#m40); \
			s_list.push_back(#m41); \
			s_list.push_back(#m42); \
			s_list.push_back(#m43); \
			s_list.push_back(#m44); \
			s_list.push_back(#m45); \
			s_list.push_back(#m46); \
			s_list.push_back(#m47); \
			s_list.push_back(#m48); \
			s_list.push_back(#m49); \
			s_list.push_back(#m50); \
			s_list.push_back(#m51); \
			s_list.push_back(#m52); \
			s_list.push_back(#m53); \
			s_list.push_back(#m54); \
			s_list.push_back(#m55); \
			s_list.push_back(#m56); \
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
			s_list.push_back(#t10); \
			s_list.push_back(#t11); \
			s_list.push_back(#t12); \
			s_list.push_back(#t13); \
			s_list.push_back(#t14); \
			s_list.push_back(#t15); \
			s_list.push_back(#t16); \
			s_list.push_back(#t17); \
			s_list.push_back(#t18); \
			s_list.push_back(#t19); \
			s_list.push_back(#t20); \
			s_list.push_back(#t21); \
			s_list.push_back(#t22); \
			s_list.push_back(#t23); \
			s_list.push_back(#t24); \
			s_list.push_back(#t25); \
			s_list.push_back(#t26); \
			s_list.push_back(#t27); \
			s_list.push_back(#t28); \
			s_list.push_back(#t29); \
			s_list.push_back(#t30); \
			s_list.push_back(#t31); \
			s_list.push_back(#t32); \
			s_list.push_back(#t33); \
			s_list.push_back(#t34); \
			s_list.push_back(#t35); \
			s_list.push_back(#t36); \
			s_list.push_back(#t37); \
			s_list.push_back(#t38); \
			s_list.push_back(#t39); \
			s_list.push_back(#t40); \
			s_list.push_back(#t41); \
			s_list.push_back(#t42); \
			s_list.push_back(#t43); \
			s_list.push_back(#t44); \
			s_list.push_back(#t45); \
			s_list.push_back(#t46); \
			s_list.push_back(#t47); \
			s_list.push_back(#t48); \
			s_list.push_back(#t49); \
			s_list.push_back(#t50); \
			s_list.push_back(#t51); \
			s_list.push_back(#t52); \
			s_list.push_back(#t53); \
			s_list.push_back(#t54); \
			s_list.push_back(#t55); \
			s_list.push_back(#t56); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8(), m9(), m10(), m11(), m12(), m13(), m14(), m15(), m16(), m17(), m18(), m19(), m20(), m21(), m22(), m23(), m24(), m25(), m26(), m27(), m28(), m29(), m30(), m31(), m32(), m33(), m34(), m35(), m36(), m37(), m38(), m39(), m40(), m41(), m42(), m43(), m44(), m45(), m46(), m47(), m48(), m49(), m50(), m51(), m52(), m53(), m54(), m55(), m56() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24; t25 m25; t26 m26; t27 m27; t28 m28; t29 m29; t30 m30; t31 m31; t32 m32; t33 m33; t34 m34; t35 m35; t36 m36; t37 m37; t38 m38; t39 m39; t40 m40; t41 m41; t42 m42; t43 m43; t44 m44; t45 m45; t46 m46; t47 m47; t48 m48; t49 m49; t50 m50; t51 m51; t52 m52; t53 m53; t54 m54; t55 m55; t56 m56; \
}
#define CARP_JSON_MACRO_117(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8, t9, m9, t10, m10, t11, m11, t12, m12, t13, m13, t14, m14, t15, m15, t16, m16, t17, m17, t18, m18, t19, m19, t20, m20, t21, m21, t22, m22, t23, m23, t24, m24, t25, m25, t26, m26, t27, m27, t28, m28, t29, m29, t30, m30, t31, m31, t32, m32, t33, m33, t34, m34, t35, m35, t36, m36, t37, m37, t38, m38, t39, m39, t40, m40, t41, m41, t42, m42, t43, m43, t44, m44, t45, m45, t46, m46, t47, m47, t48, m48, t49, m49, t50, m50, t51, m51, t52, m52, t53, m53, t54, m54, t55, m55, t56, m56, t57, m57) \
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
		CarpJsonTemplate::Serialize(m10, #m10, (int)sizeof(#m10)-1, v, a); \
		CarpJsonTemplate::Serialize(m11, #m11, (int)sizeof(#m11)-1, v, a); \
		CarpJsonTemplate::Serialize(m12, #m12, (int)sizeof(#m12)-1, v, a); \
		CarpJsonTemplate::Serialize(m13, #m13, (int)sizeof(#m13)-1, v, a); \
		CarpJsonTemplate::Serialize(m14, #m14, (int)sizeof(#m14)-1, v, a); \
		CarpJsonTemplate::Serialize(m15, #m15, (int)sizeof(#m15)-1, v, a); \
		CarpJsonTemplate::Serialize(m16, #m16, (int)sizeof(#m16)-1, v, a); \
		CarpJsonTemplate::Serialize(m17, #m17, (int)sizeof(#m17)-1, v, a); \
		CarpJsonTemplate::Serialize(m18, #m18, (int)sizeof(#m18)-1, v, a); \
		CarpJsonTemplate::Serialize(m19, #m19, (int)sizeof(#m19)-1, v, a); \
		CarpJsonTemplate::Serialize(m20, #m20, (int)sizeof(#m20)-1, v, a); \
		CarpJsonTemplate::Serialize(m21, #m21, (int)sizeof(#m21)-1, v, a); \
		CarpJsonTemplate::Serialize(m22, #m22, (int)sizeof(#m22)-1, v, a); \
		CarpJsonTemplate::Serialize(m23, #m23, (int)sizeof(#m23)-1, v, a); \
		CarpJsonTemplate::Serialize(m24, #m24, (int)sizeof(#m24)-1, v, a); \
		CarpJsonTemplate::Serialize(m25, #m25, (int)sizeof(#m25)-1, v, a); \
		CarpJsonTemplate::Serialize(m26, #m26, (int)sizeof(#m26)-1, v, a); \
		CarpJsonTemplate::Serialize(m27, #m27, (int)sizeof(#m27)-1, v, a); \
		CarpJsonTemplate::Serialize(m28, #m28, (int)sizeof(#m28)-1, v, a); \
		CarpJsonTemplate::Serialize(m29, #m29, (int)sizeof(#m29)-1, v, a); \
		CarpJsonTemplate::Serialize(m30, #m30, (int)sizeof(#m30)-1, v, a); \
		CarpJsonTemplate::Serialize(m31, #m31, (int)sizeof(#m31)-1, v, a); \
		CarpJsonTemplate::Serialize(m32, #m32, (int)sizeof(#m32)-1, v, a); \
		CarpJsonTemplate::Serialize(m33, #m33, (int)sizeof(#m33)-1, v, a); \
		CarpJsonTemplate::Serialize(m34, #m34, (int)sizeof(#m34)-1, v, a); \
		CarpJsonTemplate::Serialize(m35, #m35, (int)sizeof(#m35)-1, v, a); \
		CarpJsonTemplate::Serialize(m36, #m36, (int)sizeof(#m36)-1, v, a); \
		CarpJsonTemplate::Serialize(m37, #m37, (int)sizeof(#m37)-1, v, a); \
		CarpJsonTemplate::Serialize(m38, #m38, (int)sizeof(#m38)-1, v, a); \
		CarpJsonTemplate::Serialize(m39, #m39, (int)sizeof(#m39)-1, v, a); \
		CarpJsonTemplate::Serialize(m40, #m40, (int)sizeof(#m40)-1, v, a); \
		CarpJsonTemplate::Serialize(m41, #m41, (int)sizeof(#m41)-1, v, a); \
		CarpJsonTemplate::Serialize(m42, #m42, (int)sizeof(#m42)-1, v, a); \
		CarpJsonTemplate::Serialize(m43, #m43, (int)sizeof(#m43)-1, v, a); \
		CarpJsonTemplate::Serialize(m44, #m44, (int)sizeof(#m44)-1, v, a); \
		CarpJsonTemplate::Serialize(m45, #m45, (int)sizeof(#m45)-1, v, a); \
		CarpJsonTemplate::Serialize(m46, #m46, (int)sizeof(#m46)-1, v, a); \
		CarpJsonTemplate::Serialize(m47, #m47, (int)sizeof(#m47)-1, v, a); \
		CarpJsonTemplate::Serialize(m48, #m48, (int)sizeof(#m48)-1, v, a); \
		CarpJsonTemplate::Serialize(m49, #m49, (int)sizeof(#m49)-1, v, a); \
		CarpJsonTemplate::Serialize(m50, #m50, (int)sizeof(#m50)-1, v, a); \
		CarpJsonTemplate::Serialize(m51, #m51, (int)sizeof(#m51)-1, v, a); \
		CarpJsonTemplate::Serialize(m52, #m52, (int)sizeof(#m52)-1, v, a); \
		CarpJsonTemplate::Serialize(m53, #m53, (int)sizeof(#m53)-1, v, a); \
		CarpJsonTemplate::Serialize(m54, #m54, (int)sizeof(#m54)-1, v, a); \
		CarpJsonTemplate::Serialize(m55, #m55, (int)sizeof(#m55)-1, v, a); \
		CarpJsonTemplate::Serialize(m56, #m56, (int)sizeof(#m56)-1, v, a); \
		CarpJsonTemplate::Serialize(m57, #m57, (int)sizeof(#m57)-1, v, a); \
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
		CarpJsonTemplate::Deserialize(m10, #m10, (int)sizeof(#m10)-1, v); \
		CarpJsonTemplate::Deserialize(m11, #m11, (int)sizeof(#m11)-1, v); \
		CarpJsonTemplate::Deserialize(m12, #m12, (int)sizeof(#m12)-1, v); \
		CarpJsonTemplate::Deserialize(m13, #m13, (int)sizeof(#m13)-1, v); \
		CarpJsonTemplate::Deserialize(m14, #m14, (int)sizeof(#m14)-1, v); \
		CarpJsonTemplate::Deserialize(m15, #m15, (int)sizeof(#m15)-1, v); \
		CarpJsonTemplate::Deserialize(m16, #m16, (int)sizeof(#m16)-1, v); \
		CarpJsonTemplate::Deserialize(m17, #m17, (int)sizeof(#m17)-1, v); \
		CarpJsonTemplate::Deserialize(m18, #m18, (int)sizeof(#m18)-1, v); \
		CarpJsonTemplate::Deserialize(m19, #m19, (int)sizeof(#m19)-1, v); \
		CarpJsonTemplate::Deserialize(m20, #m20, (int)sizeof(#m20)-1, v); \
		CarpJsonTemplate::Deserialize(m21, #m21, (int)sizeof(#m21)-1, v); \
		CarpJsonTemplate::Deserialize(m22, #m22, (int)sizeof(#m22)-1, v); \
		CarpJsonTemplate::Deserialize(m23, #m23, (int)sizeof(#m23)-1, v); \
		CarpJsonTemplate::Deserialize(m24, #m24, (int)sizeof(#m24)-1, v); \
		CarpJsonTemplate::Deserialize(m25, #m25, (int)sizeof(#m25)-1, v); \
		CarpJsonTemplate::Deserialize(m26, #m26, (int)sizeof(#m26)-1, v); \
		CarpJsonTemplate::Deserialize(m27, #m27, (int)sizeof(#m27)-1, v); \
		CarpJsonTemplate::Deserialize(m28, #m28, (int)sizeof(#m28)-1, v); \
		CarpJsonTemplate::Deserialize(m29, #m29, (int)sizeof(#m29)-1, v); \
		CarpJsonTemplate::Deserialize(m30, #m30, (int)sizeof(#m30)-1, v); \
		CarpJsonTemplate::Deserialize(m31, #m31, (int)sizeof(#m31)-1, v); \
		CarpJsonTemplate::Deserialize(m32, #m32, (int)sizeof(#m32)-1, v); \
		CarpJsonTemplate::Deserialize(m33, #m33, (int)sizeof(#m33)-1, v); \
		CarpJsonTemplate::Deserialize(m34, #m34, (int)sizeof(#m34)-1, v); \
		CarpJsonTemplate::Deserialize(m35, #m35, (int)sizeof(#m35)-1, v); \
		CarpJsonTemplate::Deserialize(m36, #m36, (int)sizeof(#m36)-1, v); \
		CarpJsonTemplate::Deserialize(m37, #m37, (int)sizeof(#m37)-1, v); \
		CarpJsonTemplate::Deserialize(m38, #m38, (int)sizeof(#m38)-1, v); \
		CarpJsonTemplate::Deserialize(m39, #m39, (int)sizeof(#m39)-1, v); \
		CarpJsonTemplate::Deserialize(m40, #m40, (int)sizeof(#m40)-1, v); \
		CarpJsonTemplate::Deserialize(m41, #m41, (int)sizeof(#m41)-1, v); \
		CarpJsonTemplate::Deserialize(m42, #m42, (int)sizeof(#m42)-1, v); \
		CarpJsonTemplate::Deserialize(m43, #m43, (int)sizeof(#m43)-1, v); \
		CarpJsonTemplate::Deserialize(m44, #m44, (int)sizeof(#m44)-1, v); \
		CarpJsonTemplate::Deserialize(m45, #m45, (int)sizeof(#m45)-1, v); \
		CarpJsonTemplate::Deserialize(m46, #m46, (int)sizeof(#m46)-1, v); \
		CarpJsonTemplate::Deserialize(m47, #m47, (int)sizeof(#m47)-1, v); \
		CarpJsonTemplate::Deserialize(m48, #m48, (int)sizeof(#m48)-1, v); \
		CarpJsonTemplate::Deserialize(m49, #m49, (int)sizeof(#m49)-1, v); \
		CarpJsonTemplate::Deserialize(m50, #m50, (int)sizeof(#m50)-1, v); \
		CarpJsonTemplate::Deserialize(m51, #m51, (int)sizeof(#m51)-1, v); \
		CarpJsonTemplate::Deserialize(m52, #m52, (int)sizeof(#m52)-1, v); \
		CarpJsonTemplate::Deserialize(m53, #m53, (int)sizeof(#m53)-1, v); \
		CarpJsonTemplate::Deserialize(m54, #m54, (int)sizeof(#m54)-1, v); \
		CarpJsonTemplate::Deserialize(m55, #m55, (int)sizeof(#m55)-1, v); \
		CarpJsonTemplate::Deserialize(m56, #m56, (int)sizeof(#m56)-1, v); \
		CarpJsonTemplate::Deserialize(m57, #m57, (int)sizeof(#m57)-1, v); \
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
		CarpJsonTemplate::SerializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::SerializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::SerializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::SerializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::SerializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::SerializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::SerializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::SerializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::SerializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::SerializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::SerializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::SerializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::SerializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::SerializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::SerializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::SerializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::SerializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::SerializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::SerializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::SerializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::SerializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::SerializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::SerializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::SerializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::SerializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::SerializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::SerializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::SerializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::SerializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::SerializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::SerializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::SerializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::SerializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::SerializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::SerializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::SerializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::SerializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::SerializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::SerializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::SerializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::SerializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::SerializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::SerializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::SerializeValueMap(m53, #m53, v); \
		CarpJsonTemplate::SerializeValueMap(m54, #m54, v); \
		CarpJsonTemplate::SerializeValueMap(m55, #m55, v); \
		CarpJsonTemplate::SerializeValueMap(m56, #m56, v); \
		CarpJsonTemplate::SerializeValueMap(m57, #m57, v); \
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
		CarpJsonTemplate::DeserializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::DeserializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::DeserializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::DeserializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::DeserializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::DeserializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::DeserializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::DeserializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::DeserializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::DeserializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::DeserializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::DeserializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::DeserializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::DeserializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::DeserializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::DeserializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::DeserializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::DeserializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::DeserializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::DeserializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::DeserializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::DeserializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::DeserializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::DeserializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::DeserializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::DeserializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::DeserializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::DeserializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::DeserializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::DeserializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::DeserializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::DeserializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::DeserializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::DeserializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::DeserializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::DeserializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::DeserializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::DeserializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::DeserializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::DeserializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::DeserializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::DeserializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::DeserializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::DeserializeValueMap(m53, #m53, v); \
		CarpJsonTemplate::DeserializeValueMap(m54, #m54, v); \
		CarpJsonTemplate::DeserializeValueMap(m55, #m55, v); \
		CarpJsonTemplate::DeserializeValueMap(m56, #m56, v); \
		CarpJsonTemplate::DeserializeValueMap(m57, #m57, v); \
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
			s_list.push_back(#m10); \
			s_list.push_back(#m11); \
			s_list.push_back(#m12); \
			s_list.push_back(#m13); \
			s_list.push_back(#m14); \
			s_list.push_back(#m15); \
			s_list.push_back(#m16); \
			s_list.push_back(#m17); \
			s_list.push_back(#m18); \
			s_list.push_back(#m19); \
			s_list.push_back(#m20); \
			s_list.push_back(#m21); \
			s_list.push_back(#m22); \
			s_list.push_back(#m23); \
			s_list.push_back(#m24); \
			s_list.push_back(#m25); \
			s_list.push_back(#m26); \
			s_list.push_back(#m27); \
			s_list.push_back(#m28); \
			s_list.push_back(#m29); \
			s_list.push_back(#m30); \
			s_list.push_back(#m31); \
			s_list.push_back(#m32); \
			s_list.push_back(#m33); \
			s_list.push_back(#m34); \
			s_list.push_back(#m35); \
			s_list.push_back(#m36); \
			s_list.push_back(#m37); \
			s_list.push_back(#m38); \
			s_list.push_back(#m39); \
			s_list.push_back(#m40); \
			s_list.push_back(#m41); \
			s_list.push_back(#m42); \
			s_list.push_back(#m43); \
			s_list.push_back(#m44); \
			s_list.push_back(#m45); \
			s_list.push_back(#m46); \
			s_list.push_back(#m47); \
			s_list.push_back(#m48); \
			s_list.push_back(#m49); \
			s_list.push_back(#m50); \
			s_list.push_back(#m51); \
			s_list.push_back(#m52); \
			s_list.push_back(#m53); \
			s_list.push_back(#m54); \
			s_list.push_back(#m55); \
			s_list.push_back(#m56); \
			s_list.push_back(#m57); \
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
			s_list.push_back(#t10); \
			s_list.push_back(#t11); \
			s_list.push_back(#t12); \
			s_list.push_back(#t13); \
			s_list.push_back(#t14); \
			s_list.push_back(#t15); \
			s_list.push_back(#t16); \
			s_list.push_back(#t17); \
			s_list.push_back(#t18); \
			s_list.push_back(#t19); \
			s_list.push_back(#t20); \
			s_list.push_back(#t21); \
			s_list.push_back(#t22); \
			s_list.push_back(#t23); \
			s_list.push_back(#t24); \
			s_list.push_back(#t25); \
			s_list.push_back(#t26); \
			s_list.push_back(#t27); \
			s_list.push_back(#t28); \
			s_list.push_back(#t29); \
			s_list.push_back(#t30); \
			s_list.push_back(#t31); \
			s_list.push_back(#t32); \
			s_list.push_back(#t33); \
			s_list.push_back(#t34); \
			s_list.push_back(#t35); \
			s_list.push_back(#t36); \
			s_list.push_back(#t37); \
			s_list.push_back(#t38); \
			s_list.push_back(#t39); \
			s_list.push_back(#t40); \
			s_list.push_back(#t41); \
			s_list.push_back(#t42); \
			s_list.push_back(#t43); \
			s_list.push_back(#t44); \
			s_list.push_back(#t45); \
			s_list.push_back(#t46); \
			s_list.push_back(#t47); \
			s_list.push_back(#t48); \
			s_list.push_back(#t49); \
			s_list.push_back(#t50); \
			s_list.push_back(#t51); \
			s_list.push_back(#t52); \
			s_list.push_back(#t53); \
			s_list.push_back(#t54); \
			s_list.push_back(#t55); \
			s_list.push_back(#t56); \
			s_list.push_back(#t57); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8(), m9(), m10(), m11(), m12(), m13(), m14(), m15(), m16(), m17(), m18(), m19(), m20(), m21(), m22(), m23(), m24(), m25(), m26(), m27(), m28(), m29(), m30(), m31(), m32(), m33(), m34(), m35(), m36(), m37(), m38(), m39(), m40(), m41(), m42(), m43(), m44(), m45(), m46(), m47(), m48(), m49(), m50(), m51(), m52(), m53(), m54(), m55(), m56(), m57() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24; t25 m25; t26 m26; t27 m27; t28 m28; t29 m29; t30 m30; t31 m31; t32 m32; t33 m33; t34 m34; t35 m35; t36 m36; t37 m37; t38 m38; t39 m39; t40 m40; t41 m41; t42 m42; t43 m43; t44 m44; t45 m45; t46 m46; t47 m47; t48 m48; t49 m49; t50 m50; t51 m51; t52 m52; t53 m53; t54 m54; t55 m55; t56 m56; t57 m57; \
}
#define CARP_JSON_MACRO_119(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8, t9, m9, t10, m10, t11, m11, t12, m12, t13, m13, t14, m14, t15, m15, t16, m16, t17, m17, t18, m18, t19, m19, t20, m20, t21, m21, t22, m22, t23, m23, t24, m24, t25, m25, t26, m26, t27, m27, t28, m28, t29, m29, t30, m30, t31, m31, t32, m32, t33, m33, t34, m34, t35, m35, t36, m36, t37, m37, t38, m38, t39, m39, t40, m40, t41, m41, t42, m42, t43, m43, t44, m44, t45, m45, t46, m46, t47, m47, t48, m48, t49, m49, t50, m50, t51, m51, t52, m52, t53, m53, t54, m54, t55, m55, t56, m56, t57, m57, t58, m58) \
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
		CarpJsonTemplate::Serialize(m10, #m10, (int)sizeof(#m10)-1, v, a); \
		CarpJsonTemplate::Serialize(m11, #m11, (int)sizeof(#m11)-1, v, a); \
		CarpJsonTemplate::Serialize(m12, #m12, (int)sizeof(#m12)-1, v, a); \
		CarpJsonTemplate::Serialize(m13, #m13, (int)sizeof(#m13)-1, v, a); \
		CarpJsonTemplate::Serialize(m14, #m14, (int)sizeof(#m14)-1, v, a); \
		CarpJsonTemplate::Serialize(m15, #m15, (int)sizeof(#m15)-1, v, a); \
		CarpJsonTemplate::Serialize(m16, #m16, (int)sizeof(#m16)-1, v, a); \
		CarpJsonTemplate::Serialize(m17, #m17, (int)sizeof(#m17)-1, v, a); \
		CarpJsonTemplate::Serialize(m18, #m18, (int)sizeof(#m18)-1, v, a); \
		CarpJsonTemplate::Serialize(m19, #m19, (int)sizeof(#m19)-1, v, a); \
		CarpJsonTemplate::Serialize(m20, #m20, (int)sizeof(#m20)-1, v, a); \
		CarpJsonTemplate::Serialize(m21, #m21, (int)sizeof(#m21)-1, v, a); \
		CarpJsonTemplate::Serialize(m22, #m22, (int)sizeof(#m22)-1, v, a); \
		CarpJsonTemplate::Serialize(m23, #m23, (int)sizeof(#m23)-1, v, a); \
		CarpJsonTemplate::Serialize(m24, #m24, (int)sizeof(#m24)-1, v, a); \
		CarpJsonTemplate::Serialize(m25, #m25, (int)sizeof(#m25)-1, v, a); \
		CarpJsonTemplate::Serialize(m26, #m26, (int)sizeof(#m26)-1, v, a); \
		CarpJsonTemplate::Serialize(m27, #m27, (int)sizeof(#m27)-1, v, a); \
		CarpJsonTemplate::Serialize(m28, #m28, (int)sizeof(#m28)-1, v, a); \
		CarpJsonTemplate::Serialize(m29, #m29, (int)sizeof(#m29)-1, v, a); \
		CarpJsonTemplate::Serialize(m30, #m30, (int)sizeof(#m30)-1, v, a); \
		CarpJsonTemplate::Serialize(m31, #m31, (int)sizeof(#m31)-1, v, a); \
		CarpJsonTemplate::Serialize(m32, #m32, (int)sizeof(#m32)-1, v, a); \
		CarpJsonTemplate::Serialize(m33, #m33, (int)sizeof(#m33)-1, v, a); \
		CarpJsonTemplate::Serialize(m34, #m34, (int)sizeof(#m34)-1, v, a); \
		CarpJsonTemplate::Serialize(m35, #m35, (int)sizeof(#m35)-1, v, a); \
		CarpJsonTemplate::Serialize(m36, #m36, (int)sizeof(#m36)-1, v, a); \
		CarpJsonTemplate::Serialize(m37, #m37, (int)sizeof(#m37)-1, v, a); \
		CarpJsonTemplate::Serialize(m38, #m38, (int)sizeof(#m38)-1, v, a); \
		CarpJsonTemplate::Serialize(m39, #m39, (int)sizeof(#m39)-1, v, a); \
		CarpJsonTemplate::Serialize(m40, #m40, (int)sizeof(#m40)-1, v, a); \
		CarpJsonTemplate::Serialize(m41, #m41, (int)sizeof(#m41)-1, v, a); \
		CarpJsonTemplate::Serialize(m42, #m42, (int)sizeof(#m42)-1, v, a); \
		CarpJsonTemplate::Serialize(m43, #m43, (int)sizeof(#m43)-1, v, a); \
		CarpJsonTemplate::Serialize(m44, #m44, (int)sizeof(#m44)-1, v, a); \
		CarpJsonTemplate::Serialize(m45, #m45, (int)sizeof(#m45)-1, v, a); \
		CarpJsonTemplate::Serialize(m46, #m46, (int)sizeof(#m46)-1, v, a); \
		CarpJsonTemplate::Serialize(m47, #m47, (int)sizeof(#m47)-1, v, a); \
		CarpJsonTemplate::Serialize(m48, #m48, (int)sizeof(#m48)-1, v, a); \
		CarpJsonTemplate::Serialize(m49, #m49, (int)sizeof(#m49)-1, v, a); \
		CarpJsonTemplate::Serialize(m50, #m50, (int)sizeof(#m50)-1, v, a); \
		CarpJsonTemplate::Serialize(m51, #m51, (int)sizeof(#m51)-1, v, a); \
		CarpJsonTemplate::Serialize(m52, #m52, (int)sizeof(#m52)-1, v, a); \
		CarpJsonTemplate::Serialize(m53, #m53, (int)sizeof(#m53)-1, v, a); \
		CarpJsonTemplate::Serialize(m54, #m54, (int)sizeof(#m54)-1, v, a); \
		CarpJsonTemplate::Serialize(m55, #m55, (int)sizeof(#m55)-1, v, a); \
		CarpJsonTemplate::Serialize(m56, #m56, (int)sizeof(#m56)-1, v, a); \
		CarpJsonTemplate::Serialize(m57, #m57, (int)sizeof(#m57)-1, v, a); \
		CarpJsonTemplate::Serialize(m58, #m58, (int)sizeof(#m58)-1, v, a); \
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
		CarpJsonTemplate::Deserialize(m10, #m10, (int)sizeof(#m10)-1, v); \
		CarpJsonTemplate::Deserialize(m11, #m11, (int)sizeof(#m11)-1, v); \
		CarpJsonTemplate::Deserialize(m12, #m12, (int)sizeof(#m12)-1, v); \
		CarpJsonTemplate::Deserialize(m13, #m13, (int)sizeof(#m13)-1, v); \
		CarpJsonTemplate::Deserialize(m14, #m14, (int)sizeof(#m14)-1, v); \
		CarpJsonTemplate::Deserialize(m15, #m15, (int)sizeof(#m15)-1, v); \
		CarpJsonTemplate::Deserialize(m16, #m16, (int)sizeof(#m16)-1, v); \
		CarpJsonTemplate::Deserialize(m17, #m17, (int)sizeof(#m17)-1, v); \
		CarpJsonTemplate::Deserialize(m18, #m18, (int)sizeof(#m18)-1, v); \
		CarpJsonTemplate::Deserialize(m19, #m19, (int)sizeof(#m19)-1, v); \
		CarpJsonTemplate::Deserialize(m20, #m20, (int)sizeof(#m20)-1, v); \
		CarpJsonTemplate::Deserialize(m21, #m21, (int)sizeof(#m21)-1, v); \
		CarpJsonTemplate::Deserialize(m22, #m22, (int)sizeof(#m22)-1, v); \
		CarpJsonTemplate::Deserialize(m23, #m23, (int)sizeof(#m23)-1, v); \
		CarpJsonTemplate::Deserialize(m24, #m24, (int)sizeof(#m24)-1, v); \
		CarpJsonTemplate::Deserialize(m25, #m25, (int)sizeof(#m25)-1, v); \
		CarpJsonTemplate::Deserialize(m26, #m26, (int)sizeof(#m26)-1, v); \
		CarpJsonTemplate::Deserialize(m27, #m27, (int)sizeof(#m27)-1, v); \
		CarpJsonTemplate::Deserialize(m28, #m28, (int)sizeof(#m28)-1, v); \
		CarpJsonTemplate::Deserialize(m29, #m29, (int)sizeof(#m29)-1, v); \
		CarpJsonTemplate::Deserialize(m30, #m30, (int)sizeof(#m30)-1, v); \
		CarpJsonTemplate::Deserialize(m31, #m31, (int)sizeof(#m31)-1, v); \
		CarpJsonTemplate::Deserialize(m32, #m32, (int)sizeof(#m32)-1, v); \
		CarpJsonTemplate::Deserialize(m33, #m33, (int)sizeof(#m33)-1, v); \
		CarpJsonTemplate::Deserialize(m34, #m34, (int)sizeof(#m34)-1, v); \
		CarpJsonTemplate::Deserialize(m35, #m35, (int)sizeof(#m35)-1, v); \
		CarpJsonTemplate::Deserialize(m36, #m36, (int)sizeof(#m36)-1, v); \
		CarpJsonTemplate::Deserialize(m37, #m37, (int)sizeof(#m37)-1, v); \
		CarpJsonTemplate::Deserialize(m38, #m38, (int)sizeof(#m38)-1, v); \
		CarpJsonTemplate::Deserialize(m39, #m39, (int)sizeof(#m39)-1, v); \
		CarpJsonTemplate::Deserialize(m40, #m40, (int)sizeof(#m40)-1, v); \
		CarpJsonTemplate::Deserialize(m41, #m41, (int)sizeof(#m41)-1, v); \
		CarpJsonTemplate::Deserialize(m42, #m42, (int)sizeof(#m42)-1, v); \
		CarpJsonTemplate::Deserialize(m43, #m43, (int)sizeof(#m43)-1, v); \
		CarpJsonTemplate::Deserialize(m44, #m44, (int)sizeof(#m44)-1, v); \
		CarpJsonTemplate::Deserialize(m45, #m45, (int)sizeof(#m45)-1, v); \
		CarpJsonTemplate::Deserialize(m46, #m46, (int)sizeof(#m46)-1, v); \
		CarpJsonTemplate::Deserialize(m47, #m47, (int)sizeof(#m47)-1, v); \
		CarpJsonTemplate::Deserialize(m48, #m48, (int)sizeof(#m48)-1, v); \
		CarpJsonTemplate::Deserialize(m49, #m49, (int)sizeof(#m49)-1, v); \
		CarpJsonTemplate::Deserialize(m50, #m50, (int)sizeof(#m50)-1, v); \
		CarpJsonTemplate::Deserialize(m51, #m51, (int)sizeof(#m51)-1, v); \
		CarpJsonTemplate::Deserialize(m52, #m52, (int)sizeof(#m52)-1, v); \
		CarpJsonTemplate::Deserialize(m53, #m53, (int)sizeof(#m53)-1, v); \
		CarpJsonTemplate::Deserialize(m54, #m54, (int)sizeof(#m54)-1, v); \
		CarpJsonTemplate::Deserialize(m55, #m55, (int)sizeof(#m55)-1, v); \
		CarpJsonTemplate::Deserialize(m56, #m56, (int)sizeof(#m56)-1, v); \
		CarpJsonTemplate::Deserialize(m57, #m57, (int)sizeof(#m57)-1, v); \
		CarpJsonTemplate::Deserialize(m58, #m58, (int)sizeof(#m58)-1, v); \
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
		CarpJsonTemplate::SerializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::SerializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::SerializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::SerializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::SerializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::SerializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::SerializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::SerializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::SerializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::SerializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::SerializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::SerializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::SerializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::SerializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::SerializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::SerializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::SerializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::SerializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::SerializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::SerializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::SerializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::SerializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::SerializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::SerializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::SerializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::SerializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::SerializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::SerializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::SerializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::SerializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::SerializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::SerializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::SerializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::SerializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::SerializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::SerializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::SerializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::SerializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::SerializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::SerializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::SerializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::SerializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::SerializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::SerializeValueMap(m53, #m53, v); \
		CarpJsonTemplate::SerializeValueMap(m54, #m54, v); \
		CarpJsonTemplate::SerializeValueMap(m55, #m55, v); \
		CarpJsonTemplate::SerializeValueMap(m56, #m56, v); \
		CarpJsonTemplate::SerializeValueMap(m57, #m57, v); \
		CarpJsonTemplate::SerializeValueMap(m58, #m58, v); \
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
		CarpJsonTemplate::DeserializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::DeserializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::DeserializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::DeserializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::DeserializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::DeserializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::DeserializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::DeserializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::DeserializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::DeserializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::DeserializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::DeserializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::DeserializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::DeserializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::DeserializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::DeserializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::DeserializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::DeserializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::DeserializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::DeserializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::DeserializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::DeserializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::DeserializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::DeserializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::DeserializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::DeserializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::DeserializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::DeserializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::DeserializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::DeserializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::DeserializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::DeserializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::DeserializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::DeserializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::DeserializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::DeserializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::DeserializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::DeserializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::DeserializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::DeserializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::DeserializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::DeserializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::DeserializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::DeserializeValueMap(m53, #m53, v); \
		CarpJsonTemplate::DeserializeValueMap(m54, #m54, v); \
		CarpJsonTemplate::DeserializeValueMap(m55, #m55, v); \
		CarpJsonTemplate::DeserializeValueMap(m56, #m56, v); \
		CarpJsonTemplate::DeserializeValueMap(m57, #m57, v); \
		CarpJsonTemplate::DeserializeValueMap(m58, #m58, v); \
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
			s_list.push_back(#m10); \
			s_list.push_back(#m11); \
			s_list.push_back(#m12); \
			s_list.push_back(#m13); \
			s_list.push_back(#m14); \
			s_list.push_back(#m15); \
			s_list.push_back(#m16); \
			s_list.push_back(#m17); \
			s_list.push_back(#m18); \
			s_list.push_back(#m19); \
			s_list.push_back(#m20); \
			s_list.push_back(#m21); \
			s_list.push_back(#m22); \
			s_list.push_back(#m23); \
			s_list.push_back(#m24); \
			s_list.push_back(#m25); \
			s_list.push_back(#m26); \
			s_list.push_back(#m27); \
			s_list.push_back(#m28); \
			s_list.push_back(#m29); \
			s_list.push_back(#m30); \
			s_list.push_back(#m31); \
			s_list.push_back(#m32); \
			s_list.push_back(#m33); \
			s_list.push_back(#m34); \
			s_list.push_back(#m35); \
			s_list.push_back(#m36); \
			s_list.push_back(#m37); \
			s_list.push_back(#m38); \
			s_list.push_back(#m39); \
			s_list.push_back(#m40); \
			s_list.push_back(#m41); \
			s_list.push_back(#m42); \
			s_list.push_back(#m43); \
			s_list.push_back(#m44); \
			s_list.push_back(#m45); \
			s_list.push_back(#m46); \
			s_list.push_back(#m47); \
			s_list.push_back(#m48); \
			s_list.push_back(#m49); \
			s_list.push_back(#m50); \
			s_list.push_back(#m51); \
			s_list.push_back(#m52); \
			s_list.push_back(#m53); \
			s_list.push_back(#m54); \
			s_list.push_back(#m55); \
			s_list.push_back(#m56); \
			s_list.push_back(#m57); \
			s_list.push_back(#m58); \
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
			s_list.push_back(#t10); \
			s_list.push_back(#t11); \
			s_list.push_back(#t12); \
			s_list.push_back(#t13); \
			s_list.push_back(#t14); \
			s_list.push_back(#t15); \
			s_list.push_back(#t16); \
			s_list.push_back(#t17); \
			s_list.push_back(#t18); \
			s_list.push_back(#t19); \
			s_list.push_back(#t20); \
			s_list.push_back(#t21); \
			s_list.push_back(#t22); \
			s_list.push_back(#t23); \
			s_list.push_back(#t24); \
			s_list.push_back(#t25); \
			s_list.push_back(#t26); \
			s_list.push_back(#t27); \
			s_list.push_back(#t28); \
			s_list.push_back(#t29); \
			s_list.push_back(#t30); \
			s_list.push_back(#t31); \
			s_list.push_back(#t32); \
			s_list.push_back(#t33); \
			s_list.push_back(#t34); \
			s_list.push_back(#t35); \
			s_list.push_back(#t36); \
			s_list.push_back(#t37); \
			s_list.push_back(#t38); \
			s_list.push_back(#t39); \
			s_list.push_back(#t40); \
			s_list.push_back(#t41); \
			s_list.push_back(#t42); \
			s_list.push_back(#t43); \
			s_list.push_back(#t44); \
			s_list.push_back(#t45); \
			s_list.push_back(#t46); \
			s_list.push_back(#t47); \
			s_list.push_back(#t48); \
			s_list.push_back(#t49); \
			s_list.push_back(#t50); \
			s_list.push_back(#t51); \
			s_list.push_back(#t52); \
			s_list.push_back(#t53); \
			s_list.push_back(#t54); \
			s_list.push_back(#t55); \
			s_list.push_back(#t56); \
			s_list.push_back(#t57); \
			s_list.push_back(#t58); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8(), m9(), m10(), m11(), m12(), m13(), m14(), m15(), m16(), m17(), m18(), m19(), m20(), m21(), m22(), m23(), m24(), m25(), m26(), m27(), m28(), m29(), m30(), m31(), m32(), m33(), m34(), m35(), m36(), m37(), m38(), m39(), m40(), m41(), m42(), m43(), m44(), m45(), m46(), m47(), m48(), m49(), m50(), m51(), m52(), m53(), m54(), m55(), m56(), m57(), m58() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24; t25 m25; t26 m26; t27 m27; t28 m28; t29 m29; t30 m30; t31 m31; t32 m32; t33 m33; t34 m34; t35 m35; t36 m36; t37 m37; t38 m38; t39 m39; t40 m40; t41 m41; t42 m42; t43 m43; t44 m44; t45 m45; t46 m46; t47 m47; t48 m48; t49 m49; t50 m50; t51 m51; t52 m52; t53 m53; t54 m54; t55 m55; t56 m56; t57 m57; t58 m58; \
}
#define CARP_JSON_MACRO_121(Name, t0, m0, t1, m1, t2, m2, t3, m3, t4, m4, t5, m5, t6, m6, t7, m7, t8, m8, t9, m9, t10, m10, t11, m11, t12, m12, t13, m13, t14, m14, t15, m15, t16, m16, t17, m17, t18, m18, t19, m19, t20, m20, t21, m21, t22, m22, t23, m23, t24, m24, t25, m25, t26, m26, t27, m27, t28, m28, t29, m29, t30, m30, t31, m31, t32, m32, t33, m33, t34, m34, t35, m35, t36, m36, t37, m37, t38, m38, t39, m39, t40, m40, t41, m41, t42, m42, t43, m43, t44, m44, t45, m45, t46, m46, t47, m47, t48, m48, t49, m49, t50, m50, t51, m51, t52, m52, t53, m53, t54, m54, t55, m55, t56, m56, t57, m57, t58, m58, t59, m59) \
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
		CarpJsonTemplate::Serialize(m10, #m10, (int)sizeof(#m10)-1, v, a); \
		CarpJsonTemplate::Serialize(m11, #m11, (int)sizeof(#m11)-1, v, a); \
		CarpJsonTemplate::Serialize(m12, #m12, (int)sizeof(#m12)-1, v, a); \
		CarpJsonTemplate::Serialize(m13, #m13, (int)sizeof(#m13)-1, v, a); \
		CarpJsonTemplate::Serialize(m14, #m14, (int)sizeof(#m14)-1, v, a); \
		CarpJsonTemplate::Serialize(m15, #m15, (int)sizeof(#m15)-1, v, a); \
		CarpJsonTemplate::Serialize(m16, #m16, (int)sizeof(#m16)-1, v, a); \
		CarpJsonTemplate::Serialize(m17, #m17, (int)sizeof(#m17)-1, v, a); \
		CarpJsonTemplate::Serialize(m18, #m18, (int)sizeof(#m18)-1, v, a); \
		CarpJsonTemplate::Serialize(m19, #m19, (int)sizeof(#m19)-1, v, a); \
		CarpJsonTemplate::Serialize(m20, #m20, (int)sizeof(#m20)-1, v, a); \
		CarpJsonTemplate::Serialize(m21, #m21, (int)sizeof(#m21)-1, v, a); \
		CarpJsonTemplate::Serialize(m22, #m22, (int)sizeof(#m22)-1, v, a); \
		CarpJsonTemplate::Serialize(m23, #m23, (int)sizeof(#m23)-1, v, a); \
		CarpJsonTemplate::Serialize(m24, #m24, (int)sizeof(#m24)-1, v, a); \
		CarpJsonTemplate::Serialize(m25, #m25, (int)sizeof(#m25)-1, v, a); \
		CarpJsonTemplate::Serialize(m26, #m26, (int)sizeof(#m26)-1, v, a); \
		CarpJsonTemplate::Serialize(m27, #m27, (int)sizeof(#m27)-1, v, a); \
		CarpJsonTemplate::Serialize(m28, #m28, (int)sizeof(#m28)-1, v, a); \
		CarpJsonTemplate::Serialize(m29, #m29, (int)sizeof(#m29)-1, v, a); \
		CarpJsonTemplate::Serialize(m30, #m30, (int)sizeof(#m30)-1, v, a); \
		CarpJsonTemplate::Serialize(m31, #m31, (int)sizeof(#m31)-1, v, a); \
		CarpJsonTemplate::Serialize(m32, #m32, (int)sizeof(#m32)-1, v, a); \
		CarpJsonTemplate::Serialize(m33, #m33, (int)sizeof(#m33)-1, v, a); \
		CarpJsonTemplate::Serialize(m34, #m34, (int)sizeof(#m34)-1, v, a); \
		CarpJsonTemplate::Serialize(m35, #m35, (int)sizeof(#m35)-1, v, a); \
		CarpJsonTemplate::Serialize(m36, #m36, (int)sizeof(#m36)-1, v, a); \
		CarpJsonTemplate::Serialize(m37, #m37, (int)sizeof(#m37)-1, v, a); \
		CarpJsonTemplate::Serialize(m38, #m38, (int)sizeof(#m38)-1, v, a); \
		CarpJsonTemplate::Serialize(m39, #m39, (int)sizeof(#m39)-1, v, a); \
		CarpJsonTemplate::Serialize(m40, #m40, (int)sizeof(#m40)-1, v, a); \
		CarpJsonTemplate::Serialize(m41, #m41, (int)sizeof(#m41)-1, v, a); \
		CarpJsonTemplate::Serialize(m42, #m42, (int)sizeof(#m42)-1, v, a); \
		CarpJsonTemplate::Serialize(m43, #m43, (int)sizeof(#m43)-1, v, a); \
		CarpJsonTemplate::Serialize(m44, #m44, (int)sizeof(#m44)-1, v, a); \
		CarpJsonTemplate::Serialize(m45, #m45, (int)sizeof(#m45)-1, v, a); \
		CarpJsonTemplate::Serialize(m46, #m46, (int)sizeof(#m46)-1, v, a); \
		CarpJsonTemplate::Serialize(m47, #m47, (int)sizeof(#m47)-1, v, a); \
		CarpJsonTemplate::Serialize(m48, #m48, (int)sizeof(#m48)-1, v, a); \
		CarpJsonTemplate::Serialize(m49, #m49, (int)sizeof(#m49)-1, v, a); \
		CarpJsonTemplate::Serialize(m50, #m50, (int)sizeof(#m50)-1, v, a); \
		CarpJsonTemplate::Serialize(m51, #m51, (int)sizeof(#m51)-1, v, a); \
		CarpJsonTemplate::Serialize(m52, #m52, (int)sizeof(#m52)-1, v, a); \
		CarpJsonTemplate::Serialize(m53, #m53, (int)sizeof(#m53)-1, v, a); \
		CarpJsonTemplate::Serialize(m54, #m54, (int)sizeof(#m54)-1, v, a); \
		CarpJsonTemplate::Serialize(m55, #m55, (int)sizeof(#m55)-1, v, a); \
		CarpJsonTemplate::Serialize(m56, #m56, (int)sizeof(#m56)-1, v, a); \
		CarpJsonTemplate::Serialize(m57, #m57, (int)sizeof(#m57)-1, v, a); \
		CarpJsonTemplate::Serialize(m58, #m58, (int)sizeof(#m58)-1, v, a); \
		CarpJsonTemplate::Serialize(m59, #m59, (int)sizeof(#m59)-1, v, a); \
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
		CarpJsonTemplate::Deserialize(m10, #m10, (int)sizeof(#m10)-1, v); \
		CarpJsonTemplate::Deserialize(m11, #m11, (int)sizeof(#m11)-1, v); \
		CarpJsonTemplate::Deserialize(m12, #m12, (int)sizeof(#m12)-1, v); \
		CarpJsonTemplate::Deserialize(m13, #m13, (int)sizeof(#m13)-1, v); \
		CarpJsonTemplate::Deserialize(m14, #m14, (int)sizeof(#m14)-1, v); \
		CarpJsonTemplate::Deserialize(m15, #m15, (int)sizeof(#m15)-1, v); \
		CarpJsonTemplate::Deserialize(m16, #m16, (int)sizeof(#m16)-1, v); \
		CarpJsonTemplate::Deserialize(m17, #m17, (int)sizeof(#m17)-1, v); \
		CarpJsonTemplate::Deserialize(m18, #m18, (int)sizeof(#m18)-1, v); \
		CarpJsonTemplate::Deserialize(m19, #m19, (int)sizeof(#m19)-1, v); \
		CarpJsonTemplate::Deserialize(m20, #m20, (int)sizeof(#m20)-1, v); \
		CarpJsonTemplate::Deserialize(m21, #m21, (int)sizeof(#m21)-1, v); \
		CarpJsonTemplate::Deserialize(m22, #m22, (int)sizeof(#m22)-1, v); \
		CarpJsonTemplate::Deserialize(m23, #m23, (int)sizeof(#m23)-1, v); \
		CarpJsonTemplate::Deserialize(m24, #m24, (int)sizeof(#m24)-1, v); \
		CarpJsonTemplate::Deserialize(m25, #m25, (int)sizeof(#m25)-1, v); \
		CarpJsonTemplate::Deserialize(m26, #m26, (int)sizeof(#m26)-1, v); \
		CarpJsonTemplate::Deserialize(m27, #m27, (int)sizeof(#m27)-1, v); \
		CarpJsonTemplate::Deserialize(m28, #m28, (int)sizeof(#m28)-1, v); \
		CarpJsonTemplate::Deserialize(m29, #m29, (int)sizeof(#m29)-1, v); \
		CarpJsonTemplate::Deserialize(m30, #m30, (int)sizeof(#m30)-1, v); \
		CarpJsonTemplate::Deserialize(m31, #m31, (int)sizeof(#m31)-1, v); \
		CarpJsonTemplate::Deserialize(m32, #m32, (int)sizeof(#m32)-1, v); \
		CarpJsonTemplate::Deserialize(m33, #m33, (int)sizeof(#m33)-1, v); \
		CarpJsonTemplate::Deserialize(m34, #m34, (int)sizeof(#m34)-1, v); \
		CarpJsonTemplate::Deserialize(m35, #m35, (int)sizeof(#m35)-1, v); \
		CarpJsonTemplate::Deserialize(m36, #m36, (int)sizeof(#m36)-1, v); \
		CarpJsonTemplate::Deserialize(m37, #m37, (int)sizeof(#m37)-1, v); \
		CarpJsonTemplate::Deserialize(m38, #m38, (int)sizeof(#m38)-1, v); \
		CarpJsonTemplate::Deserialize(m39, #m39, (int)sizeof(#m39)-1, v); \
		CarpJsonTemplate::Deserialize(m40, #m40, (int)sizeof(#m40)-1, v); \
		CarpJsonTemplate::Deserialize(m41, #m41, (int)sizeof(#m41)-1, v); \
		CarpJsonTemplate::Deserialize(m42, #m42, (int)sizeof(#m42)-1, v); \
		CarpJsonTemplate::Deserialize(m43, #m43, (int)sizeof(#m43)-1, v); \
		CarpJsonTemplate::Deserialize(m44, #m44, (int)sizeof(#m44)-1, v); \
		CarpJsonTemplate::Deserialize(m45, #m45, (int)sizeof(#m45)-1, v); \
		CarpJsonTemplate::Deserialize(m46, #m46, (int)sizeof(#m46)-1, v); \
		CarpJsonTemplate::Deserialize(m47, #m47, (int)sizeof(#m47)-1, v); \
		CarpJsonTemplate::Deserialize(m48, #m48, (int)sizeof(#m48)-1, v); \
		CarpJsonTemplate::Deserialize(m49, #m49, (int)sizeof(#m49)-1, v); \
		CarpJsonTemplate::Deserialize(m50, #m50, (int)sizeof(#m50)-1, v); \
		CarpJsonTemplate::Deserialize(m51, #m51, (int)sizeof(#m51)-1, v); \
		CarpJsonTemplate::Deserialize(m52, #m52, (int)sizeof(#m52)-1, v); \
		CarpJsonTemplate::Deserialize(m53, #m53, (int)sizeof(#m53)-1, v); \
		CarpJsonTemplate::Deserialize(m54, #m54, (int)sizeof(#m54)-1, v); \
		CarpJsonTemplate::Deserialize(m55, #m55, (int)sizeof(#m55)-1, v); \
		CarpJsonTemplate::Deserialize(m56, #m56, (int)sizeof(#m56)-1, v); \
		CarpJsonTemplate::Deserialize(m57, #m57, (int)sizeof(#m57)-1, v); \
		CarpJsonTemplate::Deserialize(m58, #m58, (int)sizeof(#m58)-1, v); \
		CarpJsonTemplate::Deserialize(m59, #m59, (int)sizeof(#m59)-1, v); \
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
		CarpJsonTemplate::SerializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::SerializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::SerializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::SerializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::SerializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::SerializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::SerializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::SerializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::SerializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::SerializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::SerializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::SerializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::SerializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::SerializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::SerializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::SerializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::SerializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::SerializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::SerializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::SerializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::SerializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::SerializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::SerializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::SerializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::SerializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::SerializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::SerializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::SerializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::SerializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::SerializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::SerializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::SerializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::SerializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::SerializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::SerializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::SerializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::SerializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::SerializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::SerializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::SerializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::SerializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::SerializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::SerializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::SerializeValueMap(m53, #m53, v); \
		CarpJsonTemplate::SerializeValueMap(m54, #m54, v); \
		CarpJsonTemplate::SerializeValueMap(m55, #m55, v); \
		CarpJsonTemplate::SerializeValueMap(m56, #m56, v); \
		CarpJsonTemplate::SerializeValueMap(m57, #m57, v); \
		CarpJsonTemplate::SerializeValueMap(m58, #m58, v); \
		CarpJsonTemplate::SerializeValueMap(m59, #m59, v); \
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
		CarpJsonTemplate::DeserializeValueMap(m10, #m10, v); \
		CarpJsonTemplate::DeserializeValueMap(m11, #m11, v); \
		CarpJsonTemplate::DeserializeValueMap(m12, #m12, v); \
		CarpJsonTemplate::DeserializeValueMap(m13, #m13, v); \
		CarpJsonTemplate::DeserializeValueMap(m14, #m14, v); \
		CarpJsonTemplate::DeserializeValueMap(m15, #m15, v); \
		CarpJsonTemplate::DeserializeValueMap(m16, #m16, v); \
		CarpJsonTemplate::DeserializeValueMap(m17, #m17, v); \
		CarpJsonTemplate::DeserializeValueMap(m18, #m18, v); \
		CarpJsonTemplate::DeserializeValueMap(m19, #m19, v); \
		CarpJsonTemplate::DeserializeValueMap(m20, #m20, v); \
		CarpJsonTemplate::DeserializeValueMap(m21, #m21, v); \
		CarpJsonTemplate::DeserializeValueMap(m22, #m22, v); \
		CarpJsonTemplate::DeserializeValueMap(m23, #m23, v); \
		CarpJsonTemplate::DeserializeValueMap(m24, #m24, v); \
		CarpJsonTemplate::DeserializeValueMap(m25, #m25, v); \
		CarpJsonTemplate::DeserializeValueMap(m26, #m26, v); \
		CarpJsonTemplate::DeserializeValueMap(m27, #m27, v); \
		CarpJsonTemplate::DeserializeValueMap(m28, #m28, v); \
		CarpJsonTemplate::DeserializeValueMap(m29, #m29, v); \
		CarpJsonTemplate::DeserializeValueMap(m30, #m30, v); \
		CarpJsonTemplate::DeserializeValueMap(m31, #m31, v); \
		CarpJsonTemplate::DeserializeValueMap(m32, #m32, v); \
		CarpJsonTemplate::DeserializeValueMap(m33, #m33, v); \
		CarpJsonTemplate::DeserializeValueMap(m34, #m34, v); \
		CarpJsonTemplate::DeserializeValueMap(m35, #m35, v); \
		CarpJsonTemplate::DeserializeValueMap(m36, #m36, v); \
		CarpJsonTemplate::DeserializeValueMap(m37, #m37, v); \
		CarpJsonTemplate::DeserializeValueMap(m38, #m38, v); \
		CarpJsonTemplate::DeserializeValueMap(m39, #m39, v); \
		CarpJsonTemplate::DeserializeValueMap(m40, #m40, v); \
		CarpJsonTemplate::DeserializeValueMap(m41, #m41, v); \
		CarpJsonTemplate::DeserializeValueMap(m42, #m42, v); \
		CarpJsonTemplate::DeserializeValueMap(m43, #m43, v); \
		CarpJsonTemplate::DeserializeValueMap(m44, #m44, v); \
		CarpJsonTemplate::DeserializeValueMap(m45, #m45, v); \
		CarpJsonTemplate::DeserializeValueMap(m46, #m46, v); \
		CarpJsonTemplate::DeserializeValueMap(m47, #m47, v); \
		CarpJsonTemplate::DeserializeValueMap(m48, #m48, v); \
		CarpJsonTemplate::DeserializeValueMap(m49, #m49, v); \
		CarpJsonTemplate::DeserializeValueMap(m50, #m50, v); \
		CarpJsonTemplate::DeserializeValueMap(m51, #m51, v); \
		CarpJsonTemplate::DeserializeValueMap(m52, #m52, v); \
		CarpJsonTemplate::DeserializeValueMap(m53, #m53, v); \
		CarpJsonTemplate::DeserializeValueMap(m54, #m54, v); \
		CarpJsonTemplate::DeserializeValueMap(m55, #m55, v); \
		CarpJsonTemplate::DeserializeValueMap(m56, #m56, v); \
		CarpJsonTemplate::DeserializeValueMap(m57, #m57, v); \
		CarpJsonTemplate::DeserializeValueMap(m58, #m58, v); \
		CarpJsonTemplate::DeserializeValueMap(m59, #m59, v); \
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
			s_list.push_back(#m10); \
			s_list.push_back(#m11); \
			s_list.push_back(#m12); \
			s_list.push_back(#m13); \
			s_list.push_back(#m14); \
			s_list.push_back(#m15); \
			s_list.push_back(#m16); \
			s_list.push_back(#m17); \
			s_list.push_back(#m18); \
			s_list.push_back(#m19); \
			s_list.push_back(#m20); \
			s_list.push_back(#m21); \
			s_list.push_back(#m22); \
			s_list.push_back(#m23); \
			s_list.push_back(#m24); \
			s_list.push_back(#m25); \
			s_list.push_back(#m26); \
			s_list.push_back(#m27); \
			s_list.push_back(#m28); \
			s_list.push_back(#m29); \
			s_list.push_back(#m30); \
			s_list.push_back(#m31); \
			s_list.push_back(#m32); \
			s_list.push_back(#m33); \
			s_list.push_back(#m34); \
			s_list.push_back(#m35); \
			s_list.push_back(#m36); \
			s_list.push_back(#m37); \
			s_list.push_back(#m38); \
			s_list.push_back(#m39); \
			s_list.push_back(#m40); \
			s_list.push_back(#m41); \
			s_list.push_back(#m42); \
			s_list.push_back(#m43); \
			s_list.push_back(#m44); \
			s_list.push_back(#m45); \
			s_list.push_back(#m46); \
			s_list.push_back(#m47); \
			s_list.push_back(#m48); \
			s_list.push_back(#m49); \
			s_list.push_back(#m50); \
			s_list.push_back(#m51); \
			s_list.push_back(#m52); \
			s_list.push_back(#m53); \
			s_list.push_back(#m54); \
			s_list.push_back(#m55); \
			s_list.push_back(#m56); \
			s_list.push_back(#m57); \
			s_list.push_back(#m58); \
			s_list.push_back(#m59); \
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
			s_list.push_back(#t10); \
			s_list.push_back(#t11); \
			s_list.push_back(#t12); \
			s_list.push_back(#t13); \
			s_list.push_back(#t14); \
			s_list.push_back(#t15); \
			s_list.push_back(#t16); \
			s_list.push_back(#t17); \
			s_list.push_back(#t18); \
			s_list.push_back(#t19); \
			s_list.push_back(#t20); \
			s_list.push_back(#t21); \
			s_list.push_back(#t22); \
			s_list.push_back(#t23); \
			s_list.push_back(#t24); \
			s_list.push_back(#t25); \
			s_list.push_back(#t26); \
			s_list.push_back(#t27); \
			s_list.push_back(#t28); \
			s_list.push_back(#t29); \
			s_list.push_back(#t30); \
			s_list.push_back(#t31); \
			s_list.push_back(#t32); \
			s_list.push_back(#t33); \
			s_list.push_back(#t34); \
			s_list.push_back(#t35); \
			s_list.push_back(#t36); \
			s_list.push_back(#t37); \
			s_list.push_back(#t38); \
			s_list.push_back(#t39); \
			s_list.push_back(#t40); \
			s_list.push_back(#t41); \
			s_list.push_back(#t42); \
			s_list.push_back(#t43); \
			s_list.push_back(#t44); \
			s_list.push_back(#t45); \
			s_list.push_back(#t46); \
			s_list.push_back(#t47); \
			s_list.push_back(#t48); \
			s_list.push_back(#t49); \
			s_list.push_back(#t50); \
			s_list.push_back(#t51); \
			s_list.push_back(#t52); \
			s_list.push_back(#t53); \
			s_list.push_back(#t54); \
			s_list.push_back(#t55); \
			s_list.push_back(#t56); \
			s_list.push_back(#t57); \
			s_list.push_back(#t58); \
			s_list.push_back(#t59); \
		} return s_list; \
	} \
	virtual void Reset() { *this = Name(); } \
	virtual ~Name() { } \
	Name() : m0(), m1(), m2(), m3(), m4(), m5(), m6(), m7(), m8(), m9(), m10(), m11(), m12(), m13(), m14(), m15(), m16(), m17(), m18(), m19(), m20(), m21(), m22(), m23(), m24(), m25(), m26(), m27(), m28(), m29(), m30(), m31(), m32(), m33(), m34(), m35(), m36(), m37(), m38(), m39(), m40(), m41(), m42(), m43(), m44(), m45(), m46(), m47(), m48(), m49(), m50(), m51(), m52(), m53(), m54(), m55(), m56(), m57(), m58(), m59() {} \
	t0 m0; t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24; t25 m25; t26 m26; t27 m27; t28 m28; t29 m29; t30 m30; t31 m31; t32 m32; t33 m33; t34 m34; t35 m35; t36 m36; t37 m37; t38 m38; t39 m39; t40 m40; t41 m41; t42 m42; t43 m43; t44 m44; t45 m45; t46 m46; t47 m47; t48 m48; t49 m49; t50 m50; t51 m51; t52 m52; t53 m53; t54 m54; t55 m55; t56 m56; t57 m57; t58 m58; t59 m59; \
}

#endif // _ALITTLE_JSON_6_H_
