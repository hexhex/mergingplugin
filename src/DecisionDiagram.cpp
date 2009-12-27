#include <DecisionDiagram.h>

#include <map>
#include <stack>
#include <iostream>
#include <sstream>

using namespace dlvhex::asp;


// ------------------------------ InvalidDecisionDiagram ------------------------------

DecisionDiagram::InvalidDecisionDiagram::InvalidDecisionDiagram(std::string m) : msg(m){
}

std::string DecisionDiagram::InvalidDecisionDiagram::getMessage(){
	return msg;
}


// ------------------------------ Node ------------------------------

DecisionDiagram::Node::Node(std::string l) : label(l){
}

DecisionDiagram::Node::~Node(){
}

void DecisionDiagram::Node::setLabel(std::string l){
	label = l;
}

std::string DecisionDiagram::Node::getLabel() const{
	return label;
}

void DecisionDiagram::Node::addEdge(Edge* e){
	if (e->getFrom() == this) outEdges.insert(e);
	else if (e->getTo() == this) inEdges.insert(e);
	else throw InvalidDecisionDiagram(std::string("Tried to add edge to node. Error: Node \"") + getLabel() + std::string("\" is not an endpoint of the given edge (\"") + e->toString() + std::string("\")"));
}

void DecisionDiagram::Node::removeEdge(Edge* e){
	outEdges.erase(e);
	inEdges.erase(e);
}

std::set<DecisionDiagram::Edge*> DecisionDiagram::Node::getEdges() const{
	std::set<Edge*> edges;
	edges.insert(inEdges.begin(), inEdges.end());
	edges.insert(outEdges.begin(), outEdges.end());
	return edges;
}

std::set<DecisionDiagram::Edge*> DecisionDiagram::Node::getInEdges() const{
	return inEdges;
}

std::set<DecisionDiagram::Edge*> DecisionDiagram::Node::getOutEdges() const{
	return outEdges;
}

int DecisionDiagram::Node::getEdgesCount() const{
	return getInEdgesCount() + getOutEdgesCount();
}

int DecisionDiagram::Node::getInEdgesCount() const{
	return inEdges.size();
}

int DecisionDiagram::Node::getOutEdgesCount() const{
	return outEdges.size();
}

std::string DecisionDiagram::Node::toString() const{
	return label;
}


// ------------------------------ LeafNode ------------------------------

DecisionDiagram::LeafNode::LeafNode(std::string l, std::string c) : Node(l), classification(c){
}

DecisionDiagram::LeafNode::~LeafNode(){
}

std::string DecisionDiagram::LeafNode::getClassification(){
	return classification;
}


void DecisionDiagram::LeafNode::setClassification(std::string c){
	classification = c;
}

std::string DecisionDiagram::LeafNode::toString() const{
	return label + std::string("[") + classification + std::string("]");
}


// ------------------------------ Condition ------------------------------

DecisionDiagram::Condition::Condition(std::string operand1_, std::string operand2_, CmpOp operation_) : operand1(operand1_), operand2(operand2_), operation(operation_){
}

DecisionDiagram::Condition::Condition(std::string operand1_, std::string operand2_, std::string operation_) : operand1(operand1_), operand2(operand2_){
	operation = DecisionDiagram::Condition::stringToCmpOp(operation_);
}

DecisionDiagram::Condition::~Condition(){
}

std::string DecisionDiagram::Condition::getOperand1(){
	return operand1;
}

std::string DecisionDiagram::Condition::getOperand2(){
	return operand2;
}

DecisionDiagram::Condition::CmpOp DecisionDiagram::Condition::getOperation(){
	return operation;
}

DecisionDiagram::Condition::CmpOp DecisionDiagram::Condition::stringToCmpOp(std::string operation_){
	if (operation_ == std::string("<")) return Condition::lt;
	if (operation_ == std::string("<=")) return Condition::le;
	if (operation_ == std::string("=")) return Condition::eq;
	if (operation_ == std::string(">")) return Condition::ge;
	if (operation_ == std::string(">=")) return Condition::gt;
	throw InvalidDecisionDiagram(std::string("Unreconized operator: ") + operation_);
}

std::string DecisionDiagram::Condition::cmpOpToString(DecisionDiagram::Condition::CmpOp op){
	switch(op){
		case Condition::lt: return std::string("<");
		case Condition::le: return std::string("<=");
		case Condition::eq: return std::string("=");
		case Condition::ge: return std::string(">=");
		case Condition::gt: return std::string(">");
		default: assert(false);
	}
}

