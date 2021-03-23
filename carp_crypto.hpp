
#ifndef CARP_CRYPT_INCLUDED
#define CARP_CRYPT_INCLUDED

#include <cstring>
#include <string>

#include "carp_lua.hpp"

class CarpCrypto
{
public:
	/**
	* Crypt block by XXTEA.
	* Params:
	*   block - block of input data
	*   len   - length of block, use XXTEA_BLOCK_SIZE
	*   key   - 128b key
	*/
#define XXTEA_BLOCK_SIZE 128
	static void XXTeaEncode(unsigned int* block, int len, unsigned int* key)
	{
		static unsigned char XXTEA_KEY[] = { '5','9','5','e','f','a','a','3','e','3','d','2','6','3','6','b' };
		
		if (len <= 1) return;

		unsigned int z = block[len - 1], y = block[0], sum = 0, e, DELTA = 0x9e3779b9;
		int p, q;

		if (!key) key = (unsigned int*)XXTEA_KEY;

		q = 6 + 52 / len;
		while (q-- > 0)
		{
			sum += DELTA;
			e = (sum >> 2) & 3;
			for (p = 0; p < len - 1; p++)
			{
				y = block[p + 1];
				block[p] += (z >> 5 ^ y << 2) + (y >> 3 ^ z << 4) ^ (sum ^ y) + (key[(p & 3) ^ e] ^ z);
				z = block[p];
			}
			y = block[0];
			block[len - 1] += (z >> 5 ^ y << 2) + (y >> 3 ^ z << 4) ^ (sum ^ y) + (key[(p & 3) ^ e] ^ z);
			z = block[len - 1];
		}
	}
	static void XXTeaEncodeMemory(char* memory, int size, const char* key)
	{
		if (!memory) return;

		char* real_key = 0;
		char adjust_key[16] = { 0 };
		if (key)
		{
			size_t key_len = strlen(key);
			for (size_t i = 0; i < key_len; ++i) adjust_key[i] = key[i];
			real_key = adjust_key;
		}

		while (size >= XXTEA_BLOCK_SIZE * sizeof(unsigned int))
		{
			XXTeaEncode((unsigned int*)memory, XXTEA_BLOCK_SIZE, (unsigned int*)real_key);
			memory += XXTEA_BLOCK_SIZE * sizeof(unsigned int);
			size -= XXTEA_BLOCK_SIZE * sizeof(unsigned int);
		}

		if (size >= sizeof(unsigned int))
			XXTeaEncode((unsigned int*)memory, size / sizeof(unsigned int), (unsigned int*)real_key);
	}

	/**
	* Decrypt block by XXTEA.
	* Params:
	*   block - block of encrypted data
	*   len   - length of block, use XXTEA_BLOCK_SIZE
	*   key   - 128bit key
	*/
	static void XXTeaDecode(unsigned int* block, int len, unsigned int* key)
	{
		static unsigned char XXTEA_KEY[] = { '5','9','5','e','f','a','a','3','e','3','d','2','6','3','6','b' };

		if (len <= 1) return;

		unsigned int z = block[len - 1], y = block[0], sum = 0, e, DELTA = 0x9e3779b9;
		int p, q;

		if (!key) key = (unsigned int*)XXTEA_KEY;

		q = 6 + 52 / len;
		sum = q * DELTA;
		while (sum != 0)
		{
			e = (sum >> 2) & 3;
			for (p = len - 1; p > 0; p--)
			{
				z = block[p - 1];
				block[p] -= (z >> 5 ^ y << 2) + (y >> 3 ^ z << 4) ^ (sum ^ y) + (key[(p & 3) ^ e] ^ z);
				y = block[p];
			}
			z = block[len - 1];
			block[0] -= (z >> 5 ^ y << 2) + (y >> 3 ^ z << 4) ^ (sum ^ y) + (key[(p & 3) ^ e] ^ z);
			y = block[0];
			sum -= DELTA;
		}
	}
	static void XXTeaDecodeMemory(char* memory, int size, const char* key)
	{
		if (!memory) return;

		char* real_key = 0;

		char adjust_key[16] = { 0 };
		if (key)
		{
			size_t key_len = strlen(key);
			for (size_t i = 0; i < key_len; ++i) adjust_key[i] = key[i];
			real_key = adjust_key;
		}

		while (size >= XXTEA_BLOCK_SIZE * sizeof(unsigned int))
		{
			XXTeaDecode((unsigned int*)memory, XXTEA_BLOCK_SIZE, (unsigned int*)real_key);
			memory += XXTEA_BLOCK_SIZE * sizeof(unsigned int);
			size -= XXTEA_BLOCK_SIZE * sizeof(unsigned int);
		}

		if (size >= sizeof(unsigned int))
			XXTeaDecode((unsigned int*)memory, size / sizeof(unsigned int), (unsigned int*)real_key);
	}

public:
	// ¼ÆËã¹þÏ£Öµ
	static int JSHash(const char* content)
	{
		if (content == nullptr) return 0;
		int l = static_cast<int>(strlen(content));
		int h = l;
		int step = (l >> 5) + 1;

		for (int i = l; i >= step; i -= step)
		{
			h = h ^ ((h << 5) + content[i - 1] + (h >> 2));
		}
		return h;
	}

public:
	/*
	 *  This structure will hold context information for the hashing
	 *  operation
	 */
	struct SHA1Context
	{
		unsigned Message_Digest[5]; /* Message Digest (output)          */

