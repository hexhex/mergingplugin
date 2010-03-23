#ifndef _IPARSER_h_
#define _IPARSER_h_

/**
 * \file IParser.h
 * 	Defines the minimum requirements that parsers must satisfy.
 */

#include <ParseTreeNode.h>

#include <iostream>
#include <string>

namespace dlvhex{
namespace merging{
namespace tools{
namespace rpcompiler{

class IParser{
protected:
	// input
	std::vector<std::string> inputFiles;
	FILE *stdinFile;
	std::string input;
	bool parsefiles;	// if true, input is a list of files, otherwise input is a string
	FILE *currentInputFile;
	int inputFileCounter;

	// output
	bool wasParsed;
	int warningCount, errorCount;
	std::string errorMsg;
	ParseTreeNode* parseTree;

	// utility functions
	//	read a file into a string
	std::string readFile(FILE* file);
	//	return a pointer to the next input file
	FILE* getNextInputFile();
	//	return the index of the next input file
	int getCurrentInputFileIndex();
	//	return the name of a file with a given index
	std::string getInputFileName(int index);
public:
	IParser(std::vector<std::string> inputFiles, FILE *stdinFile);
	IParser(std::string input);
	virtual void reset();
	virtual void parse() = 0;
	virtual bool parsed();
	virtual bool succeeded();
	virtual int getErrorCount();
	virtual int getWarningCount();
	virtual ParseTreeNode* getParseTree();
};

}
}
}
}

#endif

/*! \fn dlvhex::merging::tools::rpcompiler::IParser::IParser(std::vector<std::string> inputFiles, FILE *stdinfile)
 *  \brief Initializes the parser to parse a given list of input files.
 *  \param inputfiles The list of files to parse. A -- in this list will insert standard input at this position.
 *  \param stdinFile A pointer to the virtual file representing standard input. Note: This parameter must always be passed. However, this does not mean that is actually always used. If standard input shall be read, a -- must be in the inputFiles list.
 */

/*! \fn dlvhex::merging::tools::rpcompiler::IParser::IParser(std::string input)
 *  \brief Initializes the parser to parse a given input string.
 *  \param input The string to parse
 */

/*! \fn void dlvhex::merging::tools::rpcompiler::IParser::reset()
 *  \brief Resets the internal data structures to prepare a new parsing call.
 */

/*! \fn void dlvhex::merging::tools::rpcompiler::IParser::parse()
 *  \brief Actually parses the input prepared by the constructor
 */

/*! \fn ParseTreeNode* dlvhex::merging::tools::rpcompiler::IParser::getParseTree()
 *  \brief Returns a pointer to the root element of the parse tree.
 *  \brief \return A pointer to the root element of the parse tree. If a parsing error occurred, NULL will be returned.
 */

/*! \fn bool dlvhex::merging::tools::rpcompiler::IParser::parsed()
 *  \brief Returns true iff the input has been parsed so far (i.e. at least one call of parse() occurred). If parsing failed, true is returned nevertheless.
 *  \return bool True if the input has been parsed so far, otherwise false
 */

/*! \fn bool dlvhex::merging::tools::rpcompiler::IParser::succeeded()
 *  \brief Returns true iff parsing succeeded, otherwise false is returned. If parse() was not called yet, false is returned.
 *  \return True iff parsing succeeded.
 */

/*! \fn int dlvhex::merging::tools::rpcompiler::IParser::getErrorCount()
 *  \brief Returns the number of errors which occurred during parsing.
 *  \return int Number of errors
 */

/*! \fn int dlvhex::merging::tools::rpcompiler::IParser::getWarningCount()
 *  \brief Returns the number of warnings which occurred during parsing.
 *  \return int Number of warnings
 */
