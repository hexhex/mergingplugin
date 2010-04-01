#ifndef __OPHAMMINGMIN_H_
#define __OPHAMMINGMIN_H_

#include "IOperator.h"

#include <dlvhex/Program.h>
#include <dlvhex/AtomSet.h>

DLVHEX_NAMESPACE_USE

using namespace dlvhex::merging;

namespace dlvhex{
	namespace merging{
		namespace plugin{
			/**
			 * This class implements Dalal's operator (using Dalal's distance measure).
			 * Usage:
			 * &operator["dalal", A, K](A)
			 *	A(H1), ..., A(Hn)	... Handles to n answers
			 *	K(constraint, c)	... c = arbitrary constraints of kind ":-list-of-literals."
			 *	K(constraintfile, f)	... f = the name of a file that contains additional constraints for the group decision
			 *	K(ignore, c)		... c=arbitrary list of predicate names of kind "pred1,pred2,...,predn" that shall be ignored, i.e.
			 *				    it is irrelevant if the truth value of these predicates in the result coincides with the sources
			 *	K(weights, c)		... assigns weight values to the belief bases. c is a string of form "w1,w2,..,wn", where n is
			 *				    the number of knowledge bases and wi are integer value. default weight is 1 for all bases, higher values
			 *				    denote higher impact of this source
			 *	K(aggregate, a)		... "a" is either a program that compuates the value to be optimized or the name of a built-in aggregate function
			 *
			 *				    In case that a program is passed, it may uses
			 *					cost(B,sum,C)
			 *				    to access the total cost C for a certain belief base B
			 *				    	(cost(B,I,C) delivers costs for individual atoms)
			 *				... The program is expected to derive an atom
			 *				    	optimize(C)
			 *				    where C denotes the total costs to be minimized
			 *
			 *	K(penalize, p)		... Sets the costs for a certain kind of difference between an individual's opinion and the aggregated one.
			 *	                            Arbitrary many of penalize entries may occur, where each is a triplet of kind:
			 *	                            	{+,not,-,not-},{+,not,-,not-},int
			 *				    
			 *	                            Example: +,-,10
			 *				    
			 *	                            The first entry refers to an atom in an individual result set, the second one to an entry in the aggregated
			 *	                            decision, the integer is the cost factor for a violation of this constraint (multiplied with the weight of
			 *				    the source)
			 *				    + = positive, e.g. { ab(heater) }
			 *				    not = positive atom not contained, e.g. { }
			 *				    - = strongly negated atom contained, e.g. { -ab(heater) }
			 *				    not- = strongly negated atom not contained, e.g. { }
			 *				    
			 *				    Example: "+,-,10" means, if the individual votes for a positive atom and the group decision is the strongly
			 *				    negated version of the proposition, then the penalty is 10 times the weight of the individual
			 *				    
			 *				    Supported shortcuts: "ignoring" for penaltizing individual's beliefs that are not in the aggregated decision,
			 *								i.e. "+,not,1" and "-,not-,1"
			 *				                         "unfounded" for penaltizing aggregated beliefs that are not in the individual's,
			 *								i.e. "not,+,1"
			 *				                         "aberration" for penaltizing both ignoring and unfounded beliefs
			 *								i.e. "not,+,1", "not,+,1" and "not-,-,1"
			 *				    
			 *				    Built-In aggregate functions are "sum", "max"
			 *	K(maxint, i)		... Defines the maximum integer that may occurrs in the computation of the aggregate function
			 *	                            The operator provides a default value that is high enough for sum aggregate function
			 *	A			... Handle to the answer of the operator result
			 */
			class OpDalal : public IOperator{
			private:
				HexAnswer addSource(HexAnswer* source, int weight, int& maxint, std::string costAtom, std::string costSum, float penalize[4][4], std::set<Atom>& sourceAtoms, std::set<std::string>& usedAtoms, std::set<std::string>& ignoredPredicates, dlvhex::Program& program, dlvhex::AtomSet& facts);
				void optimize(HexAnswer& result, std::string optAtom);
			public:
				virtual std::string getName();
				virtual std::string getInfo();
				virtual HexAnswer apply(bool debug, int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) throw (OperatorException);
			};
		}
	}
}

#endif