		unsigned Length_Low;        /* Message length in bits           */
		unsigned Length_High;       /* Message length in bits           */

		unsigned char Message_Block[64]; /* 512-bit message blocks      */
		int Message_Block_Index;    /* Index into message block array   */

		int Computed;               /* Is the digest computed?          */
		int Corrupted;              /* Is the message digest corruped?  */
	};

	static void Sha1(const std::string& msg, unsigned int dest[5])
	{
		SHA1Context sha = {0};
		SHA1Reset(&sha);
		SHA1Input(&sha, (const unsigned char*)msg.c_str(), (unsigned int)msg.size());
		if (!SHA1Result(&sha)) return;

		for (int i = 0; i < 5; ++i)
			dest[i] = sha.Message_Digest[i];
	}

private:
	/*
	 *  Define the circular shift macro
	 */
#define CARPSHA1CircularShift(bits,word) ((((word) << (bits)) & 0xFFFFFFFF) | ((word) >> (32-(bits))))

	/*
	 *  Function Prototypes
	 */
	static void SHA1Reset(SHA1Context* context)
	{
		context->Length_Low = 0;
		context->Length_High = 0;
		context->Message_Block_Index = 0;

		context->Message_Digest[0] = 0x67452301;
		context->Message_Digest[1] = 0xEFCDAB89;
		context->Message_Digest[2] = 0x98BADCFE;
		context->Message_Digest[3] = 0x10325476;
		context->Message_Digest[4] = 0xC3D2E1F0;

		context->Computed = 0;
		context->Corrupted = 0;
	}

	/*
	 *  SHA1Result
	 *
	 *  Description:
	 *      This function will return the 160-bit message digest into the
	 *      Message_Digest array within the SHA1Context provided
	 *
	 *  Parameters:
	 *      context: [in/out]
	 *          The context to use to calculate the SHA-1 hash.
	 *
	 *  Returns:
	 *      1 if successful, 0 if it failed.
	 *
	 *  Comments:
	 *
	 */
	static int SHA1Result(SHA1Context* context)
	{
		if (context->Corrupted) return 0;

		if (!context->Computed)
		{
			SHA1PadMessage(context);
			context->Computed = 1;
		}

		return 1;
	}

	/*
	 *  SHA1Input
	 *
	 *  Description:
	 *      This function accepts an array of octets as the next portion of
	 *      the message.
	 *
	 *  Parameters:
	 *      context: [in/out]
	 *          The SHA-1 context to update
	 *      message_array: [in]
	 *          An array of characters representing the next portion of the
	 *          message.
	 *      length: [in]
	 *          The length of the message in message_array
	 *
	 *  Returns:
	 *      Nothing.
	 *
	 *  Comments:
	 *
	 */
	static void SHA1Input(SHA1Context* context, const unsigned char* message_array, unsigned length)
	{
		if (!length) return;

		if (context->Computed || context->Corrupted)
		{
			context->Corrupted = 1;
			return;
		}

		while (length-- && !context->Corrupted)
		{
			context->Message_Block[context->Message_Block_Index++] = (*message_array & 0xFF);

			context->Length_Low += 8;
			/* Force it to 32 bits */
			context->Length_Low &= 0xFFFFFFFF;
			if (context->Length_Low == 0)
			{
				context->Length_High++;
				/* Force it to 32 bits */
				context->Length_High &= 0xFFFFFFFF;

				/* Message is too long */
				if (context->Length_High == 0)
					context->Corrupted = 1;
			}

			if (context->Message_Block_Index == 64)
				SHA1ProcessMessageBlock(context);

			message_array++;
		}
	}

