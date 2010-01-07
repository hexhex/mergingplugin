#include <CodeGenerator.h>
#include <ParseTreeNodeIterator.h>

#include <sstream>
#include <stdlib.h>

CodeGenerator::CodeGenerator(ParseTreeNode *parsetree) : parsetreeroot(parsetree), codegenerated(false), errorcount(0), warningcount(0){
}

int CodeGenerator::getErrorCount(){
	return errorcount;
}

int CodeGenerator::getWarningCount(){
	return warningcount;
}

bool CodeGenerator::succeeded(){
	return codegenerated && errorcount == 0;
}

bool CodeGenerator::codeGenerated(){
	return codegenerated;
}

// Main code generation method
void CodeGenerator::generateCode(std::ostream &os, std::ostream &err){
	// Reset
	ParseTreeNode *parsetree = parsetreeroot;
	errorcount = 0;
	warningcount = 0;

	// Write mappings for belief bases
	os << "% -------------------- Mappings for belief bases -------------------- " << std::endl;
	for (ParseTreeNodeIterator it = parsetree->begin(ParseTreeNode::section_beliefbase); it != parsetree->end(); ++it){
		translateBeliefBase(&(*it), os, err);
	}
	os << std::endl << std::endl;

	// Write revision plan
	os << "% -------------------- Revision plan -------------------- " << std::endl;
	for (ParseTreeNodeIterator it = parsetree->begin(ParseTreeNode::section_revisionplan); it != parsetree->end(); ++it){
		std::string finalresult = translateRevisionPlan(&(*it->getChild(0)), os, err);			// Works recursively
		os << "% Final result" << std::endl;
		os << "finalresult(AnswerNr) :- results(result" << finalresult << ", AnswerNr)." << std::endl;
	}
	os << std::endl << std::endl;

	// Extract answer sets from sub programs and transfer their content into the real answer sets of this hex program
	writeAnswerSetExtraction(parsetree, os, err);
	os << std::endl << std::endl;

	// Assemble dlvhex call string and put it as comment under the code
	// Essentially the dlvhex output is filtered such that only predicates of the common signature are displayed
	os << "% -------------------- Additional information --------------------" << std::endl;
	std::string filter("");
	bool firstPred = true;
	if (parsetree->begin(ParseTreeNode::section_commonsignature) != parsetree->end()){
		for (ParseTreeNodeIterator it = parsetree->begin(ParseTreeNode::section_commonsignature)->begin(ParseTreeNode::predicate); it != parsetree->begin(ParseTreeNode::section_commonsignature)->end(); ++it){
			// Concatenate predicate names
			std::string currentPred = ((StringTreeNode*)it->getChild(1))->getValue();
			if (!firstPred) filter += ",";
			filter += currentPred;
			firstPred = false;
		}
	}
	os << "% dlvhex call:" << std::endl;
	os << "%   dlvhex -filter=" << filter << " [name of hex file]" << std::endl;

	// Code was generated
	codegenerated = true;
}

// Translates one belief base definition including the mappings
void CodeGenerator::translateBeliefBase(ParseTreeNode *parsetree, std::ostream &os, std::ostream &err){
	std::string name;
	std::string mappings;
	std::string inputrewriter;
	bool useInputRewriter = false;

	// A belief base section has the sub-element "beliefbase" with key-value pairs as it's 0th child
	for (ParseTreeNodeIterator it = parsetree->getChild(0)->getChild(0)->begin(ParseTreeNode::kvpair); it != parsetree->getChild(0)->getChild(0)->end(); ++it){
		// Extract key and value of the current pair
		std::string key = ((StringTreeNode*)(it->getChild(0)))->getValue();
		std::string value = ((StringTreeNode*)(it->getChild(1)))->getValue();
		// Only certain keys have meanings
		if (key == (std::string("name"))){
			name = value;
		}
		if (key == (std::string("inputrewriter"))){
			inputrewriter = value;
			useInputRewriter = true;
		}
		if (key == (std::string("mapping"))){
			mappings = mappings + "     " + value + "\n";
		}
	}

	os << "% Belief base \"" << name << "\"" << std::endl;
	os << "sources(" << name << ", AnswerNr) :- &hex[\"" << std::endl;
	os << mappings;
	os << "\", \"" << (useInputRewriter ? std::string("--inputrewriter=\"") + inputrewriter + std::string("\"") : "") << "\"](" << "AnswerNr" << ")." << std::endl;
}