std::string DecisionDiagram::Condition::toString() const{
	return operand1 + cmpOpToString(operation) + operand2;
}

// ------------------------------ Edge ------------------------------

DecisionDiagram::Edge::Edge(Node *f, Node *t, Condition c) : from(f), to(t), condition(c){
}

DecisionDiagram::Edge::~Edge(){
}

DecisionDiagram::Node* DecisionDiagram::Edge::getFrom(){
	return from;
}

DecisionDiagram::Node* DecisionDiagram::Edge::getTo(){
	return to;
}

DecisionDiagram::Condition::Condition DecisionDiagram::Edge::getCondition(){
	return condition;
}

std::string DecisionDiagram::Edge::toString() const{
	return from->getLabel() + std::string(" --(") + condition.toString() + std::string(")--> ") + to->getLabel();
}


// ------------------------------ ElseEdge ------------------------------

DecisionDiagram::ElseEdge::ElseEdge(Node *u, Node *v) : Edge(u, v, Condition(std::string("none"), std::string("none"), Condition::le)){
}

DecisionDiagram::ElseEdge::~ElseEdge(){
}

std::string DecisionDiagram::ElseEdge::toString() const{
	return from->getLabel() + std::string(" --> ") + to->getLabel();
}


// ------------------------------ DecisionDiagram ------------------------------

DecisionDiagram::DecisionDiagram() : root(NULL){
}

DecisionDiagram::DecisionDiagram(const DecisionDiagram &dd2){
	this->operator=(dd2);
}

DecisionDiagram::DecisionDiagram(AtomSet as){

	// Create list of nodes
	//    inner nodes
	AtomSet innernodes;
	as.matchPredicate(std::string("innernode"), innernodes);
	for (AtomSet::const_iterator it = innernodes.begin(); it != innernodes.end(); it++){
		addNode(it->getArguments()[0].getUnquotedString());
	}

	//    leaf nodes
	AtomSet leafnodes;
	as.matchPredicate(std::string("leafnode"), leafnodes);
	for (AtomSet::const_iterator it = leafnodes.begin(); it != leafnodes.end(); it++){
		addLeafNode(it->getArguments()[0].getUnquotedString(), it->getArguments()[1].getUnquotedString());
	}

	// Create list of edges
	//    conditional edges
	AtomSet conditionaledges;
	as.matchPredicate(std::string("conditionaledge"), conditionaledges);
	for (AtomSet::const_iterator it = conditionaledges.begin(); it != conditionaledges.end(); it++){
		std::string n1 = it->getArguments()[0].getString();
		std::string n2 = it->getArguments()[1].getString();
		Condition::CmpOp cmpop;
		if (it->getArguments()[3].getUnquotedString() == std::string("<")) cmpop = Condition::lt;
		if (it->getArguments()[3].getUnquotedString() == std::string("<=")) cmpop = Condition::le;
		if (it->getArguments()[3].getUnquotedString() == std::string("=")) cmpop = Condition::eq;
		if (it->getArguments()[3].getUnquotedString() == std::string(">")) cmpop = Condition::ge;
		if (it->getArguments()[3].getUnquotedString() == std::string(">=")) cmpop = Condition::gt;
		Condition c(it->getArguments()[2].getString(), it->getArguments()[4].getString(), cmpop);
		addEdge(n1, n2, c);
	}

	//    else edges
	AtomSet elseedges;
	as.matchPredicate(std::string("elseedge"), elseedges);
	for (AtomSet::const_iterator it = elseedges.begin(); it != elseedges.end(); it++){
		std::string n1 = it->getArguments()[0].getUnquotedString();
		std::string n2 = it->getArguments()[1].getUnquotedString();
		addElseEdge(n1, n2);
	}

	// Root node
	AtomSet rootnode;
	as.matchPredicate(std::string("root"), rootnode);
	root = NULL;
	for (AtomSet::const_iterator it = rootnode.begin(); it != rootnode.end(); it++){
		if (root != NULL){
			throw InvalidDecisionDiagram(std::string("Error: Multiple roots specified (first one with label \"") + root->getLabel() + std::string("\", second one with label \"") + it->getArguments()[0].getUnquotedString() + std::string("\"."));
		}
		root = getNodeByLabel(it->getArguments()[0].getUnquotedString());
	}
}

DecisionDiagram::~DecisionDiagram(){

	// Cleanup
	clear();
}

