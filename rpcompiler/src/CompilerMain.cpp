/**
 * @author Christoph Redl
 * @date   December 07, 2009
 *
 * @brief  Revision plan compiler used as stand-alone application or by bm-plugin for DLVHEX
 */

#include <Parser.h>
#include <CodeGenerator.h>
#include <iostream>
#include <fstream>
#include <RPParser.h>

std::vector<std::string> parseArgs(int argc, char *argv[], bool *showparsetree, bool *showhelp);
void showHelp();

/**
 * Program entry
 */
int main(int argc, char *argv[]){

	// Read list of input files and parse other command line arguments
	bool showparsetree = false;
	bool showhelp = false;
	std::vector<std::string> inputFiles = parseArgs(argc, argv, &showparsetree, &showhelp);

	if (showhelp){
		showHelp();
	}else{
		// Try to parse the input
		// Note: This call has the side effect that stdin is read up to eof
		Parser parserinst(inputFiles, stdin);
//		RPParser parserinst(inputFiles, stdin);
		parserinst.parse();

		// Generate output code
		if (parserinst.succeeded()){
			// Retrieve parse tree
			ParseTreeNode* parseTree = parserinst.getParseTree();

			// Parse tree or code output?
			if (showparsetree){
				std::cout << "Parse tree:" << std::endl;
				parseTree->print(std::cout);
			}else{
				// GenerateCode
				// On success, write the code to std::cout and "errors" (in this case warnings) to std::cerr
				CodeGenerator cginst(parseTree);
				cginst.generateCode(std::cout, std::cerr);

				if (cginst.succeeded()){
					// std::cout << "Code generation finished with " << cginst.getWarningCount() << " warning" << (cginst.getWarningCount() == 0 || cginst.getWarningCount() > 1 ? "s" : "") << std::endl;
				}else{
					// Code generation failed
					std::cerr << "Code generation finished with errors:" << std::endl;
					std::cerr << "   " << cginst.getErrorCount() << " error" << (cginst.getErrorCount() == 0 || cginst.getErrorCount() > 1 ? "s" : "") << ", " << cginst.getWarningCount() << " warning" << (cginst.getWarningCount() == 0 || cginst.getWarningCount() > 1 ? "s" : "") << std::endl;
					delete parseTree;
					return 1;
				}
			}

			delete parseTree;
			return 0;
		}else{
			std::cerr << "Parsing finished with errors:" << std::endl;
			std::cerr << "   " << parserinst.getErrorCount() << " error" << (parserinst.getErrorCount() == 0 || parserinst.getErrorCount() > 1 ? "s" : "") << ", " << parserinst.getWarningCount() << " warning" << (parserinst.getWarningCount() == 0 || parserinst.getWarningCount() > 1 ? "s" : "") << std::endl;
			return 1;
		}
	}
}

/**
 * Recognizes the flags -parsetree and -help as well as filenames and -- and sets the approprite booleans.
 */
std::vector<std::string> parseArgs(int argc, char *argv[], bool *showparsetree, bool *showhelp){
	std::vector<std::string> inputFiles;
	if (argc == 1){
		// No arguments: only read from standard input
		inputFiles.push_back(std::string("--"));
	}else{
		// Read all files passed as parameter ("--" means standard input)
		// Flags are prefixed by -
		for (int i = 1; i < argc; i++){
			// Flag?
			if (argv[i][0] == '-'){
				if (std::string(argv[i]) == std::string("-parsetree")){
					*showparsetree = true;
				}
				if (std::string(argv[i]) == std::string("-help")){
					*showhelp = true;
				}
				if (std::string(argv[i]) == std::string("--")){
					inputFiles.push_back(std::string(argv[i]));
				}
			}else{
				// No: Filename
				inputFiles.push_back(std::string(argv[i]));
			}
		}
	}
	return inputFiles;
}

/**
 * Displays a help message.
 */
void showHelp(){
	std::cout <<	"Revision Plan Compiler" << std::endl <<
					"-----------------------" << std::endl << std::endl <<

					"This tool translates a belief revision scenario into a dlvhex program." <<
					"The scenario is defined in one or more input files or is read from standard input." << std::endl << std::endl <<

					"The compiler recognizes the following command line arguments:"  << std::endl <<
					"-parsetree ... Generates a parse tree rather than dlvhex code" << std::endl <<
					"-help      ... Displays this text" << std::endl << std::endl <<

					"If no filenames are passed, the compiler will read from standard input." <<
					"If at least filename is passed, standard input will _not_ be processed by default." <<
					"However, if -- is passed as additional parameter, standard input will be read additionally." << std::endl;
}
