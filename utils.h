#ifndef __UTILS__
#define __UTILS__

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/version.hpp>

#if BOOST_VERSION < 104700
#error Boost 1.47 or later is required
#endif

#include <openssl/dh.h>
#include "types.h"

#define QUALITY_ONE			1000000000	// 10e9

#define nothing()			do {} while (0)
#define fallthru()			do {} while (0)
#define NUMBER(x)			(sizeof(x)/sizeof((x)[0]))
#define ADDRESS(p)			strHex(uint64( ((char*) p) - ((char*) 0)))
#define ADDRESS_SHARED(p)	strHex(uint64( ((char*) (p).get()) - ((char*) 0)))

#define isSetBit(x,y)		(!!((x) & (y)))

// maybe use http://www.mail-archive.com/licq-commits@googlegroups.com/msg02334.html
#ifdef WIN32
#include <winsock.h>
#define htobe32(x) htonl(x)
#define be32toh(x) ntohl(x)

extern uint64_t htobe64(uint64_t value);
extern uint64_t be64toh(uint64_t value);
//extern uint32_t htobe32(uint32_t value);
//extern uint32_t be32toh(uint32_t value);
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


#define vt_f_black          "\033[30m"
#define vt_f_red            "\033[31m"
#define vt_f_green          "\033[32m"
#define vt_f_yellow         "\033[33m"
#define vt_f_blue           "\033[34m"
#define vt_f_megenta        "\033[35m"
#define vt_f_cyan           "\033[36m"
#define vt_f_white          "\033[37m"
#define vt_f_default        "\033[39m"

#define vt_b_black          "\033[40m"
#define vt_b_red            "\033[41m"
#define vt_b_green          "\033[42m"
#define vt_b_yellow         "\033[43m"
#define vt_b_blue           "\033[44m"
#define vt_b_megenta        "\033[45m"
#define vt_b_cyan           "\033[46m"
#define vt_b_white          "\033[47m"
#define vt_b_default        "\033[49m"

#define vt_f_bold_black    "\033[1m\033[30m"
#define vt_f_bold_red      "\033[1m\033[31m"
#define vt_f_bold_green    "\033[1m\033[32m"
#define vt_f_bold_yellow   "\033[1m\033[33m"
#define vt_f_bold_blue     "\033[1m\033[34m"
#define vt_f_bold_megenta  "\033[1m\033[35m"
#define vt_f_bold_cyan     "\033[1m\033[36m"
#define vt_f_bold_white    "\033[1m\033[37m"
#define vt_f_bold_default  "\033[1m\033[39m"

#define vt_bold             "\033[1m"
#define vt_dim              "\033[2m"     // does not work for xterm
#define vt_normal           "\033[22m"    // intensity

#define vt_n_enable         "\033[7m"     // negative
#define vt_n_disable        "\033[27m"

#define vt_u_single         "\033[4m"     // underline
#define vt_u_double         "\033[21m"    // does not work for xterm
#define vt_u_disable        "\033[24m"

#define vt_reset    vt_f_default vt_b_default vt_normal vt_n_disable vt_u_disable

boost::posix_time::ptime ptEpoch();
int iToSeconds(boost::posix_time::ptime ptWhen);
boost::posix_time::ptime ptFromSeconds(int iSeconds);
uint64_t utFromSeconds(int iSeconds);

extern std::string urlEncode(const std::string& strSrc);

/*
void intIPtoStr(int ip,std::string& retStr);
int strIPtoInt(std::string& ipStr);
*/

template<class Iterator>
std::string strJoin(Iterator first, Iterator last, std::string strSeperator)
{
	std::ostringstream	ossValues;

	for (Iterator start = first; first != last; first++)
	{
		ossValues << str(boost::format("%s%s") % (start == first ? "" : strSeperator) % *first);
	}

	return ossValues.str();
}

char charHex(int iDigit);

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

inline const std::string strHex(const std::string& strSrc)
{
	return strHex(strSrc.begin(), strSrc.size());
}

inline std::string strHex(const std::vector<unsigned char>& vucData)
{
	return strHex(vucData.begin(), vucData.size());
}

inline std::string strHex(const uint64 uiHost)
{
	uint64_t	uBig	= htobe64(uiHost);

	return strHex((unsigned char*) &uBig, sizeof(uBig));
}

inline static std::string sqlEscape(const std::string& strSrc)
{
	static boost::format f("X'%s'");
	return str(boost::format(f) % strHex(strSrc));
}