void DecisionDiagram::clear(){

	// Delete all edges and nodes
	for (std::set<Edge*>::iterator it = edges.begin(); it != edges.end(); it++){
		delete *it;
	}
	for (std::set<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++){
		delete *it;
	}
	edges.erase(edges.begin(), edges.end());
	nodes.erase(nodes.begin(), nodes.end());
	root = NULL;
}

DecisionDiagram& DecisionDiagram::operator=(const DecisionDiagram &dd2){
	clear();

	std::set<Node*> dd2nodes = dd2.getNodes();
	std::set<Edge*> dd2edges = dd2.getEdges();

	for (std::set<Node*>::iterator it = dd2nodes.begin(); it != dd2nodes.end(); it++){
		addNode(*it);
	}
	for (std::set<Edge*>::iterator it = dd2edges.begin(); it != dd2edges.end(); it++){
		addEdge(*it);
	}
	if (dd2.getRoot() != NULL) this->root = getNodeByLabel(dd2.getRoot()->getLabel());
	return *this;
}

DecisionDiagram::Node* DecisionDiagram::addNode(std::string label){
	// Check if the label is unique
	for (std::set<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++){
		if ((*it)->getLabel() == label) throw InvalidDecisionDiagram(std::string("Tried to add node. Error: The label \"") + label + std::string("\" is not unique."));
	}
	Node *n = new Node(label);
	nodes.insert(n);
	return n;
}

DecisionDiagram::LeafNode* DecisionDiagram::addLeafNode(std::string label, std::string classification){
	LeafNode *n = new LeafNode(label, classification);
	nodes.insert(n);
	return n;
}

DecisionDiagram::Node* DecisionDiagram::addNode(DecisionDiagram::Node* template_){
	if (dynamic_cast<LeafNode*>(template_) != NULL){
		LeafNode *ln = dynamic_cast<LeafNode*>(template_);
		return addLeafNode(ln->getLabel(), ln->getClassification());
	}else{
		return addNode(template_->getLabel());
	}
}

DecisionDiagram::Edge* DecisionDiagram::addEdge(DecisionDiagram::Node* from, DecisionDiagram::Node* to, DecisionDiagram::Condition c){
	// Check if both endpoints of the edge are members of this decision diagram
	if (nodes.find(from) == nodes.end() || nodes.find(to) == nodes.end()){
		throw InvalidDecisionDiagram(std::string("Tried to add an edge from \"") + from->getLabel() + std::string("\" to \"") + to->getLabel() + std::string("\". Error: Both endpoints of an edge need to be part of the decision diagram before it can be added. ") + (nodes.find(from) == nodes.end() ? from->getLabel() : to->getLabel()) + std::string(" is not a member."));
	}else{
		// Create the edge
		Edge *e = new Edge(from, to, c);
		edges.insert(e);

		// Add it to it's inzident nodes
		from->addEdge(e);
		to->addEdge(e);

		return e;
	}
}

DecisionDiagram::Edge* DecisionDiagram::addEdge(std::string from, std::string to, DecisionDiagram::Condition c){
	addEdge(getNodeByLabel(from), getNodeByLabel(to), c);
}

DecisionDiagram::ElseEdge* DecisionDiagram::addElseEdge(DecisionDiagram::Node* from, DecisionDiagram::Node* to){
	// Check if both endpoints of the edge are members of this decision diagram
	if (nodes.find(from) == nodes.end() || nodes.find(to) == nodes.end()){
		throw InvalidDecisionDiagram(std::string("Tried to add an edge from \"") + from->getLabel() + std::string("\" to \"") + to->getLabel() + std::string("\". Error: Both endpoints of an edge need to be part of the decision diagram before it can be added. ") + (nodes.find(from) == nodes.end() ? from->getLabel() : to->getLabel()) + std::string(" is not a member."));
	}else{
		// Create the edge
		ElseEdge *e = new ElseEdge(from, to);
		edges.insert(e);

		// Add it to it's inzident nodes
		from->addEdge(e);
		to->addEdge(e);

		return e;
	}
}

DecisionDiagram::ElseEdge* DecisionDiagram::addElseEdge(std::string from, std::string to){
	addElseEdge(getNodeByLabel(from), getNodeByLabel(to));
}

DecisionDiagram::Edge* DecisionDiagram::addEdge(DecisionDiagram::Edge* template_){
	if (dynamic_cast<ElseEdge*>(template_) != NULL){
		ElseEdge *ee = dynamic_cast<ElseEdge*>(template_);
		return addElseEdge(getNodeByLabel(template_->getFrom()->getLabel()), getNodeByLabel(template_->getTo()->getLabel()));
	}else{
		return addEdge(getNodeByLabel(template_->getFrom()->getLabel()), getNodeByLabel(template_->getTo()->getLabel()), template_->getCondition());
	}
}

