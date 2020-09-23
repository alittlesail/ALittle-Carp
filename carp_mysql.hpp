#ifndef CARP_MYSQL_INCLUDED
#define CARP_MYSQL_INCLUDED (1)

#include <map>

#include "Mysql/mysql.h"
#include "Mysql/errmsg.h"

#include "Carp/carp_log.hpp"

class MysqlConnection
{
public:
	MysqlConnection() {}
	~MysqlConnection() { Close(); }

public:
	/**
	 * invoke before use mysql
	 */
	static void Setup()
	{
		if (mysql_library_init(0, NULL, NULL))
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
		m_mysql = mysql_init(0);

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
			mysql_close(m_mysql); m_mysql = 0;
			CARP_ERROR("mysql_options failed: MYSQL_REPORT_DATA_TRUNCATION is unknow option");
			return false;
		}
		// use utf8
		if (mysql_options(m_mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4"))
		{
			mysql_close(m_mysql); m_mysql = 0;
			CARP_ERROR("mysql_options failed: MYSQL_SET_CHARSET_NAME is unknow option");
			return false;
		}
		// can auto reconnect
		bool_option = false;
		if (mysql_options(m_mysql, MYSQL_OPT_RECONNECT, &bool_option))
		{
			mysql_close(m_mysql); m_mysql = 0;
			CARP_ERROR("mysql_options failed: MYSQL_OPT_RECONNECT is unknow option");
			return false;
		}

		// mysql temp
		MYSQL* mysql = 0;

		// start connect
		mysql = mysql_real_connect(m_mysql
			, ip
			, username
			, password
			, db_name
			, port
			, 0
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
		mysql_close(m_mysql); m_mysql = 0;

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
	bool Ping()
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
		StmtMap::iterator it, end = m_stmt_map.end();
		for (it = m_stmt_map.begin(); it != end; ++it)
		{
			for (size_t i = 0; i < it->second->bind_output.size(); ++i)
			{
				if (it->second->bind_output[i].buffer != nullptr)
					free(it->second->bind_output[i].buffer);
			}
			// free stmt
			mysql_stmt_close(it->second->stmt);
			// flag stmt to 0
			it->second->stmt = 0;
		}
		m_stmt_map.clear();

		// release mysql object
		mysql_close(m_mysql);
		m_mysql = 0;
	}

	/* current is open
	 * @return open or not
	 */
	bool IsOpen() const { return m_mysql != 0; }

public:
	// mysql stmt info
	struct MysqlStmtInfo
	{
		MYSQL_STMT* stmt;									// stmt object
		std::vector<MYSQL_BIND> bind_output;				// output bind for read
		std::vector<unsigned long> value_length;		// output max length for read
		MysqlConnection* conn;								// come from
	};
	typedef std::shared_ptr<MysqlStmtInfo> MysqlStmtInfoPtr;

	/* get stmt object
	 * @param sql: sql string
	 * @return MysqlStmtInfo
	 */
	MysqlStmtInfoPtr GetStmt(const std::string& sql, bool& need_reconnect)
	{
		need_reconnect = false;
		if (!m_mysql)
		{
			CARP_ERROR("m_mysql is null!");
			return MysqlStmtInfoPtr();
		}

		// find stmt
		auto it = m_stmt_map.find(sql);
		if (it != m_stmt_map.end()) return it->second;

		// mysql stmt info
		MysqlStmtInfoPtr info = MysqlStmtInfoPtr(new MysqlStmtInfo);
		info->conn = this;

		// create stmt
		info->stmt = mysql_stmt_init(m_mysql);
		if (!info->stmt)
		{
			CARP_ERROR("mysql_stmt_init failed: out of memory!");
			return MysqlStmtInfoPtr();
		}

		// check succeed or not
		if (mysql_stmt_prepare(info->stmt, sql.c_str(), (unsigned long)sql.size()))
		{
			auto error = mysql_stmt_errno(info->stmt);
			need_reconnect = error == CR_SERVER_GONE_ERROR || error == CR_SERVER_LOST;
			CARP_ERROR("mysql_stmt_prepare failed: code:" << mysql_stmt_errno(info->stmt) << ", reason:" << mysql_stmt_error(info->stmt));
			mysql_stmt_close(info->stmt);
			return MysqlStmtInfoPtr();
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
					return MysqlStmtInfoPtr();
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
					// 这里定1024，是因为减少字符串阶段通知
					if (buffer_length <= 0 || buffer_length >= 1024) buffer_length = 1024;
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
		it->second->stmt = 0;

		m_stmt_map.erase(it);
		return true;
	}

	// execute simple sql
	bool ExecuteQuery(const char* sql, std::string& reason)
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

	typedef std::map<std::string, MysqlStmtInfoPtr> StmtMap;
	StmtMap m_stmt_map;

private:
	std::string m_db_name;
	std::string m_ip;
	std::string m_username;
	std::string m_password;
	int m_port = 0;
};

class MysqlStatementQuery
{
public:
	MysqlStatementQuery() {}
	~MysqlStatementQuery() { Clear(); }

	//===================================================================
public:
	/* set sql string
	 * @param sql: set sql string
	 * notice:!!!!  if you use SUM, then please used like this CAST(IFNULL(SUM(XXXX),0) AS UNSIGNED)
	 *				and use longlong to receive the value
	 * notice:!!!!  if you use COUNT, then please use longlong to receive the value
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
	void SetConnection(MysqlConnection* conn)
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
		bind.buffer_length = (unsigned long)size;
		bind.is_unsigned = is_unsigned;
		bind.buffer_type = buffer_type;

		m_bind_input.push_back(bind);
	}

public:
	void PushBool(bool param) { CommonBindForInput((void*)&param, sizeof(bool), false, MYSQL_TYPE_TINY); }
	void PushChar(char param) { CommonBindForInput((void*)&param, sizeof(char), false, MYSQL_TYPE_TINY); }
	void PushUChar(unsigned char param) { CommonBindForInput((void*)&param, sizeof(unsigned char), true, MYSQL_TYPE_TINY); }
	void PushShort(short param) { CommonBindForInput((void*)&param, sizeof(short), false, MYSQL_TYPE_SHORT); }
	void PushUShort(unsigned short param) { CommonBindForInput((void*)&param, sizeof(unsigned short), true, MYSQL_TYPE_SHORT); }
	void PushInt(int param) { CommonBindForInput((void*)&param, sizeof(int), false, MYSQL_TYPE_LONG); }
	void PushUInt(unsigned int param) { CommonBindForInput((void*)&param, sizeof(unsigned int), true, MYSQL_TYPE_LONG); }
	void PushLong(long param) { CommonBindForInput((void*)&param, sizeof(long), false, MYSQL_TYPE_LONG); }
	void PushULong(unsigned long param) { CommonBindForInput((void*)&param, sizeof(unsigned long), true, MYSQL_TYPE_LONG); }
	void PushLongLong(long long param) { CommonBindForInput((void*)&param, sizeof(long long), false, MYSQL_TYPE_LONGLONG); }
	void PushULongLong(unsigned long long param) { CommonBindForInput((void*)&param, sizeof(unsigned long long), true, MYSQL_TYPE_LONGLONG); }
	void PushFloat(float param) { CommonBindForInput((void*)&param, sizeof(float), false, MYSQL_TYPE_FLOAT); }
	void PushDouble(double param) { CommonBindForInput((void*)&param, sizeof(double), false, MYSQL_TYPE_DOUBLE); }
	void PushString(const char* param) { CommonBindForInput((void*)param, strlen(param), false, MYSQL_TYPE_STRING); }

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
		MYSQLRT_STRING = 6
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
			CARP_ERROR("m_col_index out of range:" << m_col_index << " >= " << m_col_count);
			return false;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count);
			return false;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		bool result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length);
			return false;
		}
		if (data.buffer_type != MYSQL_TYPE_TINY)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_TINY, is:" << data.buffer_type);
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
			CARP_ERROR("m_col_index out of range:" << m_col_index << " >= " << m_col_count);
			return '?';
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count);
			return '?';
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		char result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length);
			return '?';
		}
		if (data.buffer_type != MYSQL_TYPE_TINY)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_TINY, is:" << data.buffer_type);
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
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count);
			return 0;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count);
			return 0;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		short result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length);
			return 0;
		}
		if (data.buffer_type != MYSQL_TYPE_SHORT)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_SHORT, is:" << data.buffer_type);
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
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count);
			return 0;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count);
			return 0;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		int result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length);
			return 0;
		}
		if (data.buffer_type != MYSQL_TYPE_LONG)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_LONG, is:" << data.buffer_type);
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
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count);
			return 0;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count);
			return 0;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		long result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length);
			return 0;
		}
		if (data.buffer_type != MYSQL_TYPE_LONG)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_LONG, is:" << data.buffer_type);
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
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count);
			return 0;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count);
			return 0;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		long long result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length);
			return 0;
		}
		if (data.buffer_type != MYSQL_TYPE_LONGLONG)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_LONGLONG, is:" << data.buffer_type);
			return 0;
		}
		memcpy(&result, data.buffer.data(), sizeof(result));
		++m_col_index;
		if (m_col_index >= m_col_count) Next();
		return (int)result;
	}

	float ReadFloat()
	{
		if (m_col_index >= m_col_count)
		{
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count);
			return 0;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count);
			return 0;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		float result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length);
			return 0;
		}
		if (data.buffer_type != MYSQL_TYPE_FLOAT)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_FLOAT, is:" << data.buffer_type);
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
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count);
			return 0;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count);
			return 0;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		double result;
		if (data.value_length < sizeof(result))
		{
			CARP_ERROR("field length is too small:" << data.value_length);
			return 0;
		}
		if (data.buffer_type != MYSQL_TYPE_DOUBLE)
		{
			CARP_ERROR("field type is not MYSQL_TYPE_DOUBLE, is:" << data.buffer_type);
			return 0;
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
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count);
			m_temp_string.resize(0);
			return m_temp_string.c_str();
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count);
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
			CARP_ERROR("out of range:" << m_col_index << " >= " << m_col_count);
			return -1;
		}
		if (m_row_index >= m_row_count)
		{
			CARP_ERROR("m_row_index out of range:" << m_row_index << " >= " << m_row_count);
			return -1;
		}

		MysqlBind& data = m_bind_outputs[m_row_index][m_col_index];

		if (data.buffer_type == MYSQL_TYPE_TINY) return MYSQLRT_BOOL;
		if (data.buffer_type == MYSQL_TYPE_SHORT) return MYSQLRT_SHORT;
		if (data.buffer_type == MYSQL_TYPE_LONG) return MYSQLRT_INT;
		if (data.buffer_type == MYSQL_TYPE_LONGLONG) return MYSQLRT_LONGLONG;
		if (data.buffer_type == MYSQL_TYPE_FLOAT) return MYSQLRT_FLOAT;
		if (data.buffer_type == MYSQL_TYPE_DOUBLE) return MYSQLRT_DOUBLE;
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
		MysqlConnection::MysqlStmtInfoPtr stmt_info = m_conn->GetStmt(m_sql, need_reconnect);
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
		m_row_count = (unsigned int)mysql_stmt_num_rows(stmt_info->stmt);
		m_col_count = (unsigned int)stmt_info->bind_output.size();
		m_affect_count = (unsigned int)mysql_stmt_affected_rows(stmt_info->stmt);

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
					data.buffer_type = (enum_field_types)(stmt_info->bind_output[i].buffer_type);
					data.buffer.resize(stmt_info->value_length[i] + 1, 0);
					data.value_length = stmt_info->value_length[i];
					if (stmt_info->value_length[i] > 0)
						memcpy(data.buffer.data(), stmt_info->bind_output[i].buffer, stmt_info->value_length[i]);
				}
				++row;
			}
		}

		mysql_stmt_free_result(stmt_info->stmt);
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
	MysqlConnection* m_conn = nullptr;		// connect object

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

#endif