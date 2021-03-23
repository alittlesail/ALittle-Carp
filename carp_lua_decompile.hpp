#ifndef CARP_LUA_DECOMPILE_INCLUDED
#define CARP_LUA_DECOMPILE_INCLUDED

#include <string>
#include <set>

#include "carp_lua.hpp"

#ifdef _WIN32
#ifdef _DEBUG
#pragma comment(lib, "luad.lib")
#else
#pragma comment(lib, "lua.lib")
#endif
#endif

extern "C" {
#include <lobject.h>
#include <lstate.h>
#include <lundump.h>
#include <lopcodes.h>
}

#include "carp_log.hpp"
#include "carp_rwops_bind.hpp"
#include "carp_string.hpp"

class CarpLuaDecompile
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<CarpLuaDecompile>("CarpLuaDecompile")
			.addConstructor<void(*)()>()
			.addFunction("Decompile", &CarpLuaDecompile::Decompile)
			.endClass()
			.endNamespace();
	}
public:
	CarpLuaDecompile() {}
	virtual ~CarpLuaDecompile() { }

public:
	bool Decompile(const char* lua_path, const char* out_path)
	{
		// 加载文件
		std::vector<char> content;
		if (!CarpRWops::LoadFile(lua_path, false, content))
		{
			CARP_ERROR("can't find lua file:" << lua_path);
			return false;
		}

		// 按行数
		std::string split_content(content.data(), content.size());
		CarpString::Split(split_content, "\n", false, m_lines);
		for (auto& line : m_lines)
		{
			if (!line.empty() && line.back() == '\r')
				line.pop_back();
		}

		// 打开文件
		m_out_file = CarpRWops::OpenFile(out_path, "wb");
		if (m_out_file == nullptr)
		{
			CARP_ERROR("out file open failed:" << out_path);
			return false;
		}

		// 创建虚拟机
		auto* L = ::luaL_newstate();
		if (L == nullptr)
		{
			CARP_ERROR("luaL_newstate failed!");
			return false;
		}

		bool result = true;
		do
		{
			// 加载脚本
			if (luaL_loadbuffer(L, content.data(), content.size(), lua_path) != 0)
			{
				CARP_ERROR("luaL_loadbuffer failed!");
				result = false;
				break;
			}

			// 获取闭包
			LClosure* f = clLvalue(s2v(L->top - 1));  /* get newly created function */
			if (!lua_isfunction(L, -1))
			{
				CARP_ERROR("luaL_loadbuffer result is not a function!");
				result = false;
				break;
			}
			auto* p = f->p;

			// Write("HEADER\n");
			// DumpHeader("\t");

			Write("FUNCTION\n");
			DumpFunction("\t", p);

		} while (false);

		SDL_RWclose(m_out_file);
		m_out_file = nullptr;
		lua_close(L);
		return result;
	}

	// 这个代码是从ldebug.c中拷贝出来的
	static int GetBaseLine(const Proto* f, int pc, int* basepc) {
		if (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
			*basepc = -1;  /* start from the beginning */
			return f->linedefined;
		}
		else {
			unsigned int i;
			if (pc >= f->abslineinfo[f->sizeabslineinfo - 1].pc)
				i = f->sizeabslineinfo - 1;  /* instruction is after last saved one */
			else {  /* binary search */
				unsigned int j = f->sizeabslineinfo - 1;  /* pc < anchorlines[j] */
				i = 0;  /* abslineinfo[i] <= pc */
				while (i < j - 1) {
					unsigned int m = (j + i) / 2;
					if (pc >= f->abslineinfo[m].pc)
						i = m;
					else
						j = m;
				}
			}
			*basepc = f->abslineinfo[i].pc;
			return f->abslineinfo[i].line;
		}
	}

	static int GetFuncLine(const Proto* f, int pc)
	{
		if (f->lineinfo == NULL)  /* no debug information? */
			return -1;
		else {
			int basepc;
			int baseline = GetBaseLine(f, pc, &basepc);
			while (basepc++ < pc) {  /* walk until given instruction */
				lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
				baseline += f->lineinfo[basepc];  /* correct line */
			}
			return baseline;
		}
	}

