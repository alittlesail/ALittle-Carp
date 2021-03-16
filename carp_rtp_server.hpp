#ifndef CARP_RTP_SERVER_INCLUDED
#define CARP_RTP_SERVER_INCLUDED

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "carp_udp_server.hpp"
#include "carp_schedule.hpp"

typedef unsigned int CARP_RTP_CLIENT_ID;
typedef unsigned char CARP_RTP_CMD_TYPE;
typedef unsigned char CARP_RTP_PAYLOAD_TYPE;
typedef unsigned short CARP_RTP_SEQUENCE_NUMBER;
typedef unsigned int CARP_RTP_TIMESTAMP;

class CarpRtpServer;
typedef std::shared_ptr<CarpRtpServer> CarpRtpServerPtr;
typedef std::weak_ptr<CarpRtpServer> CarpRtpServerWeakPtr;

#define CARP_RTP_VERSION 2 // RTP version field must equal 2 (p66)

struct CarpRtpHeader
{
	uint32_t v : 2;		/* protocol version */
	uint32_t p : 1;		/* padding flag */
	uint32_t x : 1;		/* header extension flag */
	uint32_t cc : 4;		/* CSRC count */
	uint32_t m : 1;		/* marker bit */
	uint32_t pt : 7;		/* payload type */
	uint32_t seq : 16;	/* sequence number */
	uint32_t timestamp; /* timestamp */
	uint32_t ssrc;		/* synchronization source */
};

#define CARP_RTP_V(v)	((v >> 30) & 0x03) /* protocol version */
#define CARP_RTP_P(v)	((v >> 29) & 0x01) /* padding flag */
#define CARP_RTP_X(v)	((v >> 28) & 0x01) /* header extension flag */
#define CARP_RTP_CC(v)	((v >> 24) & 0x0F) /* CSRC count */
#define CARP_RTP_M(v)	((v >> 23) & 0x01) /* marker bit */
#define CARP_RTP_PT(v)	((v >> 16) & 0x7F) /* payload type */
#define CARP_RTP_SEQ(v)	((v >> 00) & 0xFFFF) /* sequence number */

#define CARP_RTP_FIXED_HEADER 12

struct CarpRtpPacket
{
	CarpRtpHeader rtp;
	uint32_t csrc[16];
	const void* extension; // extension(valid only if rtp.x = 1)
	uint16_t extlen; // extension length in bytes
	uint16_t reserved; // extension reserved
	const void* payload; // payload
	int payloadlen; // payload length in bytes
};

class CarpRtpServer : public std::enable_shared_from_this<CarpRtpServer>
{
public:
	CarpRtpServer() {}
	~CarpRtpServer() { Close(); }

public:
	static inline uint16_t CarpRtpReadUint16(const uint8_t* ptr) { return (((uint16_t)ptr[0]) << 8) | ptr[1]; }
	static inline uint32_t CarpRtpReadUint32(const uint8_t* ptr) { return (((uint32_t)ptr[0]) << 24) | (((uint32_t)ptr[1]) << 16) | (((uint32_t)ptr[2]) << 8) | ptr[3]; }
	static inline void CarpRtpWriteUint16(uint8_t* ptr, uint16_t val)
	{
		ptr[0] = (uint8_t)(val >> 8);
		ptr[1] = (uint8_t)val;
	}
	static inline void CarpRtpWriteUint32(uint8_t* ptr, uint32_t val)
	{
		ptr[0] = (uint8_t)(val >> 24);
		ptr[1] = (uint8_t)(val >> 16);
		ptr[2] = (uint8_t)(val >> 8);
		ptr[3] = (uint8_t)val;
	}
	static inline void CarpWriteRtpHeader(uint8_t* ptr, const CarpRtpHeader* header)
	{
		ptr[0] = (uint8_t)((header->v << 6) | (header->p << 5) | (header->x << 4) | header->cc);
		ptr[1] = (uint8_t)((header->m << 7) | header->pt);
		ptr[2] = (uint8_t)(header->seq >> 8);
		ptr[3] = (uint8_t)(header->seq & 0xFF);

		CarpRtpWriteUint32(ptr + 4, header->timestamp);
		CarpRtpWriteUint32(ptr + 8, header->ssrc);
	}

