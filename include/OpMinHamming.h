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
			 * This class implements the minimum hamming distance operator. It merges the answer-sets pairwise by computing the union.
			 * Usage:
			 * &operator["minhamming", A, K](A)
			 *	A(H1), ..., A(Hn)	... handles to n answers
			 *	K(constraint, c)	... c=arbitrary constraints of kind ":-list-of-literals."
			 *	K(ignore, c)		... c=arbitrary list of predicate names of kind "pred1,pred2,...,predn" that shall be ignored, i.e.
			 *				    it is irrelevant if the truth value of these predicates in the result coincides with the sources
			 *	A			... answer to the operator result
			 */
			class OpMinHamming : public IOperator{
			private:
				HexAnswer addSource(HexAnswer* source, std::set<std::string>& atoms, std::set<std::string>& ignoredPredicates, dlvhex::Program& program, dlvhex::AtomSet& facts);
				void optimize(HexAnswer& result, std::set<std::string>& usedAtoms);
			public:
				virtual std::string getName();
				virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) throw (OperatorException);
			};
		}
	}
}

#endif
