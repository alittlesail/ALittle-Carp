#ifndef CARP_MYSQL_INCLUDED
#define CARP_MYSQL_INCLUDED

#include <map>

#include "Mysql/mysql.h"
#include "Mysql/errmsg.h"

#include "Carp/carp_log.hpp"

#ifdef _WIN32
#pragma comment(lib, "libmysql.lib")
#endif

class CarpMysqlConnection
{
public:
	CarpMysqlConnection() {}
	~CarpMysqlConnection() { Close(); }

public:
	/**
	 * invoke before use mysql
	 */
	static void Setup()
	{
		if (mysql_library_init(0, nullptr, nullptr))
		{
			CARP_ERROR("could not initialize MySQL library");
		}
	}

	/**
	 * invoke before application end
	 */
	static void Shutdown()
	{
		mysql_library_end();
	}

public:
	/* connect mysql
	 * @param ip: target ip dress
	 * @param username: username
	 * @param password: password
	 * @param port: port
	 * @param db_name: database name
	 * @return succeed or not
	 */
	bool Open(const char* ip
		, const char* username
		, const char* password
		, unsigned int port
		, const char* db_name)
	{
		// check param
		if (ip == nullptr) { CARP_ERROR("ip == nullptr"); return false; }
		if (username == nullptr) { CARP_ERROR("username == nullptr"); return false; }
		if (password == nullptr) { CARP_ERROR("password == nullptr"); return false; }
		if (db_name == nullptr) { CARP_ERROR("db_name == nullptr"); return false; }

		if (m_mysql) return true;

		// create mysql object
		m_mysql = mysql_init(nullptr);

		// check create succeed or not
		if (!m_mysql)
		{
			CARP_ERROR("mysql_init failed: out of memory");
			return false;
		}

		// allow report data truncation error
		bool bool_option = true;
		if (mysql_options(m_mysql, MYSQL_REPORT_DATA_TRUNCATION, &bool_option))
		{
			mysql_close(m_mysql); m_mysql = nullptr;
			CARP_ERROR("mysql_options failed: MYSQL_REPORT_DATA_TRUNCATION is unknow option");
			return false;
		}
		// use utf8
		if (mysql_options(m_mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4"))
		{
			mysql_close(m_mysql); m_mysql = nullptr;
			CARP_ERROR("mysql_options failed: MYSQL_SET_CHARSET_NAME is unknow option");
			return false;
		}
		// can auto reconnect
		bool_option = false;
		if (mysql_options(m_mysql, MYSQL_OPT_RECONNECT, &bool_option))
		{
			mysql_close(m_mysql); m_mysql = nullptr;
			CARP_ERROR("mysql_options failed: MYSQL_OPT_RECONNECT is unknow option");
			return false;
		}

		// mysql temp
		MYSQL* mysql = nullptr;

		// start connect
		mysql = mysql_real_connect(m_mysql
			, ip
			, username
			, password
			, db_name
			, port
			, nullptr
			, CLIENT_MULTI_STATEMENTS | CLIENT_INTERACTIVE);

		// if connect succeed
		if (mysql)
		{
			// save current database name
			m_db_name = db_name;
			m_ip = ip;
			m_username = username;
			m_password = password;
			m_port = port;
			return true;
		}

		CARP_ERROR("mysql_real_connect failed:" << mysql_error(m_mysql));
		// connect failed
		mysql_close(m_mysql); m_mysql = nullptr;

		return false;
	}

	bool ReOpen()
	{
		Close();
		return Open(m_ip.c_str(), m_username.c_str(), m_password.c_str(), m_port, m_db_name.c_str());
	}

	/* reconnect
	 * @param error_message: error message
	 * @return succeed or not
	 */
	bool Ping() const
	{
		if (!m_mysql)
		{
			CARP_ERROR("m_mysql is null!");
			return false;
		}

		// check disconnect
		if (mysql_ping(m_mysql))
		{
			CARP_ERROR("mysql_ping failed: " << mysql_error(m_mysql));
			return false;
		}

		return true;
	}

	// close
	void Close()
	{
		if (!m_mysql) return;

		// release all stmt
		for (auto& pair : m_stmt_map)
		{
			for (size_t i = 0; i < pair.second->bind_output.size(); ++i)
			{
				if (pair.second->bind_output[i].buffer != nullptr)
					free(pair.second->bind_output[i].buffer);
			}
			// free stmt
			mysql_stmt_close(pair.second->stmt);
			// flag stmt to 0
			pair.second->stmt = nullptr;
		}
		m_stmt_map.clear();

		// release mysql object
		mysql_close(m_mysql);
		m_mysql = nullptr;
	}

	/* current is open
	 * @return open or not
	 */
	bool IsOpen() const { return m_mysql != nullptr; }

	uint64_t GetLastInsertId() const
	{
		CARP_CHECK_RETURN(m_mysql, 0);
		return mysql_insert_id(m_mysql);
	}

	MYSQL* GetMysql()
	{
		return m_mysql;
	}

public:
	// mysql stmt info
	struct CarpMysqlStmtInfo
	{
		MYSQL_STMT* stmt = nullptr;									// stmt object
		std::vector<MYSQL_BIND> bind_output;				// output bind for read
		std::vector<unsigned long> value_length;		// output max length for read
		CarpMysqlConnection* conn = nullptr;								// come from
	};
	typedef std::shared_ptr<CarpMysqlStmtInfo> CarpMysqlStmtInfoPtr;

	/* get stmt object
	 * @param sql: sql string
	 * @return MysqlStmtInfo
	 */
	CarpMysqlStmtInfoPtr GetStmt(const std::string& sql, bool& need_reconnect)
	{
		need_reconnect = false;
		if (!m_mysql)
		{
			CARP_ERROR("m_mysql is null!");
			return CarpMysqlStmtInfoPtr();
		}

		// find stmt
		auto it = m_stmt_map.find(sql);
		if (it != m_stmt_map.end()) return it->second;

		// mysql stmt info
		auto info = std::make_shared<CarpMysqlStmtInfo>();
		info->conn = this;

		// create stmt
		info->stmt = mysql_stmt_init(m_mysql);
		if (!info->stmt)
		{
			CARP_ERROR("mysql_stmt_init failed: out of memory!");
			return CarpMysqlStmtInfoPtr();
		}

		// check succeed or not
		if (mysql_stmt_prepare(info->stmt, sql.c_str(), static_cast<unsigned long>(sql.size())))
		{
			auto error = mysql_stmt_errno(info->stmt);
			need_reconnect = error == CR_SERVER_GONE_ERROR || error == CR_SERVER_LOST;
			CARP_ERROR("mysql_stmt_prepare failed: code:" << mysql_stmt_errno(info->stmt) << ", reason:" << mysql_stmt_error(info->stmt));
			mysql_stmt_close(info->stmt);
			return CarpMysqlStmtInfoPtr();
		}

		// get param meta
		MYSQL_RES* field_meta_result = mysql_stmt_result_metadata(info->stmt);
		if (field_meta_result != nullptr)
		{
			// get field count
			unsigned int field_count = mysql_num_fields(field_meta_result);
			if (field_count > 0)
			{
				// init bind input
				info->bind_output.resize(field_count);
				info->value_length.resize(field_count, 0);
				memset(&info->bind_output[0], 0, sizeof(MYSQL_BIND) * field_count);

				// get param field
				MYSQL_FIELD* field_list = mysql_fetch_field(field_meta_result);
				if (field_list == nullptr)
				{
					CARP_ERROR("mysql_fetch_field failed: " << mysql_stmt_error(info->stmt));
					mysql_free_result(field_meta_result);
					mysql_stmt_close(info->stmt);
					return CarpMysqlStmtInfoPtr();
				}

				// set bind input info
				for (unsigned int i = 0; i < field_count; ++i)
				{
					MYSQL_BIND& bind = info->bind_output[i];
					bind.buffer_type = field_list[i].type;
					bind.is_unsigned = (field_list[i].flags & UNSIGNED_FLAG) > 0;
					auto buffer_length = field_list[i].length;
					// 首先，对于数值类型的变量，mysql_stmt_fetch会无视buffer_length参数，强行写入
					// 所以这里的内存长度，必须大于或等于8个字节
					if (buffer_length <= sizeof(uint64_t)) buffer_length = sizeof(uint64_t);
					// 这里定1024，是因为减少字符串截断通知
					else if (buffer_length >= 1024) buffer_length = 1024;
					bind.buffer = malloc(buffer_length);
					bind.buffer_length = buffer_length;
					bind.length = &info->value_length[i];
				}
			}
			mysql_free_result(field_meta_result);
		}

		// save and return
		m_stmt_map[sql] = info;
		return info;
	}

	bool ReleaseStmt(const char* sql)
	{
		StmtMap::iterator it = m_stmt_map.find(sql);
		if (it == m_stmt_map.end())
		{
			CARP_ERROR("can't find stmt by sql:" << sql);
			return false;
		}
		for (size_t i = 0; i < it->second->bind_output.size(); ++i)
		{
			if (it->second->bind_output[i].buffer != nullptr)
				free(it->second->bind_output[i].buffer);
		}
		// free stmt
		mysql_stmt_close(it->second->stmt);
		// flag stmt to 0
		it->second->stmt = nullptr;

		m_stmt_map.erase(it);
		return true;
	}

	// execute simple sql
	bool ExecuteQuery(const char* sql, std::string& reason) const
	{
		if (sql == nullptr)
		{
			reason = "sql == nullptr";
			return false;
		}
		if (m_mysql == nullptr)
		{
			reason = "m_mysql == nullptr";
			return false;
		}

		if (mysql_query(m_mysql, sql))
		{
			reason = mysql_error(m_mysql);
			return false;
		}

		MYSQL_RES* store_result = mysql_store_result(m_mysql);
		if (store_result) mysql_free_result(store_result);

		return true;
	}

public:
	/* current connect database name
	 * return database name
	 */
	const std::string& GetDBName() const { return m_db_name; }

private:
	MYSQL* m_mysql = nullptr;

	typedef std::map<std::string, CarpMysqlStmtInfoPtr> StmtMap;
	StmtMap m_stmt_map;

private:
	std::string m_db_name;
	std::string m_ip;
	std::string m_username;
	std::string m_password;
	unsigned int m_port = 0;
};

class CarpMysqlStatementQuery
{
public:
	CarpMysqlStatementQuery() {}
	~CarpMysqlStatementQuery() { Clear(); }

	//===================================================================
public:
	/* set sql string
	 * @param sql: set sql string
	 * notice:!!!!  if you use SUM, then please used like this CAST(IFNULL(SUM(XXXX),0) AS UNSIGNED)
	 *				and use longlong to receive the value
	 * notice:!!!!  if you use COUNT, then please use long long to receive the value
	 */
	void SetSQL(const char* sql)
	{
		Reset();
		m_sql = sql;
	}
	const char* GetSQL() const
	{
		return m_sql.c_str();
	}

public:
	/* set connect
	 * @param conn: connect object
	 */
	void SetConnection(CarpMysqlConnection* conn)
	{
		if (conn == 0) return;
		m_conn = conn;
	}

	//===================================================================
public:
	/* execute
	 * @return succeed or not
	 */
	bool Execute(std::string& reason)
	{
		return Begin(reason);
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
	void CommonBindForInput(void* ptr, size_t size, bool is_unsigned, enum_field_types buffer_type)
	{
		// try reset
		Reset();

		MYSQL_BIND bind;
		memset(&bind, 0, sizeof(bind));

		bind.buffer = malloc(size + 1);
		memcpy(bind.buffer, ptr, size);
		bind.buffer_length = static_cast<unsigned long>(size);
		bind.is_unsigned = is_unsigned;
		bind.buffer_type = buffer_type;

		m_bind_input.push_back(bind);
	}

public:
	void PushBool(bool param) { CommonBindForInput(static_cast<void*>(&param), sizeof(bool), false, MYSQL_TYPE_TINY); }
	void PushChar(char param) { CommonBindForInput(static_cast<void*>(&param), sizeof(char), false, MYSQL_TYPE_TINY); }
	void PushUChar(unsigned char param) { CommonBindForInput(static_cast<void*>(&param), sizeof(unsigned char), true, MYSQL_TYPE_TINY); }
	void PushShort(short param) { CommonBindForInput(static_cast<void*>(&param), sizeof(short), false, MYSQL_TYPE_SHORT); }
	void PushUShort(unsigned short param) { CommonBindForInput(static_cast<void*>(&param), sizeof(unsigned short), true, MYSQL_TYPE_SHORT); }
	void PushInt(int param) { CommonBindForInput(static_cast<void*>(&param), sizeof(int), false, MYSQL_TYPE_LONG); }
	void PushUInt(unsigned int param) { CommonBindForInput(static_cast<void*>(&param), sizeof(unsigned int), true, MYSQL_TYPE_LONG); }
	void PushLong(long param) { CommonBindForInput(static_cast<void*>(&param), sizeof(long), false, MYSQL_TYPE_LONG); }
	void PushULong(unsigned long param) { CommonBindForInput(static_cast<void*>(&param), sizeof(unsigned long), true, MYSQL_TYPE_LONG); }
	void PushLongLong(long long param) { CommonBindForInput(static_cast<void*>(&param), sizeof(long long), false, MYSQL_TYPE_LONGLONG); }
	void PushULongLong(unsigned long long param) { CommonBindForInput(static_cast<void*>(&param), sizeof(unsigned long long), true, MYSQL_TYPE_LONGLONG); }
	void PushFloat(float param) { CommonBindForInput(static_cast<void*>(&param), sizeof(float), false, MYSQL_TYPE_FLOAT); }
	void PushDouble(double param) { CommonBindForInput(static_cast<void*>(&param), sizeof(double), false, MYSQL_TYPE_DOUBLE); }
	void PushString(const char* param) { CommonBindForInput((void*)param, strlen(param), false, MYSQL_TYPE_STRING); }

	void PushTime(const MYSQL_TIME* param) { CommonBindForInput((void*)param, sizeof(MYSQL_TIME), false, MYSQL_TYPE_TIME); }
	void PushDate(const MYSQL_TIME* param) { CommonBindForInput((void*)param, sizeof(MYSQL_TIME), false, MYSQL_TYPE_DATE); }
	void PushDateTime(const MYSQL_TIME* param) { CommonBindForInput((void*)param, sizeof(MYSQL_TIME), false, MYSQL_TYPE_DATETIME); }
	void PushTimestamp(const MYSQL_TIME* param) { CommonBindForInput((void*)param, sizeof(MYSQL_TIME), false, MYSQL_TYPE_TIMESTAMP); }

	//===================================================================
public:
	enum ReadTypes
	{
		MYSQLRT_BOOL = 0,
		MYSQLRT_SHORT = 1,
		MYSQLRT_INT = 2,
		MYSQLRT_LONGLONG = 3,
		MYSQLRT_FLOAT = 4,
		MYSQLRT_DOUBLE = 5,
		MYSQLRT_STRING = 6,
		MYSQLRT_TIME = 7,
	};

	/* get total count
	 * @return total count
	 */
	unsigned int GetCount() const { return m_row_count; }

	/* get affect count
	 * @return affect count
	 */
	unsigned int GetAffectCount() const { return m_affect_count; }

	/* has next
	 * @return succeed or not
	 */
	bool HasNext() const { return m_row_index < m_row_count; }

	/* get next record
	 * @return succeed or not
	 */
	bool Next()
	{
		// check row index
		if (m_row_index >= m_row_count)
		{
			End();
			return false;
		}

		// reset index
		m_col_index = 0;
		++m_row_index;

		if (m_row_index >= m_row_count)
		{
			End();
			return false;
		}
		return true;
	}

	//===================================================================
	bool ReadBool()
	{
		if (m_col_index >= m_col_count)
		{
			CARP_ERROR("m_col_index out of range:" << m_col_index << " >= " << m_col_count << ", sql:" << m_sql);
			return false;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count << ", sql:" << m_sql);
			return false;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		bool result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length << ", sql:" << m_sql);
			return false;
		}
		if (data.buffer_type != MYSQL_TYPE_TINY)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_TINY, is:" << data.buffer_type << ", sql:" << m_sql);
			return false;
		}
		memcpy(&result, data.buffer.data(), sizeof(result));
		++m_col_index;
		if (m_col_index >= m_col_count) Next();
		return result;
	}

