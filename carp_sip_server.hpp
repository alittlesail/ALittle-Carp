#ifndef CARP_SIP_SERVER_INCLUDED
#define CARP_SIP_SERVER_INCLUDED

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <map>

#include "carp_udp_server.hpp"
#include "carp_schedule.hpp"
#include "carp_string.hpp"
#include "carp_crypto.hpp"

class CarpSipServer;
typedef std::shared_ptr<CarpSipServer> CarpSipServerPtr;
typedef std::weak_ptr<CarpSipServer> CarpSipServerWeakPtr;

class CarpSipServer : public std::enable_shared_from_this<CarpSipServer>
{
public:
	CarpSipServer() {}
	~CarpSipServer() { Close(); }

public:
	// 启动服务器
	// self_sip_ip self_sip_port 己端的SIP的IP和端口
	// remote_sip_ip remote_sip_port 对端的SIP的IP和端口
	// register_uri 对端的注册域名，比如移动的ims.fj.chinamobile.com
	// register_expires 对端注册的超时时间，一般是3600
	// sip_log SIP日志回调函数(type, call_id, info)
	// register_succeed 注册成功回调函数(nickname)
	bool Start(const std::string& self_sip_ip, unsigned int self_sip_port
		, const std::string& remote_sip_ip, unsigned int remote_sip_port
		, const std::string& register_uri, unsigned int register_expires
		, CarpSchedule* schedule
		, std::function<void(const std::string&, const std::string&, const std::string&)> sip_log
		, std::function<void(const std::string&)> register_succeed)
	{
		// 获取弱引用
		CarpSipServerWeakPtr self_weak_ptr = this->shared_from_this();

		// 创建线路rtp
		m_udp_self_sip = std::make_shared<CarpUdpServer>(schedule->GetIOService());
		m_udp_self_sip->RegisterUdpHandle(std::bind(HandleRemoteSip, std::placeholders::_1, self_weak_ptr));
		if (!m_udp_self_sip->Start(self_sip_ip, self_sip_port))
		{
			Close();
			return false;
		}

		m_self_sip_ip = self_sip_ip;
		m_self_sip_port = self_sip_port;
		m_self_uri = m_self_sip_ip + ":" + std::to_string(m_self_sip_port);
		
		m_remote_sip_ip = remote_sip_ip;
		m_remote_sip_port = remote_sip_port;
		m_register_uri = register_uri;
		m_register_expires = register_expires;
		if (m_register_uri.empty()) m_register_uri = m_remote_sip_ip + ":" + std::to_string(m_remote_sip_port);
		m_remote_sip_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(m_remote_sip_ip), m_remote_sip_port);

		m_sip_log = std::move(sip_log);
		m_register_succeed = std::move(register_succeed);
		return true;
	}

	// 关闭服务器
	void Close()
	{
		if (m_udp_self_sip) m_udp_self_sip->Close();
		m_udp_self_sip = CarpUdpServerPtr();
	}

public:
	// 注册
	void RegisterAccount(const std::string& account, const std::string& password)
	{
		m_account_map[account] = password;

		const auto via_branch = CarpCrypto::StringMd5(CarpString::GenerateID("via_branch"));
		const auto from_tag = CarpCrypto::StringMd5(CarpString::GenerateID("from_tag"));
		const auto call_id = CarpCrypto::StringMd5(CarpString::GenerateID("call_id"));
		
		const std::string cmd = "REGISTER sip:" + m_register_uri + " SIP/2.0\r\n"
			"Via: SIP/2.0/UDP " + m_self_uri + ";rport;branch=z9hG4bK-" + via_branch + "\r\n"
			"Max-Forwards: 70\r\n"
			"Contact: <sip:" + account + "@" + m_self_uri + ">\r\n"
			"From: <sip:" + account + "@" + m_register_uri + ">;tag=" + from_tag + "\r\n"
			"To: <sip:" + account + "@" + m_register_uri + ">\r\n"
			"Call-ID: " + call_id + "\r\n"
			"CSeq: 1 REGISTER\r\n"
			"Expires: " + std::to_string(m_register_expires) + "\r\n"
			"Allow: INVITE,ACK,CANCEL,OPTIONS,BYE,REFER,NOTIFY,INFO,MESSAGE,SUBSCRIBE,INFO\r\n"
			"User-Agent: ALittle\r\n"
			"Content-Length: 0\r\n\r\n";

		m_udp_self_sip->Send(cmd, m_remote_sip_endpoint);
		if (m_sip_log) m_sip_log("SEND", call_id, cmd);
	}

	// 清理账号
	void ClearAccount() { m_account_map.clear(); }

