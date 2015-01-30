/**
******************************************************************************
* @file    asciibin.hpp
* @brief   Ascii to binary (and viceversa) conversion routines
*
* @author  AA Dinema s.p.a.
* @version V1.0.0
* @date    25-Nov-2014
*          
* @verbatim
* @endverbatim
*
******************************************************************************
* @attention
*
******************************************************************************
* @note
*
*****************************************************************************/

/*Include only once */
#ifndef __ASCIIBIN_HPP_INCLUDED
#define __ASCIIBIN_HPP_INCLUDED

#ifndef __cplusplus
#error asciibin.hpp is C++ only.
#endif

//////////////////////////////////////////////////////////////////////////////
//                         I N C L U D E S                                  //
//////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

class AsciiBin {
	/*************************************************************************//**
	** Append an element to a fixed-size array, making sure that maximum
	** array capacity will never be exceeded
	** @param bptr pointer to array
	** @param value element to be appended
	** @param avail available element count
	*/
	template <class T>
	static inline void safe_bput(T* &bptr, T const value, size_t &avail) {
		if (avail > 0) {
			avail--;
			*bptr++ = value;
		}
	}

public:
	/*************************************************************************//**
	** Convert an hexadecimal string into its binary equivalent
	** @param dst destination buffer for binary data
	** @param src source buffer of hexadecimal string
	** @param dst_len maximum destination buffer size
	** @return number of bytes written into destination buffer
	*/
	static int hex_to_binary(uint8_t * const dst, const char * src, size_t const dst_len)
	{
		char a, flag = 0;
		size_t avail = dst_len;
		uint8_t * b = dst;
		uint8_t c;

		a = 0;
		while ((c = *src++)) {
			if ((c == ':') || (c == '-'))
				continue;
			c = (c-48 > 9 ? (c-55 > 41 ? c-87 : c-55) : c-48);
			if (c <	16)
				a |= c;
			else
				break;
			if (flag) {
				safe_bput<uint8_t>(b, a, avail);
				flag = 0;
				a = 0;
			} else {
				a <<= 4;
				flag = 1;
			}
		}
		return (b - dst);
	}

	/*************************************************************************//**
	** Convert a binary octet string into its hexadecimal form
	** @param dst destination buffer for hexadecimal string
	** @param src source buffer of binary data
	** @param dst_len maximum destination buffer size
	** @param src_len number of octets to be converted
	** @return number of bytes written into destination buffer
	*/
	static int binary_to_hex(char *dst, const uint8_t * const src, size_t const dst_len, size_t const src_len)
	{
		const char hdigits[16] = {'0','1','2','3','4','5','6','7',
								  '8','9','A','B','C','D','E','F'};
		const uint8_t * s = static_cast<const uint8_t *>(src);
		size_t sp, dp;
		if ((dst_len == 0) || (src_len == 0))
			return 0;
		dp = dst_len;
		for(sp = 0; sp < src_len; sp++, s++) {
			safe_bput<char>(dst, hdigits[*s  >>  4], dp);
			safe_bput<char>(dst, hdigits[*s & 0x0F], dp);
		}
		safe_bput<char>(dst, 0, dp);
		return (dst_len - dp) - 1;
	}


	/*************************************************************************//**
	** Convert a binary octet string into "dotted form"
	** @param dst destination buffer for hexadecimal string
	** @param src source buffer of binary data
	** @param dst_len maximum destination buffer size
	** @param src_len number of octets to be converted
	** @param base optional numeration base (default 10)
	** @param sep_char optional separator character (default:'.')
	** @return number of bytes written into destination buffer
	*/
	static int binary_to_dotted(char *dst, const uint8_t *src, size_t const dst_len, size_t const src_len, int const base = 10, char const sep_char = '.')
	{
		const char * fmt;
		const size_t tmp_size = 8;
		char tmp[8];
		char sep[2] = {0, 0};
		size_t i, out_len;
		
		switch (base) {
			case 10: fmt = "%s%u"; break;
			case 16: fmt = "%s%02X"; break;
			default:
				return -1;
		}
		*dst = 0;
		out_len = 0;
		for (i=0; i < src_len; i++) {
			int l = snprintf(tmp, tmp_size, fmt, sep, src[i]);
			if ((out_len + l) <= dst_len) {
				strcat(dst + out_len, tmp);
				out_len += l;
				sep[0] = sep_char;
			}
		}
		return out_len;
	}

