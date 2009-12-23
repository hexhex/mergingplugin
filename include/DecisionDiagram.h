#ifndef __DECISIONDIAGRAM_H_
#define __DECISIONDIAGRAM_H_

#include <dlvhex/AtomSet.h>
#include <vector>
#include <set>
#include <string>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
	namespace asp{

		/**
		 * This class provides the data structures and methodes necessary to build decision diagrams from answer sets representing them.
		 */
		class DecisionDiagram{
		public:
			/**
			 * General exception class for errors in the context of decision diagrams.
			 */
			class InvalidDecisionDiagram{
			private:
				std::string msg;
			public:
				InvalidDecisionDiagram(std::string msg);
				std::string getMessage();
			};

			/*! \fn DecisionDiagram::InvalidDecisionDiagram::InvalidDecisionDiagram(std::string msg)
			 *  \brief Constructs a new exception with a certain error messasge.
			 *  \param msg The error message to use
			 */

			/*! \fn std::string DecisionDiagram::InvalidDecisionDiagram::getMessage()
			 *  \brief Retrieves the error message of this exception.
			 *  \param std::string The error message of this exception
			 */


			class Edge;

			/**
			 * A class representing one node of a decision diagram.
			 */
			class Node{
			private:
				std::string label;		// This node's label
				std::set<Edge*> inEdges;	// All the ingoing edges this Node is involved in
				std::set<Edge*> outEdges;	// All the outgoing edges this Node is involved in

				// The following methods are only called by members of DecisionDiagram in order to maintain the decision diagram's integrity
				friend class DecisionDiagram;
				Node(std::string l);
				void addEdge(Edge* e);
				void removeEdge(Edge* e);
			public:
				virtual ~Node();

				void setLabel(std::string l);
				std::string getLabel() const;

				std::set<Edge*> getEdges() const;
				std::set<Edge*> getInEdges() const;
				std::set<Edge*> getOutEdges() const;
				int getEdgesCount() const;
				int getInEdgesCount() const;
				int getOutEdgesCount() const;

				virtual std::string toString() const;
			};

			/*! \fn DecisionDiagram::LeafNode::Node(std::string l)
			 *  \brief Construct a new node with a certain label.
			 *  \param l The label of the node
			 */

			/*! \fn DecisionDiagram::Node::~Node()
			 *  \brief Destructor
			 */

			/*! \fn void DecisionDiagram::LeafNode::addEdge(Edge* e)
			 *  \brief Adds a new edge to this node. Note that the edge must be inzident with this node, otherwise an InvalidDecisionDiagram instance will be thrown.
			 *  \param e The edge to add
			 *  \throws InvalidDecisionDiagram If e is not inzident with this node.
			 */

			/*! \fn void DecisionDiagram::LeafNode::removeEdge(Edge* e)
			 *  \brief Removes an edge from this node. Note that the edge must have been added previously.
			 *  \param e The edge to remove
			 *  \throws InvalidDecisionDiagram If e was not added to this node.
			 */

			/*! \fn void DecisionDiagram::LeafNode::setLabel(std::string l)
			 *  \brief Changes the label of this node.
			 *  \param l The new label of the node
			 */

			/*! \fn std::string DecisionDiagram::LeafNode::getLabel()
			 *  \brief Retrieves the label of this node.
			 *  \return std::string The label of the node
			 */

			/*! \fn std::set<Edge*> DecisionDiagram::LeafNode::getEdges() const
			 *  \brief Retrieves the edges inzident with this node.
			 *  \return std::set<Edge*> A set of all edges inzident with this node.
			 */

			/*! \fn std::set<Edge*> DecisionDiagram::LeafNode::getOutEdges() const
			 *  \brief Retrieves the outgoing edges of this node.
			 *  \return std::set<Edge*> A set of outgoing edges of this node.
			 */

			/*! \fn std::set<Edge*> DecisionDiagram::LeafNode::getInEdges() const
			 *  \brief Retrieves the ingoing edges of this node.
			 *  \return std::set<Edge*> A set of ingoing edges of this node.
			 */

			/*! \fn int DecisionDiagram::LeafNode::getEdgesCount() const
			 *  \brief Returns the number of edges inzident with this node.
			 *  \return int The number of edges inzident with this node.
			 */

			/*! \fn int DecisionDiagram::LeafNode::getOutEdgesCount() const
			 *  \brief Returns the number of outgoing edges of this node.
			 *  \return int The number of edges outgoing edges of this node.
			 */

			/*! \fn int DecisionDiagram::LeafNode::getInEdgesCount() const
			 *  \brief Returns the number of ingoing edges of this node.
			 *  \return int The number of edges ingoing edges of this node.
			 */

			/*! \fn std::string DecisionDiagram::Node::toString()
			 *  \brief Returns a string representation of this node.
			 *  \return std::string The string representation of this node.
			 */


			/**
			 * A class representing one leaf node of a decision diagram.
			 */
			class LeafNode : public Node{
			private:
				std::string classification;

				// The following methods are only called by members of DecisionDiagram in order to maintain the decision diagram's integrity
				friend class DecisionDiagram;
				LeafNode(std::string l, std::string c);
			public:
				virtual ~LeafNode();
				std::string getClassification();

				virtual std::string toString() const;
			};

			/*! \fn DecisionDiagram::LeafNode::LeafNode(std::string l, std::string c)
			 *  \brief Construct a new leaf node with a certain label and classification.
			 *  \param l The label of the leaf node
			 *  \param c The classification of the leaf node
			 */

			/*! \fn DecisionDiagram::LeafNode::~LeafNode()
			 *  \brief Destructor
			 */

			/*! \fn std::string DecisionDiagram::LeafNode::toString()
			 *  \brief Returns a string representation of this leaf node.
			 *  \return std::string The string representation of this leaf node.
			 */


			/**
			 * A class representing the condition of an edge of a decision diagram.
			 */
			class Condition{
			public:
				/**
				 * Defines the possible comparison operators.
				 */
				enum CmpOp{
					lt,
					le,
					eq,
					ge,
					gt
				};
			private:
				std::string operand1;
				std::string operand2;
				CmpOp operation;
			public:
				Condition(std::string operand1_, std::string operand2_, CmpOp operation_);
				Condition(std::string operand1_, std::string operand2_, std::string operation_);

				virtual ~Condition();
				std::string getOperand1();
				std::string getOperand2();
				CmpOp getOperation();

				static CmpOp stringToCmpOp(std::string operation_);
				static std::string cmpOpToString(CmpOp op);

				virtual std::string toString() const;
			};

			/*! \fn DecisionDiagram::Condition::Condition(std::string operand1_, std::string operand2_, CmpOp operation_)
			 *  \brief Returns a string representation of this condition.
			 *  \param operand1_ An arbitrary operand of a range query (basically an arbitrary string)
			 *  \param operand2_ An arbitrary operand of a range query (basically an arbitrary string)
			 *  \param operation_ The comparison operator used in the range query
			 */

			/*! \fn DecisionDiagram::Condition::Condition(std::string operand1_, std::string operand2_, std::string operation_)
			 *  \brief Returns a string representation of this condition.
			 *  \param operand1_ An arbitrary operand of a range query (basically an arbitrary string)
			 *  \param operand2_ An arbitrary operand of a range query (basically an arbitrary string)
			 *  \param operation_ The comparison operator used in the range query given as string
			 *  \throws InvalidDecisionDiagram If the operation given as string is not a valid comparison operator (<, <=, =, >, >=)
			 */

			/*! \fn DecisionDiagram::Condition::~Condition()
			 *  \brief Destructor
			 */

			/*! \fn std::string DecisionDiagram::Condition::getOperand1()
			 *  \brief Returns the first operand of the range query.
			 *  \return std::string The first operand of the range query
			 */

			/*! \fn std::string DecisionDiagram::Condition::getOperand2()
			 *  \brief Returns the second operand of the range query.
			 *  \return std::string The second operand of the range query
			 */

			/*! \fn CmpOp DecisionDiagram::Condition::getOperation()
			 *  \brief Returns the operation of the range query.
			 *  \return CmpOp The operation of the range query
			 */

			/*! \fn static CmpOp DecisionDiagram::Condition::stringToCmpOp(std::string operation_)
			 *  \brief Converts a string into an element of the enumeration CmpOp.
			 *  \param operation_ A comparison operator given as string
			 *  \return CmpOp The operation specified by the string operation_.
			 *  \throws InvalidDecisionDiagram If the operation given as string is not a valid comparison operator (<, <=, =, >, >=)
			 */

			/*! \fn static std::string DecisionDiagram::Condition::cmpOpToString(CmpOp op)
			 *  \brief Converts an element of the enumeration CmpOp into a string representation.
			 *  \param op A comparison operator
			 *  \return std::string A string representation of op.
			 */

			/*! \fn std::string DecisionDiagram::Condition::toString()
			 *  \brief Returns a string representation of this condition.
			 *  \return std::string The string representation of this condition.
			 */


			/**
			 * A class representing one edge of a decision diagram.
			 */
			class Edge{
			private:
				Node *from, *to;
				Condition condition;

				// The following methods are only called by members of DecisionDiagram in order to maintain the decision diagram's integrity
				friend class DecisionDiagram;
				Edge(Node *f, Node *t, Condition c);
			public:
				virtual ~Edge();

				Node* getFrom();
				Node* getTo();
				Condition getCondition();

				virtual std::string toString() const;
			};

			/*! \fn DecisionDiagram::Edge::Edge(Node *f, Node *t, Condition c)
			 *  \brief Construct a directed conditional edge from two nodes and one condition
			 *  \param f The source node of this edge
			 *  \param t The destination node of this edge
			 *  \param c The condition of this edge
			 */

			/*! \fn DecisionDiagram::Edge::~Edge()
			 *  \brief Destructor
			 */

			/*! \fn Node* DecisionDiagram::Edge::getFrom()
			 *  \brief Returns the source node of this directed edge.
			 *  \return Node* The source node of this edge
			 */

			/*! \fn Node* DecisionDiagram::Edge::getTo()
			 *  \brief Returns the destination node of this directed edge.
			 *  \return Node* The destination node of this edge
			 */

			/*! \fn Condition DecisionDiagram::Edge::getCondition()
			 *  \brief Returns the condition of this conditional edge.
			 *  \return Condition The condition of this conditional edge
			 */

			/*! \fn std::string DecisionDiagram::Edge::toString()
			 *  \brief Returns a string representation of this edge.
			 *  \return std::string The string representation of this edge.
			 */


			/**
			 * A class representing one unconditional (else) edge of a decision diagram.
			 */
			class ElseEdge : public Edge{
			private:
				// The following methods are only called by members of DecisionDiagram in order to maintain the decision diagram's integrity
				friend class DecisionDiagram;
				ElseEdge(Node *f, Node *t);
			public:
				virtual ~ElseEdge();

				virtual std::string toString() const;
			};

			/*! \fn DecisionDiagram::ElseEdge::ElseEdge(Node *f, Node *t)
			 *  \brief Construct a directed unconditional (else) edge from two nodes
			 *  \param f The source node of this edge
			 *  \param t The destination node of this edge
			 */

			/*! \fn DecisionDiagram::ElseEdge::~Edge()
			 *  \brief Destructor
			 */

			/*! \fn std::string DecisionDiagram::ElseEdge::toString()
			 *  \brief Returns a string representation of this unconditional (else) edge.
			 *  \return std::string The string representation of this unconditional (else) edge.
			 */


		private:
			std::set<Node*> nodes;
			std::set<Edge*> edges;
			Node* root;

		public:
			// Constructors and destructors
			DecisionDiagram();
			DecisionDiagram(const DecisionDiagram &dd2);
			DecisionDiagram(AtomSet as);
			~DecisionDiagram();
			void clear();

			DecisionDiagram& operator=(const DecisionDiagram &dd2);

			// Modification
			Node* addNode(std::string label);
			LeafNode* addLeafNode(std::string label, std::string classification);
			Node* addNode(Node* template_);
			void removeNode(Node *n);
			void removeNode(Node *n, bool forceRemoveEdges);

			Edge* addEdge(Node* from, Node* to, Condition c);
			Edge* addEdge(std::string from, std::string to, Condition c);
			ElseEdge* addElseEdge(Node* from, Node* to);
			ElseEdge* addElseEdge(std::string from, std::string to);
			Edge* addEdge(Edge* template_);
			void removeEdge(Edge* e);

			void setRoot(Node* root);

			// Getter
			std::set<Node*> getNodes() const;
			std::set<Edge*> getEdges() const;
			Node* getRoot() const;
			Node* getNodeByLabel(std::string label) const;
			int nodeCount() const;
			int leafCount() const;
			int edgeCount() const;

			// Advanced
			std::vector<Node*> containsCycles() const;

			// Output generation
			AtomSet toAnswerSet() const;
			std::string toString() const;
		};
	}
}

