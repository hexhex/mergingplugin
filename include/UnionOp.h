#ifndef __UNIONOP_H_
#define __UNIONOP_H_

#include <IOperator.h>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
	namespace asp{

		/**
		 * This class implements the union operator. It merges the answers (not answer sets!) by simply union the sets of answer sets.
		 * Usage:
		 * &operator["union", A1, A2](A)
		 *	A1, A2	... handles of two answers
		 *	A	... answer to the operator result
		 */
		class UnionOp : public IOperator{
		public:
			virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters);
		};
	}
}

#endif
