#ifndef __OPERATORS_H_
#define __OPERATORS_H_

#include <InternalTypes.h>
#include <dlvhex/PluginInterface.h>
#include <stdlib.h>
#include <IOperator.h>

namespace dlvhex {
	namespace asp {
		/**
		 * This class implements an external atom which can be used to call an external operator.
		 * Operators take sets of answer sets as arguments and compute another set of answer sets as result.
		 * Usage:
		 * &operator[Op, Params](R)
		 *	Op		... path to an operator library
		 *	Params		... name of a unary predicate containing all the result indices which shall be passed to the operator
		 *	R		... handle to the answer of the operator applicatoin
		 */
		typedef IOperator* (*t_getOperator)();
		class OperatorAtom : public PluginAtom
		{
		private:
			HexAnswerCache &resultsetCache;
			std::map<std::string, IOperator*> builtinOperators;
			std::vector<std::string> operatorpaths;
		public:

			OperatorAtom(HexAnswerCache &rsCache);
			virtual ~OperatorAtom();
			virtual void retrieve(const Query& query, Answer& answer) throw (PluginError);
			void setSearchPaths(std::vector<std::string> paths);
		};
	}
}

#endif