private:
	// 处理
	bool HandleRegisterAuth(const std::string& call_id, const std::string& status, const std::vector<std::string>& content_list)
	{
		// 获取CSeq
		int seq = 0;
		std::string method;
		if (!GetCSeqFromHeader(content_list, seq, method)) return false;

		// 判定是不是REGISTER
		if (method != "REGISTER") return false;

		std::string account;
		std::string from_tag;
		if (!GetFromFromHeader(content_list, account, from_tag)) return false;

		// 注册成功
		if (status == "200")
		{
			if (m_register_succeed) m_register_succeed(account);
			return true;
		}

		// 处理鉴权
		if (status == "401")
		{
			std::string nonce, realm, auth;

			auto it = m_account_map.find(account);
			if (it == m_account_map.end())
			{
				CARP_ERROR("can't find account by account:" << account);
				return true;
			}
			auto password = it->second;

			bool result = CalcAuth("WWW-AUTHENTICATE", content_list
				, account, password
				, "REGISTER", "sip:" + m_register_uri
				, nonce, realm, auth);

			if (nonce.empty())
			{
				CARP_ERROR("can't find nonce");
				return true;
			}
			if (realm.empty())
			{
				CARP_ERROR("can't find realm");
				return true;
			}

			const auto via_branch = CarpCrypto::StringMd5(CarpString::GenerateID("via_branch"));

			std::string cmd = "REGISTER sip:" + m_register_uri + " SIP/2.0\r\n"
				"Via: SIP/2.0/UDP " + m_self_uri + ";rport;branch=z9hG4bK-" + via_branch + "\r\n"
				"Max-Forwards: 70\r\n"
				"Contact: <sip:" + account + "@" + m_self_uri + ">\r\n"
				"From: <sip:" + account + "@" + m_register_uri + ">;tag=" + from_tag + "\r\n"
				"To: <sip:" + account + "@" + m_register_uri + ">\r\n"
				"Call-ID: " + call_id + "\r\n"
				"CSeq: " + std::to_string(seq + 1) + " REGISTER\r\n"
				"Expires: " + std::to_string(m_register_expires) + "\r\n"
				"Allow: INVITE,ACK,CANCEL,OPTIONS,BYE,REFER,NOTIFY,INFO,MESSAGE,SUBSCRIBE,INFO\r\n"
				"Supported: eventlist\r\n"
				"User-Agent: ALittle\r\n"
				"Authorization: " + auth + "\r\n"
				"Content-Length: 0\r\n\r\n";

			m_udp_self_sip->Send(cmd, m_remote_sip_endpoint);
			if (m_sip_log) m_sip_log("SEND", call_id, cmd);
			return true;
		}

		return true;
	}


private:
	// 与线路方互发sip的udp
	CarpUdpServerPtr m_udp_self_sip;
	std::string m_self_sip_ip;
	unsigned int m_self_sip_port = 0;
	std::string m_self_uri;

	// 对端的信息
	std::string m_remote_sip_ip;
	unsigned int m_remote_sip_port = 0;
	std::string m_register_uri;
	unsigned int m_register_expires = 0;
	asio::ip::udp::endpoint m_remote_sip_endpoint;

	// 日志回调
	std::function<void(const std::string&, const std::string&, const std::string&)> m_sip_log;
	// 注册成功回调
	std::function<void(const std::string&)> m_register_succeed;

