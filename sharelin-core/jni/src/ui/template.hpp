/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2009 Andrey Stroganov <savthe@gmail.com>

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

#ifndef TEMPLATE_HPP
#define TEMPLATE_HPP

#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>
#include <ostream>
#include <cassert>

#include "../types.hpp"
	
namespace Ui {

class Template 
{
public:
	Template() {};
	Template(const std::string& fname);
	bool Load(const std::string& fname);
	void Format(std::string& out) const;
	void ClearValues();
	void Print() const;

	template <typename T>
	void Put(const std::string& key, const T& value)
	{
		std::stringstream tmp;
		tmp << value;
		values_[key].push_back(tmp.str());
	}

	void PutLine(const std::string& s) { lines_.push_back(s); }
	
private:
	typedef std::vector <std::string> Array;
	typedef std::map <std::string, Array> Values;
	typedef std::map <std::string, std::string> Frame;

	std::vector<std::string> lines_;
	Values values_;
	std::string name_;
	mutable std::string buffer_;
	mutable Frame frame_;

	template <typename T>
	bool MakeFrame(uint index, T begin, T end) const
	{
		for(T i = begin; i != end; ++i)
		{
			Values::const_iterator ival = values_.find(*i);
			if(ival == values_.end() || index >= ival->second.size())
			       return false;	
		}

		for(T i = begin; i != end; ++i)
		{
			Values::const_iterator ival = values_.find(*i);
			assert(ival != values_.end());
			assert(index < ival->second.size());
			frame_[*i] = ival->second[index];
		}
		return true;
	}

	class Error: public std::runtime_error
	{
		public:
		Error(const std::string& err): std::runtime_error(err) {}	
		Error(const std::string& msg, const std::string& line): std::runtime_error(msg + ": " + line) {}
	};
	
	enum {MAX_DEPTH = 5};
	enum {MAX_SIZE = 100 * 1024};

	void Run(uint, uint) const;
	void LoadChain(const std::string& name, int depth = 0);
	std::string EvaluateLine(const std::string& str) const;
	bool Evaluate(const std::string&) const;
	std::string GetValue(const std::string&) const;
	void OnDef(uint) const;
	uint OnIf(uint) const;
	uint OnForeach(uint) const;
};

} //namespace Ui

template <typename Elem, typename Traits > 
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& os, const Ui::Template& tpl)
{
	std::string result;
	tpl.Format(result);
	os << result;
	return os;
}	

#endif //TEMPLATE_HPP
