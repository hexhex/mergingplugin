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
			 *	A(H1), ..., A(Hn)	... handles to n answers
			 *	K(constraint, c)	... c=arbitrary constraints of kind ":-list-of-literals."
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
			 *				    Built-In aggregate functions are "sum", "max"
			 *	K(maxint, i)		... defines the maximum integer that may occurrs in the computation of the aggregate function
			 *	A			... answer to the operator result
			 */
			class OpDalal : public IOperator{
			private:
				HexAnswer addSource(HexAnswer* source, int weight, std::string costAtom, std::set<std::string>& atoms, std::set<std::string>& ignoredPredicates, dlvhex::Program& program, dlvhex::AtomSet& facts);
				void optimize(HexAnswer& result, std::set<std::string>& usedAtoms);
			public:
				virtual std::string getName();
				virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) throw (OperatorException);
			};
		}
	}
}

#endif