	/*************************************************************************//**
	** Convert a "dotted form" character string into binary octet string
	** @param dst destination buffer for binary string
	** @param src source buffer of text data
	** @param dst_len maximum destination buffer size
	** @param base optional numeration base (default 10)
	** @return number of bytes written into destination buffer
	** @note this function accepts the following characters as separators:
	**       dot   '.' (used for IPv4 addresses),
	**       colon ':' (used for network hardware addresses),
	**       minus '-' (also used for network hardware addresses)
	*/
	static int dotted_to_binary(uint8_t *dst, const char *src, size_t const dst_len, int const base = 10)
	{
		const char * const match = ".-:";
		const char *dotp, *last_dotp = src;
		const size_t tmp_size = 8;
		char tmp[8], *eptr;
		size_t tok_len, dp;
		int32_t lval = 0;

		dp = 0;
		dotp = last_dotp = src;
		while (dotp) {
			dotp = strpbrk(dotp, match);
			if (dotp == NULL) {
				memset(tmp, 0, tmp_size);
				strncpy(tmp, last_dotp, tmp_size-1);
				tok_len = strlen(tmp);
			} else {
				tok_len = dotp - last_dotp;
				if (tok_len > 3)
					return -1;
				strncpy(tmp, last_dotp, tok_len);
				tmp[tok_len] = 0;
				dotp++;
				last_dotp = dotp;
			}
			if (tok_len == 0) {
				return -1;
			} else {
				eptr = tmp;
				if (base != 0)
					lval = strtoul(tmp, &eptr, base);
				else {
					const int guess_bases[] = {10, 16, 0};
					int i = 0;
					while ((guess_bases[i] != 0) && (*eptr != 0))
						lval = strtol(tmp, &eptr, guess_bases[i++]);
				}
				if ((*eptr != 0) || (lval > 255))
					return -1;
			}
			if (dp < dst_len) {
				dst[dp] = lval;
				lval = 0;
			}
			dp++;
		}
		return dp;
	}

	/*************************************************************************//**
	** Convert a binary octet string into a human readable representation
	** @param dst destination buffer for hexadecimal string
	** @param src source buffer of binary data
	** @param dst_len maximum destination buffer size
	** @param src_len number of octets to be converted
	** @param hex_codes insert unprintable characters as hexadecimal escapes
	** @return number of bytes written into destination buffer
	*/
	static int asciify(char * const dst, const uint8_t * const src, size_t const dst_len, size_t const src_len, bool hex_codes = false) {
		const char hdigits[16] = {'0','1','2','3','4','5','6','7',
								  '8','9','A','B','C','D','E','F'};
		size_t avail = dst_len;
		char * dp = dst;
		for (unsigned i=0; i<src_len; i++) {
			if ((src[i] < 32) || (src[i] > 127)) {
				if (hex_codes) {
					safe_bput<char>(dp, '\\', avail);
					safe_bput<char>(dp, 'x', avail);
					safe_bput<char>(dp, hdigits[src[i]  >>  4], avail);
					safe_bput<char>(dp, hdigits[src[i] & 0x0F], avail);
				} else
					safe_bput<char>(dp, '.', avail);
			} else
				safe_bput<char>(dp, src[i], avail);
		}
		safe_bput<char>(dp, 0, avail);
		return (dp - dst) - 1;
	}


