#ifndef __OPSETMINUS_H_
#define __OPSETMINUS_H_

#include "IOperator.h"

DLVHEX_NAMESPACE_USE

using namespace dlvhex::merging;

namespace dlvhex{
	namespace merging{

		/**
		 * This class implements the setminus operator. It computes the pairwise differences between answer sets.
		 * Usage:
		 * &operator["setminus", A1, A2](A)
		 *	A1, A2	... handles of two answers
		 *	A	... answer to the operator result
		 */
		class OpSetminus : public IOperator{
		public:
			virtual std::string getName();
			virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) throw (OperatorException);
		};
	}
}

#endif
