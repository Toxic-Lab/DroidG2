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

#ifndef DISCOVERY_HPP
#define DISCOVERY_HPP

#include "io.hpp"
#include <map>
#include <string>
#include "url.hpp"
#include <boost/enable_shared_from_this.hpp>

class Discovery: public boost::enable_shared_from_this<Discovery>
{
public:
	Discovery(ba::io_service&);
	void Start();
	void Shutdown();

private:
	struct WebCache
	{
		WebCache(): timestamp(0), banned(false) {}
		std::string address;
		time_t timestamp;
		bool banned;
	};
	enum {RESPONSE_MAX_SIZE = 512 * 1024};
	enum {REST_TIME = 60};
	typedef std::map<std::string, WebCache> Caches;

	void Load();
	void Save();
	void LoadDefaults();
	void Stop();
	void ScheduleRun();
	void OnResolved(const bs::error_code& err, ba::ip::tcp::resolver::iterator iEndpoint);
	void OnConnected(const bs::error_code& err, ba::ip::tcp::resolver::iterator iEndpoint);
	void OnMessageSent(const bs::error_code& err);
	void OnResponse(const bs::error_code& err, std::size_t);
	void OnFile(const bs::error_code& err);
	void OnFileReceived();
	void AddCache(const std::string& address);
	void AddCache(const WebCache& gwc);
	void Run(const bs::error_code&);
	void Timeout(const bs::error_code& err);
	void SessionFailed();
	void OnData(const bs::error_code&, std::size_t);
	void HandleMessage();
	bool SelectGwc();

	ba::ip::tcp::socket socket_;
	ba::ip::tcp::resolver resolver_;
	Caches caches_;
	std::string address_;
	ba::streambuf message_;
	ba::deadline_timer timer_;
	bool closed_;
};

#endif //DISCOVERY_HPP