	char ReadChar()
	{
		if (m_col_index >= m_col_count)
		{
			CARP_ERROR("m_col_index out of range:" << m_col_index << " >= " << m_col_count << ", sql:" << m_sql);
			return '?';
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count << ", sql:" << m_sql);
			return '?';
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		char result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length << ", sql:" << m_sql);
			return '?';
		}
		if (data.buffer_type != MYSQL_TYPE_TINY)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_TINY, is:" << data.buffer_type << ", sql:" << m_sql);
			return '?';
		}
		memcpy(&result, data.buffer.data(), sizeof(result));
		++m_col_index;
		if (m_col_index >= m_col_count) Next();
		return result;
	}

	short ReadShort()
	{
		if (m_col_index >= m_col_count)
		{
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count << ", sql:" << m_sql);
			return 0;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count << ", sql:" << m_sql);
			return 0;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		short result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length << ", sql:" << m_sql);
			return 0;
		}
		if (data.buffer_type != MYSQL_TYPE_SHORT)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_SHORT, is:" << data.buffer_type << ", sql:" << m_sql);
			return 0;
		}
		memcpy(&result, data.buffer.data(), sizeof(result));
		++m_col_index;
		if (m_col_index >= m_col_count) Next();
		return result;
	}

	int ReadInt()
	{
		if (m_col_index >= m_col_count)
		{
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count << ", sql:" << m_sql);
			return 0;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count << ", sql:" << m_sql);
			return 0;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		int result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length << ", sql:" << m_sql);
			return 0;
		}
		if (data.buffer_type != MYSQL_TYPE_LONG)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_LONG, is:" << data.buffer_type << ", sql:" << m_sql);
			return 0;
		}
		memcpy(&result, data.buffer.data(), sizeof(result));
		++m_col_index;
		if (m_col_index >= m_col_count) Next();
		return result;
	}

	long ReadLong()
	{
		if (m_col_index >= m_col_count)
		{
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count << ", sql:" << m_sql);
			return 0;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count << ", sql:" << m_sql);
			return 0;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		long result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length << ", sql:" << m_sql);
			return 0;
		}
		if (data.buffer_type != MYSQL_TYPE_LONG)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_LONG, is:" << data.buffer_type << ", sql:" << m_sql);
			return 0;
		}
		memcpy(&result, data.buffer.data(), sizeof(result));
		++m_col_index;
		if (m_col_index >= m_col_count) Next();
		return result;
	}

	int ReadLongLong()
	{
		if (m_col_index >= m_col_count)
		{
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count << ", sql:" << m_sql);
			return 0;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count << ", sql:" << m_sql);
			return 0;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		long long result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length << ", sql:" << m_sql);
			return 0;
		}
		if (data.buffer_type != MYSQL_TYPE_LONGLONG)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_LONGLONG, is:" << data.buffer_type << ", sql:" << m_sql);
			return 0;
		}
		memcpy(&result, data.buffer.data(), sizeof(result));
		++m_col_index;
		if (m_col_index >= m_col_count) Next();
		return static_cast<int>(result);
	}

	float ReadFloat()
	{
		if (m_col_index >= m_col_count)
		{
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count << ", sql:" << m_sql);
			return 0;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count << ", sql:" << m_sql);
			return 0;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		float result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length << ", sql:" << m_sql);
			return 0;
		}
		if (data.buffer_type != MYSQL_TYPE_FLOAT)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_FLOAT, is:" << data.buffer_type << ", sql:" << m_sql);
			return 0;
		}
		memcpy(&result, data.buffer.data(), sizeof(result));
		++m_col_index;
		if (m_col_index >= m_col_count) Next();
		return result;
	}

	double ReadDouble()
	{
		if (m_col_index >= m_col_count)
		{
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count << ", sql:" << m_sql);
			return 0;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count << ", sql:" << m_sql);
			return 0;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		double result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length << ", sql:" << m_sql);
			return 0;
		}
		if (data.buffer_type != MYSQL_TYPE_DOUBLE)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_DOUBLE, is:" << data.buffer_type << ", sql:" << m_sql);
			return 0;
		}
		memcpy(&result, data.buffer.data(), sizeof(result));
		++m_col_index;
		if (m_col_index >= m_col_count) Next();
		return result;
	}

	MYSQL_TIME ReadTime()
	{
		if (m_col_index >= m_col_count)
		{
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count << ", sql:" << m_sql);
			return {0};
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count << ", sql:" << m_sql);
			return {0};
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		MYSQL_TIME result = { 0 };
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length << ", sql:" << m_sql);
			return { 0 };
		}
		if (data.buffer_type != MYSQL_TYPE_TIME
			&& data.buffer_type != MYSQL_TYPE_DATE
			&& data.buffer_type != MYSQL_TYPE_DATETIME
			&& data.buffer_type != MYSQL_TYPE_TIMESTAMP)
		{
			CARP_ERROR("field type is not MYSQL_TIME type, is:" << data.buffer_type << ", sql:" << m_sql);
			return { 0 };
		}
		memcpy(&result, data.buffer.data(), sizeof(result));
		++m_col_index;
		if (m_col_index >= m_col_count) Next();
		return result;
	}

	const char* ReadString()
	{
		if (m_col_index >= m_col_count)
		{
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count << ", sql:" << m_sql);
			m_temp_string.resize(0);
			return m_temp_string.c_str();
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count << ", sql:" << m_sql);
			m_temp_string.resize(0);
			return m_temp_string.c_str();
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		m_temp_string.resize(0);
		const char* result = m_temp_string.c_str();
		if (!data.buffer.empty()) result = data.buffer.data();
		++m_col_index;
		if (m_col_index >= m_col_count)
		{
			if (m_row_index + 1 >= m_row_count)
			{
				m_temp_string = result;
				result = m_temp_string.c_str();
			}
			Next();
		}
		return result;
	}

	int ReadType()
	{
		if (m_col_index >= m_col_count)
		{
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count << ", sql:" << m_sql);
			return -1;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count << ", sql:" << m_sql);
			return -1;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		if (data.buffer_type == MYSQL_TYPE_TINY) return MYSQLRT_BOOL;
		if (data.buffer_type == MYSQL_TYPE_SHORT) return MYSQLRT_SHORT;
		if (data.buffer_type == MYSQL_TYPE_LONG) return MYSQLRT_INT;
		if (data.buffer_type == MYSQL_TYPE_LONGLONG) return MYSQLRT_LONGLONG;
		if (data.buffer_type == MYSQL_TYPE_FLOAT) return MYSQLRT_FLOAT;
		if (data.buffer_type == MYSQL_TYPE_DOUBLE) return MYSQLRT_DOUBLE;
		if (data.buffer_type == MYSQL_TYPE_STRING) return MYSQLRT_STRING;
		if (data.buffer_type == MYSQL_TYPE_TIME
			|| data.buffer_type == MYSQL_TYPE_DATE
			|| data.buffer_type == MYSQL_TYPE_DATETIME
			|| data.buffer_type == MYSQL_TYPE_TIMESTAMP) return MYSQLRT_TIME;
		return MYSQLRT_STRING;
	}

	//===================================================================
