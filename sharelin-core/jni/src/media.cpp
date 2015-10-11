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

#include "media.hpp"
#include <boost/algorithm/string.hpp>
#include <cstring>
#include <string>
#include <stdexcept>

namespace Media {

const char* Exts(Type t)
{
	switch(t)
	{
		case video: return extVideo;
		case audio: return extAudio;
		case archive: return extArchive;
		case msapp: return extMsapp;
		case cdimage: return extCdimage;
		case document: return extDocument;
		case picture: return extPicture;
	}

	throw std::runtime_error("Unknown media type: " + StaticName::ToString(t));
}

bool Match(Type t, const std::string& ext)
{
	const std::string lowerExt = boost::to_lower_copy(ext);
	const char* extList = Exts(t);
	return std::strstr(extList, lowerExt.c_str()) != NULL;
}

Type ResolveByName(const std::string& name)
{
	std::string::size_type pos = name.find_last_of(".");
	if(pos >= name.size() - 1) return unknown;
	return Resolve(name.substr(pos + 1));
}

Type Resolve(const std::string& ext)
{
	if(Match(video, ext)) return video;
	if(Match(audio, ext)) return audio;
	if(Match(archive, ext)) return archive;
	if(Match(msapp, ext)) return msapp;
	if(Match(cdimage, ext)) return cdimage;
	if(Match(document, ext)) return document;
	if(Match(picture, ext)) return picture;

	return unknown;
}

} //namespace Media

