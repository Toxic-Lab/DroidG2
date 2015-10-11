#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>
#include <assert.h>

namespace Debug {

template <typename T> 
void PrintHex(T begin, T end)
{
	for(T i = begin; i != end; ++i)
	{
		std::cout << std::uppercase << std::hex << (int(*i) & 0xFF);
		std::cout << ' ';
	}
	std::cout << std::dec;
	std::cout << std::endl;
}

template <typename T>
void PrintStream(T& s)
{
	std::istream is(&s);
	std::streampos pos = is.tellg();
	while(is) std::cout << char(is.get());
	is.seekg(pos);
}

} //namespace Debug

#endif //DEBUG_HPP

