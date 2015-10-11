/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2009  Andrey Stroganov <savthe@gmail.com>

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

#include "hashes.hpp"
/*#include <stdexcept>

byte HexByte(char c)
{
	switch(c)
	{
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': case 'A': return 10;
		case 'b': case 'B': return 11;
		case 'c': case 'C': return 12;
		case 'd': case 'D': return 13;
		case 'e': case 'E': return 14;
		case 'f': case 'F': return 15;
	}
	return 0xff;
}

std::string Hashes::GenericHex::Encode(const byte* hash, int size)
{
	const char* digits = "0123456789abcdef";
	const byte* p = hash;
	std::string hex;
	hex.resize(size * 2);
	for(int i = 0; i < size; i++, p++) {
		hex[2*i] = digits[((*p) & 0xf0) >> 4];
		hex[2*i + 1] = digits[(*p) & 0x0f];
	}
	
	return hex;
}

bool Hashes::GenericHex::Decode(const std::string& from, byte* hash, int hash_size)
{
	int size = from.size() / 2;
	byte* p = hash;
	
	if (size != hash_size) 
		return false;
	
	for(int i = 0; i < size; i++, p++)	{
		byte b1 = HexByte(from[2 * i]);
		byte b2 = HexByte(from[2 * i + 1]);
		
		if(b1 == 0xff || b2 == 0xff) 
			return false;
		
		*p = (b1 << 4) | b2; //TODO error check
	}
	
	return true;	
}

std::string Hashes::GenericBase32::Encode(const byte* hash, int size)
{
	int encoded_len = CyoEncode::Base32EncodeGetLength(size);
	char* buf = new char[encoded_len];
	
	try{
		CyoEncode::Base32Encode(buf, hash, size);
	} catch(std::runtime_error&) {
		delete[] buf;
		return std::string();
	}
	while(encoded_len && buf[encoded_len-1] == '=') 
		--encoded_len;
	
	assert(encoded_len > 0);
	std::string str(buf, encoded_len);
	delete[] buf;
	
	return str;
}

bool Hashes::GenericBase32::Decode(std::string from, byte* hash, int hash_size)
{
	while((from.size() * 5) % 8) 
		from.push_back('=');
	
	int decoded_len;
	try {
		decoded_len = CyoDecode::Base32DecodeGetLength(from.size());
	} catch(std::runtime_error) {
		return false;
	}
	
	if(decoded_len < hash_size) 
		return false;
	
	char* buf = new char[decoded_len];
	try {
		CyoDecode::Base32Decode(buf, from.c_str(), from.size());
	} catch(std::runtime_error) {
		delete[] buf;
		return false;
	}
	std::copy(buf, buf + hash_size, hash);
	delete[] buf;
	
	return true;
}

std::string Hashes::BPBase32::Encode(const byte* hash, int size)
{	
	assert(size == Hashes::SHA1::Size() + Hashes::TTR::Size());
	return GenericBase32::Encode(hash, Hashes::SHA1::Size()) + "." + GenericBase32::Encode(hash + Hashes::SHA1::Size(), Hashes::TTR::Size());
}

bool Hashes::BPBase32::Decode(std::string from, byte* hash, int hash_size)
{
	std::string::size_type separator = from.find('.');
	if(separator == std::string::npos || separator == from.size() - 1)
		return false;
	
	if(!GenericBase32::Decode(from.substr(0, separator), hash, Hashes::SHA1::Size()))
		return false;
	
	if(!GenericBase32::Decode(from.substr(separator + 1), hash + Hashes::SHA1::Size(), Hashes::TTR::Size()))
		return false;
	
	return true;
}

Hashes::BP Hashes::ComposeBP(const Hashes::SHA1& sha1, const Hashes::TTR& ttr)
{
	byte* buf = new byte[Hashes::BP::Size()];
	memcpy(buf, sha1.Get(), Hashes::SHA1::Size());
	memcpy(buf + Hashes::SHA1::Size(), ttr.Get(), Hashes::TTR::Size());
	Hashes::BP bp(buf);
	delete[] buf;
	return bp;
}

void Hashes::DecomposeBP(const Hashes::BP& bp, Hashes::SHA1& sha1, Hashes::TTR& ttr)
{
	sha1.Set(bp.Get());
	ttr.Set(bp.Get() + Hashes::SHA1::Size());
}
*/
