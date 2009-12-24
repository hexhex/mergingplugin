#include <UnfoldOp.h>
#include <DecisionDiagram.h>

#include <sstream>
#include <set>

using namespace dlvhex::asp;

DecisionDiagram UnfoldOp::unfold(DecisionDiagram::Node* root, DecisionDiagram& ddin){

	DecisionDiagram ddResult;
	DecisionDiagram::Node* newRoot;
	if (dynamic_cast<DecisionDiagram::LeafNode*>(root) != NULL){
		// Just copy the leaf node and use it in a new decision diagram
		newRoot = ddResult.addNode(root);
		ddResult.setRoot(newRoot);
	}else{
		// Copy the current root node
		newRoot = ddResult.addNode(root);
		ddResult.setRoot(newRoot);

		// Unfold all child decision diagrams
		std::set<DecisionDiagram::Edge*> outEdges = root->getOutEdges();
		for (std::set<DecisionDiagram::Edge*>::iterator childIt = outEdges.begin(); childIt != outEdges.end(); childIt++){
			DecisionDiagram::Node *currentChild = (*childIt)->getTo();
			DecisionDiagram unfoldedChild = unfold(currentChild, ddin);

			// Avoid duplicate node names
			unfoldedChild.useUniqueLabels(&ddResult);
			ddResult.addDecisionDiagram(&unfoldedChild);

/*
			bool dupfound = false;
			std::set<DecisionDiagram::Node*> ccNodes = unfoldedChild.getNodes();
			std::set<DecisionDiagram::Edge*> ccEdges = unfoldedChild.getEdges();
			//    Check all nodes of the currently unfolded child
			for (std::set<DecisionDiagram::Node*>::iterator currentChildIt = ccNodes.begin(); currentChildIt != ccNodes.end(); currentChildIt++){
				std::string originalname;
				int appendixctr = 1;
				dupfound = true;
				while (dupfound){
					dupfound = false;
					std::stringstream newname;
					//   Compare each of them with all nodes of the (partial) result
					std::set<DecisionDiagram::Node*> resultnodes = ddResult.getNodes();
					for (std::set<DecisionDiagram::Node*>::iterator resultIt = resultnodes.begin(); resultIt != resultnodes.end(); resultIt++){
						if ((*currentChildIt)->getLabel() == (*resultIt)->getLabel()){
							if (appendixctr == 1) originalname = (*resultIt)->getLabel();
							else appendixctr++;

							// rename node
							newname << originalname << "_" << appendixctr;
							(*currentChildIt)->setLabel(newname.str());

							// recheck for duplicates until a unique name was found
							dupfound = true;
							break;
						}
					}
				}
			}

			// A unique name was found:
			// Incorporate the child decision diagram into this one
			//   take all nodes ...
			for (std::set<DecisionDiagram::Node*>::iterator currentChildIt = ccNodes.begin(); currentChildIt != ccNodes.end(); currentChildIt++){
				ddResult.addNode(*currentChildIt);
			}
			//   ... and all edges from the child decision diagram
			for (std::set<DecisionDiagram::Edge*>::iterator currentChildIt = ccEdges.begin(); currentChildIt != ccEdges.end(); currentChildIt++){
				ddResult.addEdge(*currentChildIt);
			}
*/
		}

		// Connect the current root node with it's children
		std::set<DecisionDiagram::Edge*> rootEdges = root->getOutEdges();
		for (std::set<DecisionDiagram::Edge*>::iterator rootEdgeIt = rootEdges.begin(); rootEdgeIt != rootEdges.end(); rootEdgeIt++){
			ddResult.addEdge(*rootEdgeIt);
		}
	}

	return ddResult;
}

HexAnswer UnfoldOp::apply(int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters){
	try{
		HexAnswer result;
		for (int answerSetNr = 0; answerSetNr < arguments[0]->size(); answerSetNr++){
			// Construct a decision diagram from the answer set
			AtomSet as = (*arguments[0])[answerSetNr];
			DecisionDiagram dd(as);

			// Check for cycles in the input decision diagram
			std::vector<DecisionDiagram::Node*> cycle;
			if (!(cycle = dd.containsCycles()).empty()){
				std::stringstream cyclestring;
				bool first = true;
				for (std::vector<DecisionDiagram::Node*>::iterator it = cycle.begin(); it != cycle.end(); it++){
					cyclestring << (first ? "" : ", ") << (*it)->getLabel();
					first = false;
				}
				throw DecisionDiagram::InvalidDecisionDiagram("Cycle detected: " + cyclestring.str());
			}

			// Unfold the decision diagram
			dd = unfold(dd.getRoot(), dd);
			// The unfolded decision diagram does not need to be checked for cycles.
			// Since unfolding is equivalence preserving, there can be no cycles if there were none in the input decision diagram.

			// Translate the decision diagram back into an answer set
			result.push_back(dd.toAnswerSet());
		}
		return result;
	}catch(DecisionDiagram::InvalidDecisionDiagram idde){
		throw IOperator::OperatorException(std::string("InvalidDecisionDiagram: ") + idde.getMessage());
	}
}
