#include <iostream>
#include <string>
#include <cstring>
#include <parser_math.h>
#include <fstream>
#include <sstream>


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

	Parser_Mathematica file_parser;
	file_parser.load_struct_file(struct_file);

	std::ifstream file_process(file_in);
	std::string formula_str;
	if(file_process){
		std::ostringstream ss;
		ss << file_process.rdbuf();
		formula_str = ss.str();
	}

	// Removing endl characters
	formula_str.erase(std::remove(formula_str.begin(), formula_str.end(), '\n'), formula_str.end());

	FormulaBlock *file_formula = file_parser.parse_formula(formula_str);

	// std::cout << *file_formula;

	std::ofstream file_print(file_out);
	file_print << file_formula->c_output();

	delete file_formula;

	// std::string test_string = "-5 + (a + asd a/b) / (c + d)^(-z) + e * (x + y) - Sin[a*x + b]";
	// // std::string test_string = "Sin[c*(a + b) + 2^x] + f";
	// // std::string test_string = "- (a + b) + b - c + d";
	// // std::string test_string = "Cos[2 x] a";
	// // std::string test_string = "a + asd a b";
	// // std::string test_string = "- 5 + (a + s) * (c + d)^2 + e * (x + y) - sin[a*x + b]";
	// // std::string test_string = "-a*b/c*d*e/f";
	// // std::string test_string = "a";
	// // std::string test_string = "Cos[2*x] (1 - A0[x])";
	// // std::string test_string = "-a";
	// // std::string test_string = "- a * (d + c) + b^2 - E^(-(c + d)) + q * Sin[2 x]";

	// Parser_Mathematica parser;
	// parser.add_variables({"a", "b", "c", "z", "d", "x", "y", "e"});
	// parser.add_variables_substitutions({{"asd", "r"}});
	// FormulaBlock *formula = parser.parse_formula(test_string);

	// std::cout << *formula;

	// // std::ofstream out("formula_out.txt");
	// // out << formula->c_output();

	// std::cout << '|' << formula->c_output() << '|' << std::endl;

	// delete formula;

	return 0;
}