#ifndef __OPRELATIONMERGING_H_
#define __OPRELATIONMERGING_H_

#include "IOperator.h"

#include <dlvhex2/Program.h>
#include <dlvhex2/AtomSet.h>

DLVHEX_NAMESPACE_USE

using namespace dlvhex::merging;

namespace dlvhex{
	namespace merging{
		namespace plugin{
			/**
			 * This class implements a relation merging operator.
			 * Usage:
			 * &operator["relationmerging", A, K](A)
			 *	A(H1), ..., A(Hn)	... handles to n answers
			 *				    each answer is expected to contain exactly one answer-set with data entries of kind
			 *				    	data(a1,...,an)
			 *				    where ai are the attribute values as well as a schema definition of kind
			 *				    	schema(n1,...,nn)
			 *				    where ni are the attribute names
			 *	K(schema, s)		... A list of kind
			 *				    	s = "a1,...,an"
			 *				    defines the arity and the attribute names of the merged schema
			 *	K(key, s)		... A list of kind
			 *				    	s = "k1,...,km"
			 *	K(default, d)		... "d" is a string of kind "a=v" where "a" is some attribute and "v" some default value
			 *	K(rule, r)		... "r" is a rule of kind
			 *				    	attribute(merged, PrimaryKey, Value) :- attribute(1, PrimaryKey, V1), ..., attribute(n, PrimaryKey, Vn)
			 *				    in order to overwrite the built-in rule that states that attribute values are copied into the result iff
			 *				    all sources agree upon it's value
			 *				    that defines the key attributes in the merged schema (they must also be contained in each source)
			 *	A			... answer to the operator result
			 */
			class OpRelationMerging : public IOperator{
			private:
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
