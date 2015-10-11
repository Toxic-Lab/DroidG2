/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008  Andrey Stroganov <savthe@gmail.com>

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

#ifndef WEBUISERVER_HPP
#define WEBUISERVER_HPP

#include "io.hpp"
#include <boost/shared_ptr.hpp>
#include <set>
#include <boost/scoped_ptr.hpp>
#include "ui/prefs.hpp"
#include <string>

class WebUIClient;

typedef boost::shared_ptr<WebUIClient> WebUIClientPtr;

class WebUIServer
{
public:
	WebUIServer(ba::io_service& app);
	void Start();
	void Detach(WebUIClientPtr p);
	void UpdateSettings();
	Ui::Prefs& GetPrefs();
	std::string GetStyleRoot() const;

private:
	void Accept(WebUIClientPtr p, const bs::error_code& err);
	void FindStyle();

	std::set<WebUIClientPtr> clients_;
	ba::ip::tcp::acceptor acceptor_;
	Ui::Prefs prefs_;
	std::string styleRoot_;
};

#endif //WEBUISERVER_HPP

