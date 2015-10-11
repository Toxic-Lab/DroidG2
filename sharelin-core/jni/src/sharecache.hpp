/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2010 by Andrew Stroganov <savthe@gmail.com>

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

#ifndef SHARECACHE_HPP
#define SHARECACHE_HPP

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <string>
#include "hashes.hpp"
#include <set>
#include <list>
#include <map>
#include <vector>
#include "types.hpp"
#include "idgen.hpp"
#include "sharedefs.hpp"
#include <boost/weak_ptr.hpp>

namespace Share {

using boost::multi_index_container;
using namespace boost::multi_index;

class Cache
{
public:
	struct CachedInfo
	{
		CachedInfo(FileInfoPtr p): path(p->path), sha1(p->sha1), ttr(p->ttr),
			ed2k(p->ed2k), md5(p->md5), id(p->id), pInfo(p) {}

		bool Compare(const FileInfo& f) const
		{
			return path == f.path && sha1 == f.sha1 && ttr == f.ttr &&
			       ed2k == f.ed2k && md5 == f.md5 && id == f.id;
		}	

		std::string path;
		Hashes::SHA1 sha1;
		Hashes::TTR ttr;
		Hashes::ED2K ed2k;
		Hashes::MD5 md5;
		ID id;
		FileInfoPtr pInfo;
	};

	typedef multi_index_container<
		CachedInfo, indexed_by<
			ordered_unique< tag<tags::path>, BOOST_MULTI_INDEX_MEMBER(CachedInfo, std::string, path)>,
			ordered_unique< tag<tags::id>, BOOST_MULTI_INDEX_MEMBER(CachedInfo, ID, id)>,
			ordered_unique< tag<tags::ptr>, BOOST_MULTI_INDEX_MEMBER(CachedInfo, FileInfoPtr, pInfo)>,
			ordered_non_unique< tag<tags::sha1>, BOOST_MULTI_INDEX_MEMBER(CachedInfo, Hashes::SHA1, sha1)>,
			ordered_non_unique< tag<tags::md5>, BOOST_MULTI_INDEX_MEMBER(CachedInfo, Hashes::MD5, md5)>,
			ordered_non_unique< tag<tags::ed2k>, BOOST_MULTI_INDEX_MEMBER(CachedInfo, Hashes::ED2K, ed2k)>,
			ordered_non_unique< tag<tags::ttr>, BOOST_MULTI_INDEX_MEMBER(CachedInfo, Hashes::TTR, ttr)>
		> 
	> InfoContainer;

	typedef InfoContainer::iterator iterator;

	void Add(FileInfoPtr);
	void Update(FileInfoPtr);
	void Remove(FileInfoPtr);
	bool Empty() const { return cache_.empty(); }
	std::size_t Size() const { return cache_.size(); }
	iterator begin() const { return cache_.begin(); }
	iterator end() const { return cache_.end(); }


	template <typename tag, typename Key>
	FileInfoPtr Get(Key key) const { return GetIterator<tag>(key)->pInfo; }

	template <typename T>
	void Dump(T out) const
	{
		for(InfoContainer::iterator i = cache_.begin(); i != cache_.end(); ++i)
			*out++ = i->pInfo;
	}

private:

	template <typename tag, typename Key> 
	typename InfoContainer::index_iterator<tag>::type GetIterator(Key key) const
	{
		typename InfoContainer::index_iterator<tag>::type i = cache_.get<tag>().find(key);
		if(i != cache_.get<tag>().end()) 
			return i;
		throw NotFound();
	}

	InfoContainer cache_;
};

} //namespace Share
#endif //SHARECACHE_HPP
