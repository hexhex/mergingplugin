#include <OpMajoritySelection.h>

#include <iostream>

using namespace dlvhex::merging::plugin;

std::string OpMajoritySelection::getName(){
	return "majorityselection";
}

HexAnswer OpMajoritySelection::apply(int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters) throw (OperatorException){
	if (arity != 1){
		throw IOperator::OperatorException("Error: The majorityselection operator expects exactly 1 argument.");
	}

	// check if a predicate was passed that serves for answer-set selection
	for (OperatorArguments::iterator it = parameters.begin(); it != parameters.end(); it++){
		if (it->first == std::string("majorityOf")){
			dlvhex::Atom* p = new dlvhex::Atom(it->second);
			dlvhex::AtomPtr pptr(p);

			// check if the majority accepts or denies p
			int acc = 0;
			int deny = 0;

			for (int answerSetNr = 0; answerSetNr < arguments[0]->size(); answerSetNr++){
				dlvhex::AtomSet as;
				(*arguments[0])[answerSetNr].matchPredicate(it->second, as);
				if (as.size() > 0 && !as.begin()->isStronglyNegated()){
					acc++;
				}else{
					deny++;
				}
			}
			// decision possible?
			if (acc == deny)
				return (*arguments[0]);		// no
		
			// keep only the answer-sets that follow the majority
			HexAnswer result;
			for (int answerSetNr = 0; answerSetNr < arguments[0]->size(); answerSetNr++){
				dlvhex::AtomSet as;
				(*arguments[0])[answerSetNr].matchAtom(pptr, as);
				if (acc > deny){
					// acc > deny
					if (as.size() > 0 && !as.begin()->isStronglyNegated()){
						result.push_back((*arguments[0])[answerSetNr]);
					}else{
						// skip
					}
				}else{
					// acc < deny
					if (as.size() > 0 && !as.begin()->isStronglyNegated()){
						// skip
					}else{
						result.push_back((*arguments[0])[answerSetNr]);
					}
				}
			}
			return result;
		}
	}
	throw OperatorException("You need to pass a predicate name among which the majority is selected.");
}
