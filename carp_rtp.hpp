#ifndef CARP_RTP_INCLUDED
#define CARP_RTP_INCLUDED

#include <memory>
#include <string>
#include <vector>
#include <map>

#define CARP_RTP_VERSION 2 // RTP version field must equal 2 (p66)

// 这个结构体正好是4个uint32，已经字节对齐了
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

class CarpRtp
{
public:
	static uint16_t CarpRtpReadUint16(const uint8_t* ptr) { return (static_cast<uint16_t>(ptr[0]) << 8) | ptr[1]; }
	static uint32_t CarpRtpReadUint32(const uint8_t* ptr) { return (static_cast<uint32_t>(ptr[0]) << 24) | (static_cast<uint32_t>(ptr[1]) << 16) | (static_cast<uint32_t>(ptr[2]) << 8) | ptr[3]; }
	static void CarpRtpWriteUint16(uint8_t* ptr, uint16_t val)
	{
		ptr[0] = static_cast<uint8_t>(val >> 8);
		ptr[1] = static_cast<uint8_t>(val);
	}
	static void CarpRtpWriteUint32(uint8_t* ptr, uint32_t val)
	{
		ptr[0] = static_cast<uint8_t>(val >> 24);
		ptr[1] = static_cast<uint8_t>(val >> 16);
		ptr[2] = static_cast<uint8_t>(val >> 8);
		ptr[3] = static_cast<uint8_t>(val);
	}
	static void CarpWriteRtpHeader(uint8_t* ptr, const CarpRtpHeader* header)
	{
		ptr[0] = static_cast<uint8_t>((header->v << 6) | (header->p << 5) | (header->x << 4) | header->cc);
		ptr[1] = static_cast<uint8_t>((header->m << 7) | header->pt);
		ptr[2] = static_cast<uint8_t>(header->seq >> 8);
		ptr[3] = static_cast<uint8_t>(header->seq & 0xFF);

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
	static bool CarpRtpPacketDeserialize(struct CarpRtpPacket* pkt, const void* data, size_t bytes)
	{
		if (bytes < CARP_RTP_FIXED_HEADER) // RFC3550 5.1 RTP Fixed Header Fields(p12)
			return false;

		const auto* ptr = static_cast<const uint8_t*>(data);
		memset(pkt, 0, sizeof(CarpRtpPacket));

		// pkt header
		const uint32_t v = CarpRtpReadUint32(ptr);
		pkt->rtp.v = CARP_RTP_V(v);
		pkt->rtp.p = CARP_RTP_P(v);
		pkt->rtp.x = CARP_RTP_X(v);
		pkt->rtp.cc = CARP_RTP_CC(v);
		pkt->rtp.m = CARP_RTP_M(v);
		pkt->rtp.pt = CARP_RTP_PT(v);
		pkt->rtp.seq = CARP_RTP_SEQ(v);
		pkt->rtp.timestamp = CarpRtpReadUint32(ptr + 4);
		pkt->rtp.ssrc = CarpRtpReadUint32(ptr + 8);
		if (CARP_RTP_VERSION != pkt->rtp.v) return false;

		const size_t hdr_len = CARP_RTP_FIXED_HEADER + pkt->rtp.cc * 4;
		if (CARP_RTP_VERSION != pkt->rtp.v || bytes < hdr_len + (pkt->rtp.x ? 4 : 0) + (pkt->rtp.p ? 1 : 0))
			return false;

		// pkt contributing source
		for (uint32_t i = 0; i < pkt->rtp.cc; i++)
			pkt->csrc[i] = CarpRtpReadUint32(ptr + 12 + i * 4);

		if (bytes < hdr_len) return false;
		pkt->payload = const_cast<uint8_t*>(ptr) + hdr_len;
		pkt->payloadlen = static_cast<int>(bytes - hdr_len);

		// pkt header extension
		if (1 == pkt->rtp.x)
		{
			const uint8_t* rtp_ext = ptr + hdr_len;
			if (pkt->payloadlen < 4) return false;
			pkt->extension = rtp_ext + 4;
			pkt->reserved = CarpRtpReadUint16(rtp_ext);
			pkt->extlen = CarpRtpReadUint16(rtp_ext + 2) * 4;
			if (pkt->extlen + 4 > pkt->payloadlen) return false;
			pkt->payload = rtp_ext + pkt->extlen + 4;
			pkt->payloadlen -= pkt->extlen + 4;
		}

		// padding
		if (1 == pkt->rtp.p)
		{
			uint8_t padding = ptr[bytes - 1];
			if (pkt->payloadlen < padding) return false;
			pkt->payloadlen -= padding;
		}

		return true;
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
		data.resize(0);
		if (!CarpRtpPacketSerializeHeader(pkt, data)) return false;
		const size_t index = data.size();
		data.resize(data.size() + pkt->payloadlen, 0);
		memcpy(data.data() + index, pkt->payload, pkt->payloadlen);
		return true;
	}
};

#endif