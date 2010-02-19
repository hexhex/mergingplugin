#ifndef __OPUNION_H_
#define __OPUNION_H_

#include "IOperator.h"

DLVHEX_NAMESPACE_USE

using namespace dlvhex::merging;

namespace dlvhex{
	namespace merging{

		/**
		 * This class implements the union operator. It merges the answer-sets pairwise by computing the union.
		 * Usage:
		 * &operator["union", A1, A2](A)
		 *	A1, A2	... handles of two answers
		 *	A	... answer to the operator result
		 */
		class OpUnion : public IOperator{
		public:
			virtual std::string getName();
			virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters);
		};
	}
}

#endif