	// RFC3550 RTP: A Transport Protocol for Real-Time Applications
	// 5.1 RTP Fixed Header Fields (p12)
	/*
	 0               1               2               3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|V=2|P|X|   CC  |M|     PT      |      sequence number          |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                           timestamp                           |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                synchronization source (SSRC) identifier       |
	+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
	|                 contributing source (CSRC) identifiers        |
	|                               ....                            |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	*/
	///@return 0-ok, other-error
	static int CarpRtpPacketDeserialize(struct CarpRtpPacket* pkt, const void* data, int bytes)
	{
		uint32_t i, v;
		int hdrlen;
		const uint8_t* ptr;

		if (bytes < CARP_RTP_FIXED_HEADER) // RFC3550 5.1 RTP Fixed Header Fields(p12)
			return -1;
		ptr = (const unsigned char*)data;
		memset(pkt, 0, sizeof(CarpRtpPacket));

		// pkt header
		v = CarpRtpReadUint32(ptr);
		pkt->rtp.v = CARP_RTP_V(v);
		pkt->rtp.p = CARP_RTP_P(v);
		pkt->rtp.x = CARP_RTP_X(v);
		pkt->rtp.cc = CARP_RTP_CC(v);
		pkt->rtp.m = CARP_RTP_M(v);
		pkt->rtp.pt = CARP_RTP_PT(v);
		pkt->rtp.seq = CARP_RTP_SEQ(v);
		pkt->rtp.timestamp = CarpRtpReadUint32(ptr + 4);
		pkt->rtp.ssrc = CarpRtpReadUint32(ptr + 8);
		assert(RTP_VERSION == pkt->rtp.v);

		hdrlen = CARP_RTP_FIXED_HEADER + pkt->rtp.cc * 4;
		if (CARP_RTP_VERSION != pkt->rtp.v || bytes < hdrlen + (pkt->rtp.x ? 4 : 0) + (pkt->rtp.p ? 1 : 0))
			return -1;

		// pkt contributing source
		for (i = 0; i < pkt->rtp.cc; i++)
			pkt->csrc[i] = CarpRtpReadUint32(ptr + 12 + i * 4);

		if (bytes < hdrlen) return -1;
		pkt->payload = (uint8_t*)ptr + hdrlen;
		pkt->payloadlen = bytes - hdrlen;

		// pkt header extension
		if (1 == pkt->rtp.x)
		{
			const uint8_t* rtpext = ptr + hdrlen;
			if (pkt->payloadlen < 4) return -1;
			pkt->extension = rtpext + 4;
			pkt->reserved = CarpRtpReadUint16(rtpext);
			pkt->extlen = CarpRtpReadUint16(rtpext + 2) * 4;
			if (pkt->extlen + 4 > pkt->payloadlen)
				return -1;

			pkt->payload = rtpext + pkt->extlen + 4;
			pkt->payloadlen -= pkt->extlen + 4;
		}

		// padding
		if (1 == pkt->rtp.p)
		{
			uint8_t padding = ptr[bytes - 1];
			if (pkt->payloadlen < padding)
				return -1;

			pkt->payloadlen -= padding;
		}

		return 0;
	}

	static bool CarpRtpPacketSerializeHeader(const CarpRtpPacket* pkt, std::vector<uint8_t>& data)
	{
		if (CARP_RTP_VERSION != pkt->rtp.v || 0 != (pkt->extlen % 4))
			return false;

		// RFC3550 5.1 RTP Fixed Header Fields(p12)
		const size_t hdr_len = CARP_RTP_FIXED_HEADER + pkt->rtp.cc * 4 + (pkt->rtp.x ? 4 : 0);
		if (data.size() < hdr_len + pkt->extlen) data.resize(hdr_len + pkt->extlen, 0);

		auto* ptr = data.data();
		CarpWriteRtpHeader(ptr, &pkt->rtp);
		ptr += CARP_RTP_FIXED_HEADER;

		// pkt contributing source
		for (uint32_t i = 0; i < pkt->rtp.cc; i++, ptr += 4)
			CarpRtpWriteUint32(ptr, pkt->csrc[i]);

		// pkt header extension
		if (1 == pkt->rtp.x)
		{
			// 5.3.1 RTP Header Extension
			if (pkt->extlen % 4 != 0) return false;
			CarpRtpWriteUint16(ptr, pkt->reserved);
			CarpRtpWriteUint16(ptr + 2, pkt->extlen / 4);
			memcpy(ptr + 4, pkt->extension, pkt->extlen);
			ptr += pkt->extlen + 4;
		}

		return true;
	}

