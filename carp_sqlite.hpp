#ifndef CARP_SQLITE_INCLUDED
#define CARP_SQLITE_INCLUDED

#include <map>
#include <unordered_map>

#include "lualib/sqlite3/sqlite3.h"
#include "Carp/carp_log.hpp"

class CarpSqliteConnection
{
public:
	CarpSqliteConnection() {}
	~CarpSqliteConnection() { Close(); }

public:
	enum class BindType
	{
		BINDTYPE_BLOB,
		BINDTYPE_DOUBLE,
		BINDTYPE_INT,
		BINDTYPE_INT64,
		BINDTYPE_NULL,
		BINDTYPE_TEXT,
	};

	struct SQLITE_BIND
	{
		BindType bind_type;
		void* memory = nullptr;
		int memory_size = 0;
	};

public:
	/* connect sqlite
	 * @param file_path: path of sqlite file
	 * @param reconnect: reconnect if lost connect
	 * @return succeed or not
	 */
	bool Open(const std::string& file_path)
	{
		Close();

		// create sqlite object
		int result = sqlite3_open(file_path.c_str(), &m_sqlite);
		// check create succeed or not
		if (result != SQLITE_OK)
		{
			CARP_ERROR("sqlite3_open failed: error code:" << sqlite3_errcode(m_sqlite) << ", message:" << sqlite3_errmsg(m_sqlite));
			sqlite3_close(m_sqlite);
			m_sqlite = nullptr;
			return false;
		}

		// save file path
		m_file_path = file_path;

		return true;
	}

	// close
	void Close()
	{
		if (!m_sqlite) return;

		// release all stmt
		for (auto& pair : m_stmt_map)
		{
			// free stmt
			sqlite3_finalize(pair.second->stmt);
			// flag stmt to 0
			pair.second->stmt = nullptr;
		}
		m_stmt_map.clear();

		// release mysql object
		sqlite3_close(m_sqlite);
		m_sqlite = nullptr;
	}

	/* current is open
	 * @return open or not
	 */
	bool IsOpen() const { return m_sqlite != nullptr; }

public:
	// sqlite stmt info
	struct CarpSqliteStmtInfo
	{
		int output_count = 0;								// output bind count of column
		sqlite3_stmt* stmt = nullptr;								// stmt object
		CarpSqliteConnection* conn = nullptr;							// come from
		bool in_used = false;									// is in used;
	};
	typedef std::shared_ptr<CarpSqliteStmtInfo> CarpSqliteStmtInfoPtr;

	/* get stmt object
	 * @param sql: sql string
	 * @param cache: create cache memory
	 * @return MysqlStmtInfo
	 */
	CarpSqliteStmtInfoPtr GetStmt(const std::string& sql)
	{
		if (!m_sqlite)
		{
			CARP_ERROR("m_sqlite is null!");
			return CarpSqliteStmtInfoPtr();
		}

		// find stmt
		auto it = m_stmt_map.find(sql);
		if (it != m_stmt_map.end())
		{
			sqlite3_reset(it->second->stmt);
			return it->second;
		}

		// sqlite stmt info
		auto info = std::make_shared<CarpSqliteStmtInfo>();
		info->conn = this;
		info->in_used = false;
		info->output_count = 0;

		// create stmt
		int result = sqlite3_prepare(m_sqlite, sql.c_str(), sql.size(), &info->stmt, 0);
		if (result != SQLITE_OK)
		{
			CARP_ERROR("sqlite3_prepare failed: error code:" << sqlite3_errcode(m_sqlite) << ", message:" << sqlite3_errmsg(m_sqlite));
			return CarpSqliteStmtInfoPtr();
		}

		// save and return
		m_stmt_map[sql] = info;
		return info;
	}

	bool ReleaseStmt(const std::string& sql)
	{
		auto it = m_stmt_map.find(sql);
		if (it == m_stmt_map.end())
		{
			CARP_ERROR("can't find stmt by sql:" << sql);
			return false;
		}
		if (it->second->in_used)
		{
			CARP_ERROR("stmt by sql is in used:" << sql);
			return false;
		}

		// free stmt
		sqlite3_finalize(it->second->stmt);
		// flag stmt to 0
		it->second->stmt = nullptr;

		m_stmt_map.erase(it);
		return true;
	}

public:
	/*
	 * execute normal sql
	 */
	bool ExecuteQuery(const char* sql)
	{
		if (!m_sqlite) return false;

		char* msg = nullptr;
		int result = sqlite3_exec(m_sqlite, sql, 0, 0, &msg);
		if (result != SQLITE_OK)
		{
			if (msg) CARP_ERROR("sqlite3_exec failed:" << msg);
			return false;
		}
		return true;
	}

private:
	sqlite3* m_sqlite = nullptr;
	std::string m_file_path;
	std::unordered_map<std::string, CarpSqliteStmtInfoPtr> m_stmt_map;
};