private:
	/* start execute sql
	 * @return succeed or not
	 */
	bool Begin(std::string& reason)
	{
		// must be reset after begin
		m_need_reset = true;

		// sql must not be empty
		if (m_sql.empty())
		{
			reason = "m_sql is empty";
			End();
			return false;
		}

		// connect must not be null
		if (!m_conn)
		{
			reason = "connection is null";
			End();
			return false;
		}

		// create stmt
		bool need_reconnect = false;
		auto stmt_info = m_conn->GetStmt(m_sql, need_reconnect);
		if (!stmt_info)
		{
			if (!need_reconnect)
			{
				reason = "stmt create failed:" + m_sql;
				End();
				return false;
			}

			CARP_WARN("try reconnect mysql");
			if (!m_conn->ReOpen())
			{
				reason = "reconnect mysql failed!";
				reason += " stmt create failed:" + m_sql;
				CARP_WARN(reason);
				End();
				return false;
			}
			CARP_WARN("reconnect succeed");

			stmt_info = m_conn->GetStmt(m_sql, need_reconnect);
			if (!stmt_info)
			{
				reason = "stmt create failed:" + m_sql;
				End();
				return false;
			}
		}

		// bind param input
		if (m_bind_input.size() && mysql_stmt_bind_param(stmt_info->stmt, &m_bind_input[0]))
		{
			reason = "mysql_stmt_bind_param failed:";
			reason += mysql_stmt_error(stmt_info->stmt);
			End();
			return false;
		}

		// execute and receive succeed or not
		if (mysql_stmt_execute(stmt_info->stmt))
		{
			reason = "mysql_stmt_execute failed:";
			reason += mysql_stmt_error(stmt_info->stmt);

			unsigned int exe_errno = mysql_stmt_errno(stmt_info->stmt);
			if (exe_errno == CR_SERVER_GONE_ERROR || exe_errno == CR_SERVER_LOST)
			{
				CARP_WARN("try reconnect mysql");
				if (!m_conn->ReOpen())
				{
					reason = "reconnect mysql failed!";
					CARP_WARN(reason);
					End();
					return false;
				}
				CARP_WARN("reconnect succeed");

				// create stmt
				stmt_info = m_conn->GetStmt(m_sql, need_reconnect);
				if (!stmt_info)
				{
					reason = "stmt create failed:" + m_sql;
					End();
					return false;
				}

				// bind param input
				if (m_bind_input.size() && mysql_stmt_bind_param(stmt_info->stmt, &m_bind_input[0]))
				{
					reason = "mysql_stmt_bind_param failed:";
					reason += mysql_stmt_error(stmt_info->stmt);
					End();
					return false;
				}

				// execute and receive succeed or not
				if (mysql_stmt_execute(stmt_info->stmt))
				{
					reason = "mysql_stmt_execute failed:";
					reason += mysql_stmt_error(stmt_info->stmt);
					End();
					return false;
				}
			}
			else
			{
				End();
				return false;
			}
		}

		// store result
		mysql_stmt_store_result(stmt_info->stmt);

		// get result info
		m_row_index = 0;
		m_col_index = 0;
		m_row_count = static_cast<unsigned int>(mysql_stmt_num_rows(stmt_info->stmt));
		m_col_count = static_cast<unsigned int>(stmt_info->bind_output.size());
		m_affect_count = static_cast<unsigned int>(mysql_stmt_affected_rows(stmt_info->stmt));

		// fetch data
		m_bind_outputs.resize(m_row_count);
		bool result = true;
		if (m_row_count > 0)
		{
			int row = 0;
			while (true)
			{
				// bind for output
				if (stmt_info->bind_output.size() && mysql_stmt_bind_result(stmt_info->stmt, &(stmt_info->bind_output[0])))
				{
					reason = "mysql_stmt_bind_result failed:";
					reason += mysql_stmt_error(stmt_info->stmt);
					result = false;
					break;
				}

				// fetch next result
				int fetch_result = mysql_stmt_fetch(stmt_info->stmt);
				if (fetch_result != 0)
				{
					if (fetch_result == MYSQL_DATA_TRUNCATED)
					{
						// 数据出现截断，那么就要重新申请内存
						for (size_t i = 0; i < stmt_info->bind_output.size(); ++i)
						{
							if (stmt_info->bind_output[i].buffer_length >= stmt_info->value_length[i])
								continue;
							if (stmt_info->bind_output[i].buffer != nullptr)
								free(stmt_info->bind_output[i].buffer);
							stmt_info->bind_output[i].buffer_length = stmt_info->value_length[i];
							stmt_info->bind_output[i].buffer = malloc(stmt_info->value_length[i]);
						}
						// 返回上一条数据
						mysql_stmt_data_seek(stmt_info->stmt, row);
						continue;
					}
					else if (fetch_result == MYSQL_NO_DATA)
					{
						// 已经没有数据了，这里不做处理
					}
					else
					{
						reason = "mysql_stmt_bind_result failed:";
						reason += mysql_stmt_error(stmt_info->stmt);
						result = false;
					}
					break;
				}

				// copy data
				m_bind_outputs[row].resize(stmt_info->bind_output.size());
				for (size_t i = 0; i < stmt_info->bind_output.size(); ++i)
				{
					MysqlBind& data = m_bind_outputs[row][i];
					data.buffer_type = static_cast<enum_field_types>(stmt_info->bind_output[i].buffer_type);
					data.buffer.resize(stmt_info->value_length[i] + 1, 0);
					data.value_length = stmt_info->value_length[i];
					if (stmt_info->value_length[i] > 0)
						memcpy(data.buffer.data(), stmt_info->bind_output[i].buffer, stmt_info->value_length[i]);
				}
				++row;
			}
		}

		mysql_stmt_free_result(stmt_info->stmt);

		while (true)
		{
			auto query_result = mysql_stmt_next_result(stmt_info->stmt);
			if (query_result == -1) break;

			if (query_result == 0)
			{
				auto result = mysql_stmt_store_result(stmt_info->stmt);
				if (result == 0) mysql_stmt_free_result(stmt_info->stmt);
				else
				{
					CARP_WARN("error:" << mysql_stmt_error(stmt_info->stmt) << " sql:" << m_sql);
					break;
				}
			}
			else
			{
				CARP_WARN("error:" << mysql_stmt_error(stmt_info->stmt) << " sql:" << m_sql);
				break;
			}
		}

		return result;
	}
	/* clear
	 */
	void End()
	{
		m_input_index = 0;
		m_col_index = 0;
		for (size_t i = 0; i < m_bind_input.size(); ++i)
		{
			if (m_bind_input[i].buffer != nullptr)
				free(m_bind_input[i].buffer);
		}
		m_bind_input.clear();
		m_bind_outputs.clear();
	}

