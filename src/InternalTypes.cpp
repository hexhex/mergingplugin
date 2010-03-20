#include <InternalTypes.h>


using namespace dlvhex;
using namespace merging;
using namespace dlvhex::merging::plugin;

HexCall::HexCall(CallType ct, std::string prog, std::string args) : type(ct), program(prog), arguments(args){
}

HexCall::HexCall(CallType ct, std::string opname, std::vector<int> as, OperatorArguments kv) : type(ct), program(opname), asParams(as), kvParams(kv){
}

bool HexCall::operator==(const HexCall &other) const{
	if (type != other.type) return false;
	switch (type){
		case HexProgram:
		case HexFile:
			// Check if the programs (or the program paths) and the command line arguments are equivalent
			if (program != other.program || arguments != other.arguments) return false;
			return true;
			break;

		case OperatorCall:
			// Check if the answer set arguments are passed in the same order
			for (int i = 0; i < asParams.size(); i++){
				if (asParams[i] != other.asParams[i]) return false;
			}
			// Check if the sets of key-value arguments are equivalent (order does not matter)
			// One direction
			for (OperatorArguments::const_iterator it = kvParams.begin(); it != kvParams.end(); it++){
				bool found = false;
				for (OperatorArguments::const_iterator it2 = other.kvParams.begin(); it2 != other.kvParams.end(); it2++){
					if ((*it) == (*it2)){
						found = true;
						break;
					}
				}
				if (!found) return false;
			}
			// Other direction
			for (OperatorArguments::const_iterator it = other.kvParams.begin(); it != other.kvParams.end(); it++){
				bool found = false;
				for (OperatorArguments::const_iterator it2 = kvParams.begin(); it2 != kvParams.end(); it2++){
					if ((*it) == (*it2)){
						found = true;
						break;
					}
				}
				if (!found) return false;
			}
			return true;
			break;

		default:
			assert(0);
			break;
	}
}
