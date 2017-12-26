/*
 * main.cpp
 * 
 * (c) 2017 Max R. P. Grossmann <m@max.pm>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include <iostream>
#include <fstream>
#include <gmpxx.h>
#include <string>
#include <vector>
#include <cmath>
#include <csignal>
#include <cassert>

class MG_NUM {
	public:
		MG_NUM();
		MG_NUM(const std::string&);
		MG_NUM operator+(MG_NUM);
		MG_NUM operator-(MG_NUM);
		MG_NUM operator*(MG_NUM);
		MG_NUM operator/(MG_NUM);
		MG_NUM pow(MG_NUM);
		MG_NUM factorial();
		template <class Stream> void divide(unsigned long long, Stream&);
		mpz_class a, b;
};

MG_NUM::MG_NUM() {
	a = 0;
	b = 1;
}

MG_NUM::MG_NUM(const std::string& a_str) {
	a = 0;
	b = 1;

	bool nonint = false;
	
	for (auto& ch: a_str) {
		if (ch != '.') {
			a += ch-'0';
			a *= 10;
		}
		else {
			nonint = true;
		}
		
		if (nonint) b *= 10;
	}
	
	a /= 10;
	if (nonint) b /= 10;
}

MG_NUM MG_NUM::operator+(MG_NUM rhs) {
	MG_NUM res;
	
	if (b == rhs.b) {
		res.a = a + rhs.a;
		res.b = b;
	}
	else {
		res.a = a*rhs.b + rhs.a*b;
		res.b = b*rhs.b;
	}
	
	return res;
}

MG_NUM MG_NUM::operator-(MG_NUM rhs) {
	MG_NUM res;
	
	if (b == rhs.b) {
		res.a = a - rhs.a;
		res.b = b;
	}
	else {
		res.a = a*rhs.b - rhs.a*b;
		res.b = b*rhs.b;
	}
	
	return res;
}

MG_NUM MG_NUM::operator*(MG_NUM rhs) {
	MG_NUM res;
	
	if (b == 1 && rhs.b == 1) {
		res.a = a * rhs.a;
	}
	else {
		res.a = a*rhs.a;
		res.b = b*rhs.b;
	}
	
	return res;
}

MG_NUM MG_NUM::operator/(MG_NUM rhs) {
	MG_NUM res;
	
	if (b == rhs.b) {
		res.a = a;
		res.b = rhs.a;
	}
	else {
		res.a = a*rhs.b;
		res.b = b*rhs.a;
	}
	
	return res;
}

MG_NUM MG_NUM::pow(MG_NUM rhs) { // todo -- better algo?
	assert(rhs.b == 1 || rhs.a%rhs.b == 0);
	
	MG_NUM res;
	bool negative = (rhs.a < 0) xor (rhs.b < 0);
	
	if (negative) {
		rhs.a = (rhs.a < 0) ? -rhs.a : rhs.a;
		rhs.b = (rhs.b < 0) ? -rhs.b : rhs.b;
	}
	
	rhs.a /= rhs.b;
	res.a = 1;
	res.b = 1;
	
	while (rhs.a > 0) {
		res.a *= a;
		res.b *= b;
		
		rhs.a--;
	}
	
	if (negative) {
		auto b2 = res.a;
		res.a = res.b;
		res.b = b2;
	}
	
	return res;
}

MG_NUM MG_NUM::factorial() {
	assert((b == 1 || a%b == 0) && !((a < 0) xor (b < 0)));
	
	MG_NUM res;
	
	res.a = a/b;
	res.b = 1;
	
	for (mpz_class i = res.a-1; i >= 2; i--) {
		res.a *= i;
	}
	
	return res;
}

template <class Stream>
void MG_NUM::divide(unsigned long long n, Stream& output) {
	assert(b != 0);
	
	mpz_class d = a/b, a2 = a-b*d;
	
	output << d << '.';
	
	while (n > 0) {
		a2 *= 10;
		d = a2/b;
		
		output << d;
		
		a2 -= b*d;
		n--;
	}
}

class MG_EXPR {
	public:
		MG_EXPR() {
			mode = 255;
		}
		MG_EXPR(const MG_NUM& lnum) {
			num = lnum;
			mode = 1;
		}
		MG_EXPR(const std::string& str) {
			expr = str;
			mode = 2;
		}
		MG_EXPR(const std::string& str, bool brace) {
			expr = str;
			requires_parsing = brace;
			mode = 2;
		}
		
		unsigned mode;		
		MG_NUM num;
		std::string expr;
		bool requires_parsing = false;
};

MG_NUM execute(const std::vector<MG_EXPR>&);

inline bool is_num(char c) {
	return (c >= '0' && c <= '9') || c == '.';
}

inline bool is_op(char c) {
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^' || c == '!';
}

inline bool is_space(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

inline bool is_opening_brace(char c) {
	return c == '(' || c == '[' || c == '{';
}

inline bool is_closing_brace(char c) {
	return c == ')' || c == ']' || c == '}';
}

std::vector<MG_EXPR> parse(const std::string& str) {
	std::vector<MG_EXPR> r;
	std::string tmp;
	
	bool lnum = true, bracemode = false;
	unsigned bracecnt = 0;
	
	for (auto& ch: str) {
		if (bracemode) {
			if (!is_closing_brace(ch)) {
				tmp.push_back(ch);
			}
			else {
				if (bracecnt == 0) {
					bracemode = false;
					
					if (tmp.size() > 0) r.push_back(MG_EXPR(tmp,true));
					tmp = "";
				}
				else {
					bracecnt--;
					tmp.push_back(ch);
				}
			}
			
			if (is_opening_brace(ch)) bracecnt++;
		}
		else if (is_opening_brace(ch)) {
			if (tmp.size() > 0) r.push_back(MG_EXPR(tmp));
			tmp = "";

			bracemode = true;
		}
		else {		
			if ((is_num(ch) && !lnum) || (is_op(ch) && lnum)) {
				if (tmp.size() > 0) r.push_back(MG_EXPR(tmp));
				tmp = "";
				
				lnum = is_num(ch);
			}
			
			if (!is_space(ch)) tmp.push_back(ch);
		}
		
		assert(is_num(ch) || is_op(ch) || is_space(ch) || is_opening_brace(ch) || is_closing_brace(ch));
	}
	
	if (tmp.size() > 0) r.push_back(MG_EXPR(tmp));
	
	return r;
}

MG_NUM primitive(const MG_EXPR& a) {
	if (a.mode == 1) {
		return a.num;
	}
	else {
		if (a.requires_parsing) {
			return execute(parse(a.expr));
		}
		else {
			return MG_NUM(a.expr);
		}
	}
}

MG_NUM resolve(const MG_EXPR& op, const MG_EXPR& a, const MG_EXPR& b) {
	assert(op.mode == 2);
	
	if (op.expr == "+") {
		return primitive(a) + primitive(b);
	}
	else if (op.expr == "-") {
		return primitive(a) - primitive(b);
	}
	else if (op.expr == "*") {
		return primitive(a) * primitive(b);
	}
	else if (op.expr == "/") {
		return primitive(a) / primitive(b);
	}
	else if (op.expr == "^") {
		return primitive(a).pow(primitive(b));
	}
	else if (op.expr == "!") {
		return primitive(a).factorial();
	}
	else {
		std::cerr << "invalid op '" << op.expr << "' [" << op.mode << ", '" << a.expr << "', '" << b.expr << "']" << std::endl;
		abort();
	}
}

MG_NUM execute(const std::vector<MG_EXPR>& stack) {
	assert(stack.size()%2 == 1);
	
	if (stack.size() == 1) {
		return primitive(stack[0]);
	}
	else {
		std::vector<MG_EXPR> nstack;
		
		nstack.resize(stack.size()-2);
		
		nstack[0] = resolve(stack[1], stack[0], stack[2]);
		
		for (size_t i = 3; i < stack.size(); i++) {
			nstack[i-2] = stack[i];
		}
		
		return execute(nstack);
	}
}

template <class Callback>
void explode_callback(const std::string& in, const std::string& delimiter, Callback F) {
	size_t pos = 0, cp, dl = delimiter.length();

	while ((cp = in.find(delimiter,pos)) != std::string::npos) {
		F(in.substr(pos,cp-pos));
		pos = cp+dl;
	}

	F(in.substr(pos,in.length()-pos));
}

std::vector<std::string> explode(const std::string& in, const std::string& delimiter) {
	std::vector<std::string> r;

	auto callback = [&r](const std::string& a)->void{r.push_back(a);};

	explode_callback(in,delimiter,callback);

	return r;
}

int main(int argc, char** argv) {
	std::ios::sync_with_stdio(false);
	
	std::string line;
	std::vector<MG_EXPR> stack;
	MG_NUM last_result;
	
	bool echo = true;
	std::ofstream file;
	
	while (std::getline(std::cin, line)) {
		if (line.size() > 0) {
			if (line[0] != '$') {
				stack = parse(line);
				
				last_result = execute(stack);
				
				if (echo) {
					(file.is_open() ? file : std::cout) << last_result.a << '/' << last_result.b << std::endl;
				}
			}
			else {
				auto command = explode(line, " ");
				
				if (command[0] == "$file") {
					if (command.size() == 2) {
						file.open(command[1]);
					}
					else {
						file.close();
					}
				}
				else if (command[0] == "$div") {
					unsigned long long n = 5;
					
					if (command.size() == 2) {
						n = stoull(command[1]);
					}
					
					last_result.divide(n, file.is_open() ? file : std::cout);
					(file.is_open() ? file : std::cout) << std::endl;
				}
				else if (command[0] == "$echo") {
					if (command.size() == 2) {
						echo = false;
					}
					else {
						echo = true;
					}
				}
			}
		}
	}
}
