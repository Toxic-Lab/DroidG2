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

#include "template.hpp"
#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <iostream>

using boost::istarts_with;

namespace {

uint Skip(const std::string& s, uint i, char c)
{
	while(i < s.size() && s[i] == c) ++i;
	return i;
}

uint SkipNot(const std::string& s, uint i, char c)
{
	while(i < s.size() && s[i] != c) ++i;
	return i;
}

} //namespace 

namespace Ui {

Template::Template(const std::string& fname)
{
	Load(fname);
}

void Template::LoadChain(const std::string& fname, int depth)
{
	std::string root;
	if(fname.find('/') != std::string::npos)
		root = fname.substr(0, fname.find_last_of('/'));

	if(depth > MAX_DEPTH) 
		throw Error("Too deep");
	
	std::ifstream f(fname.c_str());
	if(!f.good()) 
		throw Error("Template not found: " + fname);
	
	std::string line;
	while(std::getline(f, line)) 
	{
		boost::trim(line);
		if(boost::istarts_with(line, "#include ")) 
			LoadChain(root + "/" + boost::trim_copy(line.substr(line.find(' '))), depth + 1);
		else 
			PutLine(line);
	}
}

void Template::Print() const
{
	for(Values::const_iterator i = values_.begin(); i != values_.end(); ++i)
	{
		std::cout << i->first << " (" << i->second.size() << ") : ";
		for(unsigned int ii = 0; ii < i->second.size(); ++ii)
			std::cout << i->second[ii] << " ";
		std::cout << std::endl;
	}
}

bool Template::Load(const std::string& fname)
{
	try
	{
		LoadChain(fname);
		name_ = fname;
	}
	catch(Error& err)
	{
		std::cout << "FIXME: template: " << err.what() << std::endl;
		lines_.push_back("Failed to load template: " + fname);
		return false;
	}
	return true;
}

void Template::ClearValues()
{
	values_.clear();
}

std::string Template::EvaluateLine(const std::string& str) const
{

	std::string out;
	std::string::size_type offset = 0;
	while(offset < str.size()) 
	{
		std::string::size_type pos = str.find("$", offset);
		
		if(pos == std::string::npos || pos + 1 == str.size()) 
		{
			out += str.substr(offset);
			break;
		}
		
		if(pos > offset) 
			out += str.substr(offset, pos - offset);
		
		//go to the end of var
		uint i;
		for(i = pos + 1; i < str.size() && ((i == pos + 1 && str[i] == '$') || str[i] == '_' || std::isalnum(str[i])); ++i);
		
		std::string name = str.substr(pos + 1, i - pos - 1);
		
		if(i < str.size() && str[i] == ' ') ++i; //if we are on space, let's eat one
		
		offset = i;
		
		//substituting
		out += GetValue(name);
	}
	return out;
}

std::string Template::GetValue(const std::string& key) const
{
	if(key.empty() || key == "$") return "$";

	Frame::const_iterator i = frame_.find(key);
	return i == frame_.end() ? "0" : i->second;
}

void Template::Format(std::string& out) const
{
	if(lines_.empty()) return;

	for(Values::const_iterator i = values_.begin(); i != values_.end(); ++i)
		frame_[i->first] = i->second.at(0);


	try
	{
		Run(0, lines_.size() - 1);
		out = buffer_;
	}
	catch(std::exception& e)
	{
		out = "Template " + name_ + " error: " + e.what();
	}

	buffer_.clear();
	frame_.clear();
}


void Template::OnDef(uint n) const
{
	assert(n < lines_.size());
	const std::string& s = lines_[n];
	
	uint i = SkipNot(s, 0, ' ');
	i = Skip(s, i, ' ');
	if(i == s.size()) throw Error("#def is empty", s);

	const uint begin = i;
	i = SkipNot(s, i, ' ');
	std::string var = s.substr(begin, i - begin);

	i = Skip(s, i, ' ');
	std::string val;
	if(i < s.size()) val = s.substr(i, s.size() - i);

	frame_[var] = EvaluateLine(val);
}

uint Template::OnIf(uint n) const
{
	assert(n < lines_.size());
	
	uint nif = n;
	uint nelse = 0;
	uint nend = 0;

	int bal = 0;
	do
	{
		if(istarts_with(lines_[n], "#if ")) ++bal;
		else if(istarts_with(lines_[n], "#endif"))
		{
			--bal;
			if(bal == 0) break;
		}
		else if(istarts_with(lines_[n], "#else") && bal == 1)
		{
			if(nelse) throw Error("Bad if", lines_[n]);
			nelse = n;
		}
		++n;
	} while(n < lines_.size());
	if(n == lines_.size()) throw Error("If without endif", lines_[nif]);
	nend = n;
	if(nelse == 0) nelse = nend;

	const std::string& s = lines_[nif];
	uint i = SkipNot(s, 0, ' ');
	i = Skip(s, i, ' ');
	if(i == s.size()) throw Error("Empty if", lines_[nif]);

	const bool result = Evaluate(s.substr(i));

	if(result) Run(nif + 1, nelse - 1);
	else if(nelse + 1 < nend) Run(nelse + 1, nend - 1);

	return nend;
}

bool Template::Evaluate(const std::string& expr) const
{
	//a = b
	//a != b
	//a <=> a != 0

	std::string val1;
	std::string val2;
	std::string op;

	uint i = 0;
	if(i >= expr.size()) throw Error("Empty expression", expr);
	while(i < expr.size() && (std::isalnum(expr[i]) || expr[i] == '_' || expr[i] == '$')) val1.push_back(expr[i++]);
	i = Skip(expr, i, ' ');
	if(i == expr.size())
	{
		op = "!=";
		val2 = "0";
	}
	else
	{
		while(i < expr.size() && (expr[i] == '=' || expr[i] == '!')) op.push_back(expr[i++]);
		i = Skip(expr, i, ' ');
		if(i == expr.size()) throw Error("Right value operand missing", expr);
		val2 = expr.substr(i, expr.size() - i);
	}

	if(val1.empty() || op.empty() || val2.empty()) throw Error("Bad expression", expr);

	if(val1.at(0) == '$') val1 = GetValue(val1.substr(1));
	if(val2.at(0) == '$') val2 = GetValue(val2.substr(1));

	if(op == "=") return val1 == val2;
	else if(op == "!=") return val1 != val2;
	else throw Error("Unknown operator", expr);
}


uint Template::OnForeach(uint n) const
{
	uint nfor = n;
	uint nnext = n;
	while(nnext < lines_.size() && !istarts_with(lines_[nnext], "#next")) ++nnext;
	if(nnext == lines_.size()) throw Error("#foreach without next", lines_[nfor]);
	if(nfor + 1 >= nnext) throw Error("#foreach is empty", lines_[nfor]);

	std::vector<std::string> iters;
	std::stringstream ss(lines_[nfor]);
	std::string tmp;
	ss >> tmp; //skip #for
	while(ss >> tmp) iters.push_back(tmp);
	if(iters.empty()) throw Error("#No iterators in #for", lines_[nfor]);

	Frame lastFrame = frame_;
	uint index = 0;
	while(MakeFrame(index, iters.begin(), iters.end()))
	{
		Run(nfor + 1, nnext - 1);
		++index;
	}

	frame_ = lastFrame;

	return nnext;
}

void Template::Run(uint first, uint last) const
{
	for(uint i = first; i <= last; ++i)
	{
		const std::string& line = lines_.at(i);
		if(line.empty()) continue;

		if(istarts_with(line, "#def")) OnDef(i);
		else if(istarts_with(line, "#if")) i = OnIf(i);
		else if(istarts_with(line, "#foreach")) i = OnForeach(i);
		else 
		{
			buffer_ += EvaluateLine(line);
			buffer_ += "\n";
		}
		if(i > last) throw Error("End of template not found");
	}
}

} //namespace Ui
