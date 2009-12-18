#include <UnionOp.h>

#include <iostream>

using namespace dlvhex::asp;

HexAnswer UnionOp::apply(int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters){
	HexAnswer result;
	for (int answerNr = 0; answerNr < arity; answerNr++){
		HexAnswer* currentAnswer = arguments[answerNr];
		for (int answerSetNr = 0; answerSetNr < currentAnswer->size(); answerSetNr++){
			result.push_back((*currentAnswer)[answerSetNr]);
		}
	}
	return result;
}
