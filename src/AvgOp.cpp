#include <AvgOp.h>
#include <DecisionDiagram.h>

#include <iostream>
#include <sstream>
#include <set>

using namespace dlvhex::asp;

std::string AvgOp::unquote(std::string str){
	if (str.length() >= 2 && str[0] == '\"' && str[str.length() - 1] == '\"') return str.substr(1, str.length() - 2);
	else return str;
}

DecisionDiagram::Condition AvgOp::getCondition(DecisionDiagram::Node* node){
	// Search for the requested condition in the outgoing edges of this node
	DecisionDiagram::Condition c("", "", DecisionDiagram::Condition::else_);
	bool cFound = false;
	std::set<DecisionDiagram::Edge*> oedges = node->getOutEdges();
	for (std::set<DecisionDiagram::Edge*>::iterator it = oedges.begin(); it != oedges.end(); it++){
		DecisionDiagram::Condition condition = (*it)->getCondition();
		if (condition.getOperation() != DecisionDiagram::Condition::else_){
			if (cFound){
				std::stringstream msg;
				msg << "Node \"" << node->getLabel() << "\" has more than one outgoing conditional edge";
				throw IOperator::OperatorException(msg.str());
			}
			c = condition;
			cFound = true;
		}
	}
	if (!cFound){
		std::stringstream msg;
		msg << "Node \"" << node->getLabel() << "\" has no outgoing conditional edge";
		throw IOperator::OperatorException(msg.str());
	}
	return c;
}