/* eg.
ALittle::SqliteConnection conn;
conn.Open("D:\\asd.db");

bool result1 = conn.ExecuteSql("CREATE TABLE if not exists [SipBill] ("
	"[c_call_id] [nvarchar](255) NOT NULL default '',"
	"[c_bill_type] [nvarchar](255) NOT NULL default '',"
	"[c_start_time] [int] NOT NULL default 0)"
	);
{
	std::string sql = "INSERT INTO SipBill (c_call_id,c_bill_type,c_start_time) VALUES (?,?,?)";
	ALittle::SqliteStatementQuery query(&conn, sql);
	query << "asd" << "222" << 13;
	static ALittle::SqliteConnection::SqliteStmtInfoPtr stmt_info;
	bool result2 = query.Execute(stmt_info);
}

{
	std::string sql = "SELECT c_call_id,c_bill_type,c_start_time FROM SipBill WHERE c_start_time>? AND c_bill_type=?";
	ALittle::SqliteStatementQuery query(&conn, sql);
	query << 12 << "222";
	static ALittle::SqliteConnection::SqliteStmtInfoPtr stmt_info;
	bool result2 = query.Execute(stmt_info);

	while (query.HasNext())
	{
		std::string call_id;
		std::string bill_type;
		int start_time;

		query >> call_id >> bill_type >> start_time;
	}
}
 */

class CarpSqliteStatementQuery
{
public:
	CarpSqliteStatementQuery(CarpSqliteConnection* conn, const std::string& sql) : m_conn(conn), m_sql(sql) {}
	~CarpSqliteStatementQuery() { Clear(); }

	//===================================================================
public:
	/* set sql string
	 * @param sql: set sql string
	 */
	void SetSQL(const std::string& sql)
	{
		Reset();
		m_sql = sql;
	}

	//===================================================================
public:
	/* execute
	 * @return succeed or not
	 */
	bool Execute(CarpSqliteConnection::CarpSqliteStmtInfoPtr& stmt_info)
	{
		return Begin(stmt_info);
	}
	/* clear
	 */
	void Clear()
	{
		End();
		m_need_reset = false;
	}

	//===================================================================
public:
	/* common bing for input
	 * @param ptr: pointer of input param
	 * @param size: size of input param
	 * @param is_unsigned: unsigned or not
	 * @param type: input type
	 */
	CarpSqliteStatementQuery& CommonBindForInput(void* ptr, unsigned int size, CarpSqliteConnection::BindType type)
	{
		// try reset
		Reset();

		// add bind struct to list, and init to 0
		m_bind_input.emplace_back(CarpSqliteConnection::SQLITE_BIND());
		auto& bind = m_bind_input.back();
		memset(&bind, 0, sizeof(CarpSqliteConnection::SQLITE_BIND));

		// set info
		bind.memory = ptr;
		bind.memory_size = size;
		bind.bind_type = type;

		return *this;
	}

public:
	template <typename T> CarpSqliteStatementQuery& operator << (const T& param) {  }
	template <typename T> CarpSqliteStatementQuery& operator << (const T* param) {  }

