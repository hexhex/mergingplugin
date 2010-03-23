#ifndef _PARSETREENODE_h_
#define _PARSETREENODE_h_

/** \file ParseTreeNode.h
 *	\brief Provides the data structure used to construct the parse tree.
 */

#include <iostream>
#include <vector>

namespace dlvhex{
namespace merging{
namespace tools{
namespace mpcompiler{

/**
* This is the base class used to construct the parse tree. Each instances represents one node of the tree
*/
class ParseTreeNodeIterator; // to avoid cyclic includes
class ParseTreeNode {

	public:
		/**
		* Possible types of parse tree nodes
		*/
		enum NodeType{
			leaf,

			sections,
			section_commonsignature,
			section_beliefbase,
			section_revisionplan,
			
			predicate,
			predicatelist,

			beliefbase,

			revisionplansection,
			revisionsource,
			revisionsources,
			datasource,

			kvpairs,
			kvpair
		};

	private:
		NodeType type;
		std::vector<ParseTreeNode*> children;
		virtual void print(std::ostream &os, int depth);

	public:
		// Constructor and destructor
		ParseTreeNode(NodeType type, int childrenCount, ...);
		virtual ~ParseTreeNode();

		// Output
		virtual void print(std::ostream &os);
		std::ostream& operator<<(std::ostream &os);

		// Child handling
		int addChild(ParseTreeNode *newchild);
		int getChildrenCount();
		ParseTreeNode* getChild(int index);
		ParseTreeNode* flatten(NodeType flattenedType);

		// Iterators
		ParseTreeNodeIterator begin();
		ParseTreeNodeIterator end();
		ParseTreeNodeIterator begin(NodeType type);
		ParseTreeNodeIterator end(NodeType type);

		// Others
		NodeType getType();
		virtual ParseTreeNode* clone();
};

/**
* This class is used to represent leaf nodes in the parse tree. Leaf nodes are nodes with no childrean but with a value (of type parameter T) instead.
*/
template<class T> class LeafTreeNode : public ParseTreeNode{
	private:
		T value;

	public:
		LeafTreeNode(T value) : ParseTreeNode(leaf, 0){
			this->value = value;
		}

		void setValue(T value){
			this->value = value;
		}

		T getValue(){
			return value;
		}

		virtual LeafTreeNode<T>* clone(){
			return new LeafTreeNode<T>(value);
		}

		virtual void print(std::ostream &os, int depth){
			for (int i = 0; i < depth; i++){
				os << "   ";
			}
			os << "[" << value << "]" << std::endl;
		}
};

/**
 * Leaf node storing a string
 */
typedef LeafTreeNode<std::string> StringTreeNode;

/**
 * Leaf node storing an int
 */
typedef LeafTreeNode<int> IntTreeNode;

}
}
}
}

#endif


/*! \fn dlvhex::merging::tools::rpcompiler::ParseTreeNode::ParseTreeNode(NodeType type, int childrenCount, ...)
 *  \brief The constructor of the class.
 *  \param type Type of this parse tree node.
 *  \param childrenCount Number of children passed as variable parameters
 *  \param ... Child nodes
 */

/*! \fn dlvhex::merging::tools::rpcompiler::ParseTreeNode::~ParseTreeNode()
 *  \brief Destructor: frees memory allocated by this node and all child nodes (recursive deletion)
 */

/*! \fn void dlvhex::merging::tools::rpcompiler::ParseTreeNode::print(std::ostream &os)
 *  \brief Prints this parse tree (or subtree) into the given output stream.
 *  \param os Output stream to write to 
 */

/*! \fn std::ostream& dlvhex::merging::tools::rpcompiler::ParseTreeNode::operator<<(std::ostream &os)
 *  \brief Prints this parse tree (or subtree) into the given output stream.
 *  \param os Output stream to write to 
 *  \return std::ostream& Copy of reference os
 */

/*! \fn void dlvhex::merging::tools::rpcompiler::ParseTreeNode::addChild(ParseTreeNode *newchild)
 *  \brief Adds a further child to this tree node.
 *  \param newchild A pointer to the new tree node. Note: You must not add a node to two (or more) different parent nodes, since this causes troubles when the destructor is called.
 */

/*! \fn int dlvhex::merging::tools::rpcompiler::ParseTreeNode::getChlidrenCount()
 *  \brief Returns the number of children.
 *  \param int Number of children
 */

/*! \fn ParseTreeNode* dlvhex::merging::tools::rpcompiler::ParseTreeNode::getChlid(int index)
 *  \brief Returns a pointer to a child of this node.
 *  \param int Index of the child to retrieve (0-based)
 *  \return ParseTreeNode* Pointer to the requested child
 */

/*! \fn ParseTreeNode* dlvhex::merging::tools::rpcompiler::ParseTreeNode::flatten(NodeType flattenedType)
 *  \brief This method is used to flatten a subtree of the following type: ListNode(type, ContentNode, ListNode(type, ContentNode, ListNode(type, ContentNode, NULL))). "ListNode" is a node of a certain type with exactly(!) 2 children: one "content node" (of any type) and one ListNode of the same type as the node itself. In this way, recursive node structures represent lists of content nodes, where the recursion is terminated by a NULL entry instead of a further list node.
 *  \param NodeType The type of the output node.
 *  \return ParseTreeNode* A node of the given type with all the content nodes of this subtree as children. So the number of children in the output node is variable, whereas it is fixed (2) in the recursive input.
 */

/*! \fn ParseTreeNodeIterator dlvhex::merging::tools::rpcompiler::ParseTreeNode::begin()
 *  \brief Returns an iterator for all child nodes of this parse tree node.
 *  \param ParseTreeNodeIterator& Iterator through all child nodes */

/*! \fn ParseTreeNodeIterator dlvhex::merging::tools::rpcompiler::ParseTreeNode::end()
 *  \brief Returns an iterator representing the end marker.
 *  \param ParseTreeNodeIterator& End marker */

/*! \fn ParseTreeNodeIterator dlvhex::merging::tools::rpcompiler::ParseTreeNode::begin(NodeType type)
 *  \brief Returns an iterator for child nodes of a certain type of this parse tree node.
 *  \param type
 *  \param ParseTreeNodeIterator& Iterator through all child nodes */

/*! \fn ParseTreeNode* dlvhex::merging::tools::rpcompiler::ParseTreeNode::clone()
 *  \brief Returns a pointer to a (recursive) copy of this parse sub tree.
 *  \return ParseTreeNode* Pointer to a copy of the parse sub tree which's root is the this node.
 */

/*! \fn NodeType dlvhex::merging::tools::rpcompiler::ParseTreeNode::getType()
 *  \brief Returns the type of this parse tree node.
 *  \return NodeType Parse tree node type
 */

/*! \fn dlvhex::merging::tools::rpcompiler::LeafTreeNode::LeafTreeNode(T value)
 *  \brief Constructor of a leaf node.
 *  \param value The value to initialize the leaf node with
 */

/*! \fn void dlvhex::merging::tools::rpcompiler::LeafTreeNode::setValue(T value)
 *  \brief Changes the value of this leaf node
 *  \param value The new value of the leaf node
 */

/*! \fn T dlvhex::merging::tools::rpcompiler::LeafTreeNode::getValue()
 *  \brief Returns the value of this leaf node
 *  \return T The value of this leaf node
 */
