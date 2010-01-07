#ifndef _PARSETREENODEITERATOR_h_
#define _PARSETREENODEITERATOR_h_

#include <ParseTreeNode.h>

#include <iterator>

class ParseTreeNodeIterator : public std::iterator<std::bidirectional_iterator_tag, ParseTreeNode*>{
	private:
		ParseTreeNode* node;
		int cindex;
		ParseTreeNode::NodeType type;
		bool typespecific;

	public:
		ParseTreeNodeIterator(ParseTreeNode* n);
		ParseTreeNodeIterator(ParseTreeNode* n, ParseTreeNode::NodeType t);

		ParseTreeNodeIterator& operator++();
		ParseTreeNodeIterator& operator--();
		ParseTreeNode& operator*();
		ParseTreeNode* operator->();

		bool operator==(const ParseTreeNodeIterator& it2);
		bool operator!=(const ParseTreeNodeIterator& it2);
};

#endif


/*! \fn ParseTreeNodeIterator::ParseTreeNodeIterator(ParseTreeNode* n)
 *  \brief Constructs an iterator which iterates through all subnodes of n.
 *  \param n Parse tree root
 */

/*! \fn ParseTreeNodeIterator::ParseTreeNodeIterator(ParseTreeNode* n, ParseTreeNode::NodeType t)
 *  \brief Constructs an iterator which iterates through all subnodes of a certain ype.
 *  \param n Parse tree root
 *  \param t Type of nodes of interest
 */

/*! \fn ParseTreeNodeIterator& ParseTreeNodeIterator::operator++()
 *  \brief Goes to the next element (iterator is unchanged if it points to the last element).
 *  \return ParseTreeNodeIterator& Reference to the this iterator
 */

/*! \fn ParseTreeNodeIterator& ParseTreeNodeIterator::operator--()
 *  \brief Goes to the previous element (iterator is unchanged if it points to the first element).
 *  \return ParseTreeNodeIterator& Reference to the this iterator
 */

/*! \fn ParseTreeNode& ParseTreeNodeIterator::operator*()
 *  \brief Retrieves a reference to the current iterator element. This method must not be called if the operator points to the end element.
 *  \return ParseTreeNode& Reference to the current iterator element.
 */

/*! \fn ParseTreeNode* ParseTreeNodeIterator::operator->()
 *  \brief Retrieves a pointer to the current iterator element. This method must not be called if the operator points to the end element.
 *  \return ParseTreeNode* Pointer to the current iterator element.
 */

/*! \fn bool ParseTreeNodeIterator::operator==(const ParseTreeNodeIterator& it2)
 *  \brief Compares two iterators.
 *  \param it2 A second iterator to campare with.
 *  \return bool True iff it2 points to the same element as this.
 */

/*! \fn bool ParseTreeNodeIterator::operator!=(const ParseTreeNodeIterator& it2)
 *  \brief Compares two iterators.
 *  \param it2 A second iterator to campare with.
 *  \return bool False iff it2 points to the same element as this.
 */
