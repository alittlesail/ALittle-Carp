
#ifndef CARP_CRYPTHELPER_INCLUDED
#define CARP_CRYPTHELPER_INCLUDED (1)

#include <cstring>
#include <string>

class CarpCryptHelper
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
	struct CarpSHA1Context
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
		CarpSHA1Context sha;
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
	static void SHA1Reset(CarpSHA1Context* context)
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
	static int SHA1Result(CarpSHA1Context* context)
	{

		if (context->Corrupted)
		{
			return 0;
		}

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
	static void SHA1Input(CarpSHA1Context* context, const unsigned char* message_array, unsigned length)
	{
		if (!length)
		{
			return;
		}

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
				if (context->Length_High == 0)
				{
					/* Message is too long */
					context->Corrupted = 1;
				}
			}

			if (context->Message_Block_Index == 64)
			{
				SHA1ProcessMessageBlock(context);
			}

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
	static void SHA1ProcessMessageBlock(CarpSHA1Context* context)
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
		{
			W[t] = CARPSHA1CircularShift(1, W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]);
		}

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
	static void SHA1PadMessage(CarpSHA1Context* context)
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
			{
				context->Message_Block[context->Message_Block_Index++] = 0;
			}

			SHA1ProcessMessageBlock(context);

			while (context->Message_Block_Index < 56)
			{
				context->Message_Block[context->Message_Block_Index++] = 0;
			}
		}
		else
		{
			context->Message_Block[context->Message_Block_Index++] = 0x80;
			while (context->Message_Block_Index < 56)
			{
				context->Message_Block[context->Message_Block_Index++] = 0;
			}
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

private:
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
};

#endif