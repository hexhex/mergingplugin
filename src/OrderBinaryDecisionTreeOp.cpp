#include <OrderBinaryDecisionTreeOp.h>
#include <DecisionDiagram.h>

#include <iostream>
#include <sstream>
#include <set>

using namespace dlvhex::asp;

std::string OrderBinaryDecisionTreeOp::getCompareAttribute(DecisionDiagram::Node* node){
	// Search for the requested attribute in the outgoing edges of this node
	std::string attr;
	bool attrFound = false;
	std::set<DecisionDiagram::Edge*> oedges = node->getOutEdges();
	for (std::set<DecisionDiagram::Edge*>::iterator it = oedges.begin(); it != oedges.end(); it++){
		DecisionDiagram::Condition condition = (*it)->getCondition();
		if (condition.getOperation() != DecisionDiagram::Condition::else_){
			if (attrFound){
				std::stringstream msg;
				msg << "Node \"" << node->getLabel() << "\" has more than one outgoing conditional edge";
				throw IOperator::OperatorException(msg.str());
			}
			attr = condition.getOperand1();
			attrFound = true;
		}
	}
	if (!attrFound){
		std::stringstream msg;
		msg << "Node \"" << node->getLabel() << "\" has no outgoing conditional edge";
		throw IOperator::OperatorException(msg.str());
	}
	return attr;
}

DecisionDiagram::Node* OrderBinaryDecisionTreeOp::sink(DecisionDiagram* dd, DecisionDiagram::Node* root){
	// We only work with binary decision trees
	if (root->getOutEdgesCount() != 2){
		std::stringstream msg;
		msg << "Error: Decision tree is not binary. Node \"" << root->getLabel() << "\" has " << root->getOutEdgesCount() << " outgoing edges.";
		throw IOperator::OperatorException(msg.str());
	}
//std::cout << root->getLabel() << std::endl;
	// Check if root needs to be exchanged with one of it's children
	std::set<DecisionDiagram::Edge*> iedges = root->getInEdges();
	std::set<DecisionDiagram::Edge*> oedges = root->getOutEdges();
	int childNr = 0;
	int echildNr;
	DecisionDiagram::Node* exchangechild = NULL;
	for (std::set<DecisionDiagram::Edge*>::iterator it = oedges.begin(); it != oedges.end(); it++){
		DecisionDiagram::Node* child = (*it)->getTo();	// Assumption: child is already ordered!

		if (dynamic_cast<DecisionDiagram::LeafNode*>(child) == NULL && getCompareAttribute(root).compare(getCompareAttribute(child)) > 0 &&	// Exchange inner nodes only
			(exchangechild == NULL || getCompareAttribute(child).compare(getCompareAttribute(exchangechild)) < 0)){				// Exchange with the smallest child
			exchangechild = child;
			echildNr = childNr;
		}
		childNr++;
	}

	if (exchangechild != NULL){
		// Root needs to be exchanged with this child

		//              root                                child
		//        (c1)/      \(c2)       ===>       (c3)/          \(c4)
		//        sibling    child                  root            root-copy
		//              (c3)/     \(c4)        (c1)/    \(c2)  (c1)/         \(c2)
		//          resttree1    resttree2   sibling resttree1  sibling-copy  resttree2

		// First, use human readable names for the nodes of interest
//std::cout << "1" << std::endl;
		DecisionDiagram::Node* newroot = exchangechild;
		DecisionDiagram::Node* sibling1root = root->getChild(1 - echildNr);				// Take the other child
		DecisionDiagram::Condition siblingcondition = root->getOutEdge(1 - echildNr)->getCondition();	// c1
		DecisionDiagram::Condition currentchildcondition = root->getOutEdge(echildNr)->getCondition();	// c2
		DecisionDiagram siblingCopy;									// Copy the sub-tree since we need it twice
		siblingCopy.partialAddDecisionDiagram(dd, sibling1root);
		siblingCopy.useUniqueLabels(dd);
//std::cout << "{{" << siblingCopy.toString() << "}}" << std::endl;
		DecisionDiagram::Node* sibling2root = dd->addDecisionDiagram(&siblingCopy);
		DecisionDiagram::Node* newrootSuccessor1 = root;						// The old root is a successor (child) of the new root
		DecisionDiagram::Node* newrootSuccessor2 = dd->addNode(dd->getUniqueLabel(root->getLabel()));	// We need it twice
		DecisionDiagram::Node* resttree1root = newroot->getChild(0);
		DecisionDiagram::Condition resttree1condition = newroot->getOutEdge(0)->getCondition();		// c3
		DecisionDiagram::Node* resttree2root = newroot->getChild(1);
		DecisionDiagram::Condition resttree2condition = newroot->getOutEdge(1)->getCondition();		// c4
//std::cout << "2" << std::endl;

		// Cut out all edges from the old root to it's successors
		for (std::set<DecisionDiagram::Edge*>::iterator e = oedges.begin(); e != oedges.end(); e++) dd->removeEdge(*e);
		// Also cut ingoing edges
		for (std::set<DecisionDiagram::Edge*>::iterator e = iedges.begin(); e != iedges.end(); e++) dd->removeEdge(*e);
		// Cut out all edges from the child which becomes the new root root to it's successors
		std::set<DecisionDiagram::Edge*> newrootOutEdges = newroot->getOutEdges();
		for (std::set<DecisionDiagram::Edge*>::iterator e = newrootOutEdges.begin(); e != newrootOutEdges.end(); e++) dd->removeEdge(*e);
//std::cout << "3" << std::endl;

		// Create new connections
		dd->addEdge(newrootSuccessor1, sibling1root, siblingcondition);
//std::cout << "3a" << std::endl;
		dd->addEdge(newrootSuccessor1, resttree1root, currentchildcondition);
//std::cout << "3b " << (sibling2root) << std::endl;
		dd->addEdge(newrootSuccessor2, sibling2root, siblingcondition);
//std::cout << "3c" << std::endl;
		dd->addEdge(newrootSuccessor2, resttree2root, currentchildcondition);
//std::cout << "4" << std::endl;

		// Sink the new root successors since sinking might goes on further
		DecisionDiagram::Node* subtree1root = sink(dd, newrootSuccessor1);
		DecisionDiagram::Node* subtree2root = sink(dd, newrootSuccessor2);
//std::cout << "5" << std::endl;

		// Redirect the edges to the new subtree roots (since they might have changed)
		dd->addEdge(newroot, subtree1root, resttree1condition);
		dd->addEdge(newroot, subtree2root, resttree2condition);
//std::cout << "6" << std::endl;

		return newroot;
	}else{
		return root;
	}
}

