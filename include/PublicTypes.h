#ifndef __PUBLICTYPES_H_
#define __PUBLICTYPES_H_

#include <vector>
#include <dlvhex/AtomSet.h>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
	namespace merging{
		// key-value pairs as they are used as arguments of operators
		typedef std::pair<std::string, std::string> KeyValuePair;
		// Operators take a list of key-value pairs as arguments
		typedef std::vector<KeyValuePair> OperatorArguments;
		// The answer of a hex program is a set of answer sets
		typedef std::vector<AtomSet> HexAnswer;

	}
}
#endif
