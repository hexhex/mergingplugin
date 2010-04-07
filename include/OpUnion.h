#ifndef __OPUNION_H_
#define __OPUNION_H_

#include "IOperator.h"

DLVHEX_NAMESPACE_USE

using namespace dlvhex::merging;

namespace dlvhex{
	namespace merging{
		namespace plugin{
			/**
			 * This class implements the union operator. It merges the answer-sets pairwise by computing the union.
			 * Usage:
			 * &operator["union", A, K](A)
			 *	A(H1), A(H2)		... handles to exactly 2 answers
			 *	A			... answer to the operator result
			 */
			class OpUnion : public IOperator{
			public:
				virtual std::string getName();
				virtual std::string getInfo();
				virtual std::set<std::string> getRecognizedParameters();
				virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) throw (OperatorException);
			};
		}
	}
}

#endif