	///@return <0-error, >0-rtp packet size, =0-impossible
	static bool CarpRtpPacketSerialize(const struct CarpRtpPacket* pkt, std::vector<uint8_t>& data)
	{
		if (!CarpRtpPacketSerializeHeader(pkt, data)) return false;
		size_t index = data.size();
		data.resize(data.size() + pkt->payloadlen);
		memcpy(data.data() + index, pkt->payload, pkt->payloadlen);
		return true;
	}


public:
	// 初始化
	// client_rtp_ip_list 表示要使用的ip，用于和客户端互发媒体包。
	//						因为客户端的网络可能是不同运营商，运营商之前会互相拦截数据包
	//						所以这里可以开多个不同的运营商ip，来兼容客户端不同的网络
	// self_rtp_ip 表示要使用的ip，用于和线路方互发RTP包
	// inner_rtp_ip 表示要使用的ip，用于内部rtp服务器转发媒体包，用于转接的场景
	bool Create(const std::vector<std::string>& client_rtp_ip_list, unsigned int client_rtp_port
		, const std::string& self_rtp_ip, unsigned int self_rtp_port
		, const std::string& inner_rtp_ip, unsigned int inner_rtp_port
		, CarpSchedule* schedule)
	{
		// 获取弱引用
		CarpRtpServerWeakPtr self_weak_ptr = this->shared_from_this();

		// 如果已经创建了，那么就直接返回true
		if (m_udp_self_rtp)
		{
			for (const auto& ip : client_rtp_ip_list)
			{
				auto it = m_udp_client_rtp_map.find(ip);
				if (it == m_udp_client_rtp_map.end())
					CARP_ERROR("can't find client ip:" << ip);
			}
			if (m_client_rtp_port != client_rtp_port)
				CARP_ERROR("error client port, m_client_rtp_port(" << m_client_rtp_port << ") != client_rtp_port(" << client_rtp_port << ")");
			return true;
		}

		// 创建客户端rtp
		for (const auto& ip : client_rtp_ip_list)
		{
			auto it = m_udp_client_rtp_map.find(ip);
			if (it != m_udp_client_rtp_map.end()) continue;

			auto udp_server = std::make_shared<CarpUdpServer>(schedule->GetIOService());
			CarpUdpServerWeakPtr self_udp_ptr = udp_server;
			udp_server->RegisterUdpHandle(std::bind(HandleClientRtp, std::placeholders::_1, self_weak_ptr, self_udp_ptr));
			if (!udp_server->Start(ip, client_rtp_port))
			{
				Close();
				return false;
			}
			m_udp_client_rtp_map[ip] = udp_server;
		}

		// 创建线路rtp
		m_udp_self_rtp = std::make_shared<CarpUdpServer>(schedule->GetIOService());
		m_udp_self_rtp->RegisterUdpHandle(std::bind(HandleRemoteRtp, std::placeholders::_1, self_weak_ptr));
		if (!m_udp_self_rtp->Start(self_rtp_ip, self_rtp_port))
		{
			Close();
			return false;
		}

		// 创建转发rtp
		m_udp_inner_rtp = std::make_shared<CarpUdpServer>(schedule->GetIOService());
		m_udp_inner_rtp->RegisterUdpHandle(std::bind(HandleInnerRtp, std::placeholders::_1, self_weak_ptr));
		if (!m_udp_inner_rtp->Start(inner_rtp_ip, inner_rtp_port))
		{
			Close();
			return false;
		}

		// 保存相关数据
		m_udp_client_rtp = CarpUdpServerPtr();
		m_client_rtp_port = client_rtp_port;
		m_self_rtp_ip = self_rtp_ip;
		m_self_rtp_port = self_rtp_port;
		m_inner_rtp_port = inner_rtp_port;

		return true;
	}

	// 启动一个电话媒体包
	bool Start(const std::string& call_id, unsigned int client_id, unsigned int ssrc)
	{
		// 打印正在使用的警告
		if (m_in_using) CARP_ERROR("m_call_id(" << m_call_id << "), m_client_rtp_port(" << m_client_rtp_port << "), already in used!");
		// 如果没有创建直接返回false
		if (!m_udp_self_rtp) return false;
		// 标记为正在使用
		m_in_using = true;

		// 打印日志
		CARP_INFO("CarpRtpServer Start: call_id:" << call_id
			<< ", client_rtp_port:" << m_client_rtp_port
			<< ", self_rtp_port:" << m_self_rtp_port);

		// 初始化相关数据
		m_udp_client_rtp = CarpUdpServerPtr();
		m_has_client_rtp_endpoint = false;
		m_has_remote_rtp_endpoint = false;
		m_has_inner_rtp_endpoint = false;
		m_ssrc = ssrc;
		m_client_id = client_id;
		m_call_id = call_id;
		return true;
	}

