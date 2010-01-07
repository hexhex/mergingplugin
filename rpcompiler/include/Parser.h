#ifndef _PARSER_h_
#define _PARSER_h_

/**
 * \file Parser.h
 * 	Access to the parser.
 */

#include <ParseTreeNode.h>
#include <string>

typedef struct TerminalTokenStruct{
	int filenr;			/* input file number */
	int linenr;			/* source linenr */
	int columnnr;			/* source column */
	int iValue;			/* int value used to pass data from scanner to parser (predicate arities or indices into symbol table) */
} TerminalToken;

class Parser{
private:
	bool parsefiles;
	std::string inputstring;
	bool isparsed;
	std::vector<std::string> inputFiles;
	FILE *currentInputFile;
	FILE *stdin;
	int inputFileCounter;
public:
	Parser(std::vector<std::string> inputfiles, FILE *stdinfile);
	Parser(std::string i);
	void parse();
	ParseTreeNode* getParseTree();
	bool succeeded();
	bool parsed();
	int getErrorCount();
	int getWarningCount();
	FILE* getNextInputFile();
	int getCurrentInputFileIndex();
	std::string getInputFileName(int index);
};

#endif


/*! \fn Parser::Parser(std::vector<std::string> inputfiles, FILE *stdinfile)
 *  \brief Initializes the parser to parse a given list of input files.
 *  \param inputfiles The list of files to parse. A -- in this list will insert standard input at this position.
 *  \param stdinfile A pointer to the virtual file representing standard input. Note: This parameter must always be passed. However, this does not mean that is actually always used. If standard input shall be read, a -- must be in the inputfiles list.
 */

/*! \fn Parser::Parser(std::string i)
 *  \brief Initializes the parser to parse a given input string.
 *  \param i The input string to parse
 */

/*! \fn void Parser::parse()
 *  \brief Actually parses the input prepared by the constructor
 */

/*! \fn ParseTreeNode* Parser::getParseTree()
 *  \brief Returns a pointer to the root element of the parse tree.
 *  \brief \return A pointer to the root element of the parse tree. If a parsing error occurred, NULL will be returned.
 */

/*! \fn bool Parser::succeeded()
 *  \brief Returns true iff parsing succeeded, otherwise false is returned.
 *  \return True iff parsing succeeded.
 */

/*! \fn bool CodeGenerator::parsed()
 *  \brief Returns true iff the input has been parsed so far (i.e. at least one call of parse() occurred)
 *  \return bool True if the input has been parsed so far, otherwise false
 */

/*! \fn int Parser::getErrorCount()
 *  \brief Returns the number of errors which occurred during parsing.
 *  \return int Number of errors
 */

/*! \fn int Parser::getWarningCount()
 *  \brief Returns the number of warnings which occurred during parsing.
 *  \return int Number of warnings
 */

/*! \fn FILE* Parser::getNextInputFile()
 *  \brief This method returns a file pointer to one input file after the other. It can be used to iterate through the input file list. When the class is instanciated, the first call will return the first file in the list. When the end of the list is reached, NULL is returned.
 *  \return FILE* A pointer to the next file in the sequence.
 */

/*! \fn int Parser::getCurrentInputFileIndex()
 *  \brief Returns the 0-based index of the current input file.
 *  \return int 0-based index of the current input file
 */

/*! \fn std::string Parser::getInputFileName(int index)
 *  \brief Returns the name of an input file with a given 0-based index-
 *  \return std::string Name of the input file with the given 0-based index. stdin is encoded as --
 */
