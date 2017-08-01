#include "common/util/LowLevel.h"

BEGIN_Q_NAMESPACE
qu16 Swap2(qu16 _v)
{
	return qu16((_v << 8) | (_v >> 8));
}

qu32 Swap4(qu32 _v)
{
	return qu32((_v << 24)
			 | ((_v << 8) & 0x00ff0000)
			 | ((_v >> 8) & 0x0000ff00)
			 | ((_v >> 24)));
}

qu64 Swap8(qu64 _v)
{
	return qu64((_v << 56)
			 | ((_v << 40) & 0x00ff000000000000)
			 | ((_v << 24) & 0x0000ff0000000000)
			 | ((_v << 8) & 0x000000ff00000000)
			 | ((_v >> 8) & 0x00000000ff000000)
			 | ((_v >> 24) & 0x0000000000ff0000)
			 | ((_v >> 40) & 0x000000000000ff00)
			 | ((_v >> 56)));
}

qu16 GetU16(const qchar8 *_buffer)
{
	return (_buffer[1] & 0x000000FF)
		 | ((_buffer[0]<<8) & 0x0000FF00);
}

void SetU16(qchar8 *_buffer, qu16 _val)
{
	_buffer[0] = _val>>8 & 0x000000FF;
	_buffer[1] = _val & 0x000000FF;
}

qu32 GetU32(const qchar8 *_buffer)
{
	return (_buffer[3] & 0x000000FF)
		 | ((_buffer[2]<<8) & 0x0000FF00)
	     | ((_buffer[1]<<16) & 0x00FF0000)
		 | ((_buffer[0]<<24) & 0xFF000000);
}

void SetU32(qchar8 *_buffer, qu32 _val)
{
	_buffer[0] = _val>>24 & 0x000000FF;
	_buffer[1] = _val>>16 & 0x000000FF;
	_buffer[2] = _val>>8 & 0x000000FF;
	_buffer[3] = _val & 0x000000FF;
}

qu64 GetU64(const qchar8 *_buffer)
{
	return (_buffer[7] & 0x00000000000000FF)
		 | ((_buffer[6]<<8) & 0x000000000000FF00)
	     | ((_buffer[5]<<16) & 0x0000000000FF0000)
		 | ((_buffer[4]<<24) & 0x00000000FF000000)
	     | ((static_cast<qu64>(_buffer[3])<<32) & 0x000000FF00000000)
		 | ((static_cast<qu64>(_buffer[2])<<40) & 0x0000FF0000000000)
	     | ((static_cast<qu64>(_buffer[1])<<48) & 0x00FF000000000000)
		 | ((static_cast<qu64>(_buffer[0])<<56) & 0xFF00000000000000);
}

void SetU64(qchar8 *_buffer, qu64 _val)
{
	_buffer[0] = _val>>56 & 0x000000FF;
	_buffer[1] = _val>>48 & 0x000000FF;
	_buffer[2] = _val>>40 & 0x000000FF;
	_buffer[3] = _val>>32 & 0x000000FF;
	_buffer[4] = _val>>24 & 0x000000FF;
	_buffer[5] = _val>>16 & 0x000000FF;
	_buffer[6] = _val>>8 & 0x000000FF;
	_buffer[7] = _val & 0x000000FF;
}

qs32 CharToHexa(qu8 _char)
{
	if(_char <= '9' && _char >= '0')
	{
		return _char - '0';
	}
	else if(_char <= 'F' && _char >= 'A')
	{
		return _char - 'A' + 10;
	}
	else if(_char <= 'f' && _char >= 'a')
	{
		return _char - 'a' + 10;
	}
	return 0;
}

qu32 IsLittleEndian(void){
	static const qu32 NL_AT_END = 0x000A;
	return ((char*)(void*)&NL_AT_END)[0] == '\n';
}
END_Q_NAMESPACE
