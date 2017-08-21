// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2011 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.
#ifndef RIPPLE_UINT256_H
#define RIPPLE_UINT256_H

#include <algorithm>
#include <climits>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "types.h"
#include "utils.h"

#if defined(_MSC_VER) && _MSC_VER < 1300
#define for  if (false) ; else for
#endif

// These classes all store their values internally
// in big-endian form

// We have to keep a separate base class without constructors
// so the compiler will let us use it in a union
template<unsigned int BITS>
class base_uint
{
protected:
	enum { WIDTH=BITS/32 };

	// This is really big-endian in byte order.
	// We sometimes use unsigned int for speed.
	unsigned int pn[WIDTH];

public:
	bool isZero() const
	{
		for (int i = 0; i < WIDTH; i++)
			if (pn[i] != 0)
				return false;
		return true;
	}

	bool isNonZero() const
	{
		return !isZero();
	}

	bool operator!() const
	{
		return isZero();
	}

	const base_uint operator~() const
	{
		base_uint ret;

		for (int i = 0; i < WIDTH; i++)
			ret.pn[i] = ~pn[i];

		return ret;
	}

	base_uint& operator++()
	{
		// prefix operator
		for (int i = WIDTH - 1; i >= 0; --i)
		{
			pn[i] = htobe32(be32toh(pn[i]) + 1);
			if (pn[i] != 0)
				break;
		}

		return *this;
	}

	const base_uint operator++(int)
	{
		// postfix operator
		const base_uint ret = *this;
		++(*this);

		return ret;
	}

	const base_uint operator--(int)
	{
		// postfix operator
		const base_uint ret = *this;
		--(*this);

		return ret;
	}

	std::string GetHex() const
	{
		return strHex(begin(), size());
	}

	bool SetHex(const std::string& str, bool bStrict=false)
	{
		return SetHex(str.c_str(), bStrict);
	}

	std::string ToString() const
	{
		return GetHex();
	}

	unsigned char* begin()
	{
		return reinterpret_cast<unsigned char*>(pn);
	}

	unsigned char* end()
	{
		return reinterpret_cast<unsigned char*>(pn + WIDTH);
	}

	const unsigned char* begin() const
	{
		return reinterpret_cast<const unsigned char*>(pn);
	}

	const unsigned char* end() const
	{
		return reinterpret_cast<const unsigned char*>(pn + WIDTH);
	}

	unsigned int size() const
	{
		return sizeof(pn);
	}

	void zero()
	{
		memset(&pn[0], 0, sizeof(pn));
	}

	friend class uint128;
	friend class uint160;
	friend class uint256;
//DEL 	friend inline int Testuint256AdHoc(std::vector<std::string> vArg);
};

typedef base_uint<128> base_uint128;
typedef base_uint<160> base_uint160;
typedef base_uint<256> base_uint256;

//
// uint128, uint160, & uint256 could be implemented as templates, but to keep
// compile errors and debugging cleaner, they're copy and pasted.
//

//////////////////////////////////////////////////////////////////////////////
//
// uint128
//

class uint128 : public base_uint128
{
public:
	typedef base_uint128 basetype;

	uint128()
	{
		zero();
	}

	uint128(const basetype& b)
	{
		*this = b;
	}

	uint128& operator=(const basetype& b)
	{
		for (int i = 0; i < WIDTH; i++)
			pn[i] = b.pn[i];

		return *this;
	}

	explicit uint128(const base_uint256& b) {
		for (int i = 0; i < WIDTH; i++)
			pn[i] = b.pn[i];
	}

	explicit uint128(const std::vector<unsigned char>& vch)
	{
		if (vch.size() == size())
			memcpy(pn, &vch[0], size());
		else
			zero();
	}

};

//////////////////////////////////////////////////////////////////////////////
//
// uint256
//

class uint256 : public base_uint256
{
public:
	typedef base_uint256 basetype;

	uint256()
	{
		zero();
	}

	uint256(const basetype& b)
	{
		*this	= b;
	}

	uint256& operator=(const basetype& b)
	{
		if (pn != b.pn)
			memcpy(pn, b.pn, sizeof(pn));
		return *this;
	}

	uint256(uint64 b)
	{
		*this = b;
	}

	uint256& operator=(uint64 uHost)
	{
		zero();

		// Put in least significant bits.
		((uint64*) end())[-1]	= htobe64(uHost);

		return *this;
	}

	explicit uint256(const std::string& str)
	{
		SetHex(str);
	}

	explicit uint256(const std::vector<unsigned char>& vch)
	{
		if (vch.size() == sizeof(pn))
			memcpy(pn, &vch[0], sizeof(pn));
		else
		{
			assert(false);
			zero();
		}
	}
};

//////////////////////////////////////////////////////////////////////////////
//
// uint160
//

class uint160 : public base_uint160
{
public:
	typedef base_uint160 basetype;

	uint160()
	{
		zero();
	}

	uint160(const basetype& b)
	{
		*this	= b;
	}

	uint160& operator=(const basetype& b)
	{
		for (int i = 0; i < WIDTH; i++)
			pn[i] = b.pn[i];

		return *this;
	}

	uint160(uint64 b)
	{
		*this = b;
	}

	uint160& operator=(uint64 uHost)
	{
		zero();

		// Put in least significant bits.
		((uint64*) end())[-1]	= htobe64(uHost);

		return *this;
	}

	explicit uint160(const std::string& str)
	{
		SetHex(str);
	}

	explicit uint160(const std::vector<unsigned char>& vch)
	{
		if (vch.size() == sizeof(pn))
			memcpy(pn, &vch[0], sizeof(pn));
		else
			zero();
	}

	base_uint256 to256() const
	{
	  uint256 m;
	  memcpy(m.begin(), begin(), size());
	  return m;
	}

};


#endif
// vim:ts=4
