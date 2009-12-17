#include <UnionOp.h>

using namespace dlvhex::asp;

HexAnswer UnionOp::apply(int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters){
	HexAnswer result;
	dlvhex::AtomSet as1;
	dlvhex::AtomSet as2;
	result.push_back(as1);
	result.push_back(as2);
	return result;
}
