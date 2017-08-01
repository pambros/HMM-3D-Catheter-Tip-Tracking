#ifndef __Q_LOW_LEVEL_HEADER_
#define __Q_LOW_LEVEL_HEADER_
#include "common/util/Util.h"

// REFS http://www.zipcon.net/~swhite/docs/computers/languages/c_multi-char_const.html
#define LE_CHR(_a, _b, _c, _d) (((_a)<<24) | ((_b)<<16) | ((_c)<<8) | (_d))

//#define USE_ONLY_LITTLE_ENDIAN
#ifdef USE_ONLY_LITTLE_ENDIAN
	#if('q\0\0\0' & 'q')
		#error("architecture is big-endian")
	#endif

	#if('abcd' != LE_CHR('a', 'b', 'c', 'd'))
		#error("unexpected multi-character packing")
	#endif

	#if('\0abc' != 'abc')
		#error("compiler not padding multi-chars on the left")
	#endif
#endif

BEGIN_Q_NAMESPACE
	// Big endian    0x0A0B0C0D
	// Little endian 0x0D0C0B0A
	// Swap the bytes in an unsigned short.
	qu16 Swap2(qu16 _v);
	// Swap the bytes in an unsigned long.
	qu32 Swap4(qu32 _v);
	// Swap the bytes in an unsigned long long.
	qu64 Swap8(qu64 _v);

	qu16 GetU16(const qchar8 *_buffer);
	void SetU16(qchar8 *_buffer, qu16 _val);
	qu32 GetU32(const qchar8 *_buffer);
	void SetU32(qchar8 *_buffer, qu32 _val);
	qu64 GetU64(const qchar8 *_buffer);
	void SetU64(qchar8 *_buffer, qu64 _val);

	qs32 CharToHexa(qu8 _char);

	qu32 IsLittleEndian(void);
END_Q_NAMESPACE

#endif
