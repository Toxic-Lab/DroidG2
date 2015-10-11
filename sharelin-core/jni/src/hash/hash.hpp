#ifndef HASH_HPP
#define HASH_HPP

#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>


class HashConversionError: public std::runtime_error
{
public:
	HashConversionError(): std::runtime_error("Hash conversion error") {}
};

template 
<
	class HashType,
	int hash_size, 
	class Converter
> 
class Hash 
{
public:

	template <class HT, int h_s, class Cnv> 	
	friend bool operator==(const Hash<HT, h_s, Cnv>& h1, const Hash<HT, h_s, Cnv>& h2);
	
	template <class HT, int h_s, class Cnv> 	
	friend bool operator!=(const Hash<HT, h_s, Cnv>& h1, const Hash<HT, h_s, Cnv>& h2);
	
	template <class HT, int h_s, class Cnv> 	
	friend bool operator<(const Hash<HT, h_s, Cnv>& h1, const Hash<HT, h_s, Cnv>& h2);

	typedef char* iterator;
	typedef const char* const_iterator;

	Hash() { Clear(); }

	template <typename InputIterator>
	Hash(InputIterator iBegin) { Assign(iBegin); }
	Hash(const Hash<HashType, hash_size, Converter>& h) { Assign(h.begin()); }
	Hash( const std::string& s ) { FromString( s ); }

	iterator begin() { return hash_; }
	const_iterator begin() const { return hash_; }
	iterator end() { return hash_ + hash_size; }
	const_iterator end() const { return hash_ + hash_size; }

	bool Empty() const 
	{ 
		for(std::size_t i = 0; i < hash_size; ++i)
			if(hash_[i])
				return false;
		return true;
	}

	void Clear() 
	{ 
		std::fill(hash_, hash_ + hash_size, 0);
	}

	static std::size_t Size() { return hash_size; }
	const char* Bytes() const { return hash_; }
	
	template <typename T>
	void Assign(T iBegin)
	{
		for(int i = 0; i < hash_size; ++i, ++iBegin)
			hash_[i] = *iBegin;
	}

	Hash<HashType, hash_size, Converter>& operator= (const Hash<HashType, hash_size, Converter>& h) 
	{
		if(this != &h) 
			Assign(h.begin());
		
		return *this;	
	}
	
	std::string ToString() const
	{
		std::string result;
		try
		{
			Converter::Encode(begin(), end(), std::back_inserter(result));
		}
		catch(...)
		{
			throw HashConversionError();
		}
		return result;
	}

	void FromString(const std::string& str)
	{
		std::vector <char> v;
		try
		{
			Converter::Decode(str.begin(), str.end(), std::back_inserter(v));
		}
		catch(...)
		{
			throw HashConversionError();
		}
		
		if(v.size() != hash_size) throw HashConversionError();

		Assign(v.begin());
	}
	
	
protected:
	char hash_[hash_size];
};	

template <class HashType, int hash_size, class Converter> 
bool operator==(const Hash<HashType, hash_size, Converter>& h1, const Hash<HashType, hash_size, Converter>& h2)
{
	return std::equal(h1.begin(), h1.end(), h2.begin());
}

template <class HashType, int hash_size, class Converter> 
bool operator!=(const Hash<HashType, hash_size, Converter>& h1, const Hash<HashType, hash_size, Converter>& h2)
{
	return !(h1 == h2);
}

template <class HashType, int hash_size, class Converter> 
bool operator<(const Hash<HashType, hash_size, Converter>& h1, const Hash<HashType, hash_size, Converter>& h2)
{
	return std::lexicographical_compare(h1.begin(), h1.end(), h2.begin(), h2.end());
}

#endif //HASH_HPP
