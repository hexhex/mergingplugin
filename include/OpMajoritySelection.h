#ifndef __OPMAJORITYSELECTION_H_
#define __OPMAJORITYSELECTION_H_

#include "IOperator.h"

#include <dlvhex/Program.h>
#include <dlvhex/AtomSet.h>

DLVHEX_NAMESPACE_USE

using namespace dlvhex::merging;

namespace dlvhex{
	namespace merging{
		namespace plugin{
			/**
			 * This class implements the majority selection operator. It expects one set of answer-sets and will keep only those answer-sets, that
			 * build the majority concerning the acceptance or denial of a propositional predicate p.
			 * Usage:
			 * &operator["majorityselection", A, K](A)
			 *	A(H)			... handle to one answer
			 *	K(majorityOf, p)	... p is the name of a propositional predicate; the operator will keep the answer-sets that build the majority concerning p
			 *                                  (Note: Only the first occurrence of "majorityOf" is recognized!)
			 */
			class OpMajoritySelection : public IOperator{
			public:
				virtual std::string getName();
				virtual std::string getInfo();
				virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) throw (OperatorException);
			};
		}
	}
}

#endif