private:
	std::map<std::string, std::string> m_account_map;

private:
	static bool GetNonceRealm(const std::string& head, const std::vector<std::string>& content_list, std::string& nonce, std::string& realm)
	{
		const std::string content = GetKeyValueFromHeader(content_list, head);
		{
			size_t pos_1 = content.find("nonce=\"");
			if (pos_1 == std::string::npos) return false;

			pos_1 += strlen("nonce=\"");
			const size_t pos_2 = content.find('\"', pos_1);
			if (pos_2 == std::string::npos) return false;

			nonce = content.substr(pos_1, pos_2 - pos_1);
		}
		{
			size_t pos_1 = content.find("realm=\"");
			if (pos_1 == std::string::npos) return false;

			pos_1 += strlen("realm=\"");
			const size_t pos_2 = content.find('\"', pos_1);
			if (pos_2 == std::string::npos) return false;

			realm = content.substr(pos_1, pos_2 - pos_1);
		}
		return true;
	}

	static bool GetNextNonce(const std::string& head, const std::vector<std::string>& content_list, std::string& nonce)
	{
		const std::string content = GetKeyValueFromHeader(content_list, head);
		{
			size_t pos_1 = content.find("nextnonce=\"");
			if (pos_1 == std::string::npos) return false;

			pos_1 += strlen("nextnonce=\"");
			const size_t pos_2 = content.find("\"", pos_1);
			if (pos_2 == std::string::npos) return false;

			nonce = content.substr(pos_1, pos_2 - pos_1);
		}
		return true;
	}

	static std::string GenAuth(const std::string& nonce, const std::string& realm
		, const std::string& account, const std::string& password
		, const std::string& method, const std::string& uri)
	{
		const std::string response_1 = CarpCrypto::StringMd5(account + ":" + realm + ":" + password);
		const std::string response_2 = CarpCrypto::StringMd5(method + ":" + uri);
		const std::string response = CarpCrypto::StringMd5(response_1 + ":" + nonce + ":" + response_2);

		return "Digest username=\"" + account + "\",realm=\"" + realm + "\",nonce=\"" + nonce + "\",uri=\"" + uri + "\",response=\"" + response + "\",algorithm=MD5";
	}

	static bool CalcAuth(const std::string& head, const std::vector<std::string>& content_list
		, const std::string& account, const std::string& password
		, const std::string& method, const std::string& uri
		, std::string& nonce, std::string& realm, std::string& auth)
	{
		if (GetNonceRealm(head, content_list, nonce, realm) == false) return false;

		const std::string response_1 = CarpCrypto::StringMd5(account + ":" + realm + ":" + password);
		const std::string response_2 = CarpCrypto::StringMd5(method + ":" + uri);
		const std::string response = CarpCrypto::StringMd5(response_1 + ":" + nonce + ":" + response_2);

		auth = GenAuth(nonce, realm, account, password, method, uri);
		return true;
	}