void DecisionDiagram::removeNode(DecisionDiagram::Node *n){
	removeNode(n, false);
}

void DecisionDiagram::removeNode(DecisionDiagram::Node *n, bool forceRemoveEdges){
	// Check if n is part of this decision diagram
	if (nodes.find(n) == nodes.end()) throw DecisionDiagram::InvalidDecisionDiagram(std::string("Tried to remove node \"") + n->getLabel() + std::string("\". Error: This node is not a member of the decision diagram."));

	// Check if the node to remove does not have any inzident edges
	if (!forceRemoveEdges && n->getEdgesCount() > 0){
		throw DecisionDiagram::InvalidDecisionDiagram(std::string("Tried to remove node \"") + n->getLabel() + std::string("\" Error: Nodes must not have any inzident edges in the decision diagrams in order to remove them."));
	}else{
		// Remove the node's inzident edges (if forceRemoveEdges is false, there will be none at this point)
		std::set<Edge*> edges = n->getEdges();
		for (std::set<Edge*>::iterator it = edges.begin(); it != edges.end(); it++){
			removeEdge(*it);
		}

		// Remove the node
		nodes.erase(n);
		delete n;
	}
}

void DecisionDiagram::removeEdge(DecisionDiagram::Edge* e){
	// Check if e is part of this decision diagram
	if (edges.find(e) == edges.end()) throw DecisionDiagram::InvalidDecisionDiagram(std::string("Tried to remove edge \"") + e->toString() + std::string("\". Error: This edge is not a member of the decision diagram."));

	// Remove the edge from it's inzident nodes
	e->getFrom()->removeEdge(e);
	e->getTo()->removeEdge(e);

	// Remove the edge
	edges.erase(e);
	delete e;
}

DecisionDiagram::Node* DecisionDiagram::addDecisionDiagram(DecisionDiagram* dd2){
	// Check for node label uniqueness
	std::set<Node*> dd2nodes = dd2->getNodes();
	for (std::set<Node*>::iterator nodeIt = nodes.begin(); nodeIt != nodes.end(); nodeIt++){
		for (std::set<Node*>::iterator dd2it = dd2nodes.begin(); dd2it != dd2nodes.end(); dd2it++){
			if ((*nodeIt)->getLabel() == (*dd2it)->getLabel()) throw DecisionDiagram::InvalidDecisionDiagram(std::string("Tried to union decision diagrams. Node label \"") + (*nodeIt)->getLabel() + std::string("\" is not unique."));
		}
	}

	// Merge all nodes and edges
	Node* root = NULL;
	std::set<Edge*> dd2edges = dd2->getEdges();
	for (std::set<Node*>::iterator it = dd2nodes.begin(); it != dd2nodes.end(); it++){
//std::cout << "ADDING " << (*it)->toString() << std::endl;
		Node* n = addNode(*it);
		if ((*it) == dd2->getRoot()){
			root = n;
		}
	}
	for (std::set<Edge*>::iterator it = dd2edges.begin(); it != dd2edges.end(); it++){
//std::cout << "ADDING " << (*it)->toString() << std::endl;
		addEdge(*it);
	}

	return root;
}

int DecisionDiagram::nodeCount() const{
	return nodes.size();
}

int DecisionDiagram::leafCount() const{
	int lc = 0;
	for (std::set<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++){
		// inner or leaf node?
		Node *n = *it;
		if (dynamic_cast<LeafNode*>(n) != NULL){
			lc++;
		}
	}
	return lc;
}

int DecisionDiagram::edgeCount() const{
	return edges.size();
}

void DecisionDiagram::setRoot(Node* root){
	// Check if root is part of this decision diagram
	if (nodes.find(root) == nodes.end()) throw InvalidDecisionDiagram("The given root node is not part of this decision diagram.");
	this->root = root;
}