#endif


/*! \fn DecisionDiagram::DecisionDiagram()
 *  \brief Creates an empty decision diagram.
 */

/*! \fn DecisionDiagram::DecisionDiagram(const DecisionDiagram &dd2)
 *  \brief Copy-constructor
 *  \param dd2 Template decision diagram
 */

/*! \fn DecisionDiagram::DecisionDiagram(AtomSet as)
 *  \brief Constructs a decision diagram from an answer set. The predicates root(Name), innernode(Label), leafnode(Label, Classification), edge(Node1, Node2, Operand1, comparisonOperator, Operand2) and elseedge(Node1, Node2) will be interpreted. During construction some validity checks (e.g. usage of undefined nodes, cycle checks, etc.) will be performed and an instance of InvalidDecisionDiagram will be thrown in case of an error.
 *  \param as An answer set which defines a decision diagram using the following predicates: root(Name), innernode(Label), leafnode(Label, Classification), edge(Node1, Node2, Operand1, comparisonOperator, Operand2), elseedge(Node1, Node2).
 *  \throws InvalidDecisionDiagram The message text will describe the cause of the error.
 */

/*! \fn DecisionDiagram::~DecisionDiagram()
 * Destructor. Frees allocated memory.
 */

/*! \fn void DecisionDiagram::empty()
 * Removes all nodes and edges from this decision diagram.
 */

