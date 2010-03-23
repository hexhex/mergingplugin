#include <ParseTreeNodeIterator.h>

#include <iostream>

using namespace dlvhex::merging::tools::mpcompiler;

ParseTreeNodeIterator::ParseTreeNodeIterator(ParseTreeNode* n) : node(n), cindex(0){
	typespecific = false;
}

ParseTreeNodeIterator::ParseTreeNodeIterator(ParseTreeNode* n, ParseTreeNode::NodeType t) : node(n), type(t){
	typespecific = true;

	// Search for first node of the given type
	for (cindex = 0; cindex < node->getChildrenCount(); cindex++){
		if (node->getChild(cindex)->getType() == type){
			break;
		}
	}
	// No element of this type
}

ParseTreeNodeIterator& ParseTreeNodeIterator::operator++(){

	if (typespecific){
		// Go to next element of this type
		for (cindex++; cindex < node->getChildrenCount() && node->getChild(cindex)->getType() != type; cindex++);
	}else{
		cindex++;
	}
	// Keep iterator within bounds
	if (cindex > node->getChildrenCount() - 1){
		cindex = node->getChildrenCount();
	}
	return *this;
}

ParseTreeNodeIterator& ParseTreeNodeIterator::operator--(){
	if (typespecific){
		// Go to next element of this type
		for (cindex--; cindex >= 0 && node->getChild(cindex)->getType() != type; cindex--);
	}else{
		cindex--;
	}
	// Keep iterator within bounds
	if (cindex < 0){
		cindex = 0;
	}

	return *this;
}

ParseTreeNode& ParseTreeNodeIterator::operator*(){
	return *(node->getChild(cindex));
}

ParseTreeNode* ParseTreeNodeIterator::operator->(){
	return node->getChild(cindex);
}

bool ParseTreeNodeIterator::operator==(const ParseTreeNodeIterator& it2){
	return	node == it2.node && 		// Iterator on the same object
		cindex == it2.cindex;		// Either both iterators are typespecific
}

bool ParseTreeNodeIterator::operator!=(const ParseTreeNodeIterator& it2){
	return !this->operator==(it2);
}