private:
	// sip数据包处理
	static void HandleRemoteSip(CarpUdpServer::HandleInfo& info, CarpSipServerWeakPtr self)
	{
		// 获取Transfer
		CarpSipServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// 拆分SIP协议
		std::vector<std::string> content_list;
		CarpString::Split(info.memory, "\r\n", true, content_list);
		if (content_list.empty()) return;

		// 查找CALL-ID
		const std::string call_id = GetKeyValueFromHeader(content_list, "CALL-ID");
		if (call_id.empty()) return;

		// 获取接受日志
		if (self_ptr->m_sip_log) self_ptr->m_sip_log("RECEIVE", call_id, info.memory);

		// 拆分第一行
	    std::vector<std::string> first_line_split;
		CarpString::Split(content_list[0], " ", true, first_line_split);
		if (first_line_split.empty())
		{
			if (self_ptr->m_sip_log) self_ptr->m_sip_log("RECEIVE", call_id, "first_line_split.empty()");
			return;
		}

		// 获得方法
		const std::string method = first_line_split[0];

		std::string status = "";
		// 如果是应答包，那么就获取状态
		if (method == "SIP/2.0")
		{
			if (first_line_split.size() < 2)
			{
				if (self_ptr->m_sip_log) self_ptr->m_sip_log("RECEIVE", call_id, "first_line_split.size() < 2");
				return;
			}
			status = first_line_split[1];
		}

		if (self_ptr->HandleRegisterAuth(call_id, status, content_list)) return;
	}

	// 查找并获得对应字段的值
	static std::string GetKeyValueFromHeader(const std::vector<std::string>& content_list, const std::string& upper_key)
	{
		// 跳过第一行
		for (const auto& content : content_list)
		{
			std::list<std::string> key_value;
			CarpString::Split(content, ":", true, key_value);
			if (key_value.size() >= 2)
			{
				CarpString::UpperString(key_value.front());
				CarpString::TrimLeft(key_value.front());
				CarpString::TrimRight(key_value.front());
				if (key_value.front() == upper_key)
				{
					key_value.pop_front();
					std::string result = CarpString::Join(key_value, ":");
					CarpString::TrimLeft(result);
					CarpString::TrimRight(result);
					return result;
				}
			}
		}
		return "";
	}

	static bool GetViaFromHeader(const std::vector<std::string>& content_list, std::string& to_sip_ip, std::string& to_sip_port, std::string& branch)
	{
		std::string value = GetKeyValueFromHeader(content_list, "VIA");
		if (value.empty()) return false;

		// find out first space and jump SIP/2.0/UDP
		size_t pos = value.find(' ');
		if (pos == std::string::npos || pos + 1 == value.size()) return false;
		value = value.substr(pos + 1);

		std::vector<std::string> split_list;
		CarpString::Split(value, ";", true, split_list);

		// get first info
		if (split_list.empty()) return false;
		std::vector<std::string> split_list_sip;
		CarpString::Split(split_list[0], ":", true, split_list_sip);
		if (split_list_sip.empty()) return false;
		to_sip_ip = split_list_sip[0];
		to_sip_port = "5060";
		if (split_list_sip.size() > 1) to_sip_port = split_list_sip[1];

		branch = "";
		// find branch
		for (const auto& split : split_list)
		{
			pos = split.find("branch=");
			if (pos == std::string::npos || pos + 7 == split.size()) continue;
			branch = split.substr(pos + 7);
			break;
		}
		return true;
	}

	static bool GetFromFromHeader(const std::vector<std::string>& content_list, std::string& from_nickname, std::string& from_tag)
	{
		const std::string value = GetKeyValueFromHeader(content_list, "FROM");
		if (value.empty()) return false;

		size_t pos_begin = value.find("sip:");
		if (pos_begin == std::string::npos || pos_begin == value.size() - 4)
		{
			pos_begin = value.find("tel:");
			if (pos_begin == std::string::npos || pos_begin == value.size() - 4)
				return false;
		}
		size_t pos_end = value.find('@');
		if (pos_end == std::string::npos)
		{
			pos_end = value.find(';');
			if (pos_end == std::string::npos)
				pos_end = value.find('>');
			if (pos_end == std::string::npos)
				return false;
		}
		if (pos_begin >= pos_end) return false;

		from_nickname = value.substr(pos_begin + 4, pos_end - pos_begin - 4);

		std::vector<std::string> split_list;
		CarpString::Split(value, "tag=", true, split_list);
		if (split_list.size() == 2) from_tag = split_list[1];

		return true;
	}

	static bool GetToFromHeader(const std::vector<std::string>& content_list, std::string& to_nickname, std::string& to_tag)
	{
		const std::string value = GetKeyValueFromHeader(content_list, "TO");
		if (value.empty()) return false;

		size_t pos_begin = value.find("sip:");
		if (pos_begin == std::string::npos || pos_begin + 4 == value.size())
		{
			pos_begin = value.find("tel:");
			if (pos_begin == std::string::npos || pos_begin + 4 == value.size())
				return false;
		}
		size_t pos_end = value.find('@');
		if (pos_end == std::string::npos)
		{
			pos_end = value.find(';');
			if (pos_end == std::string::npos)
				pos_end = value.find('>');
			if (pos_end == std::string::npos)
				return false;
		}
		if (pos_begin >= pos_end) return false;

		to_nickname = value.substr(pos_begin + 4, pos_end - pos_begin - 4);

		std::vector<std::string> split_list;
		CarpString::Split(value, "tag=", true, split_list);
		if (split_list.size() == 2) to_tag = split_list[1];

		return true;
	}

	static bool GetCSeqFromHeader(const std::vector<std::string>& content_list, int& seq, std::string& method)
	{
		const auto value = GetKeyValueFromHeader(content_list, "CSEQ");
		std::vector<std::string> split;
		CarpString::Split(value, " ", true, split);
		if (split.size() != 2) return false;

		seq = std::atoi(split[0].c_str());
		method = split[1];
		return true;
	}

	static bool GetAudioInfoFromSDP(const std::vector<std::string>& content_list
		, std::string& audio_name, std::string& audio_number
		, std::string& dtmf_rtpmap, std::string& dtmf_fmtp, std::string& dtmf_number
		, std::string& p_rtp_ip, int& p_rtp_port)
	{
		for (const auto& content : content_list)
		{
			if (content.empty()) continue;

			const size_t pos = content.find("c=IN IP4 ");
			if (pos == std::string::npos || pos + 9 == content.size()) continue;
			p_rtp_ip = content.substr(pos + 9);
			break;
		}
		for (const auto& content : content_list)
		{
			if (content.empty()) continue;

			const size_t pos = content.find("m=audio ");
			if (pos == std::string::npos) continue;

			std::vector<std::string> split_list;
			CarpString::Split(content, " ", true, split_list);
			if (split_list.size() >= 2) p_rtp_port = std::atoi(split_list[1].c_str());
			break;
		}

		for (const auto& content : content_list)
		{
			if (content.empty()) continue;
			if (content[0] != 'a') continue;

			size_t pos = content.find('G');
			if (pos == std::string::npos || pos + 1 == content.size())
				pos = content.find('P');
			if (pos == std::string::npos || pos + 1 == content.size()) continue;
			const size_t pos_2 = content.find('/');
			if (pos_2 == std::string::npos) break;
			audio_name = content.substr(pos, pos_2 - pos);

			const size_t pos_3 = content.find(':');
			if (pos_3 == std::string::npos || pos_3 + 1 == content.size()) break;
			if (pos_3 > pos) break;

			audio_number = content.substr(pos_3 + 1, pos - pos_3 - 2);
			break;
		}

		for (const auto& content : content_list)
		{
			if (content.empty()) continue;
			if (content[0] != 'a') continue;

			// telephone-event
			const size_t pos = content.find("telephone-event");
			if (pos != std::string::npos)
			{
				dtmf_rtpmap = content;
				const size_t pos_1 = content.find(':');
				const size_t pos_2 = content.find(' ');
				if (pos_1 != std::string::npos && pos_2 != std::string::npos)
					dtmf_number = content.substr(pos_1 + 1, pos_2 - pos_1 - 1);
				break;
			}
		}

		const std::string fmtp_number = "fmtp:" + dtmf_number;
		for (const auto& content : content_list)
		{
			if (content.empty()) continue;
			if (content[0] != 'a') continue;

			const size_t pos = content.find(fmtp_number);
			if (pos != std::string::npos)
			{
				dtmf_fmtp = content;
				break;
			}
		}

		if (audio_name.empty())
		{
			audio_name = "PCMA";
			audio_number = "8";
		}

		return true;
	}
};


#endif