	// 停止一个电话媒体包
	void Stop()
	{
		// 打印日志
		CARP_INFO("CarpRtpServer Stop: call_id:" << m_call_id
			<< ", client_rtp_port:" << m_client_rtp_port
			<< ", self_rtp_port:" << m_self_rtp_port);

		// 标记为不在使用
		m_in_using = false;

		// 更新空闲时间
		m_idle_time = time(nullptr);
	}

	// 设置内部转发媒体包的ip和端口，以及语音编码
	void SetInnerRtp(const std::string& rtp_ip, unsigned int rtp_port)
	{
		CARP_INFO("===================Set Inner RTP======================");
		m_inner_rtp_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(rtp_ip), rtp_port);
		m_has_inner_rtp_endpoint = true;
	}
	// 设置和线路方互发RTP包的ip和端口
	void SetRemoteRtp(const std::string& rtp_ip, unsigned int rtp_port)
	{
		CARP_INFO("======Set Remote RTP======:(ip)" << rtp_ip << " (port)" << rtp_port << " " << m_call_id);
		m_remote_rtp_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(rtp_ip), rtp_port);
		m_has_remote_rtp_endpoint = true;
	}

	// 转移到客户端
	bool ChangeClient(unsigned int client_id)
	{
		if (!m_udp_self_rtp) return false;
		m_client_id = client_id;
		m_has_client_rtp_endpoint = false;
		m_udp_client_rtp = CarpUdpServerPtr();
		return true;
	}

	// 关闭rtp
	void Close()
	{
		for (auto& pair : m_udp_client_rtp_map)
			pair.second->Close();
		m_udp_client_rtp_map.clear();
		if (m_udp_self_rtp) m_udp_self_rtp->Close();
		m_udp_self_rtp = CarpUdpServerPtr();
		if (m_udp_inner_rtp) m_udp_inner_rtp->Close();
		m_udp_inner_rtp = CarpUdpServerPtr();
	}

public:
	time_t GetIdleTime() const { return m_idle_time; }

private:
	// 空闲时间
	time_t m_idle_time = 0;
	// 是否在使用中
	bool m_in_using = false;

private:
	// 所有用于与客户端互发的udp
	std::map<std::string, CarpUdpServerPtr> m_udp_client_rtp_map;
	// 实际正在使用的udp服务器
	CarpUdpServerPtr m_udp_client_rtp;
	unsigned int m_client_rtp_port = 0;
	bool m_has_client_rtp_endpoint = false;	// client talk
	asio::ip::udp::endpoint m_client_rtp_endpoint;

	// 与线路方互发rtp的udp
	CarpUdpServerPtr m_udp_self_rtp;
	std::string m_self_rtp_ip;
	unsigned int m_self_rtp_port = 0;
	bool m_has_remote_rtp_endpoint = false;	// phone system talk
	asio::ip::udp::endpoint m_remote_rtp_endpoint;

	// 内部转发的udp
	CarpUdpServerPtr m_udp_inner_rtp;
	unsigned int m_inner_rtp_port = 0;
	bool m_has_inner_rtp_endpoint = false; // inner rtp
	asio::ip::udp::endpoint m_inner_rtp_endpoint;

	// 与线路方识别媒体包的序列号
	unsigned int m_ssrc = 0;
	// 与客户端识别媒体包的序列号
	CARP_RTP_CLIENT_ID m_client_id = 0;

private:
	std::string m_call_id;		// SIP呼叫ID
	time_t m_last_receive_time = 0;	// 最后收到客户端数据包的时间
	std::vector<unsigned char> m_rtp_buffer; // 用于发送rtp的缓冲区

