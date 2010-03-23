#ifndef _PARSER_h_
#define _PARSER_h_

/**
 * \file Parser.h
 * 	Access to the parser.
 */

#include <IParser.h>

#include <ParseTreeNode.h>
#include <string>

namespace dlvhex{
namespace merging{
namespace tools{
namespace mpcompiler{

typedef struct TerminalTokenStruct{
	int filenr;			/* input file number */
	int linenr;			/* source linenr */
	int columnnr;			/* source column */
	int iValue;			/* int value used to pass data from scanner to parser (predicate arities or indices into symbol table) */
} TerminalToken;

class BisonParser : public IParser{
public:
	FILE* getNextInputFile();
	int getCurrentInputFileIndex();
	std::string getInputFileName(int index);

	BisonParser(std::vector<std::string> inputfiles, FILE *stdinfile);
	BisonParser(std::string i);
	void parse();
};

}
}
}
}

using namespace dlvhex::merging::tools::mpcompiler;

#endif