DecisionDiagram::Node* AvgOp::average(DecisionDiagram* result, DecisionDiagram* dd1, DecisionDiagram::Node* n1, DecisionDiagram* dd2, DecisionDiagram::Node* n2){

	// Check if one or both of the currently processed nodes is a leaf
	if (dynamic_cast<DecisionDiagram::LeafNode*>(n1) != NULL && dynamic_cast<DecisionDiagram::LeafNode*>(n2) != NULL){
//std::cout << "Merging two leafs" << std::endl;
		// Yes: Both are leafs

		DecisionDiagram::LeafNode* leaf1 = dynamic_cast<DecisionDiagram::LeafNode*>(n1);
		DecisionDiagram::LeafNode* leaf2 = dynamic_cast<DecisionDiagram::LeafNode*>(n2);

		// Check if they coincide
		if (leaf1->getClassification() == leaf2->getClassification()){
			// Yes: Add the same classification to the final result
			return result->addLeafNode(result->getUniqueLabel(leaf1->getLabel()), leaf1->getClassification());
		}else{
			// No: Classifications are contradictory
			return result->addLeafNode(result->getUniqueLabel(leaf1->getLabel()), std::string("unknown"));
		}
	}else if (dynamic_cast<DecisionDiagram::LeafNode*>(n1) != NULL || dynamic_cast<DecisionDiagram::LeafNode*>(n2) != NULL){
		// Yes: One is leaf, one is non-leaf

//std::cout << "Merging an inner node with a leaf" << std::endl;
		DecisionDiagram::Node* nonleaf = (dynamic_cast<DecisionDiagram::LeafNode*>(n1) != NULL ? n2 : n1);
		DecisionDiagram* nonleafdiag = (dynamic_cast<DecisionDiagram::LeafNode*>(n1) != NULL ? dd2 : dd1);

		// Insert the non-leaf node into the final result
		DecisionDiagram::Node* root = result->addNode(result->getUniqueLabel(nonleaf->getLabel()));

		// Further pass the request to all children of the non-leaf
		std::set<DecisionDiagram::Edge*> oedges = nonleaf->getOutEdges();
		for (std::set<DecisionDiagram::Edge*>::iterator it = oedges.begin(); it != oedges.end(); it++){
			DecisionDiagram::Node* subdiagramRoot = average(result, nonleafdiag, (*it)->getTo(), nonleafdiag == dd1 ? dd2 : dd1, nonleaf == n1 ? n2 : n1);

			// Connect the currently inserted node with the sub-diagram
			result->addEdge(root, subdiagramRoot, (*it)->getCondition());
		}

		return root;
	}else{
		// No: Both are inner nodes

		DecisionDiagram::Condition c1 = getCondition(n1);
		DecisionDiagram::Condition c2 = getCondition(n2);

		// Check if n1 and n2 both query the same variable
		if (c1.getOperand1() == c2.getOperand1()){
			// Average

//std::cout << "Merging two inner nodes with the same operands" << std::endl;
			// Both nodes request the same variable

			// Merge the sub-trees. The else-sides and the conditional edges are merged accordingly.
			std::set<DecisionDiagram::Edge*> n1edges = n1->getOutEdges();
			std::set<DecisionDiagram::Edge*> n2edges = n2->getOutEdges();
			DecisionDiagram::Node* else_subtree_1;
			DecisionDiagram::Node* conditional_subtree_1;
			DecisionDiagram::Node* else_subtree_2;
			DecisionDiagram::Node* conditional_subtree_2;
			for (std::set<DecisionDiagram::Edge*>::iterator it = n1edges.begin(); it != n1edges.end(); it++)
				if ((*it)->getCondition().getOperation() == DecisionDiagram::Condition::else_)	else_subtree_1 = (*it)->getTo();
				else										conditional_subtree_1 = (*it)->getTo();
			for (std::set<DecisionDiagram::Edge*>::iterator it = n2edges.begin(); it != n2edges.end(); it++)
				if ((*it)->getCondition().getOperation() == DecisionDiagram::Condition::else_)	else_subtree_2 = (*it)->getTo();
				else										conditional_subtree_2 = (*it)->getTo();
//std::cout << "         --> merging children: " << else_subtree_1->getLabel() << " with " << else_subtree_2->getLabel() << " (else), resp. " << conditional_subtree_1->getLabel() << " with " << conditional_subtree_2->getLabel() << " (conditional)" << std::endl;

			DecisionDiagram::Node* else_subtree = average(result, dd1, else_subtree_1, dd2, else_subtree_2);
			DecisionDiagram::Node* conditional_subtree = average(result, dd1, conditional_subtree_1, dd2, conditional_subtree_2);

			// Now the conditions must be merged
std::cerr << "Converting " << c1.getOperand2() << std::endl;
std::cerr << "Converting " << c2.getOperand2() << std::endl;
			double o1 = atof(unquote(c1.getOperand2()).c_str());
			double o2 = atof(unquote(c2.getOperand2()).c_str());
std::cerr << "--> " << o1 << ", " << o2 << std::endl;
			double o3 = (o1 + o2) / 2;
std::cerr << o3 << std::endl;
			std::stringstream op3ss;
			op3ss << o3;
			DecisionDiagram::Condition merged_condition = DecisionDiagram::Condition(c1.getOperand1(), op3ss.str(), c1.getOperation());

			// If both second operands are numbers and the condition operators are equal, we just take the average
			DecisionDiagram::Node* root = result->addNode(result->getUniqueLabel(n1->getLabel()));

			// Finally, connect the root with it's subtrees
			result->addEdge(root, conditional_subtree, merged_condition);
			result->addElseEdge(root, else_subtree);

//std::cout << "SHOULD MERGE " << c1.toString() << " vs. " << c2.toString() << std::endl;
		}else{
//std::cout << "Merging two inner nodes with different operands: " << c1.toString() << " vs. " << c2.toString() << std::endl;

			// No: Since the input decision diagrams (trees) can savely assumed to be ordered,
			// we know for sure that the lexically smaller one of the compared attributes does not occur in the other decision diagram
			// Just merge each child of the decision node with the lexically smaller attribute with the other decision diagram. The larger one may occurs somewhere below.

			// Example:
			//
			//    [A]               [B]
			//   /   \             /   \
			// ...   ...         ...   ...
			//  I     II
			//
			// We know, that A is never requested in the right decision diagram. Since B is already requested, and the diagram is ordered, there is no chance to request A below.
			// So we savely merge the right decision diagram with the children of the left diagram (I and II), since B can occur there.

			DecisionDiagram* smallerDD;
			DecisionDiagram* largerDD;
			DecisionDiagram::Node* smallerN;
			DecisionDiagram::Node* largerN;
			if (c1.getOperand1().compare(c2.getOperand1()) < 0){	// c1 is lexically smaller than c2
				smallerDD = dd1;
				smallerN = n1;
				largerDD = dd2;
				largerN = n2;
			}else{							// c2 is lexically smaller than c1
				smallerDD = dd2;
				smallerN = n2;
				largerDD = dd1;
				largerN = n1;
			}

			// Insert the smaller node into the final result
			DecisionDiagram::Node* root = result->addNode(result->getUniqueLabel(smallerN->getLabel()));

			// Merge the larger one with all children of the smaller node
			std::set<DecisionDiagram::Edge*> smaller_oedges = smallerN->getOutEdges();
			for (std::set<DecisionDiagram::Edge*>::iterator it = smaller_oedges.begin(); it != smaller_oedges.end(); it++){
				DecisionDiagram::Node* subdiagramRoot = average(result, smallerDD, (*it)->getTo(), largerDD, largerN);

				// Connect the currently inserted node with the sub-diagram
				result->addEdge(root, subdiagramRoot, (*it)->getCondition());
			}

			return root;
		}
	}
}

HexAnswer AvgOp::apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters){
	try{
		// Check arity
		if (arity != 2){
			std::stringstream msg;
			msg << "average is a binary (2-ary) operator. " << arity << " answers were passed.";
			throw IOperator::OperatorException(msg.str());
		}
		if (answers[0]->size() != 1 || answers[1]->size() != 1){
			std::stringstream msg;
			msg << "average expects each answer to contain exactly one answer set.";
			throw IOperator::OperatorException(msg.str());
		}

		// Construct input decision diagrams
		DecisionDiagram dd1((*answers[0])[0]);
		DecisionDiagram dd2((*answers[1])[0]);

		// Merge the diagrams
		DecisionDiagram result;
		average(&result, &dd1, dd1.getRoot(), &dd2, dd2.getRoot());

		// Convert the final decision diagram into a hex answer
		HexAnswer answer;
		answer.push_back(result.toAnswerSet());
		return answer;
	}catch(DecisionDiagram::InvalidDecisionDiagram idde){
		throw IOperator::OperatorException(std::string("average: ") + idde.getMessage());
	}
}
