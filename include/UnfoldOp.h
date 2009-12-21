#ifndef __UNFOLDOP_H_
#define __UNFOLDOP_H_

#include <IOperator.h>
#include <DecisionDiagram.h>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
	namespace asp{

		/**
		 * This class implements the unfolding operator. It assumes each answer to represent a general decision diagram and translates it into a tree-like one .
		 * Usage:
		 * &operator["unfold", DD](A)
		 *	DD	... handle to an answer containing decision diagrams
		 *	A	... answer to the operator result
		 */
		class UnfoldOp : public IOperator{
		private:
			DecisionDiagram unfold(DecisionDiagram::Node* root, DecisionDiagram& ddin);
		public:
			virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters);
		};
	}
}

#endif