// Translates one revision plan section recursively
std::string CodeGenerator::translateRevisionPlan(ParseTreeNode *parsetree, std::ostream &os, std::ostream &err){

	// ----- Distinction: sub-revision plans or primitive belief bases -----
	// Revision plans
	if (parsetree->getType() == ParseTreeNode::revisionplansection){
		return translateRevisionPlan_composed(parsetree, os, err);
	}

	// Primitive belief bases
	else if (parsetree->getType() == ParseTreeNode::datasource){
		return translateRevisionPlan_beliefbase(parsetree, os, err);
	}

	errorcount++;
	err << "Error during code generation for revision plan: Type of information source must be either a belief base or a composed revision plan" << std::endl;
	return std::string("ERROR");
}

// Translation of composed revision plan sections
std::string CodeGenerator::translateRevisionPlan_composed(ParseTreeNode *parsetree, std::ostream &os, std::ostream &err){

	// Prepare variables for the operator's name and a unique identifier for this operator application (consisting of the names of the operator's arguments)
	std::string operatorname = std::string("");
	std::string operatorapplicationid = std::string("");

	// Answers passed to the operator
	std::stringstream answerarguments;
	// key-value parameters for the operator
	std::stringstream kvarguments;
	// A comment written into the resulting hex file (just for readability of the generated hex program)
	std::stringstream merginginfo;

	// ------------------------------ Identifier ------------------------------

	// Determine unique operator application identifier. This identifier is unique for this operator application, not just for the operator(!).
	// This requires that all sub revision plans are generated before, since the unique identifier is constructed by concatenating the identifiers of all sub operator applications.
	std::vector<std::string> answerargidentifiers;
	if (parsetree->begin(ParseTreeNode::revisionsources) != parsetree->end()){
		for (ParseTreeNodeIterator it = parsetree->begin(ParseTreeNode::revisionsources)->begin(); it != parsetree->begin(ParseTreeNode::revisionsources)->end(); ++it){
			// Recursive generation of revision plans
			std::string subrvid = translateRevisionPlan(&(*it), os, err);
			answerargidentifiers.push_back(subrvid);
			// Unique operator application identifier consists of all identifiers of the sub-revision plans (separated by "_")
			operatorapplicationid = operatorapplicationid + subrvid;
		}
	}

	// ------------------------------ Operator arguments ------------------------------

	// Each operator gets two types of arguments:
	// 	1. sets of answer sets from the information sources it merges
	//	2. key-value pairs which can parameterize the operator's logic
	// The two types of arguments are assembled for the current operator application in the following two big blocks

	// Handle key-value pairs in the current revision plan section
	bool firstarg = true;
	if (parsetree->begin(ParseTreeNode::kvpairs) != parsetree->end()){
		for (ParseTreeNodeIterator it = parsetree->begin(ParseTreeNode::kvpairs)->begin(ParseTreeNode::kvpair); it != parsetree->begin(ParseTreeNode::kvpairs)->end(); ++it){
			// For each key-value pair: if the key can be interpretet by this compiler this is done in the following block;
			// otherwise the key-value pair is passed as parameter to the operator and can be evaluated by the operator at run-time
			std::string key = ((StringTreeNode*)(it->getChild(0)))->getValue();
			std::string value = unquote(((StringTreeNode*)(it->getChild(1)))->getValue());

			// key "operator" indicates the name of the operator to use
			if (key == (std::string("operator"))){
				operatorname = value;
			}else{
				// run-time argument
				if (!firstarg){
					kvarguments << std::endl;
				}
				// Collect the key-value pairs in a new predicate which is unique for this operator application
				kvarguments << "kv_arg" << operatorapplicationid << "(" << key << "," << value << ").";
				firstarg = false;
			}
		}
	}

	// Generate code for the sets of answer sets arguments of the operator
	int i = 1;
	firstarg = true;
	for (std::vector<std::string>::iterator it = answerargidentifiers.begin(); it != answerargidentifiers.end(); ++it){
		std::string arg = *it;

		// Generate comment
		if (!firstarg){
			merginginfo << ", ";
			answerarguments << std::endl;
		}
		merginginfo << "result" << arg;

		// "Copy" the handles to the answers which shall be passed to the parameter into a new predicate which is unique
		// for this operator application
		answerarguments << "answersets_arg" << operatorapplicationid << "(" << (i - 1) << ", R" << i << ") :- results(result" << arg << ", R" << i << ").";

		firstarg = false;
		i++;
	}

	// ------------------------------ Operator application ------------------------------

	// Generate merging code: assembling of answers passed to the operator, key-value arguments and operator call
	os << " % Merging the following sources: " << merginginfo.str() << std::endl;
	os << " %    Answers passed to the operator" << std::endl;
	os << answerarguments.str() << std::endl;
	os << " %    key-value arguments for the operator" << std::endl;
	os << kvarguments.str() << std::endl;
	os << " %    Actual merging" << std::endl;
	os << "results(result" << "_" << operatorname << operatorapplicationid << ", AnswerNr) :- " << "&operator[\"" << operatorname << "\", answersets_arg" << operatorapplicationid << ", kv_arg" << operatorapplicationid << "](AnswerNr)." << std::endl;
	os << std::endl;

	return std::string("_") + operatorname + operatorapplicationid;
}

