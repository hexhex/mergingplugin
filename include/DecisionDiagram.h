#ifndef __DECISIONDIAGRAM_H_
#define __DECISIONDIAGRAM_H_

#include <dlvhex/AtomSet.h>
#include <vector>
#include <string>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
	namespace asp{

		/**
		 * This class provides the data structures and methodes necessary to build decision diagrams from answer sets representing them.
		 */
		class DecisionDiagram{
		public:
			class Node{
			private:
				std::string label;
			public:
				Node(std::string l);
				virtual ~Node();
				void setLabel(std::string l);
				std::string getLabel();
			};

			class LeafNode : public Node{
			private:
				std::string classification;
			public:
				LeafNode(std::string l, std::string c);
				virtual ~LeafNode();
				std::string getClassification();
			};

			class Condition{
			public:
				enum CmpOp{
					lt,
					le,
					eq,
					ge,
					gt
				};
			private:
				std::string op1;
				std::string op2;
				CmpOp op;
			public:
				Condition(std::string op1_, std::string op2_, CmpOp op_);
				virtual ~Condition();
				std::string getOp1();
				std::string getOp2();
				CmpOp getOp();
			};

			class Edge{
			private:
				Node *v1, *v2;
				Condition condition;
			public:
				Edge(Node *u, Node *v, Condition c);
				virtual ~Edge();
				Node* getV1();
				Node* getV2();
				Condition getCondition();
			};

			class ElseEdge : public Edge{
			public:
				virtual ~ElseEdge();
				ElseEdge(Node *u, Node *v);
			};

		private:
			std::vector<Node*> nodes;
			std::vector<Edge*> edges;

			Node* root;
		public:
			Node* getNodeByLabel(std::string label) const;

			DecisionDiagram();
			DecisionDiagram(const DecisionDiagram &dd2);
			DecisionDiagram(AtomSet as);
			~DecisionDiagram();
			DecisionDiagram& operator=(const DecisionDiagram &dd2);

			void addNode(Node* n);
			void addEdge(Edge* e);
			void setRoot(Node* root);

			Node* getNode(int index) const;
			Edge* getEdge(int index) const;
			Node* getRoot() const;

			int nodeCount() const;
			int leafCount() const;
			int edgeCount() const;
			std::vector<Node*> getChildren(Node* n) const;

			AtomSet toAnswerSet();
		};
	}
}

#endif