DecisionDiagram::Node* OrderBinaryDecisionTreeOp::order(DecisionDiagram* dd, DecisionDiagram::Node* root){

	// First of all order the sub-trees (if root is a leaf, it is already sorted)
	// The overall algorithm is very similar to heap sort, even if the sinking procedure (especially redirecting the pointers between the nodes)
	// is more complicated in this case, since we must deal with conditional and unconditional edges.
	if (dynamic_cast<DecisionDiagram::LeafNode*>(root) == NULL){

		// root is an inner node

		std::set<DecisionDiagram::Edge*> oedges = root->getOutEdges();
		for (std::set<DecisionDiagram::Edge*>::iterator it = oedges.begin(); it != oedges.end(); it++){
			DecisionDiagram::Node* child = (*it)->getTo();

			// Redirect the edges to the new subtree roots (since they might have changed)
			DecisionDiagram::Condition condition = (*it)->getCondition();
			dd->removeEdge(*it);

			DecisionDiagram::Node* subtree = order(dd, child);

			dd->addEdge(root, subtree, condition);
		}

		// Now sink the current root node and return the new root
		return sink(dd, root);
	}
}

HexAnswer OrderBinaryDecisionTreeOp::apply(int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters){

	try{
		// Check arity
		if (arity != 1){
			std::stringstream msg;
			msg << "orderbinarydecisiontree is a unary operator. " << arity << " answers were passed.";
			throw IOperator::OperatorException(msg.str());
		}
		if (answers[0]->size() != 1){
			std::stringstream msg;
			msg << "orderbinarydecisiontree expects the answer to contain exactly one answer set.";
			throw IOperator::OperatorException(msg.str());
		}

		// Construct input decision tree
		DecisionDiagram dd((*answers[0])[0]);

		// Check preconditions
		if (!dd.isTree()){
			throw IOperator::OperatorException("orderbinarydecisiontree expects a decision tree, but the given decision diagram is not a tree.");
		}

		// Order the nodes
		order(&dd, dd.getRoot());

		// Convert the final decision diagram into a hex answer
		HexAnswer answer;
		answer.push_back(dd.toAnswerSet());
		return answer;
	}catch(DecisionDiagram::InvalidDecisionDiagram ide){
		throw IOperator::OperatorException(std::string("InvalidDecisionDiagram: ") + ide.getMessage());
	}

}