	/*
	 *  SHA1ProcessMessageBlock
	 *
	 *  Description:
	 *      This function will process the next 512 bits of the message
	 *      stored in the Message_Block array.
	 *
	 *  Parameters:
	 *      None.
	 *
	 *  Returns:
	 *      Nothing.
	 *
	 *  Comments:
	 *      Many of the variable names in the SHAContext, especially the
	 *      single character names, were used because those were the names
	 *      used in the publication.
	 *
	 *
	 */
	static void SHA1ProcessMessageBlock(SHA1Context* context)
	{
		const unsigned K[] =            /* Constants defined in SHA-1   */
		{
			0x5A827999,
			0x6ED9EBA1,
			0x8F1BBCDC,
			0xCA62C1D6
		};
		int         t;                  /* Loop counter                 */
		unsigned    temp;               /* Temporary word value         */
		unsigned    W[80];              /* Word sequence                */
		unsigned    A, B, C, D, E;      /* Word buffers                 */

		/*
		 *  Initialize the first 16 words in the array W
		 */
		for (t = 0; t < 16; t++)
		{
			W[t] = ((unsigned)context->Message_Block[t * 4]) << 24;
			W[t] |= ((unsigned)context->Message_Block[t * 4 + 1]) << 16;
			W[t] |= ((unsigned)context->Message_Block[t * 4 + 2]) << 8;
			W[t] |= ((unsigned)context->Message_Block[t * 4 + 3]);
		}

		for (t = 16; t < 80; t++)
			W[t] = CARPSHA1CircularShift(1, W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]);

		A = context->Message_Digest[0];
		B = context->Message_Digest[1];
		C = context->Message_Digest[2];
		D = context->Message_Digest[3];
		E = context->Message_Digest[4];

		for (t = 0; t < 20; t++)
		{
			temp = CARPSHA1CircularShift(5, A) +
				((B & C) | ((~B) & D)) + E + W[t] + K[0];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = CARPSHA1CircularShift(30, B);
			B = A;
			A = temp;
		}

		for (t = 20; t < 40; t++)
		{
			temp = CARPSHA1CircularShift(5, A) + (B ^ C ^ D) + E + W[t] + K[1];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = CARPSHA1CircularShift(30, B);
			B = A;
			A = temp;
		}

