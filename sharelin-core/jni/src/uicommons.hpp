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

#ifndef UICOMMONS_HPP
#define UICOMMONS_HPP

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <assert.h>
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

#include "types.hpp"
#include "hashes.hpp"
#include "ui/tag.hpp"
#include "ui/sortprefs.hpp"
#include "searchdefs.hpp"
#include "netfwd.hpp"

namespace Ui {

struct SearchResultGroup
{
	typedef std::pair<std::string, ID> NameIdPair;
	typedef std::vector<NameIdPair> RankedNames;
	uint64 fileSize;
	uint sources;
	RankedNames names;
	char alpha;
};

struct TempRankName
{
	std::string name;
	ID id;
	uint counter;
	TempRankName(): id(0), counter(0) {}
};

struct TempRankName_to_RankName
{
	SearchResultGroup::NameIdPair operator()(const TempRankName& tmp)
	{
		SearchResultGroup::NameIdPair x;
		x.first = tmp.name;
		x.second = tmp.id;
		return x;
	}
};

std::string HumanReadable(int64 x);
std::string FileType(const std::string& name);
std::string AdaptString(std::string s);
uint Progress(uint64 n, uint64 total);
uint DownloadProgress(DownloadPtr);
uint64 GetSizeValue(const std::string& s);
std::string RankedNumber(uint64 x, const std::string& separator);

typedef std::vector<SearchResultGroup> SRGroupContainer;

bool operator< (const TempRankName& r1, const TempRankName& r2);

template <typename T>
std::string FormatExtensions(T begin, T end)
{
	std::stringstream s;
	for(T i = begin; i != end; ++i)
	{
		if(i != begin) s << ", ";

		s << *i;
	}

	return s.str();
}

template <typename InputIterator>
SearchResultGroup::RankedNames MakeRankedNames(InputIterator resItersBegin, InputIterator resItersEnd)
{
	std::map<std::string, TempRankName> rankTop;
	for(InputIterator i = resItersBegin; i != resItersEnd; ++i)
	{
		const Search::Result& result = **i;
		if(rankTop.find(result.hit.dn) == rankTop.end())
		{
			TempRankName tmp;
			tmp.name = result.hit.dn;
			tmp.id = result.id;
			rankTop[result.hit.dn] = tmp;
		}
		rankTop[result.hit.dn].counter++;
	}

	std::vector<TempRankName> tempRanks;
	for(std::map<std::string, TempRankName>::iterator i = rankTop.begin(); i != rankTop.end(); ++i)
		tempRanks.push_back(i->second);	

	std::sort(tempRanks.begin(), tempRanks.end());
	SearchResultGroup::RankedNames names;
	std::transform(tempRanks.begin(), tempRanks.end(), std::back_inserter(names), TempRankName_to_RankName());
	return names;
}

template <typename InputIterator>
void CreateOrderedSRGroups(InputIterator resultsBegin, InputIterator resultsEnd, 
		SRGroupContainer& orderedGroups, SortOption sopt)
{
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "CreateOrderedSRGroups_0");
	orderedGroups.clear();

	typedef std::vector<InputIterator> ResultIteratorsContainer;
	typedef std::map<Hashes::SHA1, ResultIteratorsContainer> SHA1Table;
	SHA1Table sha1Table;

	for(InputIterator i = resultsBegin; i != resultsEnd; ++i)
		sha1Table[i->hit.sha1].push_back(i);

	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "CreateOrderedSRGroups_1");
	for(typename SHA1Table::iterator i = sha1Table.begin(); i != sha1Table.end(); ++i)
	{
		SearchResultGroup gr;
		gr.names = MakeRankedNames(i->second.begin(), i->second.end());
		gr.sources = i->second.size();
		assert(gr.names.size() > 0);
		if(gr.names.begin()->first.size() > 0)
			gr.alpha = gr.names.begin()->first[0];
		else gr.alpha = 0;
		const Search::Result& firstResult = **(i->second.begin());
		gr.fileSize = firstResult.hit.size;
		orderedGroups.push_back(gr);
	}

	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "CreateOrderedSRGroups_2");
	switch(sopt.SortBy())
	{
	case Tags::sources:
		std::sort(orderedGroups.begin(), orderedGroups.end(),
	       		boost::bind(std::less<uint>(), boost::bind(&SearchResultGroup::sources, _1), 
				boost::bind(&SearchResultGroup::sources, _2)));
		break;
	case Tags::size:
		std::sort(orderedGroups.begin(), orderedGroups.end(),
			boost::bind(std::less<uint64>(), boost::bind(&SearchResultGroup::fileSize, _1), 
				boost::bind(&SearchResultGroup::fileSize, _2)));
		break;
	case Tags::name:
		std::sort(orderedGroups.begin(), orderedGroups.end(),
			boost::bind(std::less<char>(), boost::bind(&SearchResultGroup::alpha, _1), 
				boost::bind(&SearchResultGroup::alpha, _2)));
		break;
	default:
		//do nothing
		break;
	}

	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "CreateOrderedSRGroups_3");
	if(sopt.Forward())
		std::reverse(orderedGroups.begin(), orderedGroups.end());

}
	/*
	struct SharedFileStats
	{
		std::string name;
		int64 size;
		uint hits;
		uint requests;
	};



	struct IncomingFile
	{
		std::string name;
		uint64 size;
	};

	typedef std::vector<SharedFileStats> SFStatsContainer;
	typedef std::vector<IncomingFile> IncomingFilesType;
	typedef std::vector<ID> IDList;

	SFStatsContainer GetOrderedSFStatistics(const SortOptions& sopt);

	ID DownloadWithSearch(ID sid, ID rid);
	ID DownloadWithLink(const std::string& link);
	IDList GetOrderedDownloadsIDs(const SortOptions& sopt);
	IncomingFilesType GetOrderedIncomingFiles(const SortOptions& sopt);
	std::string FormatExtensions(const srch::Criteria::FileExtensionsType& extensions);
	//Statistics::UploadedFilesType GetOrderedUploadedFiles(const SortOptions& sopt);
	*/
}; //namespace Ui


#endif //UICOMMONS_HPP

