#include <ToBinaryDecisionTreeOp.h>
#include <DecisionDiagram.h>

#include <sstream>
#include <set>

#include <iostream>

using namespace dlvhex::asp;

void ToBinaryDecisionTreeOp::toBinary(DecisionDiagram* dd, DecisionDiagram::Node* root){

	// Check arity of root node
	if (root->getOutEdgesCount() > 2){
		bool firstOutEdge = true;
		
		// Create a new intermediate node
		DecisionDiagram::Node* intermediateNode = dd->addNode(dd->getUniqueLabel(root->getLabel()));

		std::set<DecisionDiagram::Edge*> oedges = root->getOutEdges();
		for (std::set<DecisionDiagram::Edge*>::iterator it = oedges.begin(); it != oedges.end(); it++){
			// Just take the first conditional edge as it is
			if (dynamic_cast<DecisionDiagram::ElseEdge*>(*it) == NULL && firstOutEdge){
				firstOutEdge = false;
			}else{
				// All other edges are redirected to the intermediate node
				DecisionDiagram::Node *currentSubNode = (*it)->getTo();
				if (dynamic_cast<DecisionDiagram::ElseEdge*>(*it) != NULL){
					dd->addElseEdge(intermediateNode, currentSubNode);
				}else{
					dd->addEdge(intermediateNode, currentSubNode, (*it)->getCondition());
				}
				// Remove the original edge
				dd->removeEdge(*it);

				// Convert the sub decision diagram into a binary one
				toBinary(dd, (*it)->getTo());
			}
		}

		// Let the intermediate node be the root's else child
		DecisionDiagram::Edge *e = dd->addElseEdge(root, intermediateNode);
	}
}

HexAnswer ToBinaryDecisionTreeOp::apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters){

	try{
		// Check arity
		if (arity != 1){
			std::stringstream msg;
			msg << "tobinarydecisiontree is a unary operator. " << arity << " answers were passed.";
			throw IOperator::OperatorException(msg.str());
		}
		if (answers[0]->size() != 1){
			std::stringstream msg;
			msg << "majorityvoting expects the answer to contain exactly one answer set.";
			throw IOperator::OperatorException(msg.str());
		}

		// Construct input decision diagram
		DecisionDiagram dd((*answers[0])[0]);

		// Check preconditions
		if (!dd.isTree()){
			throw IOperator::OperatorException("tobinarydecisiontree expects a decision tree, but the given decision diagram is not a tree.");
		}

		// Convert it into a binary one
		toBinary(&dd, dd.getRoot());

		// Convert the final decision diagram into a hex answer
		HexAnswer answer;
		answer.push_back(dd.toAnswerSet());
		return answer;
	}catch(DecisionDiagram::InvalidDecisionDiagram ide){
		throw IOperator::OperatorException(std::string("InvalidDecisionDiagram: ") + ide.getMessage());
	}
}