	template <>	CarpSqliteStatementQuery& operator << (const int& param) { return CommonBindForInput((void*)&param, sizeof(int), CarpSqliteConnection::BindType::BINDTYPE_INT); }
	template <>	CarpSqliteStatementQuery& operator << (const long& param) { return CommonBindForInput((void*)&param, sizeof(long), CarpSqliteConnection::BindType::BINDTYPE_INT); }
	template <>	CarpSqliteStatementQuery& operator << (const long long& param) { return CommonBindForInput((void*)&param, sizeof(long long), CarpSqliteConnection::BindType::BINDTYPE_INT64); }
	template <>	CarpSqliteStatementQuery& operator << (const double& param) { return CommonBindForInput((void*)&param, sizeof(double), CarpSqliteConnection::BindType::BINDTYPE_DOUBLE); }
	template <>	CarpSqliteStatementQuery& operator << (const std::string& param) { return CommonBindForInput((void*)param.c_str(), param.size(), CarpSqliteConnection::BindType::BINDTYPE_TEXT); }
	template <>	CarpSqliteStatementQuery& operator << (const std::vector<char>& param) { return CommonBindForInput((void*)&param[0], param.size(), CarpSqliteConnection::BindType::BINDTYPE_BLOB); }
	template <>	CarpSqliteStatementQuery& operator << (const std::vector<float>& param) { return CommonBindForInput((void*)&param[0], param.size() * sizeof(float), CarpSqliteConnection::BindType::BINDTYPE_BLOB); }

	template <>	CarpSqliteStatementQuery& operator << (const char* param) { return CommonBindForInput((void*)param, strlen(param), CarpSqliteConnection::BindType::BINDTYPE_TEXT); }
	template <>	CarpSqliteStatementQuery& operator << (const unsigned char* param) { return CommonBindForInput((void*)param, strlen((char*)param), CarpSqliteConnection::BindType::BINDTYPE_TEXT); }
	template <>	CarpSqliteStatementQuery& operator << (const int* param) { return CommonBindForInput((void*)param, sizeof(int), CarpSqliteConnection::BindType::BINDTYPE_INT); }
	template <>	CarpSqliteStatementQuery& operator << (const long* param) { return CommonBindForInput((void*)param, sizeof(long), CarpSqliteConnection::BindType::BINDTYPE_INT); }
	template <>	CarpSqliteStatementQuery& operator << (const long long* param) { return CommonBindForInput((void*)param, sizeof(long long), CarpSqliteConnection::BindType::BINDTYPE_INT64); }
	template <>	CarpSqliteStatementQuery& operator << (const double* param) { return CommonBindForInput((void*)param, sizeof(double), CarpSqliteConnection::BindType::BINDTYPE_DOUBLE); }
	template <>	CarpSqliteStatementQuery& operator << (const std::string* param) { return CommonBindForInput((void*)param->c_str(), param->size(), CarpSqliteConnection::BindType::BINDTYPE_TEXT); }
	template <>	CarpSqliteStatementQuery& operator << (const std::vector<char>* param) { return CommonBindForInput((void*)&(*param)[0], param->size(), CarpSqliteConnection::BindType::BINDTYPE_BLOB); }
	template <>	CarpSqliteStatementQuery& operator << (const std::vector<float>* param) { return CommonBindForInput((void*)&(*param)[0], param->size() * sizeof(float), CarpSqliteConnection::BindType::BINDTYPE_BLOB); }

	//===================================================================
public:
	/*
	 * has next
	 */
	bool HasNext() const
	{
		return m_has_next;
	}
	/* get next record
	 * @return succeed or not
	 */
	bool Next()
	{
		// stmt must not be null
		if (!m_stmt_info)
		{
			CARP_ERROR("statement is null!");
			return false;
		}

		// check has next
		if (m_has_next == false) return false;

		// get next info
		int result = sqlite3_step(m_stmt_info->stmt);
		if (result != SQLITE_ROW)
		{
			m_has_next = false;
			return false;
		}

		// reset output index
		m_output_index = 0;

		return true;
	}

	//===================================================================
public:
	/* common bind for output
	 * @param ptr: pointer of output param
	 * @param size: size of output param
	 */
	CarpSqliteStatementQuery& IntBind(int* ptr)
	{
		if (!m_stmt_info)
		{
			CARP_ERROR("m_stmt_info is null");
			return *this;
		}

		if (m_output_index >= m_stmt_info->output_count)
		{
			CARP_ERROR("output index is out of range:" << m_output_index << ", " << m_stmt_info->output_count);
			return *this;
		}

		*ptr = sqlite3_column_int(m_stmt_info->stmt, m_output_index);

		++m_output_index;
		if (m_output_index >= m_stmt_info->output_count) Next();

		return *this;
	}
	CarpSqliteStatementQuery& Int64Bind(long long* ptr)
	{
		if (!m_stmt_info)
		{
			CARP_ERROR("m_stmt_info is null");
			return *this;
		}

		if (m_output_index >= m_stmt_info->output_count)
		{
			CARP_ERROR("output index is out of range:" << m_output_index << ", " << m_stmt_info->output_count);
			return *this;
		}

		*ptr = sqlite3_column_int64(m_stmt_info->stmt, m_output_index);

		++m_output_index;
		if (m_output_index >= m_stmt_info->output_count) Next();

		return *this;
	}
	CarpSqliteStatementQuery& DoubleBind(double* ptr)
	{
		if (!m_stmt_info)
		{
			CARP_ERROR("m_stmt_info is null");
			return *this;
		}

		if (m_output_index >= m_stmt_info->output_count)
		{
			CARP_ERROR("output index is out of range:" << m_output_index << ", " << m_stmt_info->output_count);
			return *this;
		}

		*ptr = sqlite3_column_double(m_stmt_info->stmt, m_output_index);

		++m_output_index;
		if (m_output_index >= m_stmt_info->output_count) Next();

		return *this;
	}