		for (t = 40; t < 60; t++)
		{
			temp = CARPSHA1CircularShift(5, A) +
				((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = CARPSHA1CircularShift(30, B);
			B = A;
			A = temp;
		}

		for (t = 60; t < 80; t++)
		{
			temp = CARPSHA1CircularShift(5, A) + (B ^ C ^ D) + E + W[t] + K[3];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = CARPSHA1CircularShift(30, B);
			B = A;
			A = temp;
		}

		context->Message_Digest[0] = (context->Message_Digest[0] + A) & 0xFFFFFFFF;
		context->Message_Digest[1] = (context->Message_Digest[1] + B) & 0xFFFFFFFF;
		context->Message_Digest[2] = (context->Message_Digest[2] + C) & 0xFFFFFFFF;
		context->Message_Digest[3] = (context->Message_Digest[3] + D) & 0xFFFFFFFF;
		context->Message_Digest[4] = (context->Message_Digest[4] + E) & 0xFFFFFFFF;
		context->Message_Block_Index = 0;
	}

	/*
	 *  SHA1PadMessage
	 *
	 *  Description:
	 *      According to the standard, the message must be padded to an even
	 *      512 bits.  The first padding bit must be a '1'.  The last 64
	 *      bits represent the length of the original message.  All bits in
	 *      between should be 0.  This function will pad the message
	 *      according to those rules by filling the Message_Block array
	 *      accordingly.  It will also call SHA1ProcessMessageBlock()
	 *      appropriately.  When it returns, it can be assumed that the
	 *      message digest has been computed.
	 *
	 *  Parameters:
	 *      context: [in/out]
	 *          The context to pad
	 *
	 *  Returns:
	 *      Nothing.
	 *
	 *  Comments:
	 *
	 */
	static void SHA1PadMessage(SHA1Context* context)
	{
		/*
		 *  Check to see if the current message block is too small to hold
		 *  the initial padding bits and length.  If so, we will pad the
		 *  block, process it, and then continue padding into a second
		 *  block.
		 */
		if (context->Message_Block_Index > 55)
		{
			context->Message_Block[context->Message_Block_Index++] = 0x80;
			while (context->Message_Block_Index < 64)
				context->Message_Block[context->Message_Block_Index++] = 0;

			SHA1ProcessMessageBlock(context);

			while (context->Message_Block_Index < 56)
				context->Message_Block[context->Message_Block_Index++] = 0;
		}
		else
		{
			context->Message_Block[context->Message_Block_Index++] = 0x80;
			while (context->Message_Block_Index < 56)
				context->Message_Block[context->Message_Block_Index++] = 0;
		}

		/*
		 *  Store the message length as the last 8 octets
		 */
		context->Message_Block[56] = (context->Length_High >> 24) & 0xFF;
		context->Message_Block[57] = (context->Length_High >> 16) & 0xFF;
		context->Message_Block[58] = (context->Length_High >> 8) & 0xFF;
		context->Message_Block[59] = (context->Length_High) & 0xFF;
		context->Message_Block[60] = (context->Length_Low >> 24) & 0xFF;
		context->Message_Block[61] = (context->Length_Low >> 16) & 0xFF;
		context->Message_Block[62] = (context->Length_Low >> 8) & 0xFF;
		context->Message_Block[63] = (context->Length_Low) & 0xFF;

		SHA1ProcessMessageBlock(context);
	}

private:
#define CARP_BASE64_ENCODE_OUT_SIZE(s) ((unsigned int)((((s) + 2) / 3) * 4 + 1))
#define CARP_BASE64_DECODE_OUT_SIZE(s) ((unsigned int)(((s) / 4) * 3))

#define CARP_BASE64_PAD '='
#define CARP_BASE64DE_FIRST '+'
#define CARP_BASE64DE_LAST 'z'

public:
	static std::string Base64Encode(const char* msg, int size)
	{
		char* out = (char*)malloc(CARP_BASE64_ENCODE_OUT_SIZE(size));
		Base64EncodeImpl((const unsigned char*)msg, size, out);
		std::string result = out;
		free(out);
		return result;
	}

public:
	/*
	 * out is null-terminated encode string.
	 * return values is out length, exclusive terminating `\0'
	 */
	static unsigned int Base64EncodeImpl(const unsigned char* in, unsigned int inlen, char* out)
	{
		/* BASE 64 encode table */
		static const char base64en[] = {
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
			'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
			'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
			'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
			'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
			'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
			'w', 'x', 'y', 'z', '0', '1', '2', '3',
			'4', '5', '6', '7', '8', '9', '+', '/',
		};

		int s;
		unsigned int i;
		unsigned int j;
		unsigned char c;
		unsigned char l;

		s = 0;
		l = 0;
		for (i = j = 0; i < inlen; i++) {
			c = in[i];

			switch (s) {
			case 0:
				s = 1;
				out[j++] = base64en[(c >> 2) & 0x3F];
				break;
			case 1:
				s = 2;
				out[j++] = base64en[((l & 0x3) << 4) | ((c >> 4) & 0xF)];
				break;
			case 2:
				s = 0;
				out[j++] = base64en[((l & 0xF) << 2) | ((c >> 6) & 0x3)];
				out[j++] = base64en[c & 0x3F];
				break;
			}
			l = c;
		}

		switch (s) {
		case 1:
			out[j++] = base64en[(l & 0x3) << 4];
			out[j++] = CARP_BASE64_PAD;
			out[j++] = CARP_BASE64_PAD;
			break;
		case 2:
			out[j++] = base64en[(l & 0xF) << 2];
			out[j++] = CARP_BASE64_PAD;
			break;
		}

		out[j] = 0;

		return j;
	}

	/*
	 * return values is out length
	 */
	static unsigned int Base64DecodeImpl(const char* in, unsigned int inlen, unsigned char* out)
	{
		/* ASCII order for BASE 64 decode, 255 in unused character */
		static const unsigned char base64de[] = {
			/* nul, soh, stx, etx, eot, enq, ack, bel, */
			   255, 255, 255, 255, 255, 255, 255, 255,
			/*  bs,  ht,  nl,  vt,  np,  cr,  so,  si, */
				255, 255, 255, 255, 255, 255, 255, 255,
			/* dle, dc1, dc2, dc3, dc4, nak, syn, etb, */
				255, 255, 255, 255, 255, 255, 255, 255,
			/* can,  em, sub, esc,  fs,  gs,  rs,  us, */
				255, 255, 255, 255, 255, 255, 255, 255,
			/*  sp, '!', '"', '#', '$', '%', '&', ''', */
				255, 255, 255, 255, 255, 255, 255, 255,
			/* '(', ')', '*', '+', ',', '-', '.', '/', */
				255, 255, 255,  62, 255, 255, 255,  63,
			/* '0', '1', '2', '3', '4', '5', '6', '7', */
				52,  53,  54,  55,  56,  57,  58,  59,
			/* '8', '9', ':', ';', '<', '=', '>', '?', */
				60,  61, 255, 255, 255, 255, 255, 255,
			/* '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', */
				255,   0,   1,  2,   3,   4,   5,    6,
			/* 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', */
				7,   8,   9,  10,  11,  12,  13,  14,
			/* 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', */
				15,  16,  17,  18,  19,  20,  21,  22,
			/* 'X', 'Y', 'Z', '[', '\', ']', '^', '_', */
				23,  24,  25, 255, 255, 255, 255, 255,
			/* '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', */
				255,  26,  27,  28,  29,  30,  31,  32,
			/* 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', */
				33,  34,  35,  36,  37,  38,  39,  40,
			/* 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', */
				41,  42,  43,  44,  45,  46,  47,  48,
			/* 'x', 'y', 'z', '{', '|', '}', '~', del, */
				49,  50,  51, 255, 255, 255, 255, 255
		};

		unsigned int i;
		unsigned int j;
		unsigned char c;

		if (inlen & 0x3) {
			return 0;
		}

		for (i = j = 0; i < inlen; i++) {
			if (in[i] == CARP_BASE64_PAD) {
				break;
			}
			if (in[i] < CARP_BASE64DE_FIRST || in[i] > CARP_BASE64DE_LAST) {
				return 0;
			}

			c = base64de[(unsigned char)in[i]];
			if (c == 255) {
				return 0;
			}

			switch (i & 0x3) {
			case 0:
				out[j] = (c << 2) & 0xFF;
				break;
			case 1:
				out[j++] |= (c >> 4) & 0x3;
				out[j] = (c & 0xF) << 4;
				break;
			case 2:
				out[j++] |= (c >> 2) & 0xF;
				out[j] = (c & 0x3) << 6;
				break;
			case 3:
				out[j++] |= c;
				break;
			}
		}

		return j;
	}

public:
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  INTERNAL FUNCTIONS
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  F, G, H, I
	//
	//  The basic MD5 functions. F and G are optimised compared to their RFC 1321 definitions for architectures that lack
	//  an AND-NOT instruction, just like in Colin Plumb's implementation.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define CARP_MD5_F( x, y, z )            ( (z) ^ ((x) & ((y) ^ (z))) )
#define CARP_MD5_G( x, y, z )            ( (y) ^ ((z) & ((x) ^ (y))) )
#define CARP_MD5_H( x, y, z )            ( (x) ^ (y) ^ (z) )
#define CARP_MD5_I( x, y, z )            ( (y) ^ ((x) | ~(z)) )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CARP_MD5_STEP
//
//  The MD5 transformation for all four rounds.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define CARP_MD5_STEP( f, a, b, c, d, x, t, s )                          \
    (a) += f((b), (c), (d)) + (x) + (t);                        \
    (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s))));  \
    (a) += (b);

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  TYPES
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Md5Context - This must be initialised using Md5Initialised. Do not modify the contents of this structure directly.
	typedef struct
	{
		uint32_t     lo;
		uint32_t     hi;
		uint32_t     a;
		uint32_t     b;
		uint32_t     c;
		uint32_t     d;
		uint8_t      buffer[64];
		uint32_t     block[16];
	} Md5Context;