/*! \fn DecisionDiagram& DecisionDiagram::operator=(const DecisionDiagram &dd2)
 * Assignment operator. This decision diagram will be overwritten by dd2. All the internal structures (nodes, edges) of dd2 will be copied and pointers will be adjusted accordingly. Thus dd2 and this decision diagram will be completely independent after assignment, i.e. modifications in one of the diagrams will not have any effects on the other one.
 *  \param dd2 A template for the assignment.
 *  \return DecisionDiagram& Reference to this decision diagram after assignment.
 */

/*! \fn Node* DecisionDiagram::addNode(std::string label)
 * Adds a node with a certain label to this decision diagram. Note that the label must be unique.
 *  \param label The label for the new node. If the label is not unique, an instance of InvalidDecisionDiagram will be thrown.
 *  \ŗeturn Node* A pointer to the added node.
 *  \throws InvalidDecisionDiagram If the node label is not unique
 */

/*! \fn LeafNode* DecisionDiagram::addLeafNode(std::string label, std::string classification)
 * Adds a leaf node with a certain label and classification to this decision diagram. Note that the label must be unique.
 *  \param label The label for the new node. If the label is not unique, an instance of InvalidDecisionDiagram will be thrown.
 *  \param classification The classification of this leaf node (basically an arbitrary string).
 *  \ŗeturn LeafNode* A pointer to the added leaf node.
 *  \throws InvalidDecisionDiagram If the node label is not unique
 */

