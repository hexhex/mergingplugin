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
			class OperatorException{
			private:
				std::string msg;
			public:
				OperatorException(std::string m) : msg(m){}
				std::string getMessage(){ return msg; }
			};
			virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) = 0;
		};
	}
}
#endif


/*! \fn OperatorException::OperatorException::OperatorException(std::string m)
 *  \brief Constructs a new exception with a certain error messasge.
 *  \param msg The error message to use
 */

/*! \fn std::string OperatorException::OperatorException::getMessage()
 *  \brief Retrieves the error message of this exception.
 *  \param std::string The error message of this exception
 */

/*! \fn HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) = 0
 *  \brief Is called when an operator is applied.
 *  \param arity Number of elements in the vector "answers" (i.e. number of arguments)
 *  \param answers A vector of pointers to answers which represent the arguments passed to the operator
 *  \param parameters A vector of key-value tuples representing the parameters of the operator
 *  \return HexAnswer The result of the operator application
 */
