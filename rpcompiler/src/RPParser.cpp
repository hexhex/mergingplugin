#include <RPParser.h>

#include <stdio.h>
#include <string.h>

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

struct RPParser::rp_grammar : public grammar<rp_grammar>{

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
		rule<ScannerT, parser_context<>, parser_tag<TSection> > Section;
		rule<ScannerT, parser_context<>, parser_tag<TProgram> > Program;

	        definition(rp_grammar const& self){

			// Terminal tokens (scanner)
			SECTIONCS =			str_p("[common signature]");
			SECTIONBB =			str_p("[belief base]");
			SECTIONRP =			str_p("[revision plan]");
			OCBRACKET =			str_p("{");
			CCBRACKET =			str_p("}");
			SEMICOLON =			str_p(";");
			COLON =				str_p(":");
			SLASH =				str_p("/");
			NUMBER =			token_node_d[ int_p ];
			IDENTIFIER =			token_node_d[ (ch_p('"') >> *(~ch_p('"')) >> ch_p('"')) | (+(range_p('a','z') | range_p('A','Z') | range_p('0','9'))) ];

			// Grammer
			Source =			ComposedRevisionPlan >> SEMICOLON;
			KeyValuePair =			IDENTIFIER >> COLON >> IDENTIFIER >> SEMICOLON;
			Predicate =			IDENTIFIER >> COLON >> IDENTIFIER >> SLASH >> NUMBER >> SEMICOLON;
			CommonSignatureSection =	*Predicate;
			BeliefBaseSection =		*KeyValuePair;
			RevisionPlanSection =		ComposedRevisionPlan;
			ComposedRevisionPlan =		(OCBRACKET >> *KeyValuePair >> *Source >> CCBRACKET) |
							(OCBRACKET >> IDENTIFIER >> CCBRACKET);
			Section =			(SECTIONCS >> CommonSignatureSection) |
							(SECTIONBB >> BeliefBaseSection) |
							(SECTIONRP >> RevisionPlanSection);
			Program =			*Section >> !end_p;
	        }
	        rule<ScannerT, parser_context<>,  parser_tag<TRootNode> > start() const{ 
			return Program;
		}
	};
};

// ------------------------------ Internal methods ------------------------------

std::string RPParser::readFile(FILE* file){
	std::string filecontent;
	char buffer[1024];

	while (fgets(buffer, 1024, file) != NULL){
		filecontent = filecontent + std::string(buffer);
	}
	return filecontent;
}

int RPParser::getCurrentInputFileIndex(){
	return inputFileCounter;
}

FILE* RPParser::getNextInputFile(){

	// Close current file
	if (currentInputFile != NULL) fclose(currentInputFile);

	// Go to next file
	inputFileCounter++;
	if (inputFileCounter < inputFiles.size()){
			if (inputFiles[inputFileCounter] != std::string("--")){
			currentInputFile = fopen(inputFiles[inputFileCounter].c_str(), "r");
			fseek(currentInputFile, SEEK_SET, 0);
			return currentInputFile;
		}else{
			currentInputFile = this->stdin;
			return currentInputFile;
		}
	}else{
		// No more files
		return NULL;
	}
}

std::string RPParser::getInputFileName(int index){
	return inputFiles[inputFileCounter];
}