private:
	void Reset()
	{
		if (!m_need_reset) return;
		Clear();
	}

private:
	CarpMysqlConnection* m_conn = nullptr;		// connect object

private:
	std::string m_sql;				// SQL string
	int m_input_index = 0;
	unsigned int m_col_index = 0;
	unsigned int m_row_index = 0;
	unsigned int m_row_count = 0;
	unsigned int m_col_count = 0;
	unsigned int m_affect_count = 0;
	std::string m_temp_string;

	std::vector<MYSQL_BIND> m_bind_input;

	struct MysqlBind
	{
		enum_field_types buffer_type = MYSQL_TYPE_TINY;	/* buffer type */
		unsigned long value_length = 0;
		std::vector<char> buffer;
	};
	std::vector<std::vector<MysqlBind>> m_bind_outputs;

private:
	bool m_need_reset;				// need reset or not
};

class CarpMysqlQuery
{
public:
	CarpMysqlQuery() {}
	CarpMysqlQuery(CarpMysqlConnection* conn) { m_conn = conn; }
	virtual ~CarpMysqlQuery() { Clear(); }

	//===================================================================
public:
	// 设置连接
	void SetConn(CarpMysqlConnection* conn) { Reset(); m_conn = conn; }
	// 设置脚本
	void SetSql(const std::string& sql) { Reset(); m_sql = sql; }
	// 获取脚本
	const std::string& GetSql() const { return m_sql; }

