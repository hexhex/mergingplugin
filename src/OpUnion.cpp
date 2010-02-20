#include <OpUnion.h>

#include <iostream>

using namespace dlvhex::merging;

std::string OpUnion::getName(){
	return "union";
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
