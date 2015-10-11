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

#ifndef G2_MISC_HPP
#define G2_MISC_HPP

#include "g2packet.hpp"
#include "hashes.hpp"
#include "searchdefs.hpp"

namespace G2 {

void ExtractUrn(G2::Reader r, Hashes::SHA1& sha1, Hashes::TTR& ttr, Hashes::MD5& md5, Hashes::ED2K& ed2k);

} //namespace G2

#endif //G2_MISC_HPP

