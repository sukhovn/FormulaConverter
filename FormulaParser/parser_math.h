#include <parser_main.h>
#include <unordered_set>
#include <unordered_map>
#include <fstream>

#ifndef PARSER_MATH_H
#define PARSER_MATH_H

class Parser_Mathematica{
	std::unordered_set<char> skip_characters;
	std::unordered_set<char> elementary_operations;

	std::unordered_set<std::string> variables_list;
	std::unordered_map<std::string,std::string> variables_sub_list;
	std::unordered_map<std::string,std::string> functions_list;

	FormulaBlock* create_elementary_block(const std::string &formula_string){
		using namespace ParsingRoutines;

		//Checking if sting is empty
		if(formula_string.size() == 0){
			std::cout << "Caught empty expression" << std::endl;
			throw("Invalid expression");
			return nullptr;
		}

		FormulaBlock *new_block = new FormulaBlock('e', 'n');

		//Detecting functions
		if(formula_string[formula_string.size()-1] == ']'){
			int len = group_bracket_block_backwards(formula_string, formula_string.size()-1, '[', ']');

			std::string function_name = formula_string.substr(0, formula_string.size()-len);
			if(functions_list.count(function_name)){
				new_block->type = 'f';
				new_block->content = functions_list[function_name];

				std::string function_arguments = formula_string.substr(formula_string.size()+1-len, len-2);

				int block_start = 0;
				for(int i = 0; i < function_arguments.size(); ++i){
					if(function_arguments[i] == ','){
						FormulaBlock *bracket = process_elementary_operations(parse_elementary_operations(function_arguments.substr(block_start, i-block_start)));
						new_block->subblocks.push_back(bracket);
						block_start = i+1;
					}
				}

				FormulaBlock *bracket = process_elementary_operations(parse_elementary_operations(function_arguments.substr(block_start, formula_string.size()-block_start)));
				new_block->subblocks.push_back(bracket);

				return new_block;
			}
		}

		new_block->content = formula_string;
		new_block->content.erase(std::remove(new_block->content.begin(), new_block->content.end(), ' '), new_block->content.end());

		if(is_double(new_block->content)){
			if(is_integer(new_block->content))
				new_block->type = 'i';
			else
				new_block->type = 'd';
		}
		else if(variables_list.count(new_block->content)){
			new_block->type = 'v';
		}
		else if(variables_sub_list.count(new_block->content)){
			new_block->type = 'v';
			new_block->content = variables_sub_list[new_block->content];
		}

		return new_block;
	}

