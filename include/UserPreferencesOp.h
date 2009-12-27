#ifndef __USERPREFERENCESOP_H_
#define __USERPREFERENCESOP_H_

#include <IOperator.h>
#include <DecisionDiagram.h>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
	namespace asp{

		/**
		 * This class implements the user preferences operator. It assumes that arbitrary many input decision diagrams are passed to the operator. Further it assumes
		 * user preferences to be specified in the parameters structure (as key-value pairs).
		 * The result will be another decision diagram which gives preference to one of the decision diagrams according to user prefereces.
		 * Usage:
		 * &operator["userpreferences", DD](A)
		 *	DD	... predicate with handles to arbitrary many decision diagrams
		 *	A	... answer to the operator result
		 */
		class UserPreferencesOp : public IOperator{
		public:
			virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters);
		};
	}
}

#endif