// Translation of primitive revision plan sections (belief bases)
std::string CodeGenerator::translateRevisionPlan_beliefbase(ParseTreeNode *parsetree, std::ostream &os, std::ostream &err){
	// The unique id is the name of the data source with prefix "_"
	std::string operatorapplicationid = ((StringTreeNode*)parsetree->getChild(0))->getValue();
	os << "% Using belief base " << operatorapplicationid << std::endl;
	os << "results(result_" << operatorapplicationid << ", AnswerNr) :- sources(" << operatorapplicationid << ", AnswerNr)." << std::endl;
	operatorapplicationid = std::string("_") + operatorapplicationid;
	os << std::endl;

	return operatorapplicationid;
}

// Writes code for extracting the final answer sets
void CodeGenerator::writeAnswerSetExtraction(ParseTreeNode *parsetree, std::ostream &os, std::ostream &err){

	// This method writes code which maps the "virtual" answer sets, which are just represented by certain predicates, to the real answer sets of the dlvhex program.
	// The real answer sets will in general contain many undesired additional literals which were added during exectuion. Therefore, the result must be filtered with respect to the predicates of the common signature to retrieve a clean output. The appropriate dlvhex call is added to the output as comment in the last line.

	// Write extraction code of final answer sets
	os << "% ---------- Extraction of final answer sets ---------- " << std::endl;
	os << "finalanswersets(AnswerNr, AnswerSetNr) :- finalresult(AnswerNr), &answersets[AnswerNr](AnswerSetNr)." << std::endl;
	// Now we have a predicate (finalanswersets) which is true for all handles to final answer sets. We need to map this
	// answer sets somehow to the "real" answer sets of the hex program
	// This is done by the following logic program:
	// 	1. Each answer set can be selected or not (disjunctive rule)
	//	2. There must be selected exactly one answer set
	//	3. For the selected answer set: "copy" it's content into the real answer set with simple mapping rules
	os << "% Each answer set can be selected or not" << std::endl;
	os << "selectedas(AnswerSetNr) v -selectedas(AnswerSetNr) :- finalanswersets(AnswerNr, AnswerSetNr)." << std::endl;
	os << std::endl;
	os << "% Select exactly one:" << std::endl;
	os << "%    Select at least one" << std::endl;
	os << "atleastoneasselected :- selectedas(AnswerSetNr)." << std::endl;
	os << ":- not atleastoneasselected." << std::endl;
	os << "%    Select at most one" << std::endl;
	os << ":- selectedas(AnswerSetNr1), selectedas(AnswerSetNr2), AnswerSetNr1 != AnswerSetNr2." << std::endl;
	os << std::endl;

	// 3. Answer set extraction
	os << "% Take content of the selected virtual answer set and transfer it into the \"real\" answer sets of the hex program" << std::endl;

	// Transfer all predicates of the common signature
	if (parsetree->begin(ParseTreeNode::section_commonsignature) != parsetree->end()){
		for (ParseTreeNodeIterator it = parsetree->begin(ParseTreeNode::section_commonsignature)->begin(ParseTreeNode::predicate); it != parsetree->begin(ParseTreeNode::section_commonsignature)->end(); ++it){
			// Retrieve name and arity of the current predicate
			std::string currentPred = ((StringTreeNode*)it->getChild(1))->getValue();
			int currentArity = ((IntTreeNode*)it->getChild(2))->getValue();

			// Transfer all predicates of the common signature from virtual answer sets to real answer sets
			std::stringstream currentArgListSource("");
			std::stringstream currentArgListDest("");
			for (int argIndex = 0; argIndex < currentArity; argIndex++){
				// Build argument lists for this predicate according to it's arity
				if (argIndex > 0){
					currentArgListDest << ", ";
					currentArgListSource << ", ";
				}
				currentArgListDest << "Arg" << argIndex;
				currentArgListSource << "&arguments[AnswerNr, AnswerSetNr, \"" << currentPred << "\"](RunningNr, " << argIndex << ", Arg" << argIndex << ")";
			}

			// Write code to transfer information from virtual answer set to real answer set of this hex program
			if (currentArity > 0){
				os << "%    " << currentPred << " with arity " << currentArity << std::endl;
				os << currentPred << (currentArity > 0 ? "(" + currentArgListDest.str() + ")" : "") << " :- finalresult(AnswerNr), selectedas(AnswerSetNr)" << (currentArity > 0 ? ", " + currentArgListSource.str() : "") << "." << std::endl;
			}else{
				os << "%    " << currentPred << " with arity " << currentArity << std::endl;
				os << currentPred << " :- finalresult(AnswerNr), selectedas(AnswerSetNr), &tuples[AnswerNr, AnswerSetNr](" << currentPred << ", 0)." << std::endl;
			}
		}
	}
}

// \' --> "
// \\ --> \ 
// 
std::string CodeGenerator::unquote(std::string code){
	std::string newcode;
	bool quoted = false;
	for (std::string::iterator it = code.begin(); it != code.end(); ++it){
		if (quoted){
			switch (*it){
				case '\\':
					newcode.push_back('\\');
					break;
				case '\'':
					newcode.push_back('\"');
					break;
				default:
					newcode.push_back(*it);
					break;
			}
			quoted = false;
		}else{
			switch (*it){
				case '\\':
					quoted = true;
					break;
				default:
					newcode.push_back(*it);
					break;
			}
		}
	}
	return newcode;
}

// " --> \'
// \ --> \\ 
// 
std::string CodeGenerator::quote(std::string code){
	std::string newcode;
	for (std::string::iterator it = code.begin(); it != code.end(); ++it){
		switch (*it){
			case '\\':
				newcode.push_back('\\');
				newcode.push_back('\\');
				break;
			case '\"':
				newcode.push_back('\\');
				newcode.push_back('\'');
				break;
			default:
				newcode.push_back(*it);
				break;
		}
	}
	return newcode;
}