	//===================================================================
public:
	/* execute
	 * @return succeed or not
	 */
	bool Execute() { return Begin(); }
	/* clear
	 */
	void Clear() { End(); m_need_reset = false; }

	//===================================================================
public:
	// 获取当前记录条数
	int64_t GetRowCount() { return m_row_count; }
	// 获取当前记录列数
	int64_t GetColCount() { return m_col_count; }

	// 获取当前操作影响的条数
	int64_t GetAffectCount() { return m_affect_count; }
	// 获取上一次插入的自增长ID
	uint64_t GetLastInsertId() { CARP_CHECK_RETURN(m_conn, 0); return m_conn->GetLastInsertId(); }

	// 读取下一条
	bool Next()
	{
		// 检查是否还有数据
		if (m_row_index + 1 >= static_cast<int64_t>(m_data.size())) return false;
		// 行索引增加
		++m_row_index;
		// 重置索引
		m_col_index = 0;
		return true;
	}

	// 读取数据
	bool ReadBool()
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), false);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), false);
		return std::atoi(m_data[m_row_index][m_col_index++].c_str()) != 0;
	}
	int32_t ReadInt32()
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), 0);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), 0);
		return static_cast<int32_t>(std::atoi(m_data[m_row_index][m_col_index++].c_str()));
	}
	uint32_t ReadUInt32()
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), 0);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), 0);
		return static_cast<uint32_t>(std::atoi(m_data[m_row_index][m_col_index++].c_str()));
	}
	int64_t ReadInt64()
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), 0);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), 0);
		return static_cast<int64_t>(std::atoll(m_data[m_row_index][m_col_index++].c_str()));
	}
	uint64_t ReadUInt64()
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), 0);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), 0);
		return static_cast<uint64_t>(std::atoll(m_data[m_row_index][m_col_index++].c_str()));
	}
	float ReadFloat()
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), 0.0f);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), 0.0f);
		return static_cast<float>(std::atof(m_data[m_row_index][m_col_index++].c_str()));
	}
	double ReadDouble()
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), 0.0);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), 0.0);
		return std::atof(m_data[m_row_index][m_col_index++].c_str());
	}
	const char* ReadString()
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), m_empty.c_str());
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), m_empty.c_str());
		return m_data[m_row_index][m_col_index++].c_str();
	}

	// 并判断错误
	bool TryReadBool(bool& out)
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), false);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), false);
		out = std::atoi(m_data[m_row_index][m_col_index++].c_str()) != 0;
		return true;
	}
	bool TryReadInt32(int32_t& out)
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), false);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), false);
		out = static_cast<int32_t>(std::atoi(m_data[m_row_index][m_col_index++].c_str()));
		return true;
	}
	bool TryReadUInt32(uint32_t& out)
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), false);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), false);
		out = static_cast<uint32_t>(std::atoi(m_data[m_row_index][m_col_index++].c_str()));
	}
	bool TryReadInt64(int64_t& out)
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), false);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), false);
		out = static_cast<int64_t>(std::atoll(m_data[m_row_index][m_col_index++].c_str()));
	}
	bool TryReadUInt64(uint64_t& out)
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), false);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), false);
		out = static_cast<uint64_t>(std::atoll(m_data[m_row_index][m_col_index++].c_str()));
	}
	bool TryReadFloat(float& out)
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), false);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), false);
		out = static_cast<float>(std::atof(m_data[m_row_index][m_col_index++].c_str()));
	}
	bool TryReadDouble(double& out)
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), false);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), false);
		out = std::atof(m_data[m_row_index][m_col_index++].c_str());
	}
	bool TryReadString(const char*& out)
	{
		CARP_CHECK_RETURN(m_col_index >= 0 && m_col_index < static_cast<int64_t>(m_col_count), false);
		CARP_CHECK_RETURN(m_row_index >= 0 && m_row_index < static_cast<int64_t>(m_data.size()), false);
		out = m_data[m_row_index][m_col_index++].c_str();
	}

	//===================================================================
