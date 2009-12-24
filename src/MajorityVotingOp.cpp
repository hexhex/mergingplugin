#include <MajorityVotingOp.h>
#include <DecisionDiagram.h>

#include <iostream>
#include <sstream>
#include <set>

using namespace dlvhex::asp;

DecisionDiagram::Node* MajorityVotingOp::makeInnerNode(DecisionDiagram* dd, DecisionDiagram::Node* n_old){
//std::cout << "Changing " << n_old->getLabel() << " to inner node" << std::endl;

	// Find a unique temporary name for the old leaf node and rename it (this makes the old label free for the inner node)
	std::string leaflabel = n_old->getLabel();
	std::string tmpleaflabel = dd->getUniqueLabel(leaflabel);
//std::cout << "Temporary label is " << tmpleaflabel << std::endl;
	n_old->setLabel(tmpleaflabel);

	// Add the new inner node
//std::cout << "Adding inner node " << leaflabel << std::endl;
	DecisionDiagram::Node* innernode = dd->addNode(leaflabel);
//std::cout << "Added inner node " << innernode->getLabel() << std::endl;

	// Extract all edges into n_old or out of n_old
	std::set<DecisionDiagram::Edge*> in = n_old->getInEdges();
	std::set<DecisionDiagram::Edge*> out = n_old->getOutEdges();	// Should be empty since leaf nodes have no outgoing edges
//std::cout << "Adding edges" << std::endl;

	// Copy all edges and redirect them to the new inner node
	for (std::set<DecisionDiagram::Edge*>::iterator it = in.begin(); it != in.end(); it++){
		DecisionDiagram::Edge* e = *it;
		if (dynamic_cast<DecisionDiagram::ElseEdge*>(e) != NULL){
			dd->addElseEdge(e->getFrom(), innernode);
		}else{
			dd->addEdge(e->getFrom(), innernode, e->getCondition());
		}
	}
	for (std::set<DecisionDiagram::Edge*>::iterator it = out.begin(); it != out.end(); it++){
		DecisionDiagram::Edge* e = *it;
		if (dynamic_cast<DecisionDiagram::ElseEdge*>(e) != NULL){
			dd->addElseEdge(innernode, e->getTo());
		}else{
			dd->addEdge(innernode, e->getTo(), e->getCondition());
		}
	}

	// Remove the old node from the decision diagram (force edge removal)
	dd->removeNode(n_old, true);

	return innernode;
}

HexAnswer MajorityVotingOp::apply(int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters){
	try{
		// Check arity
		if (arity != 2){
			std::stringstream msg;
			msg << "majorityvoting is a binary (2-ary) operator. " << arity << " answers were passed.";
			throw IOperator::OperatorException(msg.str());
		}
		if (arguments[0]->size() != 1 || arguments[1]->size() != 1){
			std::stringstream msg;
			msg << "majorityvoting expects each answer to contain exactly one answer set.";
			throw IOperator::OperatorException(msg.str());
		}
//std::cout << "1" << std::endl;
		// Construct input decision diagrams
		DecisionDiagram dd1((*arguments[0])[0]);
//std::cout << "." << std::endl;
		DecisionDiagram dd2((*arguments[1])[0]);
//std::cout << "2" << std::endl;

		// Merge the decision diagrams
		// Extract all distingt leaf nodes of dd1
		std::set<DecisionDiagram::LeafNode*> dd1leafs = dd1.getLeafNodes();
//std::cout << "3" << std::endl;

		// For each distinct leaf node (according to it's classification), make a copy of dd2
		std::map<std::string, DecisionDiagram> dd2copies;
		for (std::set<DecisionDiagram::LeafNode*>::iterator it = dd1leafs.begin(); it != dd1leafs.end(); it++){
			// Check if this classification is new
			if (dd2copies.find((*it)->getClassification()) == dd2copies.end()){
				// Make a copy of dd2
//std::cout << "Making a copy of dd2 for dd1-leaf " << (*it)->getClassification() << std::endl;
				dd2copies[(*it)->getClassification()] = DecisionDiagram(dd2);
			}
		}

		// Add all dd2 copies to the main decision diagram
		for (std::map<std::string, DecisionDiagram>::iterator copyIt = dd2copies.begin(); copyIt != dd2copies.end(); copyIt++){

//std::cout << "Using unique labels in dd2 copy for " << (*copyIt).first << std::endl;
			// Rename nodes if necessary
			(*copyIt).second.useUniqueLabels(&dd1);
//std::cout << "DONE using unique labels in dd2 copy for " << (*copyIt).first << std::endl;
//std::cout << "Iterating through leafs of dd2 with classification different from " << (*copyIt).first << std::endl;

			// All leafs of dd2 which come to the same classification as the leaf node in dd1 will remain unchanged; others are changed to "unknown"
			std::set<DecisionDiagram::LeafNode*> dd2leafs = (*copyIt).second.getLeafNodes();
			for (std::set<DecisionDiagram::LeafNode*>::iterator dd2it = dd2leafs.begin(); dd2it != dd2leafs.end(); dd2it++){
//std::cout << "Checking " << (*dd2it)->getClassification() << std::endl;
				if ((*dd2it)->getClassification() != (*copyIt).first){
//std::cout << "Found -> changing to unknown" << std::endl;
					(*dd2it)->setClassification("unknown");
				}
			}

			// Add
//std::cout << "Adding dd2 copy for dd1-leaf " << (*copyIt).first << std::endl;
			DecisionDiagram::Node* dd2root = dd1.addDecisionDiagram(&(*copyIt).second);
//std::cout << "dd2 root is " << dd2root->getLabel() << std::endl;

//std::cout << "Connecting dd1 leafs with classification " << (*copyIt).first << " to dd2 root " << std::endl;
			// Connect all leafs of dd1 with the according classification to the root element of dd2
			for (std::set<DecisionDiagram::LeafNode*>::iterator dd1it = dd1leafs.begin(); dd1it != dd1leafs.end(); dd1it++){
//std::cout << "Checking " << (*dd1it)->getClassification() << std::endl;
				if ((*dd1it)->getClassification() == (*copyIt).first){
//std::cout << "Found - changing to inner node" << std::endl;
					// Convert leaf node into an inner node
					DecisionDiagram::Node* innernode = makeInnerNode(&dd1, *dd1it);
//std::cout << "Done: Connecting" << std::endl;
					// Connect it with the root of dd2
					dd1.addElseEdge(innernode, dd2root);
//					dd1.addElseEdge(dd1.getNodeByLabel(innernode->getLabel()), dd1.getNodeByLabel(dd2root->getLabel()));
//std::cout << "Connected" << std::endl;
				}else{
//std::cout << "not found" << std::endl;
				}
			}
		}


//std::cout << "DONE!" << std::endl;

		// Convert the final decision diagram into a hex answer
		HexAnswer answer;
		answer.push_back(dd1.toAnswerSet());
		return answer;

	}catch(DecisionDiagram::InvalidDecisionDiagram idde){
		throw IOperator::OperatorException(std::string("InvalidDecisionDiagram: ") + idde.getMessage());
	}
}
