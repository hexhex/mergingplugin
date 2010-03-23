#include <ParseTreeNode.h>
#include <ParseTreeNodeIterator.h>

#include <iostream>
#include <stdarg.h>

using namespace dlvhex::merging::tools::mpcompiler;

// ------------------------------ Constructor / Destructor ------------------------------

ParseTreeNode::ParseTreeNode(NodeType type, int childrenCount, ...){

	this->type = type;

	// Store childrean passed as variable parameters
	if (childrenCount > 0){
		va_list params;
		va_start(params, childrenCount);
		for (int i = 0; i < childrenCount; i++){
			this->children.push_back(va_arg(params, ParseTreeNode*));
		}
		va_end(params);
	}

}

ParseTreeNode::~ParseTreeNode(){
	// Delete all child nodes
	for (int i = 0; i < getChildrenCount(); i++){
		if (children[i] != NULL) delete children[i];
	}
}

// ------------------------------ Output ------------------------------

void ParseTreeNode::print(std::ostream &os){
	print(os, 0);
}
void ParseTreeNode::print(std::ostream &os, int depth){

	// Shift to the right
	for (int i = 0; i < depth; i++){
		os << "   ";
	}

	// Print node type
	switch(type){
		case sections:
			os << "sections";
			break;
		case section_commonsignature:
			os << "section_commonsignature";
			break;
		case section_beliefbase:
			os << "section_beliefbase";
			break;
		case section_revisionplan:
			os << "section_revisionplan";
			break;
		case predicate:
			os << "predicate";
			break;
		case predicatelist:
			os << "predicatelist";
			break;
		case beliefbase:
			os << "beliefbase";
			break;
		case revisionplansection:
			os << "revisionplansection";
			break;
		case revisionsource:
			os << "revisionsource";
			break;
		case revisionsources:
			os << "revisionsources";
			break;
		case datasource:
			os << "datasource";
			break;
		case kvpairs:
			os << "kvpairs";
			break;
		case kvpair:
			os << "kvpair";
			break;
		case leaf:
			break;
		default:
			break;
	}
	os << std::endl;

	// In case of leaf nodes, print their content
	for (int i = 0; i < getChildrenCount(); i++){
		if (children[i] == NULL){
			for (int i = 0; i < (depth + 1); i++){
				os << "   ";
			}
			os << "NULL" << std::endl;
		}else{
			children[i]->print(os, depth + 1);
		}
	}
}

// TODO: Does not work
std::ostream& ParseTreeNode::operator<<(std::ostream &os){
os << "11111111111111111";
	print(os, 0);
	return os;
}

// ------------------------------ Child handling ------------------------------

// Adds a child to this node
int ParseTreeNode::addChild(ParseTreeNode *newchild){
	children.push_back(newchild);
}

// Return children count
int ParseTreeNode::getChildrenCount(){
	return children.size();
}

// Retrieve pointer to child
ParseTreeNode* ParseTreeNode::getChild(int index){
	return children[index];
}

ParseTreeNode* ParseTreeNode::flatten(NodeType flattenedType){

	// Create result node for flattened tree
	ParseTreeNode *flattened = new ParseTreeNode(flattenedType, 0);

	// Flatten predicate list subtree (getChlid(0) is the "content" of this node, getChild(1) is the continuation of the list until NULL is discovered)
	ParseTreeNode *n = this;
	while (n != NULL){
		// Content found?
		if (n->getChild(0) != NULL){
			flattened->addChild(n->getChild(0)->clone());
		}
		// Go to next list element
		n = n->getChild(1);
	}

	return flattened;
}

// ------------------------------ Iterators ------------------------------

ParseTreeNodeIterator ParseTreeNode::begin(){
	return ParseTreeNodeIterator(this);
}

ParseTreeNodeIterator ParseTreeNode::end(){
	ParseTreeNodeIterator it(this);
	for (int i = 0; i < getChildrenCount(); i++) ++it;
	return it;
}

ParseTreeNodeIterator ParseTreeNode::begin(NodeType type){
	return ParseTreeNodeIterator(this, type);
}

// ------------------------------ Others ------------------------------

// Recursive cloning of this parse sub tree
ParseTreeNode* ParseTreeNode::clone(){
	// Create a new node for the this node
	ParseTreeNode *clone = new ParseTreeNode(type, 0);

	// Clone all sub elements recursively
	for (int i = 0; i < getChildrenCount(); i++){
		clone->addChild(getChild(i) != NULL ? getChild(i)->clone() : NULL);
	}
	return clone;
}


ParseTreeNode::NodeType ParseTreeNode::getType(){
	return type;
}
