#ifndef __ORDERDECISIONTREEOP_H_
#define __ORDERDECISIONTREEOP_H_

#include <IOperator.h>
#include <DecisionDiagram.h>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
	namespace asp{

		/**
		 * This class implements the tree ordering operator. It assumes each answer to represent a binary decision tree and translates it into an ordered one.
		 * Usage:
		 * &operator["orderbinarydecisiontree", DD](A)
		 *	DD	... handle to an answer containing arbitrary many binary decision trees
		 *	A	... answer to the operator result (answer containing ordered binary decision diagrams)
		 */
		class OrderBinaryDecisionTreeOp : public IOperator{
		private:
		public:
			virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters);
		};
	}
}

#endif