	/*************************************************************************//**
	** Convert a binary octet string into Z85 ascii encoding
	** @param dst destination buffer for encoded string
	** @param src source buffer of binary data
	** @param dst_len maximum destination buffer size
	** @param src_len number of octets to be converted
	** @return number of bytes written into destination buffer
	*/
	static int binary_to_Z85(char *dst, const uint8_t * const src, size_t const dst_len, size_t const src_len)
	{
		// Maps base 256 to base 85
		const char encoder [85 + 1] = {
			"0123456789"
			"abcdefghij"
			"klmnopqrst"
			"uvwxyzABCD"
			"EFGHIJKLMN"
			"OPQRSTUVWX"
			"YZ.-:+=^!/"
			"*?&<>()[]{"
			"}@%$#"
		};
		const unsigned divisors[5] = {
			85 * 85 * 85 * 85,
			85 * 85 * 85,
			85 * 85,
			85,
			1
		};

		// Accepts only byte arrays bounded to 4 bytes
		if (src_len % 4)
			return 0;

		size_t sp, dp;
		uint32_t accum;

		if ((dst_len == 0) || (src_len == 0))
			return 0;

		accum = 0;
		dp = dst_len;
		sp = 0;
		while (sp < src_len) {
			// Accumulate value in base 256 (binary)
			accum = (accum << 8) + src[sp++];
			if (sp % 4 == 0) {
				// Output value in base 85
				for (int i=0; i < 5; i++)
					safe_bput<char>(dst, encoder[(accum / divisors[i]) % 85], dp);
				accum = 0;
			}
		}
		safe_bput<char>(dst, 0, dp);
		return (dst_len - dp) - 1;
	}

	/*************************************************************************//**
	** Convert a string encoded in Z85 into its binary equivalent
	** @param dst destination buffer for binary data
	** @param src source buffer of hexadecimal string
	** @param dst_len maximum destination buffer size
	** @return number of bytes written into destination buffer
	*/
	static int Z85_to_binary(uint8_t * const dst, const char * src, size_t const dst_len)
	{
		// Maps base 85 to base 256
		// We chop off lower 32 and higher 128 ranges
		const uint8_t decoder [96] = {
			0x00, 0x44, 0x00, 0x54, 0x53, 0x52, 0x48, 0x00,
			0x4B, 0x4C, 0x46, 0x41, 0x00, 0x3F, 0x3E, 0x45,
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x40, 0x00, 0x49, 0x42, 0x4A, 0x47,
			0x51, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A,
			0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32,
			0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A,
			0x3B, 0x3C, 0x3D, 0x4D, 0x00, 0x4E, 0x43, 0x00,
			0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
			0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
			0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
			0x21, 0x22, 0x23, 0x4F, 0x00, 0x50, 0x00, 0x00
		};
		size_t avail = dst_len;
		size_t char_n = 0;
		uint8_t * b = dst;
		uint32_t accum = 0;
		uint8_t c;

		if ((dst_len == 0) || (dst == 0))
			return 0;

		while ((c = *src++)) {
			// Accumulate value in base 85
			accum = accum * 85 + decoder[c - 32];
			if (++char_n == 5) {
				char_n = 0;
				// Output value in base 256
				safe_bput<uint8_t>(b, (accum >> 24) & 0xFF, avail);
				safe_bput<uint8_t>(b, (accum >> 16) & 0xFF, avail);
				safe_bput<uint8_t>(b, (accum >>  8) & 0xFF, avail);
				safe_bput<uint8_t>(b,  accum        & 0xFF, avail);
				accum = 0;
			}
		}
		// Input string size not bounded to 5 bytes
		if (char_n > 0)
			return -1;

		return (b - dst);
	}
	
	static size_t Z85_encoded_size(size_t const bin_len) {
		size_t const t = bin_len * 5;
		return (t / 4) + (t % 4 ? 1 : 0);
	}
	
	static size_t Z85_decoded_size(size_t const txt_len) {
		ldiv_t d = ldiv(txt_len * 4, 5);
		return d.quot + (d.rem != 0 ? 1 : 0);
	}
};


/****************************************************************************/

#endif /* __ASCIIBIN_HPP_INCLUDED */
/* EOF */
