#include <OpUnion.h>

#include <sstream>
#include <iostream>

using namespace dlvhex::merging::plugin;

std::string OpUnion::getName(){
	return "union";
}

std::string OpUnion::getInfo(){
	std::stringstream ss;
	ss <<	"     union" << std::endl <<
		"     -----" << std::endl << std::endl <<
		"Expects exactly two inputs" << std::endl;
		"Computes the pairwise union of answer-sets of it's arguments.";
	return ss.str();
}

HexAnswer OpUnion::apply(int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters) throw (OperatorException){
	if (arity != 2){
		throw IOperator::OperatorException("Error: The union operator expects exactly 2 arguments.");
	}

	HexAnswer result;
	for (int answerSetNr1 = 0; answerSetNr1 < arguments[0]->size(); answerSetNr1++){
		for (int answerSetNr2 = 0; answerSetNr2 < arguments[1]->size(); answerSetNr2++){
			// Take the first answer-set
			AtomSet set;
			set.insert((*arguments[0])[answerSetNr1]);

			// Add the second one
			set.insert((*arguments[1])[answerSetNr2]);

			result.push_back(set);
		}
	}
	return result;
}
