#include <DecisionDiagram.h>

#include <iostream>

using namespace dlvhex::asp;

DecisionDiagram::Node::Node(std::string l) : label(l){
}

DecisionDiagram::Node::~Node(){
}

void DecisionDiagram::Node::setLabel(std::string l){
	label = l;
}

std::string DecisionDiagram::Node::getLabel(){
	return label;
}

DecisionDiagram::LeafNode::LeafNode(std::string l, std::string c) : Node(l), classification(c){
}

DecisionDiagram::LeafNode::~LeafNode(){
}

std::string DecisionDiagram::LeafNode::getClassification(){
	return classification;
}

DecisionDiagram::Condition::Condition(std::string op1_, std::string op2_, CmpOp op_) : op1(op1_), op2(op2_), op(op_){
}

DecisionDiagram::Condition::~Condition(){
}

std::string DecisionDiagram::Condition::getOp1(){
	return op1;
}

std::string DecisionDiagram::Condition::getOp2(){
	return op2;
}

DecisionDiagram::Condition::CmpOp DecisionDiagram::Condition::getOp(){
	return op;
}

DecisionDiagram::Edge::Edge(Node *u, Node *v, Condition c) : v1(u), v2(v), condition(c){
}

DecisionDiagram::Edge::~Edge(){
}

DecisionDiagram::Node* DecisionDiagram::Edge::getV1(){
	return v1;
}

DecisionDiagram::Node* DecisionDiagram::Edge::getV2(){
	return v2;
}

DecisionDiagram::Condition::Condition DecisionDiagram::Edge::getCondition(){
	return condition;
}

DecisionDiagram::ElseEdge::ElseEdge(Node *u, Node *v) : Edge(u, v, Condition(std::string("none"), std::string("none"), Condition::le)){
}

DecisionDiagram::ElseEdge::~ElseEdge(){
}

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
		nodes.push_back(new Node(it->getArguments()[0].getUnquotedString()));
	}
	//    leaf nodes
	AtomSet leafnodes;
	as.matchPredicate(std::string("leafnode"), leafnodes);
	for (AtomSet::const_iterator it = leafnodes.begin(); it != leafnodes.end(); it++){
		nodes.push_back(new LeafNode(it->getArguments()[0].getUnquotedString(), it->getArguments()[1].getUnquotedString()));
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
		Edge *e = new Edge(getNodeByLabel(n1), getNodeByLabel(n2), c);
		edges.push_back(e);
	}
	//    else edges
	AtomSet elseedges;
	as.matchPredicate(std::string("elseedge"), elseedges);
	for (AtomSet::const_iterator it = elseedges.begin(); it != elseedges.end(); it++){
		std::string n1 = it->getArguments()[0].getUnquotedString();
		std::string n2 = it->getArguments()[1].getUnquotedString();
		ElseEdge *e = new ElseEdge(getNodeByLabel(n1), getNodeByLabel(n2));
		edges.push_back(e);
	}
	// Root node
	AtomSet rootnode;
	as.matchPredicate(std::string("root"), rootnode);
	for (AtomSet::const_iterator it = rootnode.begin(); it != rootnode.end(); it++){
		root = getNodeByLabel(it->getArguments()[0].getUnquotedString());
	}
}

DecisionDiagram::~DecisionDiagram(){
	// Cleanup
	for (int i = 0; i < nodes.size(); i++){
		delete nodes[i];
	}
	for (int i = 0; i < edges.size(); i++){
		delete edges[i];
	}
}

