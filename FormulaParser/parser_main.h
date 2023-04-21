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

namespace VarFunc{
	std::unordered_set<std::string> variables;
	std::unordered_map<std::string,std::string> variables_sub;
	std::unordered_map<std::string,std::string> functions;
};

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

//FormulaBlock functions
class FormulaBlock;
std::vector<FormulaBlock*> parse_elementary_operations(const std::string &formula_string);

//FormulaBlock class
class FormulaBlock{
	FormulaBlock(char type_, char mod_ = 'n'): type(type_), mod(mod_) {};

	FormulaBlock(const std::string &formula_string, char type_): type(type_), mod('n') {
		//Checking if sting is empty
		int indx = formula_string.size()-1;
		for(; indx >= 0; --indx){
			if(formula_string[indx] != ' ')
				break;
		}

		if(indx == -1){
			std::cout << "Caught empty expression" << std::endl;
			throw("Invalid expression");
			return;
		}

		//Detecting functions
		for(indx = 0; indx < formula_string.size(); ++indx){
			if(formula_string[indx] == '[') break;
		}

		if(indx != formula_string.size() && VarFunc::functions.count(formula_string.substr(0, indx))){
			int len = group_bracket_block(formula_string, indx, '[', ']');

			FormulaBlock *bracket = new FormulaBlock(formula_string.substr(indx+1, len-2));
			subblocks.push_back(bracket);
			type = 'f';
			content = VarFunc::functions[formula_string.substr(0, indx)];

			return;
		}

		content = formula_string;
		content.erase(std::remove(content.begin(), content.end(), ' '), content.end());

		if(is_double(content)){
			if(is_double(content))
				type = 'i';
			else
				type = 'd';
		}
		else if(VarFunc::variables.count(content)){
			type = 'v';
		}
		else if(VarFunc::variables_sub.count(content)){
			type = 'v';
			content = VarFunc::variables_sub[content];
		}
	}

	FormulaBlock(std::vector<FormulaBlock*> block_series): mod('n') {
		//Clunky, might consider revising
		if(block_series.size() == 1){
			type = block_series[0]->type;
			mod = block_series[0]->mod;
			content = block_series[0]->content;
			subblocks = block_series[0]->subblocks;
			(block_series[0]->subblocks).clear();
			delete block_series[0];
			return;
		}

		int add_num = 0, mult_num = 0;
		for(int i = 1; i < block_series.size(); ++i){
			if(block_series[i]->mod == 'm'){
				if(block_series[i]->type == '+' || block_series[i]->type == '-') ++add_num;
				if(block_series[i]->type == '*' || block_series[i]->type == '/') ++mult_num;
			}
		}

		if(add_num){
			type = '+';
			int indx = 0;
			for(int bl = 0; bl <= add_num; ++bl){
				std::vector<FormulaBlock*> collection;
				char sign = '+';
				if(block_series[indx]->mod == 'm'){
					if(block_series[indx]->type == '-')
						sign = '-';
					else if(block_series[indx]->type != '+'){
						std::cout << "Wrong elementary operation encountered when parsing summation" << std::endl;
						throw("Parsing error");
					}
					delete block_series[indx++];
				}

				while(indx < block_series.size() && 
						(block_series[indx]->mod != 'm' || (block_series[indx]->type != '+' && block_series[indx]->type != '-')))
				{
					collection.push_back(block_series[indx++]);
				}
				if(collection.size() == 1){
					collection[0]->mod = sign;
					subblocks.push_back(collection[0]);
				}
				else{
					FormulaBlock *new_subblock = new FormulaBlock(collection);
					new_subblock->mod = sign;
					subblocks.push_back(new_subblock);
				}
			}
			return;
		}

		if(block_series[0]->mod == 'm' && block_series[0]->type == '-'){
			type = '-';
			block_series.erase(block_series.begin());
			subblocks.push_back(new FormulaBlock(block_series));
			return;
		}

		if(mult_num){
			type = '*';
			int indx = 0;
			char mult_sign = '*';
			for(int bl = 0; bl <= mult_num; ++bl){
				std::vector<FormulaBlock*> collection;
				while(indx < block_series.size() && (block_series[indx]->mod != 'm' || (block_series[indx]->type != '*' && block_series[indx]->type != '/')))
				{
					collection.push_back(block_series[indx++]);
				}

				if(collection.size() == 1){
					collection[0]->mod = mult_sign;
					subblocks.push_back(collection[0]);
				}
				else{
					FormulaBlock *new_subblock = new FormulaBlock(collection);
					new_subblock->mod = mult_sign;
					subblocks.push_back(new_subblock);
				}
				if(indx < block_series.size()){
					mult_sign = block_series[indx]->type;
					delete block_series[indx++];
				}
			}
			return;
		}

		if(block_series.size() == 3 && block_series[1]->mod == 'm' && block_series[1]->type == '^'){
			subblocks.push_back(block_series[0]);
			subblocks.push_back(block_series[2]);
			type = '^';
			return;
		}

		std::cout << "Elementary operation of unknown type encountered parsing elementary operations" << std::endl;
		std::cout << block_series.size() << " blocks received" << std::endl;

		int i = 0;
		while(1){
			for(int i = 0; i < 50; ++i)
				std::cout << '-';
			std::cout << std::endl;
			
			if(i >= block_series.size()) break;

			std::cout << *block_series[i++];
		}

		throw("Parsing error");
	}

public:
	char type;
	char mod;
	std::vector<FormulaBlock*> subblocks;
	std::string content;