#define CARP_MD5_HASH_SIZE           ( 128 / 8 )

	typedef struct
	{
		uint8_t      bytes[CARP_MD5_HASH_SIZE];
	} MD5_HASH;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  PUBLIC FUNCTIONS
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Md5Initialise
	//
	//  Initialises an MD5 Context. Use this to initialise/reset a context.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static void Md5Initialise(Md5Context* Context)
	{
		Context->a = 0x67452301;
		Context->b = 0xefcdab89;
		Context->c = 0x98badcfe;
		Context->d = 0x10325476;

		Context->lo = 0;
		Context->hi = 0;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Md5Update
	//
	//  Adds data to the MD5 context. This will process the data and update the internal state of the context. Keep on
	//  calling this function until all the data has been added. Then call Md5Finalise to calculate the hash.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static void Md5Update(Md5Context* Context, void const* Buffer, uint32_t BufferSize)
	{
		uint32_t    saved_lo;
		uint32_t    used;
		uint32_t    free;

		saved_lo = Context->lo;
		if ((Context->lo = (saved_lo + BufferSize) & 0x1fffffff) < saved_lo)
		{
			Context->hi++;
		}
		Context->hi += (uint32_t)(BufferSize >> 29);

		used = saved_lo & 0x3f;

		if (used)
		{
			free = 64 - used;

			if (BufferSize < free)
			{
				memcpy(&Context->buffer[used], Buffer, BufferSize);
				return;
			}

			memcpy(&Context->buffer[used], Buffer, free);
			Buffer = (uint8_t*)Buffer + free;
			BufferSize -= free;
			Md5TransformFunction(Context, Context->buffer, 64);
		}

		if (BufferSize >= 64)
		{
			Buffer = Md5TransformFunction(Context, Buffer, BufferSize & ~(unsigned long)0x3f);
			BufferSize &= 0x3f;
		}

		memcpy(Context->buffer, Buffer, BufferSize);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Md5Finalise
	//
	//  Performs the final calculation of the hash and returns the digest (16 byte buffer containing 128bit hash). After
	//  calling this, Md5Initialised must be used to reuse the context.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static void Md5Finalise(Md5Context* Context, MD5_HASH* Digest)
	{
		uint32_t    used;
		uint32_t    free;

		used = Context->lo & 0x3f;

		Context->buffer[used++] = 0x80;

		free = 64 - used;

		if (free < 8)
		{
			memset(&Context->buffer[used], 0, free);
			Md5TransformFunction(Context, Context->buffer, 64);
			used = 0;
			free = 64;
		}

		memset(&Context->buffer[used], 0, free - 8);

		Context->lo <<= 3;
		Context->buffer[56] = (uint8_t)(Context->lo);
		Context->buffer[57] = (uint8_t)(Context->lo >> 8);
		Context->buffer[58] = (uint8_t)(Context->lo >> 16);
		Context->buffer[59] = (uint8_t)(Context->lo >> 24);
		Context->buffer[60] = (uint8_t)(Context->hi);
		Context->buffer[61] = (uint8_t)(Context->hi >> 8);
		Context->buffer[62] = (uint8_t)(Context->hi >> 16);
		Context->buffer[63] = (uint8_t)(Context->hi >> 24);

		Md5TransformFunction(Context, Context->buffer, 64);

		Digest->bytes[0] = (uint8_t)(Context->a);
		Digest->bytes[1] = (uint8_t)(Context->a >> 8);
		Digest->bytes[2] = (uint8_t)(Context->a >> 16);
		Digest->bytes[3] = (uint8_t)(Context->a >> 24);
		Digest->bytes[4] = (uint8_t)(Context->b);
		Digest->bytes[5] = (uint8_t)(Context->b >> 8);
		Digest->bytes[6] = (uint8_t)(Context->b >> 16);
		Digest->bytes[7] = (uint8_t)(Context->b >> 24);
		Digest->bytes[8] = (uint8_t)(Context->c);
		Digest->bytes[9] = (uint8_t)(Context->c >> 8);
		Digest->bytes[10] = (uint8_t)(Context->c >> 16);
		Digest->bytes[11] = (uint8_t)(Context->c >> 24);
		Digest->bytes[12] = (uint8_t)(Context->d);
		Digest->bytes[13] = (uint8_t)(Context->d >> 8);
		Digest->bytes[14] = (uint8_t)(Context->d >> 16);
		Digest->bytes[15] = (uint8_t)(Context->d >> 24);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Md5Calculate
	//
	//  Combines Md5Initialise, Md5Update, and Md5Finalise into one function. Calculates the MD5 hash of the buffer.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static void Md5Calculate(void  const* Buffer, uint32_t BufferSize, MD5_HASH* Digest)
	{
		Md5Context context;

		Md5Initialise(&context);
		Md5Update(&context, Buffer, BufferSize);
		Md5Finalise(&context, Digest);
	}

	static std::string Md4HashToString(MD5_HASH* Digest)
	{
		static const char HEX[16] = {
			'0', '1', '2', '3',
			'4', '5', '6', '7',
			'8', '9', 'a', 'b',
			'c', 'd', 'e', 'f'
		};

		char str[64] = { 0 };
		int index = 0;
		for (int i = 0; i < 16; ++i) {
			int t = Digest->bytes[i];
			int a = t / 16;
			int b = t % 16;
			str[index++] = HEX[a];
			str[index++] = HEX[b];
		}

		return str;
	}

	static std::string StringMd5(const std::string& value)
	{
		MD5_HASH digest;
		Md5Calculate(value.c_str(), static_cast<int>(value.size()), &digest);
		return Md4HashToString(&digest);
	}

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Md5TransformFunction
	//
	//  This processes one or more 64-byte data blocks, but does NOT update the bit counters. There are no alignment
	//  requirements.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static void* Md5TransformFunction (Md5Context* ctx, void const* data, size_t size)
	{
		uint8_t* ptr;
		uint32_t     a;
		uint32_t     b;
		uint32_t     c;
		uint32_t     d;
		uint32_t     saved_a;
		uint32_t     saved_b;
		uint32_t     saved_c;
		uint32_t     saved_d;

#define GET(n) (ctx->block[(n)])
#define SET(n) (ctx->block[(n)] =             \
            ((uint32_t)ptr[(n)*4 + 0] << 0 )      \
        |   ((uint32_t)ptr[(n)*4 + 1] << 8 )      \
        |   ((uint32_t)ptr[(n)*4 + 2] << 16)      \
        |   ((uint32_t)ptr[(n)*4 + 3] << 24) )

		ptr = (uint8_t*)data;

		a = ctx->a;
		b = ctx->b;
		c = ctx->c;
		d = ctx->d;

		do
		{
			saved_a = a;
			saved_b = b;
			saved_c = c;
			saved_d = d;

			// Round 1
			CARP_MD5_STEP(CARP_MD5_F, a, b, c, d, SET(0), 0xd76aa478, 7)
			CARP_MD5_STEP(CARP_MD5_F, d, a, b, c, SET(1), 0xe8c7b756, 12)
			CARP_MD5_STEP(CARP_MD5_F, c, d, a, b, SET(2), 0x242070db, 17)
			CARP_MD5_STEP(CARP_MD5_F, b, c, d, a, SET(3), 0xc1bdceee, 22)
			CARP_MD5_STEP(CARP_MD5_F, a, b, c, d, SET(4), 0xf57c0faf, 7)
			CARP_MD5_STEP(CARP_MD5_F, d, a, b, c, SET(5), 0x4787c62a, 12)
			CARP_MD5_STEP(CARP_MD5_F, c, d, a, b, SET(6), 0xa8304613, 17)
			CARP_MD5_STEP(CARP_MD5_F, b, c, d, a, SET(7), 0xfd469501, 22)
			CARP_MD5_STEP(CARP_MD5_F, a, b, c, d, SET(8), 0x698098d8, 7)
			CARP_MD5_STEP(CARP_MD5_F, d, a, b, c, SET(9), 0x8b44f7af, 12)
			CARP_MD5_STEP(CARP_MD5_F, c, d, a, b, SET(10), 0xffff5bb1, 17)
			CARP_MD5_STEP(CARP_MD5_F, b, c, d, a, SET(11), 0x895cd7be, 22)
			CARP_MD5_STEP(CARP_MD5_F, a, b, c, d, SET(12), 0x6b901122, 7)
			CARP_MD5_STEP(CARP_MD5_F, d, a, b, c, SET(13), 0xfd987193, 12)
			CARP_MD5_STEP(CARP_MD5_F, c, d, a, b, SET(14), 0xa679438e, 17)
			CARP_MD5_STEP(CARP_MD5_F, b, c, d, a, SET(15), 0x49b40821, 22)

			// Round 2
			CARP_MD5_STEP(CARP_MD5_G, a, b, c, d, GET(1), 0xf61e2562, 5)
			CARP_MD5_STEP(CARP_MD5_G, d, a, b, c, GET(6), 0xc040b340, 9)
			CARP_MD5_STEP(CARP_MD5_G, c, d, a, b, GET(11), 0x265e5a51, 14)
			CARP_MD5_STEP(CARP_MD5_G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)
			CARP_MD5_STEP(CARP_MD5_G, a, b, c, d, GET(5), 0xd62f105d, 5)
			CARP_MD5_STEP(CARP_MD5_G, d, a, b, c, GET(10), 0x02441453, 9)
			CARP_MD5_STEP(CARP_MD5_G, c, d, a, b, GET(15), 0xd8a1e681, 14)
			CARP_MD5_STEP(CARP_MD5_G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)
			CARP_MD5_STEP(CARP_MD5_G, a, b, c, d, GET(9), 0x21e1cde6, 5)
			CARP_MD5_STEP(CARP_MD5_G, d, a, b, c, GET(14), 0xc33707d6, 9)
			CARP_MD5_STEP(CARP_MD5_G, c, d, a, b, GET(3), 0xf4d50d87, 14)
			CARP_MD5_STEP(CARP_MD5_G, b, c, d, a, GET(8), 0x455a14ed, 20)
			CARP_MD5_STEP(CARP_MD5_G, a, b, c, d, GET(13), 0xa9e3e905, 5)
			CARP_MD5_STEP(CARP_MD5_G, d, a, b, c, GET(2), 0xfcefa3f8, 9)
			CARP_MD5_STEP(CARP_MD5_G, c, d, a, b, GET(7), 0x676f02d9, 14)
			CARP_MD5_STEP(CARP_MD5_G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)

			// Round 3
			CARP_MD5_STEP(CARP_MD5_H, a, b, c, d, GET(5), 0xfffa3942, 4)
			CARP_MD5_STEP(CARP_MD5_H, d, a, b, c, GET(8), 0x8771f681, 11)
			CARP_MD5_STEP(CARP_MD5_H, c, d, a, b, GET(11), 0x6d9d6122, 16)
			CARP_MD5_STEP(CARP_MD5_H, b, c, d, a, GET(14), 0xfde5380c, 23)
			CARP_MD5_STEP(CARP_MD5_H, a, b, c, d, GET(1), 0xa4beea44, 4)
			CARP_MD5_STEP(CARP_MD5_H, d, a, b, c, GET(4), 0x4bdecfa9, 11)
			CARP_MD5_STEP(CARP_MD5_H, c, d, a, b, GET(7), 0xf6bb4b60, 16)
			CARP_MD5_STEP(CARP_MD5_H, b, c, d, a, GET(10), 0xbebfbc70, 23)
			CARP_MD5_STEP(CARP_MD5_H, a, b, c, d, GET(13), 0x289b7ec6, 4)
			CARP_MD5_STEP(CARP_MD5_H, d, a, b, c, GET(0), 0xeaa127fa, 11)
			CARP_MD5_STEP(CARP_MD5_H, c, d, a, b, GET(3), 0xd4ef3085, 16)
			CARP_MD5_STEP(CARP_MD5_H, b, c, d, a, GET(6), 0x04881d05, 23)
			CARP_MD5_STEP(CARP_MD5_H, a, b, c, d, GET(9), 0xd9d4d039, 4)
			CARP_MD5_STEP(CARP_MD5_H, d, a, b, c, GET(12), 0xe6db99e5, 11)
			CARP_MD5_STEP(CARP_MD5_H, c, d, a, b, GET(15), 0x1fa27cf8, 16)
			CARP_MD5_STEP(CARP_MD5_H, b, c, d, a, GET(2), 0xc4ac5665, 23)

			// Round 4
			CARP_MD5_STEP(CARP_MD5_I, a, b, c, d, GET(0), 0xf4292244, 6)
			CARP_MD5_STEP(CARP_MD5_I, d, a, b, c, GET(7), 0x432aff97, 10)
			CARP_MD5_STEP(CARP_MD5_I, c, d, a, b, GET(14), 0xab9423a7, 15)
			CARP_MD5_STEP(CARP_MD5_I, b, c, d, a, GET(5), 0xfc93a039, 21)
			CARP_MD5_STEP(CARP_MD5_I, a, b, c, d, GET(12), 0x655b59c3, 6)
			CARP_MD5_STEP(CARP_MD5_I, d, a, b, c, GET(3), 0x8f0ccc92, 10)
			CARP_MD5_STEP(CARP_MD5_I, c, d, a, b, GET(10), 0xffeff47d, 15)
			CARP_MD5_STEP(CARP_MD5_I, b, c, d, a, GET(1), 0x85845dd1, 21)
			CARP_MD5_STEP(CARP_MD5_I, a, b, c, d, GET(8), 0x6fa87e4f, 6)
			CARP_MD5_STEP(CARP_MD5_I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)
			CARP_MD5_STEP(CARP_MD5_I, c, d, a, b, GET(6), 0xa3014314, 15)
			CARP_MD5_STEP(CARP_MD5_I, b, c, d, a, GET(13), 0x4e0811a1, 21)
			CARP_MD5_STEP(CARP_MD5_I, a, b, c, d, GET(4), 0xf7537e82, 6)
			CARP_MD5_STEP(CARP_MD5_I, d, a, b, c, GET(11), 0xbd3af235, 10)
			CARP_MD5_STEP(CARP_MD5_I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)
			CARP_MD5_STEP(CARP_MD5_I, b, c, d, a, GET(9), 0xeb86d391, 21)

			a += saved_a;
			b += saved_b;
			c += saved_c;
			d += saved_d;

			ptr += 64;
		} while (size -= 64);

		ctx->a = a;
		ctx->b = b;
		ctx->c = c;
		ctx->d = d;

#undef GET
#undef SET

		return ptr;
	}
};

#endif