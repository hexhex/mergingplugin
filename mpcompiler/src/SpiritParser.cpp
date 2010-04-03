#include <SpiritParser.h>

#include <stdio.h>
#include <string.h>

using namespace dlvhex::merging::tools::mpcompiler;

/*
	Grammer in compact EBNF-like notation (* means 0 to arbitrary many times):

		Program			=>	Section*;
		Section			=>	TSECTIONCS CommonSignatureSection |
						TSECTIONBB BeliefBaseSection |
						TSECTIONRP RevisionPlanSection;
		CommonSignatureSection	=>	Predicate*;
		BeliefBaseSection	=>	KeyValuePair*;
		RevisionPlanSection	=>	TOCBRACKET KeyValuePair* Source* TCCBRACKET |
						TOCBRACKET TIDENTIFIER TCCBRACKET;
		Predicate		=>	TIDENTIFIER TCOLON TIDENTIFIER TSLASH TNUMBER TSEMICOLON;
		KeyValuePair		=>	IDENTIFIER TCOLON TIDENTIFIER TSEMICOLON;
		Source			=>	RevisionPlanSection TSEMICOLON
*/

// ------------------------------ Grammer ------------------------------

struct SpiritParser::rp_grammar : public grammar<rp_grammar>{

	template<class ScannerT>
	struct definition{



		rule<ScannerT, parser_context<>, parser_tag<TSECTIONCS> > SECTIONCS;
		rule<ScannerT, parser_context<>, parser_tag<TSECTIONBB> > SECTIONBB;
		rule<ScannerT, parser_context<>, parser_tag<TSECTIONRP> > SECTIONRP;
		rule<ScannerT, parser_context<>, parser_tag<TOCBRACKET> > OCBRACKET;
		rule<ScannerT, parser_context<>, parser_tag<TCCBRACKET> > CCBRACKET;
		rule<ScannerT, parser_context<>, parser_tag<TSEMICOLON> > SEMICOLON;
		rule<ScannerT, parser_context<>, parser_tag<TCOLON> > COLON;
		rule<ScannerT, parser_context<>, parser_tag<TSLASH> > SLASH;
		rule<ScannerT, parser_context<>, parser_tag<TIDENTIFIER> > IDENTIFIER;
		rule<ScannerT, parser_context<>, parser_tag<TNUMBER> > NUMBER;

		rule<ScannerT, parser_context<>, parser_tag<TSource> > Source;
		rule<ScannerT, parser_context<>, parser_tag<TKeyValuePair> > KeyValuePair;
		rule<ScannerT, parser_context<>, parser_tag<TPredicate> > Predicate;
		rule<ScannerT, parser_context<>, parser_tag<TCommonSignatureSection> > CommonSignatureSection;
		rule<ScannerT, parser_context<>, parser_tag<TBeliefBaseSection> > BeliefBaseSection;
		rule<ScannerT, parser_context<>, parser_tag<TRevisionPlanSection> > RevisionPlanSection;
		rule<ScannerT, parser_context<>, parser_tag<TComposedRevisionPlan> > ComposedRevisionPlan;
		rule<ScannerT, parser_context<>, parser_tag<TComment> > Comment;
		rule<ScannerT, parser_context<>, parser_tag<TSection> > Section;
		rule<ScannerT, parser_context<>, parser_tag<TProgram> > Program;

