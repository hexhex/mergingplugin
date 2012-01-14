#ifndef __OPERATORS_H_
#define __OPERATORS_H_

#include <HexAnswerCache.h>
#include <dlvhex/ComfortPluginInterface.hpp>
#include <stdlib.h>
#include <vector>
#include <IOperator.h>

#include "OpUnion.h"
#include "OpSetminus.h"
/*
#include "OpDalal.h"
#include "OpDBO.h"
#include "OpMajoritySelection.h"
#include "OpRelationMerging.h"
*/

namespace dlvhex {
	namespace merging {
		namespace plugin{
			/**
			 * This class implements an external atom which can be used to call an external operator.
			 * Operators take sets of answer sets as arguments and compute another set of answer sets as result.
			 * Usage:
			 * &operator[Op, Params](R)
			 *	Op		... path to an operator library
			 *	Params		... name of a unary predicate containing all the result indices which shall be passed to the operator
			 *	R		... handle to the answer of the operator applicatoin
			 */
			typedef std::vector<IOperator*> (*t_operatorImportFunction)();
			class OperatorAtom : public PluginAtom
			{
			private:
				bool silent, debug;
				HexAnswerCache& resultsetCache;

				std::map<std::string, IOperator*> operators;

				OpUnion _union;
				OpSetminus _setminus;
/*
				OpDalal _dalal;
				OpDBO _dbo;
				OpMajoritySelection _majorityselection;
				OpRelationMerging _relationmerging;
*/
				void registerBuiltInOperators();
			public:
				OperatorAtom(HexAnswerCache &rsCache);
				virtual ~OperatorAtom();
				virtual void retrieve(const Query& query, Answer& answer) throw (PluginError);
				void setMode(bool silentMode, bool debugMode);
				void addOperators(std::string lib);
				IOperator* getOperator(std::string opname);
			};
		}
	}
}

#endif
