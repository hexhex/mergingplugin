#ifndef __OPSETMINUS_H_
#define __OPSETMINUS_H_

#include "IOperator.h"

DLVHEX_NAMESPACE_USE

using namespace dlvhex::merging;

namespace dlvhex{
	namespace merging{
		namespace plugin{
			/**
			 * This class implements the setminus operator. It computes the pairwise differences between answer sets.
			 * Usage:
			 * &operator["setminus", A,  K](A)
			 *	A(H1), A(H2)		... handles to exactly 2 answers
			 *	A			... answer to the operator result
			 */
			class OpSetminus : public IOperator{
			public:
				virtual std::string getName();
				virtual std::string getInfo();
				virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) throw (OperatorException);
			};
		}
	}
}

#endif
