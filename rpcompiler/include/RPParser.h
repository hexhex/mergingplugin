#ifndef _RPPARSER_h_
#define _RPPARSER_h_

/**
 * \file RPParser.h
 * 	Access to the parser written in Boost Spirit.
 */

#include <ParseTreeNode.h>
#include <string>

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_parse_tree.hpp>

using namespace boost::spirit::classic;

class RPParser{
private:
	// Flags and input handling
	bool parsefiles;
	std::string inputstring;
	bool isparsed;
	std::vector<std::string> inputFiles;
	FILE *currentInputFile;
	FILE *stdin;
	int inputFileCounter;

	struct rp_grammar;

	// Pointer to the final parse tree (is set when "Program" token was reduced successfully) and number of errors occurred during parsing
	ParseTreeNode *parsetree;
	int scannererrors;
	int syntaxerrors;
	int syntaxwarnings;

	// Helper method for reading files until EOF
	std::string readFile(FILE* file);

	// Tokens used during parsing
	static const int TRootNode = 0;

	static const int TSECTIONCS = 1;
	static const int TSECTIONBB = 2;
	static const int TSECTIONRP = 3;
	static const int TOCBRACKET = 4;
	static const int TCCBRACKET = 5;
	static const int TSEMICOLON = 6;
	static const int TCOLON = 7;
	static const int TSLASH = 8;
	static const int TIDENTIFIER = 9;
	static const int TNUMBER = 10;

	static const int TSource = 11;
	static const int TKeyValuePair = 12;
	static const int TPredicate = 13;
	static const int TCommonSignatureSection = 15;
	static const int TBeliefBaseSection = 16;
	static const int TRevisionPlanSection = 17;
	static const int TSection = 18;
	static const int TProgram = 19;

	int getCurrentInputFileIndex();
	FILE* getNextInputFile();
	std::string getInputFileName(int index);

	// Internal construction of the parse tree passed to the code generation module
	typedef char const*                              iterator_t;
	typedef tree_match<iterator_t>                   parse_tree_match_t;
	typedef parse_tree_match_t::const_tree_iterator  iter_t;
	ParseTreeNode* createParseTree(iter_t const& i, int l);

public:
	RPParser(std::vector<std::string> inputfiles, FILE *stdinfile);
	RPParser(std::string i);
	void parse();
	ParseTreeNode* getParseTree();
	bool succeeded();
	bool parsed();
	int getErrorCount();
	int getWarningCount();
};

#endif


/*! \fn RPParser::RPParser(std::vector<std::string> inputfiles, FILE *stdinfile)
 *  \brief Initializes the parser to parse a given list of input files.
 *  \param inputfiles The list of files to parse. A -- in this list will insert standard input at this position.
 *  \param stdinfile A pointer to the virtual file representing standard input. Note: This parameter must always be passed. However, this does not mean that is actually always used. If standard input shall be read, a -- must be in the inputfiles list.
 */

/*! \fn Parser::Parser(std::string i)
 *  \brief Initializes the parser to parse a given input string.
 *  \param i The input string to parse
 */

/*! \fn void RPParser::parse()
 *  \brief Actually parses the input prepared by the constructor
 */

/*! \fn ParseTreeNode* RPParser::getParseTree()
 *  \brief Returns a pointer to the root element of the parse tree.
 *  \brief \return A pointer to the root element of the parse tree. If a parsing error occurred, NULL will be returned.
 */

/*! \fn bool RPParser::succeeded()
 *  \brief Returns true iff parsing succeeded, otherwise false is returned.
 *  \return True iff parsing succeeded.
 */

/*! \fn bool RPParser::parsed()
 *  \brief Returns true iff the input has been parsed so far (i.e. at least one call of parse() occurred)
 *  \return bool True if the input has been parsed so far, otherwise false
 */

/*! \fn int RPParser::getErrorCount()
 *  \brief Returns the number of errors which occurred during parsing.
 *  \return int Number of errors
 */

/*! \fn int RPParser::getWarningCount()
 *  \brief Returns the number of warnings which occurred during parsing.
 *  \return int Number of warnings
 */