DecisionDiagram& DecisionDiagram::operator=(const DecisionDiagram &dd2){
	for (int i = 0; i < nodes.size(); i++){
		delete nodes[i];
	}
	for (int i = 0; i < edges.size(); i++){
		delete edges[i];
	}
	edges.clear();
	nodes.clear();
	root = NULL;

	for (int nc = 0; nc < dd2.nodeCount(); nc++){
		Node* n2 = dd2.getNode(nc);
		addNode(new Node(*n2));
	}
	for (int ec = 0; ec < dd2.edgeCount(); ec++){
		Edge* e2 = dd2.getEdge(ec);
		ElseEdge* elseedge = dynamic_cast<ElseEdge*>(e2);
		if (elseedge != NULL){
			addEdge(new ElseEdge(getNodeByLabel(elseedge->getV1()->getLabel()), getNodeByLabel(elseedge->getV2()->getLabel())));
		}else{
			addEdge(new Edge(getNodeByLabel(e2->getV1()->getLabel()), getNodeByLabel(e2->getV2()->getLabel()), DecisionDiagram::Condition(e2->getCondition().getOp1(), e2->getCondition().getOp2(), e2->getCondition().getOp())));
		}
	}
	if (dd2.getRoot() != NULL) this->root = getNodeByLabel(dd2.getRoot()->getLabel());
	return *this;
}

DecisionDiagram::Node* DecisionDiagram::getNodeByLabel(std::string label) const{
	for (int i = 0; i < nodes.size(); i++){
		if (nodes[i]->getLabel() == label){
			return nodes[i];
		}
	}
	// Not found
	assert(false);
}

void DecisionDiagram::addNode(Node* n){
	nodes.push_back(n);
}

void DecisionDiagram::addEdge(Edge* e){
	edges.push_back(e);
}

int DecisionDiagram::nodeCount() const{
	return nodes.size();
}

int DecisionDiagram::leafCount() const{
	int lc = 0;
	for (int i = 0; i < nodes.size(); i++){
		// inner or leaf node?
		Node *n = nodes[i];
		if (dynamic_cast<LeafNode*>(n) != NULL){
			lc++;
		}
	}
	return lc;
}

int DecisionDiagram::edgeCount() const{
	return edges.size();
}

DecisionDiagram::Node* DecisionDiagram::getNode(int index) const{
	return nodes[index];
}

DecisionDiagram::Edge* DecisionDiagram::getEdge(int index) const{
	return edges[index];
}

void DecisionDiagram::setRoot(Node* root){
	this->root = root;
}

DecisionDiagram::Node* DecisionDiagram::getRoot() const{
	return root;
}

std::vector<DecisionDiagram::Node*> DecisionDiagram::getChildren(Node* n) const{
	std::vector<DecisionDiagram::Node*> children;
	for (int i = 0; i < edgeCount(); i++){
		if (edges[i]->getV1() == n){
			children.push_back(edges[i]->getV2());
		}
	}
	return children;
}

AtomSet DecisionDiagram::toAnswerSet(){
	AtomSet	as;
	// Create tuples for nodes
	for (int i = 0; i < nodes.size(); i++){
		// inner or leaf node?
		Node *n = nodes[i];
		if (dynamic_cast<LeafNode*>(n) != NULL){
			// leaf node
			Tuple args;
			args.push_back(Term(nodes[i]->getLabel()));
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
	for (int i = 0; i < edges.size(); i++){
		// conditional or else edge?
		DecisionDiagram::Edge *e = edges[i];
		if (dynamic_cast<ElseEdge*>(e) == NULL){
			// conditional edge
			Tuple args;
			args.push_back(Term(e->getV1()->getLabel()));
			args.push_back(Term(e->getV2()->getLabel()));
			args.push_back(Term(e->getCondition().getOp1()));
			switch(e->getCondition().getOp()){
				case Condition::lt: args.push_back(Term("<", true)); break;
				case Condition::le: args.push_back(Term("<=", true)); break;
				case Condition::eq: args.push_back(Term("=", true)); break;
				case Condition::ge: args.push_back(Term(">", true)); break;
				case Condition::gt: args.push_back(Term(">=", true)); break;
			}
			args.push_back(Term(e->getCondition().getOp2()));
			as.insert(AtomPtr(new Atom(std::string("conditionaledge"), args)));
		}else{
			// else edge
			Tuple args;
			args.push_back(Term(e->getV1()->getLabel()));
			args.push_back(Term(e->getV2()->getLabel()));
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
