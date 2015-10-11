/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2010  Andrey Stroganov <savthe@gmail.com>

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

#include <iostream>
#include "sharecache.hpp"
#include <set>
#include "queryhashtable.hpp"
#include <algorithm>
#include <iostream>
#include "settings.hpp"
#include "filesystem.hpp"
#include "boost/algorithm/string.hpp"

namespace Share {

const time_t PERIOD_HASH = 7 * 24 * 60 * 60; //1 week

void Cache::Add(FileInfoPtr p)
{
	if( !cache_.insert(p).second ) 
		throw AlreadyShared();
}

void Cache::Remove(FileInfoPtr p)
{
	cache_.get<tags::ptr>().erase(p);
}

void Cache::Update(FileInfoPtr p)
{
	cache_.get<tags::ptr>().replace(GetIterator<tags::ptr>(p), CachedInfo(p));
}



} //namespace Share


/*void SharedCache::Show()
{
	FilesSet::iterator i;
	for(i = cache.begin(); i != cache.end(); i++) {
		std::cout << i->hashed << "   " << i->RelativeName() << " " << i->size << " " << i->sha1.Encode() << " " << i->md5.Encode() << " " << i->ttr.Encode() << " " << i->bp.Encode()<< " " << i->ed2k.Encode()<<std::endl;
	}
}*/
/*
void Cache::Add(const std::string& file_name)
{
	FileInfo fi;
	fi.name = file_name;
	Add(fi);
}

Cache::Cache()
{
	kb_size = 0;
	files_count = 0;

}

void Cache::Add(FileInfo& fi)
{
	FilesSet::index_iterator<Tag::id>::type i = cache.get<Tag::id>().find(fi.id);
	if( i != get<Tag::id>(cache).end() ) 
		Remove(fi.name);
	
	//generate unique id
	while(fi.id == INVALID_ID || get<Tag::id>(cache).find(fi.id) != get<Tag::id>(cache).end())
		fi.id = idGen_.Get();
	
	assert(cache.get<Tag::name>().find(fi.name) == get<Tag::name>(cache).end());
	std::pair<CachedIterator, bool> it = cache.get<Tag::id>().insert(fi);
	files_count++;

	if(fi.hashed) 
	{
		std::set<std::string> keywords;
		ExtractKeywords(FileSystem::FileName(fi.name), std::inserter(keywords, keywords.begin()));
		for(std::set<std::string>::iterator i = keywords.begin(); i != keywords.end(); i++) 
			map_keywords[boost::to_lower_copy(*i)].insert(it.first);
	
		kb_size += fi.size / 1024;
	}
}

bool Cache::Update(const FileInfo& f)
{
	FilesSet::index_iterator<Tag::id>::type i = get<Tag::id>(cache).find(f.id);
	if( i != get<Tag::id>(cache).end() ) 
		get<Tag::id>(cache).replace(i, f);
	return i != get<Tag::id>(cache).end();
	
}

bool Cache::GetToHash(FileInfo& sfile)
{
	FilesSet::index_iterator<Tag::hashed>::type i = cache.get<Tag::hashed>().begin();
	
	//skip partial files
	while (i != cache.get<Tag::hashed>().end() && i->partial) ++i;
	
	if(i != cache.get<Tag::hashed>().end() && i->hashed + PERIOD_HASH < time(0)) 
	{
		sfile = *i;
		return true;
	}
	
	return false;
}

void Cache::GetByDn(const std::string& dn, std::vector<FileInfo>& results)
{
	if(dn.size() < 3) return;
	
	std::string::size_type begin = 0, end = 0;
	bool include;
	std::list <std::string> incl, excl, incl_exact, excl_exact;
	//std::cout << "Got dname: " << dn << std::endl;
	while(begin < dn.size()) 
	{
		switch(dn[begin])
		{
			case '-': include = false; begin++; break;
			case '+': include = true; begin++; break;
			case ' ': begin++; continue;
			default: include = true;
		}
		
		if(begin >= dn.size()) break;
		if(dn[begin] == ' ') continue;
		
		char sep = ' ';
		bool exact = false;
		if(dn[begin] == '"') 
		{
			sep = '"';
			begin++;
			exact = true;
		}
		if(begin >= dn.size()) break;
		
		end = dn.find(sep, begin);
		end = (end != std::string::npos ? end : dn.size());
		assert(begin < end);
	
		if(end - begin > 2) 
		{
			const std::string range = dn.substr(begin, end - begin);
			if(include) 
			{
				if(exact) 
				{
					std::stringstream stream(range);
					std::string word;
					while(stream >> word) incl.push_back(word);
					incl_exact.push_back(range);
				}
				else incl.push_back(range);
			}
			else 
			{
				if(exact) excl_exact.push_back(range);
				else excl.push_back(range);
			}
		}
		
		begin = end + 1;
	}
	
	if(incl.empty()) return;
	
	std::list<CachedIterator> res_iterators;
	std::list<std::string>::iterator i = incl.begin();
	std::map<std::string, std::set<CachedIterator> > ::iterator it_kw = map_keywords.find(*i);
	if(it_kw == map_keywords.end()) return;
	std::copy(it_kw->second.begin(), it_kw->second.end(), std::back_inserter(res_iterators));
	
	i++;
	
	for(; i != incl.end() && !res_iterators.empty(); i++) ApplyKeyword(res_iterators, *i, true);
	for(i = excl.begin(); i != excl.end() && !res_iterators.empty(); i++) ApplyKeyword(res_iterators, *i, false);
	for(i = incl_exact.begin(); i != incl_exact.end() && !res_iterators.empty(); i++) 
	{
		for(std::list<CachedIterator>::iterator it_res = res_iterators.begin(); it_res != res_iterators.end();) 
		{
			if(boost::to_lower_copy(FileSystem::FileName((*it_res)->name)).find(*i) == std::string::npos) 
				it_res = res_iterators.erase(it_res);
			else it_res++;
		}
	}
	
	for(i = excl_exact.begin(); i != excl_exact.end() && !res_iterators.empty(); i++) 
	{
		for(std::list<CachedIterator>::iterator it_res = res_iterators.begin(); it_res != res_iterators.end();) 
		{
			if(boost::to_lower_copy(FileSystem::FileName((*it_res)->name)).find(*i) != std::string::npos) 
				it_res = res_iterators.erase(it_res);
			else it_res++;
		}
	}
	
	for(std::list<CachedIterator>::iterator r = res_iterators.begin(); r != res_iterators.end() && results.size() < 15; r++) 
		results.push_back(**r);

}

void Cache::ApplyKeyword(std::list<CachedIterator>& lst, const std::string& kw, bool include)
{
	if(lst.empty()) return;
	
	std::map<std::string, std::set<CachedIterator> > ::iterator it_kw = map_keywords.find(kw);
	
	if(include) 
	{
		if(it_kw == map_keywords.end()) lst.clear();
		
		for(std::list<CachedIterator>::iterator i = lst.begin(); i != lst.end();) 
		{
			if(it_kw->second.find(*i) == it_kw->second.end()) 
				i = lst.erase(i);
			else i++;
		}
	} 
	else 
	{
		if(it_kw == map_keywords.end()) return;
		
		for(std::list<CachedIterator>::iterator i = lst.begin(); i != lst.end();) 
		{
			if(it_kw->second.find(*i) != it_kw->second.end()) 
				i = lst.erase(i);
			else i++;
		}
	}
}

void Cache::Save() 
{
	verb.dev << "Saving cache" << verb.flush;
	TiXmlDocument xml_cache;
	FilesSet::iterator i;
	
	for(i = cache.begin(); i != cache.end(); i++)
	{
		if(i->partial)
			continue;
		
		TiXmlElement file( "file" );
		
		file.SetAttribute("name", i->name);
		file.SetAttribute("id", ToString(i->id));
		
		TiXmlElement size( "size" );
		size.InsertEndChild( TiXmlText( ToString( i->size ) ) );
		file.InsertEndChild( size );
		
		TiXmlElement hashed( "hashed" );
		hashed.InsertEndChild( TiXmlText( ToString( i->hashed ) ) );
		file.InsertEndChild( hashed );
		
		TiXmlElement hits( "hits" );
		hits.InsertEndChild( TiXmlText( ToString( i->hits ) ) );
		file.InsertEndChild( hits );
		
		TiXmlElement requests( "requests" );
		requests.InsertEndChild( TiXmlText( ToString( i->requests ) ) );
		file.InsertEndChild( requests );
		
		if( !i->sha1.Empty() ) 
		{
			TiXmlElement sha1( "sha1" );
			sha1.InsertEndChild( TiXmlText( i->sha1.ToString() ) );
			file.InsertEndChild(sha1);
		}
		
		if( !i->ttr.Empty() ) 
		{
			TiXmlElement ttr( "ttr" );
			ttr.InsertEndChild( TiXmlText( i->ttr.ToString() ) );
			file.InsertEndChild(ttr);
		}
		
		if( !i->md5.Empty() ) 
		{
			TiXmlElement md5( "md5" );
			md5.InsertEndChild( TiXmlText( i->md5.ToString() ) );
			file.InsertEndChild( md5 );
		}
		
		if( !i->ed2k.Empty() ) 
		{
			TiXmlElement ed2k( "ed2k" );
			ed2k.InsertEndChild( TiXmlText( i->ed2k.ToString() ) );
			file.InsertEndChild(ed2k);
		}
		
		xml_cache.InsertEndChild(file);
	}
	
	//xml_cache.Print();
	
	xml_cache.SaveFile( "share.dat" );
}

void Cache::BuildQueryHash(QueryHashTable& qh)
{
	qh.Reset();
	FilesSet::iterator i;
	for(i = cache.begin(); i != cache.end(); i++) 
	{
		qh.AddString(i->name);
		if(!i->sha1.Empty()) 
			qh.AddExact("urn:sha1:" + i->sha1.ToString());
		if(!i->ttr.Empty()) 
		{
			qh.AddExact("urn:tree:tiger/:" + i->ttr.ToString());
			qh.AddExact("urn:ttr:" + i->ttr.ToString());
		}
		if(!i->md5.Empty())
			qh.AddExact("urn:md5:" + i->md5.ToString());
		if(!i->ed2k.Empty())
			qh.AddExact("urn:ed2k:" + i->ed2k.ToString());
	}
}

void Cache::Load()
{
	TiXmlDocument xml_cache( "share.dat" );
	
	if( !xml_cache.LoadFile() ) 
	{
		verb.bas << "Error reading share.dat: " << xml_cache.ErrorDesc() << verb.flush;
		return;
	}
	
	for( TiXmlNode* p_node = xml_cache.FirstChild( "file" ); p_node; p_node = p_node->NextSibling() ) 
	{
		if( TiXmlElement* p = p_node->ToElement() ) 
		{
			FileInfo fi;
			const char* name = p->Attribute("name");
			if( !name ) continue;
			fi.name = name;
			
			const char* id = p->Attribute("id");
			if(id) fi.id = atoi(id);
			
			for( TiXmlNode* p_child = p->FirstChild(); p_child; p_child = p_child->NextSibling() ) 
			{
				if( TiXmlElement* p_el = p_child->ToElement() ) 
				{
					const char* val = p_el->GetText();
					if( !val ) continue;
					if( p_child->ValueStr() == "size" ) fi.size = atoll( val );
					if( p_child->ValueStr() == "hashed" ) fi.hashed = atoi( val );
					if( p_child->ValueStr() == "hits" ) fi.hits = atoi( val );
					if( p_child->ValueStr() == "requests" ) fi.requests = atoi( val );
					if( p_child->ValueStr() == "sha1" ) fi.sha1.FromString( val );
					if( p_child->ValueStr() == "ttr" ) fi.ttr.FromString( val );
					if( p_child->ValueStr() == "md5" ) fi.md5.FromString( val );
					if( p_child->ValueStr() == "ed2k" ) fi.ed2k.FromString( val );
				}
			}
			
			//if(!fi.sha1.Empty() && !fi.ttr.Empty())
			//	fi.bp = Hashes::ComposeBP(fi.sha1, fi.ttr);
			
			Add(fi);
		}
		
	}
}
	
void Cache::Status(uint32 files, uint32 size) const
{
	files = files_count;
	size = kb_size;
}

void Cache::Remove(const std::string& name)
{
	Remove(Find<Tag::name>(name));
}

void Cache::Remove(ID id)
{
	typedef std::set<std::string> StrSet;
	CachedIterator it = cache.get<Tag::id>().find(id);
	if(it == cache.get<Tag::id>().end()) return;
	
	StrSet keywords;
	ExtractKeywords(FileSystem::FileName(it->name), std::inserter(keywords, keywords.begin()));

	for(StrSet::iterator ikw = keywords.begin(); ikw != keywords.end(); ikw++) 
	{
		KeywordsMap::iterator it_map;
		it_map = map_keywords.find(boost::to_lower_copy(*ikw));
		
		if(it_map == map_keywords.end()) continue;
		
		it_map->second.erase(it);
		if(it_map->second.empty())
			map_keywords.erase(it_map);
		
	}
	
	kb_size -= it->size / 1024;
	files_count--;
	
	cache.get<Tag::id>().erase(it);
}

*/

