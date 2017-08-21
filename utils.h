#ifndef __UTILS__
#define __UTILS__


#include <boost/format.hpp>
//DEL #include <boost/version.hpp>

/*DEL
#if BOOST_VERSION < 104700
#error Boost 1.47 or later is required
#endif
*/

#include <openssl/dh.h>
#include "types.h"

// maybe use http://www.mail-archive.com/licq-commits@googlegroups.com/msg02334.html
#ifdef WIN32
#include <winsock.h>
#define htobe32(x) htonl(x)
#define be32toh(x) ntohl(x)

extern uint64_t htobe64(uint64_t value);
extern uint64_t be64toh(uint64_t value);
#elif __APPLE__
#include <libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)

#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#include <sys/endian.h>
#elif defined(__OpenBSD__)
#include <sys/types.h>
#define be16toh(x) betoh16(x)
#define be32toh(x) betoh32(x)
#define be64toh(x) betoh64(x)
#endif

inline static char charHex(int iDigit)
{
	return iDigit < 10 ? '0' + iDigit : 'A' - 10 + iDigit;
}

template<class Iterator>
std::string strHex(Iterator first, int iSize)
{
	std::string		strDst;

	strDst.resize(iSize*2);

	for (int i = 0; i < iSize; i++) {
		unsigned char c	= *first++;

		strDst[i*2]		= charHex(c >> 4);
		strDst[i*2+1]	= charHex(c & 15);
	}

	return strDst;
}


#endif

// vim:ts=4
