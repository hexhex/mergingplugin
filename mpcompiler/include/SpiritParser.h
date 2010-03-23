#ifndef _SPIRITPARSER_h_
#define _SPIRITPARSER_h_

/**
 * \file RPParser.h
 * 	Access to the parser written in Boost Spirit.
 */

#include <IParser.h>
#include <ParseTreeNode.h>
#include <string>

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_parse_tree.hpp>

using namespace boost::spirit::classic;

namespace dlvhex{
namespace merging{
namespace tools{
namespace rpcompiler{

class SpiritParser : public IParser{
private:
	struct rp_grammar;

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
	static const int TComposedRevisionPlan = 20;
	static const int TSection = 18;
	static const int TProgram = 19;

	// Internal construction of the parse tree passed to the code generation module
	typedef char const*                             	iterator_t;
	typedef tree_match<iterator_t>                  	parse_tree_match_t;
	typedef parse_tree_match_t::const_tree_iterator 	iter_t;
	typedef parse_tree_match_t::node_t			node_t;
	ParseTreeNode* createParseTree(node_t const& n, int l);
	//ParseTreeNode* createParseTree(iter_t const& i, int l);

public:
	SpiritParser(std::vector<std::string> inputfiles, FILE *stdinfile);
	SpiritParser(std::string i);
	void parse();
};

}
}
}
}

#endif
