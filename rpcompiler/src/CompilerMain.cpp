/**
 * @author Christoph Redl
 * @date   February 21, 2010
 *
 * @brief  This compiler translates revision plan files into semantically equivalent hex programs. The revision plan compiler
 *		(rpcompiler) can be used as stand-alone application or as input rewriter by mergingplugin for dlvhex.
 *         Note that the generated hex programs depend on the external atoms defined in the mergingplugin.
 */

#include <CodeGenerator.h>
#include <iostream>
#include <fstream>

#include <IParser.h>
#include <BisonParser.h>
#include <SpiritParser.h>

std::vector<std::string> parseArgs(int argc, char *argv[], bool *showparsetree, bool *spirit, bool *showhelp);
void showHelp();

/**
 * Program entry
 */
int main(int argc, char *argv[]){

	// Read list of input files and parse other command line arguments
	bool showparsetree = false;
	bool showhelp = false;
	bool spirit = true;
	std::vector<std::string> inputFiles = parseArgs(argc, argv, &showparsetree, &spirit, &showhelp);

	if (showhelp){
		showHelp();
	}else{
		// Create the requested type of parser
		IParser* parserInst;
		if (spirit){
			parserInst = new SpiritParser(inputFiles, stdin);
		}else{
			parserInst = new BisonParser(inputFiles, stdin);
		}

		// Try to parse the input
		// Note: This call has the side effect that stdin is read up to eof
		parserInst->parse();

		// If it was successful, generate output code
		// otherwise display an error message
		if (parserInst->succeeded()){
			// Retrieve parse tree
			ParseTreeNode* parseTree = parserInst->getParseTree();

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
			std::cerr << "   " << parserInst->getErrorCount() << " error" << (parserInst->getErrorCount() == 0 || parserInst->getErrorCount() > 1 ? "s" : "") << ", " << parserInst->getWarningCount() << " warning" << (parserInst->getWarningCount() == 0 || parserInst->getWarningCount() > 1 ? "s" : "") << std::endl;
			return 1;
		}
	}
}

/**
 * Recognizes the flags -parsetree and -help as well as filenames and -- and sets the approprite booleans.
 */
std::vector<std::string> parseArgs(int argc, char *argv[], bool *showparsetree, bool *spirit, bool *showhelp){

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
				if (std::string(argv[i]) == std::string("-spirit")){
					*spirit = true;
				}
				if (std::string(argv[i]) == std::string("-bison")){
					*spirit = false;
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
			"-parsetree ... generates a parse tree rather than dlvhex code" << std::endl <<
			"-help      ... displays this text" << std::endl << std::endl <<
			"-spirit /  ... parses the input using a boost spirit resp." << std::endl <<
			"-bison         a bison generated parser. Default is -spirit" << std::endl << std::endl <<

			"If no filenames are passed, the compiler will read from standard input." <<
			"If at least filename is passed, standard input will _not_ be processed by default." <<
			"However, if -- is passed as additional parameter, standard input will be read additionally." << std::endl;
}
