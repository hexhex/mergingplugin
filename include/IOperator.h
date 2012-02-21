#ifndef __IOPERATOR_H_
#define __IOPERATOR_H_

#include "PublicTypes.h"
#include <dlvhex2/ComfortPluginInterface.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <stdexcept>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
	namespace merging{
		namespace plugin{
			/**
			 * Defines the functions that a merging operator for the mergingplugin must provide.
			 */
			class IOperator{
			public:
				/**
				 * An instance of this class is thrown in the method IOperator::apply to inform the plugin about an operator error.
				 * This will lead to a plugin error and a termination of dlvhex
				 */
				class OperatorException : public std::runtime_error{
				public:
					OperatorException(std::string m) : std::runtime_error(m){}
					virtual ~OperatorException() throw() {};
					std::string getMessage(){ return what(); }
				};
				virtual std::string getName() = 0;
				virtual std::string getInfo(){ throw OperatorException("no info"); }
				virtual std::set<std::string> getRecognizedParameters(){ throw OperatorException("not defined"); }
				virtual HexAnswer apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) throw (OperatorException) { return apply(false, arity, answers, parameters); }
				virtual HexAnswer apply(bool debug, int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) throw (OperatorException) { return apply(arity, answers, parameters); }
			};
		}
	}
}
#endif


/*! \fn dlvhex::merging::plugin::IOperator::OperatorException::OperatorException(std::string m)
 *  \brief Constructs a new exception with a certain error messasge.
 *  \param msg The error message to use
 */

/*! \fn dlvhex::merging::plugin::IOperator::OperatorException::~OperatorException()
 *  \brief Destructor
 */

/*! \fn std::string dlvhex::merging::plugin::IOperator::OperatorException::getMessage()
 *  \brief Retrieves the error message of this exception.
 *  \param std::string The error message of this exception
 */

/*! \fn std::string dlvhex::merging::plugin::IOperator::getName() = 0
 *  \brief Returns the name of this operator
 *  \return std::string The name of this operator
 */

/*! \fn std::string dlvhex::merging::plugin::IOperator::getInfo() = 0
 *  \brief Optional. Can return information about the usage of this operator. This info can be requested using the "opinfo" command line option
 *  \return std::string Some information about this operator
 *  \throw OperatorException In case that no information is provided
 */

/*! \fn std::set<std::string> dlvhex::merging::plugin::IOperator::getRecognizedParameters()
 *  \brief Optional. Returns a vector of all keys in OperatorArguments that are processed by the apply method. If provided, the framework will detect and report parameters that are
 *  passed but not processed by the operator. Note: Also optional parameters must be included in this list. The automatic check makes sure that no parameters are passed that are not
 *  in this list. However, if there are any restrictions about the numer of occurrences of a certain key this must be checked manually in the apply methods.
 *  \return std::vector<std::string> A list of all parameters (keys) that are recognized by the operator.
 *  \throw OperatorException In case that no such list is provided (then the automatic check is disabled)
 */

/*! \fn HexAnswer dlvhex::merging::plugin::IOperator::apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) = 0
 *  \brief If the 4-ary version of this function is not overloaded, it is called when the operator is applied.
 *  \param arity Number of elements in the vector "answers" (i.e. number of arguments)
 *  \param answers A vector of pointers to answers which represent the arguments passed to the operator
 *  \param parameters A vector of key-value tuples representing the parameters of the operator; the same key can occur arbitrary many times
 *  \return HexAnswer The result of the operator application
 */

/*! \fn HexAnswer dlvhex::merging::plugin::IOperator::apply(bool debug, int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) = 0
 *  \brief Is called when an operator is applied.
 *  \param debug Tells the operator if it is called in debug mode or not
 *  \param arity Number of elements in the vector "answers" (i.e. number of arguments)
 *  \param answers A vector of pointers to answers which represent the arguments passed to the operator
 *  \param parameters A vector of key-value tuples representing the parameters of the operator; the same key can occur arbitrary many times
 *  \return HexAnswer The result of the operator application
 */