	FormulaBlock(): type('e'), mod('n') {};
	FormulaBlock(const std::string &formula_string): type('e'), mod('n') {
		//Checking if sting is empty
		int indx = formula_string.size()-1;
		for(; indx >= 0; --indx){
			if(formula_string[indx] != ' ')
				break;
		}

		if(indx == -1){
			std::cout << "Caught empty expression" << std::endl;
			throw("Invalid expression");
			return;
		}

		//Parsing line for elementary operations
		std::vector<FormulaBlock*> parsed_line = parse_elementary_operations(formula_string);

		//Clunky, might consider revising
		FormulaBlock *new_subblock = new FormulaBlock(parsed_line);
		type = new_subblock->type;
		mod = new_subblock->mod;
		content = new_subblock->content;
		subblocks = new_subblock->subblocks;
		(new_subblock->subblocks).clear();
		delete new_subblock;

		return;
	}
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
			os << "Contents: " << '|' << content << '|' << '\n';
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

	friend std::vector<FormulaBlock*> parse_elementary_operations(const std::string &formula_string);
	friend std::ostream& operator<<(std::ostream& os, const FormulaBlock& fbl);
};

std::ostream& operator<<(std::ostream& os, const FormulaBlock& fbl){
	fbl.print_block(os, 0);
    return os;
}


std::vector<FormulaBlock*> parse_elementary_operations(const std::string &formula_string){
	std::vector<FormulaBlock*> parsed_line;
	
	//Detecting side spaces
	int i, iend;
	for(i = 0; i < formula_string.size(); ++i){
		if(formula_string[i] != ' ')
			break;
	}

	if(i == formula_string.size())
		return parsed_line;

	for(iend = formula_string.size()-1; iend > i; --iend){
		if(formula_string[iend] != ' ')
			break;
	}
	++iend;

	std::unordered_set<char> skip_characters = {' ', '\n'};
	std::unordered_set<char> elementary_operations = {'+', '-', '*', '/', '^'};

	int block_start = i;
	while(i < iend){
		//Detecting () bracketed blocks
		if(formula_string[i] == '('){
			if(i != block_start){
				parsed_line.push_back(new FormulaBlock(formula_string.substr(block_start, i-block_start), 'e'));
			}

			int len = group_bracket_block(formula_string, i, '(', ')');
			
			FormulaBlock *bracket = new FormulaBlock(formula_string.substr(i+1, len-2));
			// bracket->content = "bracket";
			parsed_line.push_back(bracket);
			
			i += len;
			block_start = i;
		}
		//Skipping [] bracketed blocks
		else if(formula_string[i] == '['){
			int len = group_bracket_block(formula_string, i, '[', ']');
			i += len;
		}

		//Detecting elementary operations signs
		else if(elementary_operations.count(formula_string[i])){
			if(i != block_start){
				parsed_line.push_back(new FormulaBlock(formula_string.substr(block_start, i-block_start), 'e'));
			}
			parsed_line.push_back(new FormulaBlock(formula_string[i], 'm'));

			++i;
			block_start = i;
		}

		//Detecting skip characters
		else if(skip_characters.count(formula_string[i])){
			if(i != block_start){
				parsed_line.push_back(new FormulaBlock(formula_string.substr(block_start, i-block_start), 'e'));
			}

			while(i < iend && skip_characters.count(formula_string[i]))
				++i;

			block_start = i;
		}
		else
			++i;
	}

	//Treating last block
	if(block_start != iend)
		parsed_line.push_back(new FormulaBlock(formula_string.substr(block_start, formula_string.size()-block_start), 'e'));

	//Inferring multiplication signs
	std::vector<FormulaBlock*> parsed_line_mod = {parsed_line[0]};

	for(int i = 1; i < parsed_line.size(); ++i){
		bool is_el_operation1 = (parsed_line[i-1]->mod == 'm') && elementary_operations.count(parsed_line[i-1]->type);
		bool is_el_operation2 = (parsed_line[i]->mod == 'm') && elementary_operations.count(parsed_line[i]->type);

		if(!is_el_operation1 && !is_el_operation2)
			parsed_line_mod.push_back(new FormulaBlock('*', 'm'));

		parsed_line_mod.push_back(parsed_line[i]);
	}

	return parsed_line_mod;
}

#endif