	        definition(rp_grammar const& self){

			// Terminal tokens (scanner)
			SECTIONCS =			str_p("[common signature]");
			SECTIONBB =			str_p("[belief base]");
			SECTIONRP =			str_p("[merging plan]");
			OCBRACKET =			str_p("{");
			CCBRACKET =			str_p("}");
			SEMICOLON =			str_p(";");
			COLON =				str_p(":");
			SLASH =				str_p("/");
			NUMBER =			token_node_d[ int_p ];
			IDENTIFIER =			token_node_d[ (ch_p('"') >> *(~ch_p('"')) >> ch_p('"')) | (+(range_p('a','z') | range_p('A','Z') | range_p('0','9') | ch_p('_') | ch_p('-'))) ];

			// Grammer
			Source =			(discard_node_d[Comment]) >> ComposedRevisionPlan >> SEMICOLON;
			KeyValuePair =			(discard_node_d[Comment]) >> IDENTIFIER >> COLON >> IDENTIFIER >> SEMICOLON;
			Predicate =			(discard_node_d[Comment]) >> IDENTIFIER >> COLON >> IDENTIFIER >> SLASH >> NUMBER >> SEMICOLON;
			CommonSignatureSection =	(discard_node_d[Comment]) >> *Predicate;
			BeliefBaseSection =		(discard_node_d[Comment]) >> *KeyValuePair;
			RevisionPlanSection =		(discard_node_d[Comment]) >> ComposedRevisionPlan;
			ComposedRevisionPlan =		((discard_node_d[Comment]) >> OCBRACKET >> *KeyValuePair >> *Source >> CCBRACKET) |
							((discard_node_d[Comment]) >> OCBRACKET >> IDENTIFIER >> CCBRACKET);
			Section =			((discard_node_d[Comment]) >> SECTIONCS >> CommonSignatureSection) |
							((discard_node_d[Comment]) >> SECTIONBB >> BeliefBaseSection) |
							((discard_node_d[Comment]) >> SECTIONRP >> RevisionPlanSection);
			Program =			*Section >> (discard_node_d[Comment]) >> !end_p;
			Comment =			*(token_node_d[ch_p('%') >> *(anychar_p - eol_p)]);
	        }
	        rule<ScannerT, parser_context<>,  parser_tag<TRootNode> > start() const{ 
			return Program;
		}
	};
};

// ------------------------------ Internal methods ------------------------------

// translates the spirit-internal parsetree into one using class ParseTreeNode that is understood by the code generator
ParseTreeNode* SpiritParser::createParseTree(node_t const& n, int l){

	ParseTreeNode* result = NULL;
	std::string rs;
	int begin;
	int end;
	int j;

	switch(n.value.id().to_long()){
		case TRootNode:
			result = createParseTree(n.children[0], l + 1);
			break;
		case TProgram:
			// Programs contain sections as sub nodes
			result = new ParseTreeNode(ParseTreeNode::sections, 0);
			for (iter_t chi = n.children.begin(); chi != n.children.end(); ++chi){
				result->addChild(createParseTree(*chi, l + 1));
			}
			break;
		case TSection:
			// First child is the section keyword, second one is the section itself
			result = createParseTree(n.children[1], l + 1);
			break;
		case TCommonSignatureSection:
			result = new ParseTreeNode(ParseTreeNode::section_commonsignature, 0);
			// All children are predicate definitions
			for (iter_t chi = n.children.begin(); chi != n.children.end(); ++chi){
				result->addChild(createParseTree(*chi, l + 1));
			}
			break;
		case TBeliefBaseSection:
			result = new ParseTreeNode(ParseTreeNode::section_beliefbase, 0);
			result->addChild(new ParseTreeNode(ParseTreeNode::beliefbase, 0));
			result->getChild(0)->addChild(new ParseTreeNode(ParseTreeNode::kvpairs, 0));
			// All children are key-value pairs
			for (iter_t chi = n.children.begin(); chi != n.children.end(); ++chi){
				result->getChild(0)->getChild(0)->addChild(createParseTree(*chi, l + 1));
			}
			break;
		case TRevisionPlanSection:
			result = new ParseTreeNode(ParseTreeNode::section_revisionplan, 0);
			result->addChild(createParseTree(n.children[0], l + 1));
			break;
		case TComposedRevisionPlan:
			// Distinction: data source or sub-revision plan
			if (n.children.size() == 3){
				// Data source
				result = new ParseTreeNode(ParseTreeNode::datasource, 0);
				// First child is the bracket: skip it
				result->addChild(createParseTree(n.children[1], l + 1));
			}else{
				// Sub revision plan
				result = new ParseTreeNode(ParseTreeNode::revisionplansection, 0);
				result->addChild(new ParseTreeNode(ParseTreeNode::kvpairs, 0));
				result->addChild(new ParseTreeNode(ParseTreeNode::revisionsources, 0));

				// Children are either key-value pairs or belief sources
				for (iter_t chi = n.children.begin(); chi != n.children.end(); ++chi){
					// Check type of this child and add to the appropriate node
					if (chi->value.id().to_long() == TKeyValuePair){
						result->getChild(0)->addChild(createParseTree(*chi, l + 1));
					}else if (chi->value.id().to_long() == TSource){
						result->getChild(1)->addChild(createParseTree(*chi, l + 1));
					}
				}
			}
			break;
		case TPredicate:
			result = new ParseTreeNode(ParseTreeNode::predicate, 0);

			// First child is the keyword "predicate"
			result->addChild(createParseTree(n.children[0], l + 1));

			// Second one: colon (:) -> skip it

			// Third one: predicate name
			result->addChild(createParseTree(n.children[2], l + 1));

			// Fourth one: slash (/) -> skip it

			// Fifth one: arity
			result->addChild(createParseTree(n.children[4], l + 1));

			// Sixth one: semicolon (;) -> skip it
			break;
		case TKeyValuePair:
			result = new ParseTreeNode(ParseTreeNode::kvpair, 0);
			result->addChild(createParseTree(n.children[0], l + 1));		// Key
			// Skip colon (:)
			result->addChild(createParseTree(n.children[2], l + 1));		// Value
			break;
		case TSource:
			result = createParseTree(n.children[0], l + 1);
			break;
		case TIDENTIFIER:
			rs = std::string(n.children[0].value.begin(), n.children[0].value.end());
			// Trim (because of the fact that the skip parser does not cut out the whitespaces perfectly)
			begin = rs.length() - 1;
			end = 0;
			j = 0;
			for (std::string::iterator it = rs.begin(); it != rs.end(); it++, j++){
				if (j < begin && (*it) != ' ' && (*it) != '\t' && (*it) != '\r' && (*it) != '\n') begin = j;
				if (j > end && (*it) != ' ' && (*it) != '\t' && (*it) != '\r' && (*it) != '\n') end = j;
			}
			if (end >= begin){
				rs = rs.substr(begin, end - begin + 1);
			}else{
				rs = std::string("");
			}

			// unquote
			if (rs.length() >= 2){
				if (rs[0] == '\"' && rs[rs.length() - 1] == '\"'){
					rs = rs.substr(1, rs.length() - 2);
				}
			}

			result = new StringTreeNode(rs);
			break;
		case TNUMBER:
			result = new IntTreeNode(atoi(std::string(n.children[0].value.begin(), n.children[0].value.end()).c_str()));
			break;
	}


	return result;
}


