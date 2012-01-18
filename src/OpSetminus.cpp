#include <OpSetminus.h>

#include <sstream>
#include <iostream>

using namespace dlvhex::merging::plugin;

std::string OpSetminus::getName(){
	return "setminus";
}
std::string OpSetminus::getInfo(){
	std::stringstream ss;
	ss <<	"     setminus" << std::endl <<
		"     --------" << std::endl << std::endl <<
		"Expects exactly two inputs" << std::endl <<
		"Computes the pairwise difference of answer-sets of the first argument minus the second one.";
	return ss.str();
}

std::set<std::string> OpSetminus::getRecognizedParameters(){
	std::set<std::string> list;
	return list;
}


HexAnswer OpSetminus::apply(int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters) throw (OperatorException){
	if (arity != 2){
		throw IOperator::OperatorException("Error: The union operator expects exactly 2 arguments.");
	}

	HexAnswer result;
	for (int answerSetNr1 = 0; answerSetNr1 < arguments[0]->size(); answerSetNr1++){
		for (int answerSetNr2 = 0; answerSetNr2 < arguments[1]->size(); answerSetNr2++){
			// Take the first answer-set
			InterpretationPtr set = InterpretationPtr(new Interpretation(*(*arguments[0])[answerSetNr1]));

			// Subtract the second one
			set->getStorage() -= ((*arguments[1])[answerSetNr2])->getStorage();

			result.push_back(set);
		}
	}
	return result;
}