ParseTreeNode* RPParser::createParseTree(iter_t const& i, int l){

	ParseTreeNode* result = NULL;
	std::string rs;
	int begin;
	int end;
	int j;

	iter_t chi = i->children.begin();
	switch(i->value.id().to_long()){
		case TRootNode:
			result = createParseTree(chi, l + 1);
			break;
		case TProgram:
			// Programs contain sections as sub nodes
			result = new ParseTreeNode(ParseTreeNode::sections, 0);
			for (; chi != i->children.end(); ++chi){
				result->addChild(createParseTree(chi, l + 1));
			}
			break;
		case TSection:
			// First child is the section keyword, second one is the section itself
			++chi;
			result = createParseTree(chi, l + 1);
			break;
		case TCommonSignatureSection:
			result = new ParseTreeNode(ParseTreeNode::section_commonsignature, 0);
			// All children are predicate definitions
			for (; chi != i->children.end(); ++chi){
				result->addChild(createParseTree(chi, l + 1));
			}
			break;
		case TBeliefBaseSection:
			result = new ParseTreeNode(ParseTreeNode::section_beliefbase, 0);
			result->addChild(new ParseTreeNode(ParseTreeNode::beliefbase, 0));
			result->getChild(0)->addChild(new ParseTreeNode(ParseTreeNode::kvpairs, 0));
			// All children are key-value pairs
			for (; chi != i->children.end(); ++chi){
				result->getChild(0)->getChild(0)->addChild(createParseTree(chi, l + 1));
			}
			break;
		case TRevisionPlanSection:
			result = new ParseTreeNode(ParseTreeNode::section_revisionplan, 0);
			result->addChild(createParseTree(chi, l + 1));
			break;
		case TComposedRevisionPlan:
			// Distinction: data source or sub-revision plan
			if (i->children.size() == 3){
				// Data source
				result = new ParseTreeNode(ParseTreeNode::datasource, 0);
				chi++;	// Skip bracket
				result->addChild(createParseTree(chi, l + 1));
			}else{
				// Sub revision plan
				result = new ParseTreeNode(ParseTreeNode::revisionplansection, 0);
				result->addChild(new ParseTreeNode(ParseTreeNode::kvpairs, 0));
				result->addChild(new ParseTreeNode(ParseTreeNode::revisionsources, 0));

				// Children are either key-value pairs or belief sources
				for (; chi != i->children.end(); ++chi){
					// Check type of this child and add to the appropriate node
					if (chi->value.id().to_long() == TKeyValuePair){
						result->getChild(0)->addChild(createParseTree(chi, l + 1));
					}else if (chi->value.id().to_long() == TSource){
						result->getChild(1)->addChild(createParseTree(chi, l + 1));
					}
				}
			}
			break;
		case TPredicate:
			result = new ParseTreeNode(ParseTreeNode::predicate, 0);

			// First child is the keyword "predicate"
			result->addChild(createParseTree(chi, l + 1));
			++chi;

			// Second one: colon (:)
			// nothing to do
			++chi;

			// Third one: predicate name
			result->addChild(createParseTree(chi, l + 1));
			++chi;

			// Fourth one: slash (/)
			// nothing to do
			++chi;

			// Fifth one: arity
			result->addChild(createParseTree(chi, l + 1));
			++chi;

			// Sixth one: semicolon (;)
			// nothing to do
			++chi;
			break;
		case TKeyValuePair:
			result = new ParseTreeNode(ParseTreeNode::kvpair, 0);
			result->addChild(createParseTree(chi, l + 1));		// Key
			chi++;
			chi++;							// Skip colon (:)
			result->addChild(createParseTree(chi, l + 1));		// Value
			chi++;							// Skip semicolon (;)
			break;
		case TSource:
			result = createParseTree(chi, l + 1);
			break;
		case TIDENTIFIER:
			rs = std::string(chi->value.begin(), chi->value.end());
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
			result = new IntTreeNode(atoi(std::string(chi->value.begin(), chi->value.end()).c_str()));
			break;
	}


	return result;
}

// ------------------------------ Public methods ------------------------------

RPParser::RPParser(std::vector<std::string> inputfiles, FILE *stdinfile) : parsefiles(true), parsetree(NULL), isparsed(false), stdin(stdinfile), inputFiles(inputfiles), inputFileCounter(-1), currentInputFile(NULL) {
}

RPParser::RPParser(std::string i) : parsefiles(false), inputstring(i) {
}

void RPParser::parse(){

	RPParser::rp_grammar grammer;

	if (parsefiles){
		isparsed = false;
		scannererrors = 0;
		syntaxerrors = 0;
		syntaxwarnings = 0;

		// Parse all input files
		FILE* filePtr;
		std::string parsingContent;
		while ((filePtr = getNextInputFile()) != NULL){
			std::string fileContent = readFile(filePtr);
			parsingContent += fileContent;

			const char* it_begin = fileContent.c_str();
			const char* it_end = it_begin+fileContent.size();

			typedef node_val_data_factory<> factory_t;

			tree_parse_info<iterator_t, factory_t> info = pt_parse<factory_t>(it_begin, it_end, grammer, space_p);

			if (!info.full){
				// extract erroneous line of code
				int errorpos = info.stop - it_begin + 1; // +1 since .stop points to the end of _successful_ parsed source
				int lineendpos = ((fileContent.find_first_of("\n", errorpos) == std::string::npos) ? fileContent.length() : fileContent.find_first_of("\n", errorpos));
				std::cerr << "  Input line: " << fileContent.substr(errorpos, lineendpos - errorpos) << std::endl;
				syntaxerrors++;
			}else{
				parsetree = createParseTree(info.trees.begin(), 0);
			}
		}
		isparsed = true;
	}else{
		tree_parse_info<> info = pt_parse(inputstring.c_str(), grammer, space_p);
		parsetree = createParseTree(info.trees.begin(), 0);
	}
}

/**
 * "Entry" method for other modules. The method tries to parse the contents of standard input.
 * \return ParseTreeNode* In case of successful parsing, a pointer to the root of the constructed parse tree. Otherwise NULL is returned. Note that is parsing was successful, the tree needs to be freed by calling the root's destructor.
 */
ParseTreeNode* RPParser::getParseTree(){
	return parsetree;
}

bool RPParser::succeeded(){
	return isparsed && parsetree != NULL;
}

bool RPParser::parsed(){
	return isparsed;
}

int RPParser::getErrorCount(){
	return syntaxerrors + scannererrors;
}

int RPParser::getWarningCount(){
	return syntaxwarnings;
}