// ------------------------------ Public methods ------------------------------

SpiritParser::SpiritParser(std::vector<std::string> inputfiles, FILE *stdinfile) : IParser(inputfiles, stdinfile) {
}

SpiritParser::SpiritParser(std::string i) : IParser(i) {
}

void SpiritParser::parse(){
	reset();

	SpiritParser::rp_grammar grammer;

	if (parsefiles){
		wasParsed = false;
		errorCount = 0;
		warningCount = 0;

		// Parse all input files
		FILE* filePtr;
		while ((filePtr = getNextInputFile()) != NULL){
			// read input file
			std::string fileContent = readFile(filePtr);

			// parse it
			const char* it_begin = fileContent.c_str();
			const char* it_end = it_begin+fileContent.size();
			typedef node_val_data_factory<> factory_t;
			tree_parse_info<iterator_t, factory_t> info = pt_parse<factory_t>(it_begin, it_end, grammer, space_p);

			// check if parsing was successful
			if (!info.full){
				// extract erroneous line of code
				int errorpos = info.stop - it_begin + 1; // +1 since .stop points to the end of _successful_ parsed source
				int lineendpos = ((fileContent.find_first_of("\n", errorpos) == std::string::npos) ? fileContent.length() : fileContent.find_first_of("\n", errorpos));
				std::cerr << "  Input line: " << fileContent.substr(errorpos, lineendpos - errorpos) << std::endl;
				errorCount++;
			}else{
				if (parseTree) delete parseTree;
				parseTree = createParseTree(info.trees[0], 0);
			}
		}

		wasParsed = true;
	}else{
		// parse standard-in
		tree_parse_info<> info = pt_parse(input.c_str(), grammer, space_p);
		if (parseTree) delete parseTree;
		parseTree = createParseTree(info.trees[0], 0);
	}
}
