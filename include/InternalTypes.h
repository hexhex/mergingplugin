#ifndef __INTERNALTYPES_H_
#define __INTERNALTYPES_H_

#include <PublicTypes.h>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
	namespace asp{
		/**
		 * Unique identification of a certain hex or operator call
		 */
		class HexCall{
		public:
			/**
			 * HexProgram: Call of a hex program given as string which is directly embedded within another hex program
			 * HexFile: Call of an external hex program stored in a file
			 * OperatorCall: Application of a built-in or external operator on other hex program results
			 */
			enum CallType{
				HexProgram,
				HexFile,
				OperatorCall,
			};

		private:
			CallType type;
			std::string program;
			std::string arguments;
			std::vector<int> asParams;
			OperatorArguments kvParams;

		public:
			HexCall(CallType ct, std::string prog, std::string args);
			HexCall(CallType ct, std::string opname, std::vector<int> as, OperatorArguments kv);
			bool operator==(const HexCall &other) const;
		};

		// An answer is identified by an appropriate call
		typedef std::pair<HexCall, HexAnswer > HexAnswerCacheEntry;
		// A cache of answers consists of multiple cache entries
		typedef std::vector< HexAnswerCacheEntry > HexAnswerCache;
	}
}
#endif
