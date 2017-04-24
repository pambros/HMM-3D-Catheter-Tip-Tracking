#ifndef __Q_LOW_LEVEL_HEADER_
#define __Q_LOW_LEVEL_HEADER_
#include "common/util/Util.h"

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
END_Q_NAMESPACE

#endif
