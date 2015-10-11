/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2009  Andrey Stroganov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "zlib.hpp"
#include <zlib.h>

unsigned int ZLib::CompressBound(unsigned int value)
{
	return compressBound(value);
}

unsigned int ZLib::Compress(const char* from, unsigned int fromSize, char* to, unsigned int toSize)
{
	uLong srcSize = fromSize;
	uLong dstSize = toSize;
	if(Z_OK == compress(reinterpret_cast<Bytef*>(to), &dstSize, reinterpret_cast<const Bytef*>(from), srcSize))
		return dstSize;
	else
		throw ZLib::Error("Compress error");
	return 0;
	
}

unsigned int ZLib::Uncompress(const char* from, unsigned int fromSize, char* to, unsigned int toSize)
{
	uLong srcSize = fromSize;
	uLong dstSize = toSize;
	int result = uncompress(reinterpret_cast<Bytef*>(to), &dstSize, reinterpret_cast<const Bytef*>(from), srcSize);
	switch(result)
	{
		case Z_OK: return dstSize; break;
		case Z_MEM_ERROR: throw ZLib::Error("Z_MEM_ERROR");
		case Z_BUF_ERROR: throw ZLib::Error("Z_BUF_ERROR");
		case Z_DATA_ERROR: throw ZLib::Error("Z_DATA_ERROR");
	}

	return 0;
	
}

void ZLib::Compress(const ZLib::ByteVector& src, ZLib::ByteVector& dst)
{
	if(src.empty()) return;

	uLong srcSize = src.size();
	uLong dstSize = compressBound(srcSize);
	dst.resize(dstSize);

	if(Z_OK == compress(reinterpret_cast<Bytef*>(&dst[0]), &dstSize, reinterpret_cast<const Bytef*>(&src[0]), srcSize))
		dst.resize(dstSize);
	else
		throw ZLib::Error("Compress error");
}

void ZLib::Uncompress(const ZLib::ByteVector& src, ZLib::ByteVector& dst, unsigned int maxSize)
{
	if(src.empty()) return;

	uLong srcSize = src.size();
	uLong dstSize = srcSize * 10;
	dst.resize(dstSize);
	int result;
	while(Z_BUF_ERROR == (result = uncompress(reinterpret_cast<Bytef*>(&dst[0]), &dstSize, reinterpret_cast<const Bytef*>(&src[0]), srcSize)))
	{
		dstSize <<= 1;
		if(maxSize && dstSize > maxSize)
			throw ZLib::Error("Maximum uncompressed size exceeded");
		dst.resize(dstSize);
	}

	switch(result)
	{
		case Z_OK: dst.resize(dstSize); break;
		case Z_MEM_ERROR: throw ZLib::Error("Z_MEM_ERROR");
		case Z_BUF_ERROR: throw ZLib::Error("Z_BUF_ERROR");
		case Z_DATA_ERROR: throw ZLib::Error("Z_DATA_ERROR");
	}
}