/*! \fn Node* DecisionDiagram::addNode(Node* template_)
 * Adds a new node (can be inner or leaf) from a given template. The new node will be exactly the same as the template.
 *  \param template_ Another node which acts as a template for the new one. The new node will have the same attribute values as the template. Note that a copy is created, i.e. the new node and the template will be independent of each other.
 *  \ŗeturn Node* A pointer to the added node.
 *  \throws InvalidDecisionDiagram If the node label is not unique
 */

/*! \fn void DecisionDiagram::removeNode(Node* n)
 * Removes a node from this decision diagram.
 *  \param n A pointer to the node to remove (must be a pointer to exactly the node to remove, not only to one with the same attributes!)
 *  \throws InvalidDecisionDiagram If n is not part of this decision diagram or if n has inzident edges (which must be removed first)
 */

/*! \fn void DecisionDiagram::removeNode(Node* n, bool forceRemoveEdges)
 * Removes a node from this decision diagram.
 *  \param n A pointer to the node to remove (must be a pointer to exactly the node to remove, not only to one with the same attributes!)
 *  \param forceRemoveEdges If true, inzident edges of n will be removed too. Otherwise, removal will be denied if n has inzident edges.
 *  \throws InvalidDecisionDiagram If n is not part of this decision diagram or if n has inzident edges and forceRemoveEdges is false
 */