void DecisionDiagram::useUniqueLabels(DecisionDiagram* dd2){

	//    Check all nodes
	for (std::set<DecisionDiagram::Node*>::iterator nodeIt = nodes.begin(); nodeIt != nodes.end(); nodeIt++){
		std::string originalname = (*nodeIt)->getLabel();
		int appendixctr = 0;
		bool dupfound = true;
		while (dupfound){
			dupfound = false;
			std::stringstream newname;

			// Compare each of them with all nodes of dd2
			std::set<DecisionDiagram::Node*> dd2nodes = dd2->getNodes();
			for (std::set<DecisionDiagram::Node*>::iterator dd2it = dd2nodes.begin(); dd2it != dd2nodes.end(); dd2it++){
				if ((*nodeIt)->getLabel() == (*dd2it)->getLabel()){
					appendixctr++;

					// rename node
					newname << originalname << "_" << appendixctr;
					(*nodeIt)->setLabel(newname.str());

					// recheck for duplicates until a unique name was found
					dupfound = true;
					break;
				}
			}

			// This condition is just for the sake of performance enhancement
			//   if dupfound is true, the final result is already clear and another check is unnecessary
			//   if appendixctr == 0, the node label was not changes so far. In this case it is unique for sure, since labels are already checked for uniqueness when nodes are inserted.
			if (!dupfound && appendixctr > 0){
				//   Compare each of them with all other nodes of this decision diagram
				for (std::set<DecisionDiagram::Node*>::iterator nodeIt2 = nodes.begin(); nodeIt2 != nodes.end(); nodeIt2++){
					if ((*nodeIt) != (*nodeIt2) && (*nodeIt)->getLabel() == (*nodeIt2)->getLabel()){
						appendixctr++;

						// rename node
						newname << originalname << "_" << appendixctr;
						(*nodeIt)->setLabel(newname.str());

						// recheck for duplicates until a unique name was found
						dupfound = true;
						break;
					}
				}
			}
		}
	}
}

std::string DecisionDiagram::getUniqueLabel(std::string proposal) const{

	std::string result = proposal;
	bool dupfound = true;
	int appendixctr = 0;
	while (dupfound){
		dupfound = false;
		std::stringstream newname;

		// Compare the proposal with all nodes
		for (std::set<DecisionDiagram::Node*>::iterator it = nodes.begin(); it != nodes.end(); it++){
			// Duplicate?
			if ((*it)->getLabel() == result){
				appendixctr++;
//std::cout << "!" << std::endl;
				// rename node
				newname << proposal << "_" << appendixctr;
				result = newname.str();

				// recheck for duplicates until a unique name was found
				dupfound = true;
				break;
			}
		}
	}
//std::cout << result << std::endl;
	return result;
}

std::set<DecisionDiagram::Node*> DecisionDiagram::getNodes() const{
	return nodes;
}

std::set<DecisionDiagram::LeafNode*> DecisionDiagram::getLeafNodes() const{
	std::set<DecisionDiagram::LeafNode*> leafs;
	// For all nodes
	for (std::set<DecisionDiagram::Node*>::iterator it = nodes.begin(); it != nodes.end(); it++){
		// Check if this is a leaf
		if (dynamic_cast<DecisionDiagram::LeafNode*>(*it) != NULL){
			leafs.insert(dynamic_cast<DecisionDiagram::LeafNode*>(*it));
		}
	}
	return leafs;
}

std::set<DecisionDiagram::Edge*> DecisionDiagram::getEdges() const{
	return edges;
}

DecisionDiagram::Node* DecisionDiagram::getRoot() const{
	return root;
}

std::vector<DecisionDiagram::Node*> DecisionDiagram::containsCycles() const{

	// Prepare an associative array; we need to assign one parent node to each node of the decision diagram
	std::map<DecisionDiagram::Node*, DecisionDiagram::Node*> parents;
	parents[root] = NULL;

	// The following code performs a depth-first search. This algorithm is logically recursive. However, to avoid the necessity of an additional recursive method, is it implemented
	// non-recursive using std::stack.
	// First element each the pair identifies the node, the second one tells if the node is to expand (true) or if expansion has finished (false).
	std::stack<std::pair<DecisionDiagram::Node*, bool> > stack;
	stack.push(std::pair<DecisionDiagram::Node*, bool>(root, true));

	// Runtime: O(|E|), where E is the set of edges
	while (!stack.empty()){
		// Expand the stack's top level element
		std::pair<DecisionDiagram::Node*, bool> p = stack.top();
		stack.pop();
		DecisionDiagram::Node* expandedNode = p.first;
		if (p.second){
			stack.push(std::pair<DecisionDiagram::Node*, bool>(expandedNode, false));
			// Process all children
			std::set<DecisionDiagram::Edge*> children = expandedNode->getOutEdges();
			for (std::set<DecisionDiagram::Edge*>::iterator it = children.begin(); it != children.end(); it++){
				DecisionDiagram::Node* childNode = (*it)->getTo();

				// Check if a cycle is discovered; in case of a cycle, this child has already a parent tag
				if (parents.find(childNode) != parents.end()){
					// Cycle detected
					// Backtrack the path to extract the cycle
					std::vector<Node*> cycle;
					std::vector<Node*> cyclerev;
					Node* backtrack = expandedNode;
					cycle.push_back(childNode);
					while (backtrack != childNode){
						cycle.push_back(backtrack);
						backtrack = parents[backtrack];
					}
					cycle.push_back(childNode);
					// Reverse content due to edge direction
					for (std::vector<Node*>::reverse_iterator it = cycle.rbegin(); it != cycle.rend(); it++) cyclerev.push_back(*it);
					return cyclerev;
				}else{
					// No cycle: Store parent of this child
					parents[childNode] = expandedNode;
					stack.push(std::pair<DecisionDiagram::Node*, bool>(childNode, true));
				}
			}
		}else{
			// Node was successfully processed: Remove it's parent tag, since a node can have several parents (because we are working with general acyclic graphs rather than trees).
			// Multiple parents are only illegal if they occur on one directed path through the graph.
			parents.erase(expandedNode);
		}
	}

	// No cycle was detected
	return std::vector<Node*>();
}

