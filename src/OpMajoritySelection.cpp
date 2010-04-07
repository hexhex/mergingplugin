#include <OpMajoritySelection.h>

#include <sstream>

using namespace dlvhex::merging::plugin;

std::string OpMajoritySelection::getName(){
	return "majorityselection";
}

std::string OpMajoritySelection::getInfo(){
	std::stringstream ss;
	ss <<	"     majorityselection" << std::endl <<
		"     -----------------" << std::endl << std::endl <<
		"Expects exactly one input with arbitrary many answer-sets." << std::endl <<
		"The argument \"majorityOf\" is mandatory and defines exactly one propositional atom name." << std::endl <<
		"The operator will then check if the majority of all answer-sets accept or deny this atom." << std::endl <<
		"Finally, only those answer-sets that follow this majority" << std::endl <<
		"will remain; the others are deleted" << std::endl <<
		"Example: {a,b,c}, {a, -b, -c}, {-b, d} with majorityOf=b will deliver {a, -b, -c}, {-b, d}" << std::endl <<
		"         since the majority denies b";
	return ss.str();
}

std::set<std::string> OpMajoritySelection::getRecognizedParameters(){
	std::set<std::string> list;
	list.insert("majorityOf");
	return list;
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
				if (as.size() > 0){
					// check if the atom is propositional in this source
					if (as.begin()->getArity() > 0){
						throw OperatorException("Predicate \"" + it->second + "\" is not propositional.");
					}
					// it's propositional, thus we can safely assume that the answer-set contains exactly one element
					assert(as.size() == 1);
					if (!as.begin()->isStronglyNegated()){
						acc++;
					}else{
						deny++;
					}
				}else{
					// does not occur: neither acc nor deny
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
	throw OperatorException("You need to pass a predicate name among which the majority is selected. Use the property \"majorityOf\".");
}