inline static std::string sqlEscape(const std::vector<unsigned char>& vecSrc)
{
	size_t size = vecSrc.size();
	if (size == 0)
		return "X''";

	std::string j(size * 2 + 3, 0);

	unsigned char *oPtr = reinterpret_cast<unsigned char *>(&*j.begin());
	const unsigned char *iPtr = &vecSrc[0];

	*oPtr++ = 'X';
	*oPtr++ = '\'';

	for (int i = size; i != 0; --i)
	{
		unsigned char c = *iPtr++;
		*oPtr++ = charHex(c >> 4);
		*oPtr++ = charHex(c & 15);
	}

	*oPtr++ = '\'';
	return j;
}

template<class Iterator>
bool isZero(Iterator first, int iSize)
{
	while (iSize && !*first++)
		--iSize;

	return !iSize;
}

int charUnHex(char cDigit);
int strUnHex(std::string& strDst, const std::string& strSrc);

uint64_t uintFromHex(const std::string& strSrc);

std::vector<unsigned char> strUnHex(const std::string& strSrc);

std::vector<unsigned char> strCopy(const std::string& strSrc);
std::string strCopy(const std::vector<unsigned char>& vucSrc);

bool parseIpPort(const std::string& strSource, std::string& strIP, int& iPort);
bool parseQuality(const std::string& strSource, uint32& uQuality);

DH* DH_der_load(const std::string& strDer);
std::string DH_der_gen(int iKeyLength);

void getRand(unsigned char *buf, int num);
inline static void getRand(char *buf, int num)		{ return getRand(reinterpret_cast<unsigned char *>(buf), num); }
inline static void getRand(void *buf, int num)		{ return getRand(reinterpret_cast<unsigned char *>(buf), num); }

inline std::string strGetEnv(const std::string& strKey)
{
	return getenv(strKey.c_str()) ? getenv(strKey.c_str()) : "";
}

template<typename T> T lexical_cast_s(const std::string& string)
{ // lexically cast a string to the selected type. Does not throw
	try
	{
		return boost::lexical_cast<T>(string);
	}
	catch (...)
	{
		return 0;
	}
}

template<typename T> std::string lexical_cast_i(const T& t)
{ // lexicaly cast the selected type to a string. Does not throw
	try
	{
		return boost::lexical_cast<std::string>(t);
	}
	catch (...)
	{
		return "";
	}
}

template<typename T> T lexical_cast_st(const std::string& string)
{ // lexically cast a string to the selected type. Does throw
	return boost::lexical_cast<T>(string);
}

template<typename T> std::string lexical_cast_it(const T& t)
{ // lexicaly cast the selected type to a string. Does not throw
	return boost::lexical_cast<std::string>(t);
}

template<typename T> T range_check(const T& value, const T& minimum, const T& maximum)
{
	if ((value < minimum) || (value > maximum))
		throw std::runtime_error("Value out of range");
	return value;
}

template<typename T> T range_check_min(const T& value, const T& minimum)
{
	if (value < minimum)
		throw std::runtime_error("Value out of range");
	return value;
}

template<typename T> T range_check_max(const T& value, const T& maximum)
{
	if (value > maximum)
		throw std::runtime_error("Value out of range");
	return value;
}

template<typename T, typename U> T range_check_cast(const U& value, const T& minimum, const T& maximum)
{
	if ((value < minimum) || (value > maximum))
		throw std::runtime_error("Value out of range");
	return static_cast<T>(value);
}

bool parseUrl(const std::string& strUrl, std::string& strScheme, std::string& strDomain, int& iPort, std::string& strPath);

extern void NameThread(const char *);

extern bool HaveSustain();
extern std::string StopSustain();
extern std::string DoSustain();

#if (!defined(FORCE_NO_C11X) && (__cplusplus > 201100L)) || defined(FORCE_C11X)

#define C11X
#include			 	<functional>
#define UPTR_T			std::unique_ptr
#define MOVE_P(p)		std::move(p)
#define BIND_TYPE		std::bind
#define FUNCTION_TYPE	std::function
#define P_1				std::placeholders::_1
#define P_2				std::placeholders::_2
#define P_3				std::placeholders::_3
#define P_4				std::placeholders::_4

#else

#include 				<boost/bind.hpp>
#include				<boost/function.hpp>
#define UPTR_T			std::auto_ptr
#define MOVE_P(p)		(p)
#define BIND_TYPE		boost::bind
#define FUNCTION_TYPE	boost::function
#define P_1				_1
#define P_2				_2
#define P_3				_3
#define P_4				_4

#endif

#endif

// vim:ts=4