	/* common bind for std::string
	 * @param ptr, ref std::string object
	 * @param size: output size
	 */
	CarpSqliteStatementQuery& StringBind(std::string* ptr)
	{
		if (!m_stmt_info)
		{
			CARP_ERROR("m_stmt_info is null");
			return *this;
		}

		if (m_output_index >= m_stmt_info->output_count)
		{
			CARP_ERROR("output index is out of range:" << m_output_index << ", " << m_stmt_info->output_count);
			return *this;
		}

		// get size of string
		int bytes = sqlite3_column_bytes(m_stmt_info->stmt, m_output_index);
		const unsigned char* content = sqlite3_column_text(m_stmt_info->stmt, m_output_index);
		if (bytes && content)
			ptr->assign((char*)content, bytes);
		else
			ptr->clear();

		++m_output_index;
		if (m_output_index >= m_stmt_info->output_count) Next();
		return *this;
	}

	/* common bind for std::vector<char>
	 * @param ptr, ref std::vector<char> object
	 * @param size: output size
	 */
	const void* BlobBindImpl1(int& size)
	{
		if (!m_stmt_info)
		{
			CARP_ERROR("m_stmt_info is null");
			return 0;
		}

		if (m_output_index >= m_stmt_info->output_count)
		{
			CARP_ERROR("output index is out of range:" << m_output_index << ", " << m_stmt_info->output_count);
			return 0;
		}

		// get size of string
		int bytes = sqlite3_column_bytes(m_stmt_info->stmt, m_output_index);
		const void* content = sqlite3_column_blob(m_stmt_info->stmt, m_output_index);
		size = bytes;

		return content;
	}
	void BlobBindImpl2()
	{
		++m_output_index;
		if (m_output_index >= m_stmt_info->output_count) Next();
	}