/*! \fn Edge* DecisionDiagram::addEdge(Node* from, Node* to, Condition c)
 * Adds a directed conditional edge between two nodes of this decision diagram.
 *  \param from A pointer to a node of this decision diagram acting as the edge's source. Note that it must be a pointer to exactly the node to use, not only one with the same attribute values.
 *  \param to A pointer to a node of this decision diagram acting as the edge's destination. Note that it must be a pointer to exactly the node to use, not only one with the same attribute values.
 *  \param c The condition of the edge
 *  \return Edge* A pointer to the added edge
 *  \throws InvalidDecisionDiagram If one of the endpoints is not part of this decision diagram
 */

/*! \fn Edge* DecisionDiagram::addEdge(std::string from, std::string to, Condition c)
 * Adds a directed conditional edge between two nodes of this decision diagram.
 *  \param from The label of the node acting as the edge's source.
 *  \param to The label of the node acting as the edge's destination.
 *  \param c The condition of the edge
 *  \return Edge* A pointer to the added edge
 *  \throws InvalidDecisionDiagram If one of the endpoints is not part of this decision diagram
 */

/*! \fn Edge* DecisionDiagram::addElseEdge(Node* from, Node* to)
 * Adds a directed unconditional (else) edge between two nodes of this decision diagram.
 *  \param from A pointer to a node of this decision diagram acting as the edge's source. Note that it must be a pointer to exactly the node to use, not only one with the same attribute values.
 *  \param to A pointer to a node of this decision diagram acting as the edge's destination. Note that it must be a pointer to exactly the node to use, not only one with the same attribute values.
 *  \return Edge* A pointer to the added edge
 *  \throws InvalidDecisionDiagram If one of the endpoints is not part of this decision diagram
 */

/*! \fn Edge* DecisionDiagram::addElseEdge(std::string from, std::string to)
 * Adds a directed unconditional (else) edge between two nodes of this decision diagram.
 *  \param from The label of the node acting as the edge's source.
 *  \param to The label of the node acting as the edge's destination.
 *  \return Edge* A pointer to the added edge
 *  \throws InvalidDecisionDiagram If one of the endpoints is not part of this decision diagram
 */

