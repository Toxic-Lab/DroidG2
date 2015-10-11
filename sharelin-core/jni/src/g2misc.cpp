/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008-2010 Andrey Stroganov <savthe@gmail.com>

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

#include "g2misc.hpp"

namespace G2 {

void ExtractUrn(G2::Reader r, Hashes::SHA1& sha1, Hashes::TTR& ttr, Hashes::MD5& md5, Hashes::ED2K& ed2k)
{
	if(r.BytesLeft() < 5) return;

	const char* p = r.Data();
	const uint sz = r.BytesLeft();

	if(std::memcmp(p, "sha1\0", 5) == 0 && sz == 5 + Hashes::SHA1::Size())
 		sha1.Assign(p + 5);

	else if(std::memcmp(p, "ttr\0", 4) == 0 && sz == 4 + Hashes::TTR::Size())
 		ttr.Assign(p + 4);

	else if(std::memcmp(p, "tree:tiger/\0", 12) == 0 && sz == 12 + Hashes::TTR::Size())
 		ttr.Assign(p + 12);

	else if(std::memcmp(p, "bp\0", 3) == 0 && sz == 3 + Hashes::BP::Size())
	{
		Hashes::BP bp;
		bp.Assign(p + 3);
		bp.Split(sha1, ttr);
	}

	else if(std::memcmp(p, "md5\0", 4) == 0 && sz == 4 + Hashes::MD5::Size())
 		md5.Assign(p + 4);

	else if(std::memcmp(p, "ed2k\0", 5) == 0 && sz == 5 + Hashes::ED2K::Size())
 		ed2k.Assign(p + 5);
}


} //namespace G2

