#include <UnfoldOp.h>
#include <DecisionDiagram.h>

#include <iostream>
#include <sstream>

using namespace dlvhex::asp;

DecisionDiagram UnfoldOp::unfold(DecisionDiagram::Node* root, DecisionDiagram& ddin){
	static int rv = 0;
	rv++;

	// Leaf nodes are always unfolded by definition
	DecisionDiagram ddResult;
	if (dynamic_cast<DecisionDiagram::LeafNode*>(root) != NULL){
		// Just copy the leaf node and use it in a new decision diagram
		DecisionDiagram::LeafNode* copy = new DecisionDiagram::LeafNode(*dynamic_cast<DecisionDiagram::LeafNode*>(root));
		ddResult.addNode(copy);
		ddResult.setRoot(copy);
	}else{
		// Copy the current root node
		DecisionDiagram::Node* rootcopy = new DecisionDiagram::Node(*root);
		ddResult.addNode(rootcopy);
		ddResult.setRoot(rootcopy);

		// Unfold all child decision diagrams
		std::vector<DecisionDiagram::Node*> children = ddin.getChildren(root);
		for (int cc = 0; cc < children.size(); cc++){
std::cout << "Unfolding " << children[cc]->getLabel() << " from " << root->getLabel() << std::endl;
			DecisionDiagram unfoldedChild = unfold(children[cc], ddin);

			// Avoid duplicate node names
			bool dupfound = false;
			for (int i = 0; i < unfoldedChild.nodeCount(); i++){
				std::string originalname;
				int appendixctr = 1;
				dupfound = true;
				while (dupfound){
					dupfound = false;
					std::stringstream newname;
					for (int i2 = 0; i2 < ddResult.nodeCount(); i2++){
						if (unfoldedChild.getNode(i)->getLabel() == ddResult.getNode(i2)->getLabel()){
							if (appendixctr == 1) originalname = ddResult.getNode(i2)->getLabel();
							else appendixctr++;

							// rename node
							newname << originalname << "_" << appendixctr;
							unfoldedChild.getNode(i)->setLabel(newname.str());

							// recheck for duplicates until a unique name was found
							dupfound = true;
							break;
						}
					}
				}
			}

			// Incorporate the child decision diagram into this one
			//   take all nodes ...
			for (int nc = 0; nc < unfoldedChild.nodeCount(); nc++){
				if (dynamic_cast<DecisionDiagram::LeafNode*>(unfoldedChild.getNode(nc)) != NULL){
					DecisionDiagram::LeafNode* child = dynamic_cast<DecisionDiagram::LeafNode*>(unfoldedChild.getNode(nc));
					ddResult.addNode(new DecisionDiagram::LeafNode(child->getLabel(), child->getClassification()));
				}else{
					DecisionDiagram::Node* child = unfoldedChild.getNode(nc);
					ddResult.addNode(new DecisionDiagram::Node(child->getLabel()));
				}
			}
			//   ... and all edges from the child decision diagram
			for (int ec = 0; ec < unfoldedChild.edgeCount(); ec++){
				DecisionDiagram::Edge *edge = unfoldedChild.getEdge(ec);
				if (dynamic_cast<DecisionDiagram::ElseEdge*>(edge) != NULL){
					ddResult.addEdge(new DecisionDiagram::ElseEdge(ddResult.getNodeByLabel(edge->getV1()->getLabel()), ddResult.getNodeByLabel(edge->getV2()->getLabel())));
				}else{
					ddResult.addEdge(new DecisionDiagram::Edge(ddResult.getNodeByLabel(edge->getV1()->getLabel()), ddResult.getNodeByLabel(edge->getV2()->getLabel()), edge->getCondition()));
				}
			}
		}

		// Connect the current root node with it's children
		for (int i = 0; i < ddin.edgeCount(); i++){
			DecisionDiagram::Edge *edge = ddin.getEdge(i);
			// Only outgoing edges of the current root node
			if (edge->getV1()->getLabel() == rootcopy->getLabel()){		
				if (dynamic_cast<DecisionDiagram::ElseEdge*>(edge) != NULL){
					ddResult.addEdge(new DecisionDiagram::ElseEdge(rootcopy, ddResult.getNodeByLabel(edge->getV2()->getLabel())));
				}else{
					ddResult.addEdge(new DecisionDiagram::Edge(rootcopy, ddResult.getNodeByLabel(edge->getV2()->getLabel()), edge->getCondition()));
				}
			}
		}
	}
	return ddResult;
}

HexAnswer UnfoldOp::apply(int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters){
	HexAnswer result;
	for (int answerSetNr = 0; answerSetNr < arguments[0]->size(); answerSetNr++){
		// Construct a decision diagram from the answer set
		AtomSet as = (*arguments[0])[answerSetNr];
		DecisionDiagram dd(as);
		// Unfold the decision diagram
		dd = unfold(dd.getRoot(), dd);
		// Translate the decision diagram back into an answer set
		result.push_back(dd.toAnswerSet());
	}
	return result;
}
