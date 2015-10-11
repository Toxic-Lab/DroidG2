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

#ifndef UPDATESWATCH_HPP
#define UPDATESWATCH_HPP

#include "io.hpp"
#include <ctime>

class UpdatesWatch: public boost::enable_shared_from_this<UpdatesWatch>
{
public:
	UpdatesWatch(ba::io_service&);
	void Start();
	void Shutdown();

	uint Birthday() const { return birthday_; }
	std::string Tag() const { return tag_; }
	std::string Link() const { return link_; }
	std::string Text() const { return text_; }
	bool NewVersion() const;
private:
	void Run(const bs::error_code& err);
	void ScheduleRun();
	void Timeout(const bs::error_code& err);
	void OnResolved(const bs::error_code& err, ba::ip::tcp::resolver::iterator iep);
	void OnConnected(const bs::error_code& err, ba::ip::tcp::resolver::iterator iep);
	void OnWrite(const bs::error_code& err);
	void OnReply(const bs::error_code& err, std::size_t nbytes);

	ba::ip::tcp::socket socket_;
	ba::ip::tcp::resolver resolver_;
	ba::deadline_timer timer_;
	ba::streambuf buffer_;
	bool closed_;
	uint birthday_;
	std::string tag_;
	std::string link_;
	std::string text_;
	
};

#endif //UPDATESWATCH_HPP

