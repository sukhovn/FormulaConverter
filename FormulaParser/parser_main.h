#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <sstream>


#ifndef PARSER_MAIN_H
#define PARSER_MAIN_H

// Types
// e - unspecified expression
// i - integer
// d - double
// f - known function
// v - known variable
// + - summation
// * - multiplication
// ^ - power
// - - minus sign in front of a block

namespace ParsingRoutines{
	std::string remove_side_spaces(const std::string &str){
		int ist, iend;
		for(ist = 0; ist < str.size(); ++ist){
			if(str[ist] != ' ')
				break;
		}

		if(ist == str.size())
			return "";

		for(iend = str.size()-1; iend > ist; --iend){
			if(str[iend] != ' ')
				break;
		}

		return str.substr(ist, iend-ist+1);
	}

	int group_bracket_block(const std::string &str, int ist, char st_bracket, char end_bracket){
		if(str[ist] != st_bracket)
			return 0;

		int len = 1;
		int open_brackets = 1;
		for(int i = ist+1; i < str.size(); ++i, ++len){
			if(str[i] == st_bracket) ++open_brackets;
			if(str[i] == end_bracket) --open_brackets;

			if(open_brackets == 0)
				return ++len;
		}

		std::cout << "Ending bracket \'" << end_bracket << "\' not found" << std::endl;
		throw("Invalid expression");
	}

	bool is_integer(const std::string& s){
	    return !s.empty() && std::find_if(s.begin(), s.end(),
					[](unsigned char c) {return !std::isdigit(c) && !(c == '-');}) == s.end();
	}

	bool is_double(const std::string& s){
	    return !s.empty() && std::find_if(s.begin(), s.end(),
					[](unsigned char c) {return !std::isdigit(c) && !(c == '-') && !(c == '.');}) == s.end();
	}
}


//FormulaBlock class
class FormulaBlock{
	char type;
	char mod;
	std::vector<FormulaBlock*> subblocks;
	std::string content;

	FormulaBlock(char type_, char mod_ = 'n'): type(type_), mod(mod_) {};
public:

	FormulaBlock(): type('e'), mod('n') {};

	~FormulaBlock(){
		for(int i = 0; i < subblocks.size(); ++i)
			delete subblocks[i];
	}

	void print_block(std::ostream& os, int tab) const{
		for(int i = 0; i < tab; ++i) os << '\t';
		os << "Block type: " << type;
		if(mod != 'n')
			os << ", block mod: " << mod;
		os << '\n';
		
		if(!content.empty()){
			for(int i = 0; i < tab; ++i) os << '\t';
			os << "Contents: " << content << '\n';
		}

		if(subblocks.size() > 0){
			for(int i = 0; i < tab; ++i) os << '\t';
			os << "Subblocks:" << '\n';

			for(int i = 0; i < subblocks.size(); ++i){
				if(i) os << std::endl;
				subblocks[i]->print_block(os, tab+1);
			}
		}
	}

	std::string c_output(){
		if(subblocks.empty()){
			if(type == 'e'){
				std::cout << "Warning! Using unspecified expression: " << content << std::endl;
			}
			return content;
		}
		else if(type == '+'){
			std::ostringstream result;
			for(int i = 0; i < subblocks.size(); ++i){
				if(i == 0){
					if(subblocks[i]->mod == '-')
						result << '-'; 
				}
				else{
					result << ' ' << subblocks[i]->mod << ' ';
				}
				if(subblocks[i]->type == '+')
					result << '(' << subblocks[i]->c_output() << ')';
				else
					result << subblocks[i]->c_output();
			}
			return result.str();
		}
		else if(type == '*'){
			std::ostringstream result;
			for(int i = 0; i < subblocks.size(); ++i){
				if(i){
					result << subblocks[i]->mod;
				}
				if(subblocks[i]->type == '+' || subblocks[i]->type == '*')
					result << '(' << subblocks[i]->c_output() << ')';
				else
					result << subblocks[i]->c_output();
			}
			return result.str();
		}
		else if(type == '^'){
			if((subblocks[0]->subblocks).empty() && (subblocks[0]->content == "E"))
				return "std::exp(" + subblocks[1]->c_output() + ')';
			else
				return "std::pow(" + subblocks[0]->c_output() + ", " + subblocks[1]->c_output() + ')';
		}
		else if(type == '-'){
			if(subblocks[0]->type == '+' || subblocks[0]->type == '-')
				return "-(" + subblocks[0]->c_output() + ')';
			else
				return '-' + subblocks[0]->c_output();
		}
		else if(type == 'f'){
			std::unordered_map<std::string, std::string> c_functions = {{"sin", "std::sin"}, {"cos", "std::cos"}, {"tan", "std::tan"}};
			if(c_functions.count(content)){
				return c_functions[content] + '(' + subblocks[0]->c_output() + ')';
			}
			else
				return content + '(' + subblocks[0]->c_output() + ')';
		}

		std::cout << "Invalid construction encountered in c_output" << std::endl;
		throw("Error");
	}

	friend class Parser_Mathematica;
	friend std::vector<FormulaBlock*> parse_elementary_operations(const std::string &formula_string);
	friend std::ostream& operator<<(std::ostream& os, const FormulaBlock& fbl);
};

std::ostream& operator<<(std::ostream& os, const FormulaBlock& fbl){
	fbl.print_block(os, 0);
    return os;
}

#endif