private:
	// 开始执行
	bool Begin()
	{
		// must be reset after begin
		m_need_reset = true;

		// sql must not be empty
		if (m_sql.size() == 0) return false;

		// connect must not be null
		if (!m_conn)
		{
			End();
			CARP_ERROR("connection is null, sql:" << m_sql);
			return false;
		}

		// get mysql
		auto* mysql = m_conn->GetMysql();
		if (mysql == nullptr)
		{
			CARP_ERROR("mysql is null, sql:" << m_sql);
			End();
			return false;
		}

		int query_error = 0;
		int query_result = mysql_query(mysql, m_sql.c_str());
		if (query_result != 0)
		{
			query_error = mysql_errno(mysql);
			if (query_error == CR_SERVER_GONE_ERROR || query_error == CR_SERVER_LOST)
			{
				CARP_ERROR("error:" << query_error << " " << mysql_error(mysql) << " and try reconnect, sql:" << m_sql);
				if (!m_conn->ReOpen())
				{
					End();
					return false;
				}
				mysql = m_conn->GetMysql();
				if (mysql == nullptr)
				{
					CARP_ERROR("mysql is null, sql:" << m_sql);
					End();
					return false;
				}
				query_error = 0;
				query_result = mysql_query(mysql, m_sql.c_str());
				if (query_result != 0) query_error = mysql_errno(mysql);
			}
		}

		if (query_result != 0)
		{
			CARP_ERROR("error:" << query_error << " " << mysql_error(mysql) << " sql:" << m_sql);
			End();
			return false;
		}

		m_affect_count = mysql_affected_rows(mysql);

		m_row_count = 0;
		m_col_count = 0;
		m_row_index = -1;
		m_data.resize(0);

		MYSQL_RES* result = mysql_store_result(mysql);
		if (result != nullptr)
		{
			m_row_count = mysql_num_rows(result);
			m_col_count = mysql_num_fields(result);

			// copy data
			m_data.resize(m_row_count);
			for (int64_t row = 0; row < m_row_count; ++row)
			{
				auto rowData = mysql_fetch_row(result);
				if (rowData == nullptr)
				{
					CARP_ERROR("error:" << mysql_error(mysql) << " sql:" << m_sql);
					End();
					return false;
				}
				m_data[row].resize(m_col_count);
				for (int64_t col = 0; col < m_col_count; ++col)
				{
					if (rowData[col] != nullptr)
						m_data[row][col] = rowData[col];
				}
			}
			mysql_free_result(result);
		}

		while (true)
		{
			query_result = mysql_next_result(mysql);
			if (query_result == -1) break;

			if (query_result == 0)
			{
				auto result = mysql_store_result(mysql);
				if (result != nullptr) mysql_free_result(result);
			}
			else
			{
				CARP_WARN("error:" << mysql_error(mysql) << " sql:" << m_sql);
				break;
			}
		}

		return true;
	}
	// 清理
	void End()
	{
		m_data.resize(0);
		m_row_index = -1;
		m_row_count = 0;
		m_col_index = 0;
	}
	// 重置
	void Reset()
	{
		if (!m_need_reset) return;
		Clear();
	}

private:
	CarpMysqlConnection* m_conn = nullptr;            // 连接对象
	int64_t m_row_index = -1;                     // 当前行数据
	std::vector<std::vector<std::string>> m_data;

private:
	int64_t m_col_index = 0;
	int64_t m_row_count = 0;
	int64_t m_col_count = 0;
	int64_t m_affect_count = 0;
	std::string m_sql;                // SQL string
	std::string m_empty;

private:
	bool m_need_reset = false;                // need reset or not
};

#endif