bool DecisionDiagram::isTree() const{
	// Check for all nodes if they have more than one ingoing edges
	for (std::set<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++){
		if ((*it)->getInEdgesCount() > 1) return false;
	}
	// No such node was found
	return true;
}

DecisionDiagram::Node* DecisionDiagram::getNodeByLabel(std::string label) const{
	for (std::set<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++){
		if ((*it)->getLabel() == label){
			return *it;
		}
	}
	// Not found: error: no matching function for call to ‘dlvhex::asp::DecisionDiagram::addNode(dlvhex::asp::DecisionDiagram::Node&)’
	throw InvalidDecisionDiagram(std::string("Tried to retrieve node by label. Error: Node with label \"") + label + std::string("\" does not exist"));
}

AtomSet DecisionDiagram::toAnswerSet() const{
	AtomSet	as;
	// Create tuples for nodes
	for (std::set<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++){
		// inner or leaf node?
		Node *n = *it;
		if (dynamic_cast<LeafNode*>(n) != NULL){
			// leaf node
			Tuple args;
			args.push_back(Term(n->getLabel()));
			args.push_back(Term(dynamic_cast<struct LeafNode*>(n)->getClassification()));
			as.insert(AtomPtr(new Atom(std::string("leafnode"), args)));
		}else{
			// inner node
			Tuple args;
			args.push_back(Term(n->getLabel()));
			as.insert(AtomPtr(new Atom(std::string("innernode"), args)));
		}
	}

	// Create tuples for edges
	for (std::set<Edge*>::iterator it = edges.begin(); it != edges.end(); it++){
		// conditional or else edge?
		DecisionDiagram::Edge *e = *it;
		if (dynamic_cast<ElseEdge*>(e) == NULL){
			// conditional edge
			Tuple args;
			args.push_back(Term(e->getFrom()->getLabel()));
			args.push_back(Term(e->getTo()->getLabel()));
			args.push_back(Term(e->getCondition().getOperand1()));
			args.push_back(Term(DecisionDiagram::Condition::cmpOpToString(e->getCondition().getOperation()), true));
			args.push_back(Term(e->getCondition().getOperand2()));
			as.insert(AtomPtr(new Atom(std::string("conditionaledge"), args)));
		}else{
			// else edge
			Tuple args;
			args.push_back(Term(e->getFrom()->getLabel()));
			args.push_back(Term(e->getTo()->getLabel()));
			as.insert(AtomPtr(new Atom(std::string("elseedge"), args)));
		}
	}

	// Root node
	if (root != NULL){
		Tuple arg;
		arg.push_back(Term(root->getLabel()));
		as.insert(AtomPtr(new Atom("root", arg)));
	}
	return as;
}

std::string DecisionDiagram::toString() const{

	// Create a list of nodes
	std::stringstream output;
	bool first = true;
	output << "Nodes: ";
	for (std::set<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++){
		output << (first ? "" : ", ") << (*it)->toString();
		first = false;
	}

	// Create list of edges
	output << std::endl;
	output << "Edges: " << std::endl;
	for (std::set<Edge*>::iterator it = edges.begin(); it != edges.end(); it++){
		output << "     " << (*it)->toString() << std::endl;
	}
	return output.str();
}
