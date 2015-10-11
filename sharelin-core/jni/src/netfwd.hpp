/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008-2010  Andrey Stroganov <savthe@gmail.com>

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

#ifndef G2NETDEFS_HPP
#define G2NETDEFS_HPP

#include <boost/shared_ptr.hpp>

class Download;
class Downloader;
class G2Handshake;
class G2Node;
class NetClient;
class Gatekeeper;

typedef boost::shared_ptr<Download> DownloadPtr;
typedef boost::shared_ptr<Downloader> DownloaderPtr;
typedef boost::shared_ptr<NetClient> NetClientPtr;
typedef boost::shared_ptr<G2Node> NodePtr;
typedef boost::shared_ptr<G2Handshake> HandshakePtr;
typedef boost::shared_ptr<Gatekeeper> GatekeeperPtr;

#endif //G2NETDEFS_HPP