	template <typename T>
	CarpSqliteStatementQuery& BlobBind(std::vector<T>* ptr)
	{
		int size = 0;
		const void* memory = BlobBindImpl1(size);
		if (size == 0)
		{
			ptr->clear();
			return *this;
		}

		int vector_size = size / static_cast<int>(sizeof(T));
		ptr->resize(vector_size);
		memcpy(&(*ptr)[0], memory, vector_size * static_cast<int>(sizeof(T));

		BlobBindImpl2();

		return *this;
	}

public:
	template <typename T> CarpSqliteStatementQuery& operator >> (T& param) {  }
	template <typename T> CarpSqliteStatementQuery& operator >> (T* param) {  }

	template <>	CarpSqliteStatementQuery& operator >> (int& param) { return IntBind((int*)&param); }
	template <>	CarpSqliteStatementQuery& operator >> (long& param) { return IntBind((int*)&param); }
	template <>	CarpSqliteStatementQuery& operator >> (long long& param) { return Int64Bind((long long*)&param); }
	template <>	CarpSqliteStatementQuery& operator >> (double& param) { return DoubleBind((double*)&param); }
	template <> CarpSqliteStatementQuery& operator >> (std::string& param) { return StringBind(&param); }
	template <> CarpSqliteStatementQuery& operator >> (std::vector<char>& param) { return BlobBind(&param); }
	template <> CarpSqliteStatementQuery& operator >> (std::vector<float>& param) { return BlobBind(&param); }

	template <>	CarpSqliteStatementQuery& operator >> (int* param) { return IntBind((int*)param); }
	template <>	CarpSqliteStatementQuery& operator >> (long* param) { return IntBind((int*)param); }
	template <>	CarpSqliteStatementQuery& operator >> (long long* param) { return Int64Bind((long long*)param); }
	template <>	CarpSqliteStatementQuery& operator >> (double* param) { return DoubleBind((double*)param); }
	template <> CarpSqliteStatementQuery& operator >> (std::string* param) { return StringBind(param); }
	template <> CarpSqliteStatementQuery& operator >> (std::vector<char>* param) { return BlobBind(param); }
	template <> CarpSqliteStatementQuery& operator >> (std::vector<float>* param) { return BlobBind(param); }

	//===================================================================
private:
	/* start execute sql
	 * @return succeed or not
	 */
	bool Begin(CarpSqliteConnection::CarpSqliteStmtInfoPtr& stmt_info)
	{
		// must be reset after begin
		m_need_reset = true;

		// set has next to false
		m_has_next = false;

		// sql must not be empty
		if (m_sql.size() == 0) return false;

		// connect must not be null
		if (!m_conn)
		{
			End();
			CARP_ERROR("connection is null");
			return false;
		}

		// get stmt
		if (!stmt_info || stmt_info->stmt == 0 || stmt_info->conn != m_conn)
			stmt_info = m_conn->GetStmt(m_sql);
		if (!stmt_info)
		{
			CARP_ERROR("stmt_info is null!" << m_sql);
			End();
			return false;
		}

		// check info in used or not
		if (stmt_info->in_used)
		{
			CARP_ERROR("stmt is in used!" << m_sql);
			End();
			return false;
		}
		// flag in used
		stmt_info->in_used = true;
		m_stmt_info = stmt_info;

		for (size_t i = 0; i < m_bind_input.size(); ++i)
		{
			auto& info = m_bind_input[i];
			int result = SQLITE_ERROR;
			switch (info.bind_type)
			{
			case CarpSqliteConnection::BindType::BINDTYPE_BLOB:
				result = sqlite3_bind_blob(stmt_info->stmt, i + 1, info.memory, info.memory_size, 0); break;
			case CarpSqliteConnection::BindType::BINDTYPE_DOUBLE:
				result = sqlite3_bind_double(stmt_info->stmt, i + 1, *((double*)info.memory)); break;
			case CarpSqliteConnection::BindType::BINDTYPE_INT:
				result = sqlite3_bind_int(stmt_info->stmt, i + 1, *((int*)info.memory)); break;
			case CarpSqliteConnection::BindType::BINDTYPE_INT64:
				result = sqlite3_bind_int64(stmt_info->stmt, i + 1, *((long long*)info.memory)); break;
			case CarpSqliteConnection::BindType::BINDTYPE_NULL:
				result = sqlite3_bind_null(stmt_info->stmt, i + 1); break;
			case CarpSqliteConnection::BindType::BINDTYPE_TEXT:
				result = sqlite3_bind_text(stmt_info->stmt, i + 1, (const char*)info.memory, info.memory_size, 0); break;
			}

			if (result != SQLITE_OK)
			{
				CARP_ERROR("sqlite3_bind_* failed: error code:" << result);
				End();
				return false;
			}
		}

		// execute and receive succeed or not
		int result = sqlite3_step(stmt_info->stmt);
		if (result != SQLITE_ROW)
		{
			End();
			return true;
		}

		m_has_next = true;
		stmt_info->output_count = sqlite3_data_count(stmt_info->stmt);
		return true;
	}
	/* clear
	 */
	void End()
	{
		// free result
		if (m_stmt_info)
		{
			sqlite3_reset(m_stmt_info->stmt);
			m_stmt_info->in_used = false;
		}

		m_stmt_info = CarpSqliteConnection::CarpSqliteStmtInfoPtr();

		m_bind_input.resize(0);

		m_sql = "";
		m_output_index = 0;
		m_has_next = false;
	}

private:
	void Reset()
	{
		if (!m_need_reset) return;
		Clear();
	}

private:
	CarpSqliteConnection* m_conn = nullptr;		// connect object

private:
	CarpSqliteConnection::CarpSqliteStmtInfoPtr m_stmt_info;		// stmt
	std::vector<CarpSqliteConnection::SQLITE_BIND> m_bind_input;
	int m_output_index = 0;
	bool m_has_next = false;
	std::string m_sql;				// SQL string

private:
	bool m_need_reset = false;				// need reset or not
};

#endif