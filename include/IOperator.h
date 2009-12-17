#ifndef __IOPERATOR_H_
#define __IOPERATOR_H_

#include "PublicTypes.h"
#include <dlvhex/AtomSet.h>
#include <stdlib.h>
#include <vector>
#include <string>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
	namespace asp{
		class IOperator{
		public:
			virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) = 0;
		};
	}
}
#endif


/*! \fn HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) = 0
 *  \brief Is called when an operator is applied.
 *  \param arity Number of elements in the vector "answers" (i.e. number of arguments)
 *  \param answers A vector of pointers to answers which represent the arguments passed to the operator
 *  \param parameters A vector of key-value tuples representing the parameters of the operator
 *  \return HexAnswer The result of the operator application
 */