private:
	// rtp数据包处理
	static void HandleRemoteRtp(CarpUdpServer::HandleInfo& info, CarpRtpServerWeakPtr self)
	{
		// 获取Transfer
		CarpRtpServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// 检查是否正在使用
		if (self_ptr->m_in_using == false) return;

		// 检查是否设置好对端
		if (self_ptr->m_has_remote_rtp_endpoint == false || self_ptr->m_remote_rtp_endpoint != info.end_point) return;

		// 转发给内部
		if (self_ptr->m_has_inner_rtp_endpoint)
		{
			void* new_memory = malloc(info.memory_size);
			memcpy(new_memory, info.memory, info.memory_size);
			self_ptr->m_udp_inner_rtp->Send(new_memory, info.memory_size, self_ptr->m_inner_rtp_endpoint);
			return;
		}

		// 转发给客户端
		if (self_ptr->m_has_client_rtp_endpoint)
		{
			// 解包
			CarpRtpPacket pocket = { 0 };
			const auto ret = CarpRtpPacketDeserialize(&pocket, info.memory, static_cast<int>(info.memory_size));
			if (ret != 0)
			{
				CARP_ERROR("rtp_packet_deserialize failed!");
				return;
			}

			CARP_RTP_CMD_TYPE cmd_type = 'd'; // rtp 媒体数据
			CARP_RTP_PAYLOAD_TYPE payload_type = pocket.rtp.pt;
			CARP_RTP_SEQUENCE_NUMBER sequence_number = pocket.rtp.seq;
			CARP_RTP_TIMESTAMP timestamp = pocket.rtp.timestamp;

			// 创建客户端的rtp包。client_id,
			const size_t new_memory_size = sizeof(CARP_RTP_CLIENT_ID)
				+ sizeof(CARP_RTP_CMD_TYPE)
				+ sizeof(CARP_RTP_PAYLOAD_TYPE)
				+ sizeof(CARP_RTP_SEQUENCE_NUMBER)
				+ sizeof(CARP_RTP_TIMESTAMP)
				+ pocket.payloadlen;

			char* new_memory = static_cast<char*>(malloc(new_memory_size));
			char* body_memory = new_memory;
			memcpy(body_memory, &(self_ptr->m_client_id), sizeof(CARP_RTP_CLIENT_ID)); body_memory += sizeof(CARP_RTP_CLIENT_ID);
			memcpy(body_memory, &cmd_type, sizeof(CARP_RTP_CMD_TYPE)); body_memory += sizeof(CARP_RTP_CMD_TYPE);
			memcpy(body_memory, &payload_type, sizeof(CARP_RTP_PAYLOAD_TYPE)); body_memory += sizeof(CARP_RTP_PAYLOAD_TYPE);
			memcpy(body_memory, &sequence_number, sizeof(CARP_RTP_SEQUENCE_NUMBER)); body_memory += sizeof(CARP_RTP_SEQUENCE_NUMBER);
			memcpy(body_memory, &timestamp, sizeof(CARP_RTP_TIMESTAMP)); body_memory += sizeof(CARP_RTP_TIMESTAMP);
			memcpy(body_memory, pocket.payload, pocket.payloadlen);
			self_ptr->m_udp_client_rtp->Send(new_memory, new_memory_size, self_ptr->m_client_rtp_endpoint);
		}
	}
	static void HandleClientRtp(CarpUdpServer::HandleInfo& info, CarpRtpServerWeakPtr self, CarpUdpServerWeakPtr real_udp)
	{
		// 获取Transfer
		CarpRtpServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// 检查是否正在使用
		if (self_ptr->m_in_using == false) return;

		// 解析数据包
		const char* rtp_memory = info.memory;
		size_t check_len = info.memory_size;

		if (check_len < sizeof(CARP_RTP_CLIENT_ID))
		{
			CARP_ERROR("rtp package len error1:" << check_len);
			return;
		}
		CARP_RTP_CLIENT_ID client_id;
		memcpy(&client_id, rtp_memory, sizeof(client_id));
		rtp_memory += sizeof(client_id);
		check_len -= sizeof(client_id);

		if (check_len < sizeof(CARP_RTP_CMD_TYPE))
		{
			CARP_ERROR("rtp package len error2:" << check_len);
			return;
		}
		CARP_RTP_CMD_TYPE cmd_type = *rtp_memory;
		rtp_memory += sizeof(cmd_type);
		check_len -= sizeof(cmd_type);

		// 处理rtp媒体包
		if (cmd_type == 'd')
		{
			// 设置客户端
			if (self_ptr->m_has_client_rtp_endpoint == false && self_ptr->m_client_id == client_id)
			{
				self_ptr->m_client_rtp_endpoint = info.end_point;
				self_ptr->m_has_client_rtp_endpoint = true;
				self_ptr->m_udp_client_rtp = real_udp.lock();
				CARP_INFO("!!!!!!carp rtp!!!!!!:" << self_ptr->m_call_id);
			}

			// 间隔5秒发送一次数据包，用于反馈服务器有收到客户端的数据包
			// 因为有的客户端所在的网络和服务器的网络不连通，所以需要这个来检查
			time_t cur_time = time(nullptr);
			if (self_ptr->m_has_client_rtp_endpoint && cur_time - self_ptr->m_last_receive_time > 5)
			{
				self_ptr->m_last_receive_time = cur_time;

				const CARP_RTP_CMD_TYPE e_cmd_type = 'e'; // 控制信息

				// 发送反馈包
				size_t memory_size = sizeof(CARP_RTP_CLIENT_ID) + sizeof(CARP_RTP_CMD_TYPE) + sizeof(CARP_RTP_TIMESTAMP);
				char* memory = static_cast<char*>(malloc(memory_size));
				char* new_memory = memory;
				memcpy(new_memory, &(self_ptr->m_client_id), sizeof(CARP_RTP_CLIENT_ID)); new_memory += sizeof(CARP_RTP_CLIENT_ID);
				memcpy(new_memory, &e_cmd_type, sizeof(e_cmd_type)); new_memory += sizeof(e_cmd_type);
				memcpy(new_memory, &cur_time, sizeof(cur_time)); new_memory += sizeof(cur_time);
				self_ptr->m_udp_client_rtp->Send(memory, memory_size, self_ptr->m_client_rtp_endpoint);
			}

			// 如果没有线路，直接返回
			if (self_ptr->m_has_remote_rtp_endpoint == false) return;

			// get payload type
			if (check_len < sizeof(CARP_RTP_PAYLOAD_TYPE))
			{
				CARP_ERROR("rtp package len error3:" << check_len);
				return;
			}
			CARP_RTP_PAYLOAD_TYPE payload_type = 0;
			memcpy(&payload_type, rtp_memory, sizeof(payload_type));
			rtp_memory += sizeof(payload_type);
			check_len -= sizeof(payload_type);

			// get sequence number
			if (check_len < sizeof(CARP_RTP_SEQUENCE_NUMBER))
			{
				CARP_ERROR("rtp package len error4:" << check_len);
				return;
			}
			CARP_RTP_SEQUENCE_NUMBER sequence_number = 0;
			memcpy(&sequence_number, rtp_memory, sizeof(sequence_number));
			rtp_memory += sizeof(sequence_number);
			check_len -= sizeof(sequence_number);

			// get timestamp
			if (check_len < sizeof(CARP_RTP_TIMESTAMP))
			{
				CARP_ERROR("rtp package len error5:" << check_len);
				return;
			}
			CARP_RTP_TIMESTAMP timestamp = 0;
			memcpy(&timestamp, rtp_memory, sizeof(timestamp));
			rtp_memory += sizeof(timestamp);
			check_len -= sizeof(timestamp);

			// last len
			if (check_len <= 0)
			{
				CARP_ERROR("rtp package len error6:" << check_len);
				return;
			}

			CarpRtpPacket pocket = { 0 };
			pocket.payload = rtp_memory;
			pocket.payloadlen = static_cast<int>(check_len);
			pocket.rtp.v = CARP_RTP_VERSION;
			pocket.rtp.pt = payload_type;
			pocket.rtp.seq = sequence_number;
			pocket.rtp.timestamp = timestamp;
			pocket.rtp.ssrc = self_ptr->m_ssrc;

			int pocket_size = CarpRtpPacketSerialize(&pocket, self_ptr->m_rtp_buffer);
			if (pocket_size < 0)
			{
				CARP_ERROR("CarpRtpPacketSerialize failed check_len:" << check_len);
				return;
			}

			// 发送
			void* new_memory = malloc(pocket_size);
			memcpy(new_memory, self_ptr->m_rtp_buffer.data(), pocket_size);
			self_ptr->m_udp_self_rtp->Send(new_memory, pocket_size, self_ptr->m_remote_rtp_endpoint);
		}
	}
	static void HandleInnerRtp(CarpUdpServer::HandleInfo& info, CarpRtpServerWeakPtr self)
	{
		CarpRtpServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// check used
		if (self_ptr->m_in_using == false) return;
		if (self_ptr->m_has_remote_rtp_endpoint == false) return;

		// 转发给线路
		void* new_memory = malloc(info.memory_size);
		memcpy(new_memory, info.memory, info.memory_size);
		self_ptr->m_udp_self_rtp->Send(new_memory, info.memory_size, self_ptr->m_remote_rtp_endpoint);
	}
};


#endif