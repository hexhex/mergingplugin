
//
// this include is necessary
//

#include "IOperator.h"

#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>

#include <ltdl.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>

using namespace dlvhex::merging::plugin;

namespace dlvhex{
namespace merging{
namespace test{

class TestOp1 : public IOperator{
	std::string getName(){
		return "testop1";
	}

	HexAnswer
	apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) throw (OperatorException)
	{
		HexAnswer result;
//		dlvhex::AtomSet as;
//		as.insert(dlvhex::AtomPtr(new dlvhex::Atom("foo")));
//		as.insert(dlvhex::AtomPtr(new dlvhex::Atom("xyz")));
//		result.push_back(as);
		return result;
	}
};

class TestOp2 : public IOperator{
	std::string getName(){
		return "testop2";
	}

	HexAnswer
	apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) throw (OperatorException)
	{
		// Union
		HexAnswer result;
		for (int answerNr = 0; answerNr < arity; answerNr++){
			HexAnswer* currentAnswer = answers[answerNr];
			for (int answerSetNr = 0; answerSetNr < currentAnswer->size(); answerSetNr++){
				result.push_back((*currentAnswer)[answerSetNr]);
			}
		}
/*
	for (int answerSetNr1 = 0; answerSetNr1 < arguments[0]->size(); answerSetNr1++){
		for (int answerSetNr2 = 0; answerSetNr2 < arguments[1]->size(); answerSetNr2++){
			// Take the first answer-set
			InterpretationPtr set = InterpretationPtr(new Interpretation(*(*arguments[0])[answerSetNr1]));

			// Add the second one
			set->getStorage() |= ((*arguments[1])[answerSetNr2])->getStorage();

			result.push_back(set);
		}
	}
*/


		return result;
	}
};

}
}
}

using namespace dlvhex::merging::test;

TestOp1 testOp1Inst;
TestOp2 testOp2Inst;

extern "C"
std::vector<IOperator*>
OPERATORIMPORTFUNCTION()
{
	std::vector<IOperator*> operators;
	operators.push_back(&testOp1Inst);
	operators.push_back(&testOp2Inst);
	return operators;
}
