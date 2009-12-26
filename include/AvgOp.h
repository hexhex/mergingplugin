#ifndef __AVGOP_H_
#define __AVGOP_H_

#include <IOperator.h>
#include <DecisionDiagram.h>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
	namespace asp{

		/**
		 * This class implements the average operator. It assumes that 2 answers are passed to the operator (binary operator) with one ordered binary diagram tree each.
		 * The result will be another ordered binary diagram tree where all constants in range queries are averaged if the input decision trees differ.
		 * Usage:
		 * &operator["average", DD](A)
		 *	DD	... predicate with handles to exactly 2 answers containing one ordered binary decision tree each
		 *	A	... answer to the operator result
		 */
		class MajorityVotingOp : public IOperator{
		public:
			virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters);
		};
	}
}

#endif