/*! \fn Edge* DecisionDiagram::addElseEdge(Edge* template_)
 * Adds a directed (conditional or unconditional) edge between two nodes of this decision diagram. The edge is constructed from a template. The edge to be inserted will be semantically equivalent to the template. However, pointers to the endpoints will be adjusted accordingly, such that not the nodes in the template, but the analougous nodes in this decision diagram (with the same label) will be referenced. The new edge and the template will be completely independent.
 *  \param template_ The template to construct the edge from.
 *  \return Edge* A pointer to the added edge
 *  \throws InvalidDecisionDiagram If one of the endpoints is not part of this decision diagram
 */

/*! \fn void DecisionDiagram::removeEdge(Edge* e)
 * Removes an edge from the decision diagram.
 *  \param e A pointer to the edge to remove. Note that it must be a pointer to exactly the edge to remove, not just to one with the same attribute values.
 *  \throws InvalidDecisionDiagram If e is not part of this decision diagram
 */

/*! \fn void DecisionDiagram::setRoot(Node* root)
 * Changes the root of this decision diagram.
 *  \param root A pointer to the new root node.
 *  \throws InvalidDecisionDiagram If root is not part of this decision diagram
 */

/*! \fn std::set<Node*> DecisionDiagram::getNodes() const
 * Returns a set with the nodes (pointers) of this decision diagram.
 *  \return std::set<Node*> Nodes (pointers) of this decision diagram.
 */

/*! \fn std::set<Edge*> DecisionDiagram::getEdges() const
 * Returns a set with the edges (pointers) of this decision diagram.
 *  \return std::set<Edge*> Edges (pointers) of this decision diagram.
 */

/*! \fn Node* DecisionDiagram::getRoot() const
 * Returns a pointer to the root node of this decision diagram.
 *  \return Node* A pointer to the root node of this decision diagram.
 */

/*! \fn Node* DecisionDiagram::getNodeByLabel(std::string label) const
 * Looks up a node in this decision diagram by it's label.
 *  \return Node* A pointer to the node with the given label.
 *  \throw InvalidDecisionDiagram If the decision diagram does not contain a node with the given label.
 */

/*! \fn int DecisionDiagram::nodeCount() const
 * Returns the number of nodes in this decision diagram.
 *  \return int Number of nodes in this decision diagram.
 */

/*! \fn int DecisionDiagram::leafCount() const
 * Returns the number of leaf nodes in this decision diagram.
 *  \return int Number of leaf nodes in this decision diagram.
 */

/*! \fn int DecisionDiagram::edgeCount() const
 * Returns the number of edges in this decision diagram.
 *  \return int Number of edges in this decision diagram.
 */

/*! \fn std::vector<Node*> DecisionDiagram::bool containsCycles() const
 * Checks if the decision diagram contains at least one cycle. While most kinds of inconsistencies are automatically prevented while the decision diagram is creates, cycles are not detected on the fly because of performance reasons. This method starts a cycle detection algorithm and returns the result.
 *  \return std::vector<Node*> Will be empty if the decision diagram contains no cycle. Otherwise, the result will be the list of nodes building a cycle.
 */

/*! \fn AtomSet DecisionDiagram::toAnswerSet() const
 * Creates an answer set representing this decision diagram with the predicates root(Name), innernode(Label), leafnode(Label, Classification), edge(Node1, Node2, Operand1, comparisonOperator, Operand2) and elseedge(Node1, Node2).
 *  \return AtomSet An answer set representing this decision diagram with the predicates root(Name), innernode(Label), leafnode(Label, Classification), edge(Node1, Node2, Operand1, comparisonOperator, Operand2) and elseedge(Node1, Node2).
 */

/*! \fn std::string DecisionDiagram::toString() const
 * Returns a string representation of this decision diagram.
 *  \return A string representation of this decision diagram.
 */