private:
	void DumpHeader(const std::string& tab)
	{
		// 输出签名
		WriteKeyValue(tab + "LUA_SIGNATURE", LUA_SIGNATURE);
		// 输出luac版本
		WriteKeyValue(tab + "LUAC_VERSION", LUAC_VERSION);
		// 输出luac格式
		WriteKeyValue(tab + "LUAC_FORMAT", LUAC_FORMAT);
		// 输出luac数据头
		WriteKeyValue(tab + "LUAC_DATA", LUAC_DATA);
		// 输出指令大小
		WriteKeyValue(tab + "sizeof(Instruction)", sizeof(Instruction));
		// 输出Integer大小
		WriteKeyValue(tab + "sizeof(lua_Integer)", sizeof(lua_Integer));
		// 输出Number大小
		WriteKeyValue(tab + "sizeof(lua_Number)", sizeof(lua_Number));
		// 输出Integer数值
		WriteKeyValue(tab + "LUAC_INT", LUAC_INT);
		// 输出Number数值
		WriteKeyValue(tab + "LUAC_NUM", LUAC_NUM);
	}

	void DumpFunction(const std::string& tab, const Proto* p)
	{
		// 输出文件路径
		WriteKeyValue(tab + "source", getstr(p->source));
		// 输出起始行
		WriteKeyValue(tab + "linedefined", p->linedefined);
		// 输出结束行
		WriteKeyValue(tab + "lastlinedefined", p->lastlinedefined);
		// 输出确定参数个数
		WriteKeyValue(tab + "numparams", p->numparams);
		// 输出是否有不定长参数
		WriteKeyValue(tab + "is_vararg", p->is_vararg);
		// 输出最大栈
		WriteKeyValue(tab + "maxstacksize", p->maxstacksize);

		Write("\n");

		// 输出指令
		Write(tab + "INSTRUCTIONS\n");
		DumpCode(tab + "\t", p);

		// 输出常量列表
		Write(tab + "CONSTANTS\n");
		DumpConstants(tab + "\t", p);

		// 输出局部变量
		Write(tab + "LOCVARS\n");
		DumpLocvars(tab + "\t", p);

		// 输出闭包参数
		Write(tab + "UPVALUES\n");
		DumpUpvalues(tab + "\t", p);

		// 输出内部函数
		for (int i = 0; i < p->sizep; i++)
		{
			Write(tab + "FUNCTION\n");
			DumpFunction(tab + "\t", p->p[i]);
		}
	}

	void DumpCode(const std::string& tab, const Proto* f)
	{
		auto* CONSTANTS = f->k;
		auto* UPVALUES = f->upvalues;

		int last_line = -1;
		for (int index = 0; index < f->sizecode; index++)
		{
			int cur_line = GetFuncLine(f, index) - 1;
			if (last_line != cur_line)
			{
				if (cur_line >= 0 && cur_line < m_lines.size())
					Write(m_lines[cur_line] + "\n");
				last_line = cur_line;
			}

			Write(tab + std::to_string(index) + "\t\t");

			Instruction i = f->code[index];

			const auto a = GETARG_A(i); const auto A = std::to_string(a);
			const auto b = GETARG_B(i); const auto B = std::to_string(b);
			const auto c = GETARG_C(i); const auto C = std::to_string(c);
			const auto bx = GETARG_Bx(i); const auto BX = std::to_string(bx);
			const auto sb = GETARG_sB(i); const auto SB = std::to_string(sb);
			const auto sbx = GETARG_sBx(i); const auto SBX = std::to_string(sbx);
			const auto sj = GETARG_sJ(i); const auto SJ = std::to_string(sj);
			const auto sc = GETARG_sC(i); const auto SC = std::to_string(sc);
			const auto k = GETARG_k(i); const auto K = std::to_string(k);

			std::string RKC;

			switch(GET_OPCODE(i))
			{
			// R[A] := R[B]
			case OP_MOVE:
				Write("OP_MOVE       R[" + A + "] := R[" + B + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index));
				break;
			// R[A] := sBx
			case OP_LOADI:
				Write("OP_LOADI      R[" + A + "] := " + SBX);
				Write("; " + RegisterName(f, a, index) + " := " + SBX);
				break;
			// R[A] := (lua_Number)sBx
			case OP_LOADF:
				Write("OP_LOADF      R[" + A + "] := " + SBX);
				Write("; " + RegisterName(f, a, index) + " := " + SBX);
				break;
			// R[A] := K[Bx]
			case OP_LOADK:
				Write("OP_LOADK      R[" + A + "] := K[" + BX + "]");
				Write("; " + RegisterName(f, a, index) + " := " + ConstantValue(CONSTANTS, bx));
				break;
			// R[A] := K[extra arg]
			case OP_LOADKX:
				Write("OP_LOADKX     R[" + A + "] := K[" + std::to_string(GETARG_Ax(f->code[index + 1])) + "]");
				Write("; " + RegisterName(f, a, index) + " := " + ConstantValue(CONSTANTS, GETARG_Ax(f->code[index + 1])));
				break;
				// R[A] := false
			case OP_LOADFALSE:
				Write("OP_LOADFALSE  R[" + A + "] := false");
				Write("; " + RegisterName(f, a, index) + " := false");
				break;
			// R[A] := false; pc++
			case OP_LFALSESKIP:
				Write("OP_LFALSESKIP R[" + A + "] := false pc(" + std::to_string(index + 1) + ")++");
				Write("; " + RegisterName(f, a, index) + " := false pc(" + std::to_string(index + 1) + ")++");
				break;
			// R[A] := true
			case OP_LOADTRUE:
				Write("OP_LOADTRUE   R[" + A + "] := true");
				Write("; " + RegisterName(f, a, index) + " := true");
				break;
			// R[A], R[A+1], ..., R[A+B] := nil
			case OP_LOADNIL:
				Write("OP_LOADNIL    R[" + A + " ... " + A + " + " + B + "] := nil");
				break;
			// R[A] := UpValue[B]
			case OP_GETUPVAL:
				Write("OP_GETUPVAL   R[" + A + "] := U[" + B + "]");
				Write("; " + RegisterName(f, a, index) + " := " + getstr(UPVALUES[b].name));
				break;
			// UpValue[B] := R[A]
			case OP_SETUPVAL:
				Write("OP_SETUPVAL   U[" + B + "] := R[" + A + "]");
				Write(std::string() + "; " + getstr(UPVALUES[b].name) + " := " + RegisterName(f, a, index));
				break;
			// R[A] := UpValue[B][K[C]:string]
			case OP_GETTABUP:
				Write("OP_GETTABUP   R[" + A + "] := U[" + B + "][K[" + C + "]:string]");
				Write("; " + RegisterName(f, a, index) + " := " + getstr(UPVALUES[b].name) + "." + ConstantValue(CONSTANTS, c));
				break;
			// R[A] := R[B][R[C]]
			case OP_GETTABLE:
				Write("OP_GETTABLE   R[" + A + "] := R[" + B + "][R[" + C + "]]");
				Write(";   " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + "[" + RegisterName(f, c, index) + "]");
				break;
			// R[A] := R[B][C]
			case OP_GETI:
				Write("OP_GETI       R[" + A + "] := R[" + B + "][" + C + "]");
				Write(";   " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + "[" + C + "]");
				break;
			// R[A] := R[B][K[C]:string]
			case OP_GETFIELD:
				Write("OP_GETFIELD   R[" + A + "] := R[" + B + "][K[" + C + "]:string]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + "." + ConstantValue(CONSTANTS, c));
				break;
            // UpValue[A][K[B]:string] := RK(C)
			case OP_SETTABUP:
				Write("OP_SETTABUP   U[" + A + "][K[" + B + "]:string] := RK(" + C + ")");
				RKC = k ? ConstantValue(CONSTANTS, c) : RegisterName(f, c, index);
				Write(std::string() + "; " + getstr(UPVALUES[a].name) + "." + ConstantValue(CONSTANTS, b) + " := " + RKC);
				break;
			// R[A][R[B]] := RK(C)
			case OP_SETTABLE:
				Write("OP_SETTABLE   R[" + A + "][R[" + B + "]] := RK(" + C + ")");
				RKC = k ? ConstantValue(CONSTANTS, c) : RegisterName(f, c, index);
				Write("; " + RegisterName(f, a, index) + "[" + RegisterName(f, b, index) + "] := " + RKC);
				break;
			// R[A][B] := RK(C)
			case OP_SETI:
				Write("OP_SETI       R[" + A + "][" + B + "] := RK(" + C + ")");
				RKC = k ? ConstantValue(CONSTANTS, c) : RegisterName(f, c, index);
				Write("; " + RegisterName(f, a, index) + "[" + B + "] := " + RKC);
				break;
			// R[A][K[B]:string] := RK(C)
			case OP_SETFIELD:
				Write("OP_SETFIELD   R[" + A + "][K[" + B + "]:string] := RK(" + C + ")");
				RKC = k ? ConstantValue(CONSTANTS, c) : RegisterName(f, c, index);
				Write("; " + RegisterName(f, a, index) + "." + ConstantValue(CONSTANTS, b) + " := " + RKC);
				break;
            // R[A] := {}
			case OP_NEWTABLE:
				Write("OP_NEWTABLE   R[" + A + "] := {}");
				Write("; " + RegisterName(f, a, index) + " := {}");
				break;
			// R[A+1] := R[B]; R[A] := R[B][RK(C):string]
			case OP_SELF:
				Write("OP_SELF       R[" + A + "+1] := R[" + B + "]; R[" + A + "] := R[" + B + "][RK(" + C + "):string]");
				RKC = k ? ConstantValue(CONSTANTS, c) : RegisterName(f, c, index);
				Write("; " + RegisterName(f, a + 1, index) + " := " + RegisterName(f, b, index) + "; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + "[" + RKC + "]");
				break;
			// R[A] := R[B] + sC
			case OP_ADDI:
				Write("OP_ADDI       R[" + A + "] := R[" + B + "] + " + SC);
				Write("; " + RegisterName(f, a, index) + " : = " + RegisterName(f, b, index) + " + " + SC);
				break;
			// R[A] := R[B] + K[C]
			case OP_ADDK:
				Write("OP_ADDK       R[" + A + "] := R[" + B + "] + K[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " + " + ConstantValue(CONSTANTS, c));
				break;
			// R[A] := R[B] - K[C]
			case OP_SUBK:
				Write("OP_SUBK       R[" + A + "] := R[" + B + "] - K[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " - " + ConstantValue(CONSTANTS, c));
				break;
			// R[A] := R[B] * K[C]
			case OP_MULK:
				Write("OP_MULK       R[" + A + "] := R[" + B + "] * K[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " * " + ConstantValue(CONSTANTS, c));
				break;
			// R[A] := R[B] % K[C]
			case OP_MODK:
				Write("OP_MODK       R[" + A + "] := R[" + B + "] % K[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " % " + ConstantValue(CONSTANTS, c));
				break;
			// R[A] := R[B] ^ K[C]
			case OP_POWK:
				Write("OP_POWK       R[" + A + "] := R[" + B + "] ^ K[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " ^ " + ConstantValue(CONSTANTS, c));
				break;
			// R[A] := R[B] / K[C]
			case OP_DIVK:
				Write("OP_DIVK       R[" + A + "] := R[" + B + "] / K[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " / " + ConstantValue(CONSTANTS, c));
				break;
			// R[A] := R[B] // K[C]
			case OP_IDIVK:
				Write("OP_IDIVK      R[" + A + "] := R[" + B + "] // K[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " // " + ConstantValue(CONSTANTS, c));
				break;
            // R[A] := R[B] & K[C]:integer
			case OP_BANDK:
				Write("OP_BANDK      R[" + A + "] := R[" + B + "] & K[" + C + "]:integer");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " & " + ConstantValue(CONSTANTS, c));
				break;
			// R[A] := R[B] & K[C]:integer
			case OP_BORK:
				Write("OP_BORK       R[" + A + "] := R[" + B + "] | K[" + C + "]:integer");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " | " + ConstantValue(CONSTANTS, c));
				break;
			// R[A] := R[B] ~ K[C]:integer
			case OP_BXORK:
				Write("OP_BXORK      R[" + A + "] := R[" + B + "] ~ K[" + C + "]:integer");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " ~ " + ConstantValue(CONSTANTS, c));
				break;
			// R[A] := R[B] >> sC
			case OP_SHRI:
				Write("OP_SHRI       R[" + A + "] := R[" + B + "] >> " + SC);
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " >> " + SC);
				break;
			// R[A] := sC << R[B]
			case OP_SHLI:
				Write("OP_SHLI       R[" + A + "] := " + SC + " << R[" + B + "]");
				Write("; " + RegisterName(f, a, index) + " := " + SC + " << " + RegisterName(f, b, index));
				break;
			// R[A] := R[B] + R[C]
			case OP_ADD:
				Write("OP_ADD        R[" + A + "] := R[" + B + "] + R[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " + " + RegisterName(f, c, index));
				break;
			// R[A] := R[B] - R[C]
			case OP_SUB:
				Write("OP_SUB        R[" + A + "] := R[" + B + "] - R[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " - " + RegisterName(f, c, index));
				break;
			// R[A] := R[B] * R[C]
			case OP_MUL:
				Write("OP_MUL        R[" + A + "] := R[" + B + "] * R[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " * " + RegisterName(f, c, index));
				break;
			// R[A] := R[B] % R[C]
			case OP_MOD:
				Write("OP_MOD        R[" + A + "] := R[" + B + "] % R[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " % " + RegisterName(f, c, index));
				break;
			// R[A] := R[B] ^ R[C]
			case OP_POW:
				Write("OP_POW        R[" + A + "] := R[" + B + "] ^ R[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " ^ " + RegisterName(f, c, index));
				break;
			// R[A] := R[B] / R[C]
			case OP_DIV:
				Write("OP_DIV        R[" + A + "] := R[" + B + "] / R[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " / " + RegisterName(f, c, index));
				break;
			// R[A] := R[B] // R[C]
			case OP_IDIV:
				Write("OP_IDIV       R[" + A + "] := R[" + B + "] // R[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " // " + RegisterName(f, c, index));
				break;
			// R[A] := R[B] & R[C]
			case OP_BAND:
				Write("OP_BAND       R[" + A + "] := R[" + B + "] & R[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " & " + RegisterName(f, c, index));
				break;
			// R[A] := R[B] | R[C]
			case OP_BOR:
				Write("OP_BOR        R[" + A + "] := R[" + B + "] | R[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " | " + RegisterName(f, c, index));
				break;
			// R[A] := R[B] ~ R[C]
			case OP_BXOR:
				Write("OP_BXOR       R[" + A + "] := R[" + B + "] ~ R[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " ~ " + RegisterName(f, c, index));
				break;
			// R[A] := R[B] >> R[C]
			case OP_SHR:
				Write("OP_SHR        R[" + A + "] := R[" + B + "] >> R[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " >> " + RegisterName(f, c, index));
				break;
			// R[A] := R[B] << R[C]
			case OP_SHL:
				Write("OP_SHL        R[" + A + "] := R[" + B + "] << R[" + C + "]");
				Write("; " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index) + " << " + RegisterName(f, c, index));
				break;
			// call C metamethod over R[A] and R[B]
			case OP_MMBIN:
				Write("OP_MMBIN      call C metamethod(" + std::to_string(c) + ") over R[" + A + "] and R[" + B + "]");
				Write("; call C metamethod(" + std::to_string(c) + ") over " + RegisterName(f, a, index) + " and " + RegisterName(f, b, index));
				break;
			// call C metamethod over R[A] and sB
			case OP_MMBINI:
				Write("OP_MMBINI     call C metamethod(" + std::to_string(c) + ") over R[" + A + "] and " + SB);
				Write("; call C metamethod(" + std::to_string(c) + ") over " + RegisterName(f, a, index) + " and " + SB);
				break;
			// call C metamethod over R[A] and K[B]
			case OP_MMBINK:
				Write("OP_MMBINK     call C metamethod(" + std::to_string(c) + ") over R[" + A + "] and K[" + B + "]");
				Write("; call C metamethod(" + std::to_string(c) + ") over " + RegisterName(f, a, index) + " and " + ConstantValue(CONSTANTS, b));
				break;
			// R[A] := -R[B]
			case OP_UNM:
				Write("OP_UNM        R[" + A + "] := -R[" + B + "]");
				Write("; " + RegisterName(f, a, index) + " := -" + RegisterName(f, b, index));
				break;
			// R[A] := ~R[B]
			case OP_BNOT:
				Write("OP_BNOT       R[" + A + "] := ~R[" + B + "]");
				Write("; " + RegisterName(f, a, index) + " := ~" + RegisterName(f, b, index));
				break;
			// R[A] := not R[B]
			case OP_NOT:
				Write("OP_NOT        R[" + A + "] := not R[" + B + "]");
				Write("; " + RegisterName(f, a, index) + " := not " + RegisterName(f, b, index));
				break;
			// R[A] := #R[B] (length operator)
			case OP_LEN:
				Write("OP_LEN        R[" + A + "] := #R[" + B + "] (length operator)");
				Write("; " + RegisterName(f, a, index) + " := #" + RegisterName(f, b, index) + "  (length operator)");
				break;
			// R[A] := R[A].. ... ..R[A + B - 1]
			case OP_CONCAT:
				Write("OP_CONCAT     R[" + A + "] := R[" + A + "].. ... ..R[" + A + " + " + B + " - 1]");
				Write("; " + RegisterName(f, a, index) + " := ");
				for (int ri = a; ri <= a + b - 1; ++ri)
				{
					Write(RegisterName(f, ri, index));
					if (ri < a + b - 1) Write("..");
				}
				break;
			// close all upvalues >= R[A]
			case OP_CLOSE:
				Write("OP_CLOSE      close all upvalues >= R[" + A + "]");
				Write("; close all upvalues >= " + RegisterName(f, a, index));
				break;
			// mark variable R[A] "to be closed"
			case OP_TBC:
				Write("OP_TBC        mark variable R[" + A + "] \"to be closed\"");
				Write("; mark variable " + RegisterName(f, a, index) + " \"to be closed\"");
				break;
			// pc += sJ
			case OP_JMP:
				Write("OP_JMP        pc(" + std::to_string(index + 1) + ") += " + SJ);
				break;
			// if ((R[A] == R[B]) ~= k) then pc++
			case OP_EQ:
				Write("OP_EQ         if ((R[" + A + "] == R[" + B + "]) ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				Write("; if ((" + RegisterName(f, a, index) + " == " + RegisterName(f, b, index) + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				break;
			// if ((R[A] <  R[B]) ~= k) then pc++
			case OP_LT:
				Write("OP_LT         if ((R[" + A + "] < R[" + B + "]) ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				Write("; if ((" + RegisterName(f, a, index) + " < " + RegisterName(f, b, index) + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				break;
			// if ((R[A] <=  R[B]) ~= k) then pc++
			case OP_LE:
				Write("OP_LE         if ((R[" + A + "] <= R[" + B + "]) ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				Write("; if ((" + RegisterName(f, a, index) + " <= " + RegisterName(f, b, index) + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				break;
			// if ((R[A] ==  K[B]) ~= k) then pc++
			case OP_EQK:
				Write("OP_EQK        if ((R[" + A + "] == K[" + B + "]) ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				Write("; if ((" + RegisterName(f, a, index) + " == " + ConstantValue(CONSTANTS, b) + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				break;
			// if ((R[A] == sB) ~= k) then pc++
			case OP_EQI:
				Write("OP_EQI        if ((R[" + A + "] == " + SB + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				Write("; if ((" + RegisterName(f, a, index) + " == " + SB + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				break;
			// if ((R[A] < sB) ~= k) then pc++
			case OP_LTI:
				Write("OP_LTI        if ((R[" + A + "] < " + SB + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				Write("; if ((" + RegisterName(f, a, index) + " < " + SB + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				break;
			// if ((R[A] <= sB) ~= k) then pc++
			case OP_LEI:
				Write("OP_LEI        if ((R[" + A + "] <= " + SB + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				Write("; if ((" + RegisterName(f, a, index) + " <= " + SB + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				break;
			// if ((R[A] > sB) ~= k) then pc++
			case OP_GTI:
				Write("OP_GTI        if ((R[" + A + "] > " + SB + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				Write("; if ((" + RegisterName(f, a, index) + " > " + SB + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				break;
			// if ((R[A] >= sB) ~= k) then pc++
			case OP_GEI:
				Write("OP_GEI        if ((R[" + A + "] >= " + SB + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				Write("; if ((" + RegisterName(f, a, index) + " >= " + SB + ") ~= " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				break;
			// if (not R[A] == k) then pc++
			case OP_TEST:
				Write("OP_TEST       if (not R[" + A + "] == " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				Write("; if (not " + RegisterName(f, a, index) + " == " + K + ") then pc(" + std::to_string(index + 1) + ")++");
				break;
			// if (not R[B] == k) then pc++ else R[A] := R[B]
			case OP_TESTSET:
				Write("OP_TESTSET    if (not R[" + B + "] == " + K + ") then pc(" + std::to_string(index + 1) + ")++ else R[" + A + "] := R[" + B + "]");
				Write("; if (not " + RegisterName(f, b, index) + " == " + K + ") then pc(" + std::to_string(index + 1) + ")++ else " + RegisterName(f, a, index) + " := " + RegisterName(f, b, index));
				break;
			// R[A], ... ,R[A+C-2] := R[A](R[A+1], ... ,R[A+B-1])
			case OP_CALL:
				Write("OP_CALL       R[" + A + "], ... ,R[" + A + "+"+ C + "-2] := R[" + A + "](R[" + A + "+1], ... ,R[" + A + "+" + B + "-1])");
				Write("; ");
				for (int ri = a; ri <= a + c - 2; ++ri)
				{
					Write(RegisterName(f, ri, index));
					if (ri < a + c - 2) Write(", ");
					else Write(" := ");
				}
				Write(RegisterName(f, a, index) + "(");
				for (int ri = a + 1; ri <= a + b - 1; ++ri)
				{
					Write(RegisterName(f, ri, index));
					if (ri < a + b - 1) Write(", ");
				}
				Write(")");
				break;
			// return R[A](R[A+1], ... ,R[A+B-1])
			case OP_TAILCALL:
				Write("OP_TAILCALL   return R[" + A + "](R[" + A + "+1], ... ,R[" + A + "+" + B + "-1])");
				Write("; return " + RegisterName(f, a, index) + "(");
				for (int ri = a + 1; ri <= a + b - 1; ++ri)
				{
					Write(RegisterName(f, ri, index));
					if (ri < a + b - 1) Write(", ");
				}
				Write(")");
				break;
			// return R[A], ... ,R[A+B-2]
			case OP_RETURN:
				Write("OP_RETURN     return R[" + A + "], ... ,R[" + A + "+" + B + "-2]");
				Write("; return ");
				for (int ri = a; ri <= a + b -2; ++ri)
				{
					Write(RegisterName(f, ri, index));
					if (ri < a + b - 2) Write(", ");
				}
				break;
			// return
			case OP_RETURN0:
				Write("OP_RETURN0    return");
				break;
			// return R[A]
			case OP_RETURN1:
				Write("OP_RETURN1    return R[" + A + "]");
				Write("; return " + RegisterName(f, a, index));
				break;
			// update counters; if loop continues then pc-=Bx;
			case OP_FORLOOP:
				Write("OP_FORLOOP    update counters; if loop continues then pc(" + std::to_string(index + 1) + ")-=" + BX);
				break;
			// <check values and prepare counters>;
			// if not to run then pc += Bx + 1;
			case OP_FORPREP:
				Write("OP_FORPREP    <check values and prepare counters>; if not to run then pc(" + std::to_string(index + 1) + ") += " + BX + " + 1");
				break;
			// create upvalue for R[A + 3]; pc+=Bx
			case OP_TFORPREP:
				Write("OP_TFORPREP   create upvalue for R[" + A + " + 3]; pc(" + std::to_string(index + 1) + ")+=" + BX);
				Write("; create upvalue for " + RegisterName(f, a + 3, index) + "; pc(" + std::to_string(index + 1) + ")+=" + BX);
				break;
			// R[A+4], ... ,R[A+3+C] := R[A](R[A+1], R[A+2]);
			case OP_TFORCALL:
				Write("OP_TFORCALL   R[" + A + "+4], ... ,R[" + A + "+3+" + C + "] := R[" + A + "](R[" + A + "+1], R[" + A + "+2])");
				Write(";");
				for (int ri = a + 4; ri <= a + 3 + c; ++ri)
				{
					Write(RegisterName(f, ri, index));
					if (ri < a + 3 + c) Write(", ");
					else Write(" := ");
				}
				Write(RegisterName(f, a, index) + "(" + RegisterName(f, a + 1, index) + ", " + RegisterName(f, a + 2, index) + ")");
				break;
			// if R[A+2] ~= nil then { R[A]=R[A+2]; pc -= Bx }
			case OP_TFORLOOP:
				Write("OP_TFORLOOP   if R[" + A + "+2] ~= nil then { R[" + A + "]=R[" + A + "+2]; pc(" + std::to_string(index + 1) + ") -= " + BX + " }");
				Write(";   if " + RegisterName(f, a + 2, index) + " ~= nil then { " + RegisterName(f, a, index) + "=" + RegisterName(f, a + 2, index) + "; pc(" + std::to_string(index + 1) + ") -= " + BX + " }");
				break;
			// R[A][C+i] := R[A+i], 1 <= i <= B
			case OP_SETLIST:
				Write("OP_SETLIST    R[" + A + "][" + C + "+i] := R[" + A + "+i], 1 <= i <= " + B);
				Write(";");
				for (int ri = 1; ri <= b; ++ri)
				{
					Write(RegisterName(f, a, index) + "[" + std::to_string(c + ri) + "] := " + RegisterName(f, a + ri, index));
					if (ri < b) Write(" ");
				}
				break;
			// R[A] := closure(KPROTO[Bx])
			case OP_CLOSURE:
				Write("OP_CLOSURE    R[" + A + "] := closure(KPROTO[" + BX + "])");
				Write("; " + RegisterName(f, a, index) + " := closure(KPROTO[" + BX + "])");
				break;
			// R[A], R[A+1], ..., R[A+C-2] = vararg
			case OP_VARARG:
				Write("OP_VARARG     R[" + A + "], R[" + A + "+1], ..., R[" + A + "+" + C + "-2] = vararg");
				Write(";");
				for (int ri = a; ri <= a + c - 2; ++ri)
				{
					Write(RegisterName(f, ri, index));
					if (ri < a + c -2)
						Write(", ");
					else
						Write(" = vararg");
				}
				break;
			// (adjust vararg parameters)
			case OP_VARARGPREP:
				Write("OP_VARARGPREP (adjust vararg parameters)");
				break;
			}

		    Write("\n");
		}
	}

	void DumpConstants(const std::string& tab, const Proto* f)
	{
		for (int i = 0; i < f->sizek; i++) {
			const TValue* o = &f->k[i];
			int tt = ttypetag(o);
			switch (tt) {
				// 输出Number
			case LUA_VNUMFLT:
				WriteKeyValue(tab + std::to_string(i) + " number", fltvalue(o));
				break;
				// 输出Integer
			case LUA_VNUMINT:
				WriteKeyValue(tab + std::to_string(i) + " integer", ivalue(o));
				break;
				// 输出字符串
			case LUA_VSHRSTR:
			case LUA_VLNGSTR:
				WriteKeyValue(tab + std::to_string(i) + " string", getstr(tsvalue(o)));
				break;
				// 输出nil
			case LUA_VNIL:
				Write(tab + std::to_string(i) + " nil\n");
				break;
				// 输出false
			case LUA_VFALSE:
				Write(tab + std::to_string(i) + " false\n");
				break;
				// 输出true
			case LUA_VTRUE:
				Write(tab + std::to_string(i) + " true\n");
				break;
			default:
				break;
			}
		}
	}

	std::string ConstantValue(const TValue* k, int index)
	{
		const TValue* o = k + index;
		int tt = ttypetag(o);
		switch (tt) {
			// 输出nil
		case LUA_VNIL:
			return "nil";
			// 输出false
		case LUA_VFALSE:
			return "false";
			// 输出true
		case LUA_VTRUE:
			return "true";
			// 输出Number
		case LUA_VNUMFLT:
			return std::to_string(fltvalue(o));
			// 输出Integer
		case LUA_VNUMINT:
			return std::to_string(ivalue(o));
			// 输出字符串
		case LUA_VSHRSTR:
		case LUA_VLNGSTR:
			return getstr(tsvalue(o));
		default:
			return "";
		}
	}

	std::string RegisterName(const Proto* f, int index, int pc)
	{
		if (index < f->sizelocvars && pc >= f->locvars[index].startpc && pc < f->locvars[index].endpc) return getstr(f->locvars[index].varname);
		return "R[" + std::to_string(index) + "]";
	}

	void DumpUpvalues(const std::string& tab, const Proto* f)
	{
		// 输出闭包参数数量
		int i, n = f->sizeupvalues;
		for (i = 0; i < n; i++) {
			Write(tab + std::to_string(i));
			// 输出闭包名称
			Write(" name:");
			Write(getstr(f->upvalues[i].name));
			// 输出是否在栈中
			Write(" instack:");
			Write(f->upvalues[i].instack);
			// 输出位置
			Write(" idx:");
			Write(f->upvalues[i].idx);
			// 输出变量类型
			Write(" kind:");
			Write(f->upvalues[i].kind);
			Write("\n");
		}
	}

	void DumpLocvars(const std::string& tab, const Proto* f)
	{
		for (int i = 0; i < f->sizelocvars; i++) {
			Write(tab + std::to_string(i));
			// 输出变量名
			Write(" varname:");
			Write(getstr(f->locvars[i].varname));
			// 输出生命周期开始的位置
			Write(" startpc:");
			Write(f->locvars[i].startpc);
			// 输出生命周期结束的位置
			Write(" endpc:");
			Write(f->locvars[i].endpc);
			Write("\n");
		}
	}

	template <typename T>
	void WriteKeyValue(const std::string& key, const T& value)
	{
		Write(key);
		Write(":");
		Write(value);
		Write("\n");
	}

	void Write(const int& n)
	{
		Write(std::to_string(n));
	}

	template <typename T>
	void Write(T n)
	{
		Write(std::to_string(n));
	}

	void Write(const char* content)
	{
		Write(std::string(content));
	}

	void Write(char* content)
	{
		Write(std::string(content));
	}

	void Write(const std::string& content)
	{
		SDL_RWwrite(m_out_file, content.c_str(), 1, content.size());
	}

private:
	SDL_RWops* m_out_file = nullptr;
	std::vector<std::string> m_lines;
};

#endif