	std::vector<FormulaBlock*> parse_elementary_operations(const std::string &formula_string){
		using namespace ParsingRoutines;

		std::vector<FormulaBlock*> parsed_line;
		
		//Detecting side spaces
		int i, iend;
		for(i = 0; i < formula_string.size(); ++i){
			if(formula_string[i] != ' ')
				break;
		}

		if(i == formula_string.size()){
			std::cout << "Caught empty expression" << std::endl;
			throw("Invalid expression");
			return parsed_line;
		}

		for(iend = formula_string.size()-1; iend > i; --iend){
			if(formula_string[iend] != ' ')
				break;
		}
		++iend;

		int block_start = i;
		while(i < iend){
			//Detecting () bracketed blocks
			if(formula_string[i] == '('){
				if(i != block_start){
					parsed_line.push_back(create_elementary_block(formula_string.substr(block_start, i-block_start)));
				}

				int len = group_bracket_block(formula_string, i, '(', ')');
				
				FormulaBlock *bracket = process_elementary_operations(parse_elementary_operations(formula_string.substr(i+1, len-2)));

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
					parsed_line.push_back(create_elementary_block(formula_string.substr(block_start, i-block_start)));
				}
				parsed_line.push_back(new FormulaBlock(formula_string[i], 'm'));

				++i;
				block_start = i;
			}

			//Detecting skip characters
			else if(skip_characters.count(formula_string[i])){
				if(i != block_start){
					parsed_line.push_back(create_elementary_block(formula_string.substr(block_start, i-block_start)));
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
			parsed_line.push_back(create_elementary_block(formula_string.substr(block_start, formula_string.size()-block_start)));

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

	FormulaBlock* process_elementary_operations(std::vector<FormulaBlock*> block_series){
		if(block_series.size() == 1)
			return block_series[0];

		int add_num = 0, mult_num = 0;
		for(int i = 1; i < block_series.size(); ++i){
			if(block_series[i]->mod == 'm'){
				if(block_series[i]->type == '+' || block_series[i]->type == '-') ++add_num;
				if(block_series[i]->type == '*' || block_series[i]->type == '/') ++mult_num;
			}
		}

		if(add_num){
			FormulaBlock *new_block = new FormulaBlock('+','n');
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
					new_block->subblocks.push_back(collection[0]);
				}
				else{
					FormulaBlock *new_subblock = process_elementary_operations(collection);
					new_subblock->mod = sign;
					new_block->subblocks.push_back(new_subblock);
				}
			}
			return new_block;
		}

		if(block_series[0]->mod == 'm' && block_series[0]->type == '-'){
			delete block_series[0];
			block_series.erase(block_series.begin());

			FormulaBlock *new_block = new FormulaBlock('-','n');
			new_block->subblocks.push_back(process_elementary_operations(block_series));
			return new_block;
		}

		if(mult_num){
			FormulaBlock *new_block = new FormulaBlock('*','n');
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
					new_block->subblocks.push_back(collection[0]);
				}
				else{
					FormulaBlock *new_subblock = process_elementary_operations(collection);
					new_subblock->mod = mult_sign;
					new_block->subblocks.push_back(new_subblock);
				}
				if(indx < block_series.size()){
					mult_sign = block_series[indx]->type;
					delete block_series[indx++];
				}
			}
			return new_block;
		}

		if(block_series.size() == 3 && block_series[1]->mod == 'm' && block_series[1]->type == '^'){
			FormulaBlock *new_block = new FormulaBlock('^','n');
			new_block->subblocks.push_back(block_series[0]);
			new_block->subblocks.push_back(block_series[2]);
			delete block_series[1];
			return new_block;
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
		return nullptr;
	}

public:
	Parser_Mathematica(): skip_characters({' ', '\n'}), elementary_operations({'+', '-', '*', '/', '^'}),
		functions_list({{"Sin", "sin"}, {"Cos", "cos"}, {"Tan", "tan"}}) {};

	FormulaBlock *parse_formula(const std::string &formula_string){
		return process_elementary_operations(parse_elementary_operations(formula_string));
	}

	void add_variables(const std::vector<std::string> &variables){
		for(auto el : variables)
			variables_list.insert(ParsingRoutines::remove_side_spaces(el));
	}

	void add_variables_substitutions(const std::vector<std::pair<std::string,std::string>> &variables_sub){
		for(auto el : variables_sub)
			variables_sub_list[ParsingRoutines::remove_side_spaces(el.first)] = ParsingRoutines::remove_side_spaces(el.second);
	}

	void add_functions(const std::vector<std::pair<std::string,std::string>> &functions){
		for(auto el : functions)
			functions_list[ParsingRoutines::remove_side_spaces(el.first)] = ParsingRoutines::remove_side_spaces(el.second);
	}

	void load_struct_file(const std::string &file_name){
		using namespace ParsingRoutines;

		std::ifstream file(file_name);
		if(!file){
			std::cout << "Structure description file not found" << std::endl;
			throw("file read error");
		}

		std::string line;
		int mode = 0;
		while(std::getline(file, line)){
			if(line == "Variables:"){
				mode = 1;
				continue;
			}
			if(line == "Substitute Variables:"){
				mode = 2;
				continue;
			}
			if(line == "Functions:"){
				mode = 3;
				continue;
			}
			if(mode == 1){
				variables_list.insert(remove_side_spaces(line));
				continue;
			}
			
			std::istringstream iss(line);
			std::string token1, token2;
			std::getline(iss, token1, ',');
			std::getline(iss, token2, ',');
			token1 = remove_side_spaces(token1);
			token2 = remove_side_spaces(token2);

			if(mode == 2){
				variables_sub_list[token1] = token2;
			}
			if(mode == 3){
				functions_list[token1] = token2;
			}
		}
	}
};

#endif