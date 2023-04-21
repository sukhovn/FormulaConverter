#include <iostream>
#include <string>
#include <cstring>
#include <parser_main.h>
#include <fstream>
#include <sstream>

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

void read_struct_file(const std::string &file_name, bool clear = true){
	if(clear){
		VarFunc::variables.clear();
		VarFunc::variables_sub.clear();
		VarFunc::functions.clear();
	}

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
			VarFunc::variables.insert(remove_side_spaces(line));
			continue;
		}
		
		std::istringstream iss(line);
		std::string token1, token2;
		std::getline(iss, token1, ',');
		std::getline(iss, token2, ',');
		token1 = remove_side_spaces(token1);
		token2 = remove_side_spaces(token2);

		if(mode == 2){
			VarFunc::variables_sub[token1] = token2;
		}
		if(mode == 3){
			VarFunc::functions[token1] = token2;
		}
	}
}

int main(int argc, char const *argv[]){
	std::string struct_file = "struct.dat";
	std::string file_in = "formula.txt";
	std::string file_out = "formula_out.txt";

	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "-in") == 0){
			file_in = argv[i+1];
			i++;
		}
		else if(strcmp(argv[i], "-str") == 0){
			struct_file = argv[i+1];
			i++;
		}
		else if(strcmp(argv[i], "-out") == 0){
			file_out = argv[i+1];
			i++;
		}
		else{
			std::cout << "Unknown key received" << std::endl;
			return 0;
		}
	}

	// VarFunc::variables = {"a", "b", "c", "z", "d", "x", "y", "e"};
	// VarFunc::variables_sub = {{"q", "r"}}; 
	// VarFunc::functions = {{"Sin", "sin"}, {"Cos", "cos"}, {"Tan", "tan"}};

	read_struct_file(struct_file);

	std::ifstream file_process(file_in);
	std::string formula_str;
	if(file_process){
		std::ostringstream ss;
		ss << file_process.rdbuf();
		formula_str = ss.str();
	}

	//Removing endl characters
	formula_str.erase(std::remove(formula_str.begin(), formula_str.end(), '\n'), formula_str.end());

	FormulaBlock file_formula(formula_str);

	// std::cout << file_formula;

	std::ofstream file_print(file_out);
	file_print << file_formula.c_output();

	// std::string test_string = "-5 + (a + asd a/b) / (c + d)^(-z) + e * (x + y) - Sin[a*x + b]";
	// std::string test_string = "sin[c*(a + b) + 2^x] + f";
	// std::string test_string = "- (a + b) + b - c + d";
	// std::string test_string = "Cos[2 x] a";
	// std::string test_string = "a + asd a b";
	// std::string test_string = "- 5 + (a + s) * (c + d)^2 + e * (x + y) - sin[a*x + b]";
	// std::string test_string = "-a*b/c*d*e/f";
	// std::string test_string = "a";
	// std::string test_string = "Cos[2*x] (1 - A0[x])";

	// std::vector<FormulaBlock*> parsed_string = parse_elementary_operations(test_string);

	// int i = 0;
	// while(1){
	// 	for(int i = 0; i < 50; ++i)
	// 		std::cout << '-';
	// 	std::cout << std::endl;
		
	// 	if(i >= parsed_string.size()) break;

	// 	std::cout << *parsed_string[i++];
	// }

	// std::string test_string = "- a * (d + c) + b^2 - E^(-(c + d)) + q * Sin[2 x]";

	// FormulaBlock formula(test_string);

	// std::cout << formula;

	// std::ofstream out("formula_out.txt");
	// out << formula.c_output();

	// std::cout << '|' << formula.c_output() << '|' << std::endl;

	return 0;
}