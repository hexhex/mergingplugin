#ifndef __OPHAMMINGMIN_H_
#define __OPHAMMINGMIN_H_

#include "IOperator.h"

#include <dlvhex2/Program.h>
#include <dlvhex2/AtomSet.h>

DLVHEX_NAMESPACE_USE

using namespace dlvhex::merging;

namespace dlvhex{
	namespace merging{
		namespace plugin{
			/**
			 * This class implements Dalal's operator (using Dalal's distance measure).
			 * Usage:
			 * &operator["dalal", A, K](A)
			 *	A(H1), ..., A(Hn)	... Handles to n answers
			 *	K(constraint, c)	... c = arbitrary constraints of kind ":-list-of-literals."
			 *	K(constraintfile, f)	... f = the name of a file that contains additional constraints for the group decision
			 *	K(ignore, c)		... c=arbitrary list of predicate names of kind "pred1,pred2,...,predn" that shall be ignored, i.e.
			 *				    it is irrelevant if the truth value of these predicates in the result coincides with the sources
			 *	K(weights, c)		... assigns weight values to the belief bases. c is a string of form "w1,w2,..,wn", where n is
			 *				    the number of knowledge bases and wi are integer value. default weight is 1 for all bases, higher values
			 *				    denote higher impact of this source
			 *	K(aggregate, a)		... "a" is either a program that compuates the value to be optimized or the name of a built-in aggregate function
			 *
			 *				    In case that a program is passed, it may uses
			 *					cost(B,sum,C)
			 *				    to access the total cost C for a certain belief base B
			 *				    	(cost(B,I,C) delivers costs for individual atoms)
			 *				... The program is expected to derive an atom
			 *				    	optimize(C)
			 *				    where C denotes the total costs to be minimized
			 *
			 *	K(penalize, p)		... Sets the costs for a certain kind of difference between an individual's opinion and the aggregated one.
			 *	                            Arbitrary many of penalize entries may occur, where each is a triplet of kind:
			 *	                            	{+,not,-,not-},{+,not,-,not-},int
			 *				    
			 *	                            Example: +,-,10
			 *				    
			 *	                            The first entry refers to an atom in an individual result set, the second one to an entry in the aggregated
			 *	                            decision, the integer is the cost factor for a violation of this constraint (multiplied with the weight of
			 *				    the source)
			 *				    + = positive, e.g. { ab(heater) }
			 *				    not = positive atom not contained, e.g. { }
			 *				    - = strongly negated atom contained, e.g. { -ab(heater) }
			 *				    not- = strongly negated atom not contained, e.g. { }
			 *				    
			 *				    Example: "+,-,10" means, if the individual votes for a positive atom and the group decision is the strongly
			 *				    negated version of the proposition, then the penalty is 10 times the weight of the individual
			 *				    
			 *				    Supported shortcuts: "ignoring" for penaltizing individual's beliefs that are not in the aggregated decision,
			 *								i.e. "+,not,1" and "-,not-,1"
			 *				                         "unfounded" for penaltizing aggregated beliefs that are not in the individual's,
			 *								i.e. "not,+,1"
			 *				                         "aberration" for penaltizing both ignoring and unfounded beliefs
			 *								i.e. "not,+,1", "not,+,1" and "not-,-,1"
			 *				    
			 *				    Built-In aggregate functions are "sum", "max"
			 *	K(maxint, i)		... Defines the maximum integer that may occurrs in the computation of the aggregate function
			 *	                            The operator provides a default value that is high enough for sum aggregate function
			 *	A			... Handle to the answer of the operator result
			 */
			class OpDalal : public IOperator{
			private:
				// helper methods
std::string findUniqueAtomName(const std::string prefix, std::set<std::string>& usedPredNames);

				// preprocessing
				void parseParameters(int arity, OperatorArguments& parameters, std::string optAtom, std::string costSum, std::vector<int>& weights, int& maxint, std::set<std::string>& usedAtoms, std::set<std::string>& ignoredPredicates, dlvhex::Program& program, dlvhex::AtomSet& facts, std::string& aggregation, float penalize[4][4]);
				void parsePenalize(std::string& rule, float penalize[4][4]);

				// subprogram generation
				void buildAggregateFunction(const int arity, const std::string aggregation, const std::string optAtom, const std::string costSum, dlvhex::Program& program, dlvhex::AtomSet& facts);
				void writeAtomSelectionRule(const Atom* atom, dlvhex::Program& program);
				void writeAtomSelectionRule(const std::set<Atom>& usedAtoms, dlvhex::Program& program);
				void createAtomList(const std::vector<HexAnswer*>& arguments, const std::set<std::string>& ignoredPredicates, std::set<Atom>& usedAtoms);
				void writeCostComputation(const AtomPtr costAtom, Literal* lit_individual, Literal* lit_agg, dlvhex::Program& program);
				void renameSourceAtoms(const HexAnswer* source, const std::set<Atom>& sourceAtoms, std::set<std::string>& usedPredNames, std::map<std::string, std::string>& localAtoms);
				void writeAnswerSetSelectionRules(const HexAnswer* source, const std::map<std::string, std::string>& localAtomMapping, const std::set<std::string>& ignoredPredicates, std::set<std::string>& usedPredNames, dlvhex::Program& program);
				void writeCostComputationRules(const HexAnswer* source, const std::map<std::string, std::string>& localAtomMapping, const std::set<Atom>& sourceAtoms, const std::set<std::string>& ignoredPredicates, const std::set<std::string>& usedPredNames, const std::string costAtom, const std::string costSumAtom, const int weight, const float penalize[4][4], int& maxint, dlvhex::Program& program);
				void writeSumRule(const int sourceNr, const std::string costAtom, const std::string costSumAtom, dlvhex::Program& program);

				// postprocessing
				void optimize(HexAnswer& result, std::string optAtom);
			public:
				virtual std::string getName();
				virtual std::string getInfo();
				virtual std::set<std::string> getRecognizedParameters();
				virtual HexAnswer apply(bool debug, int arity, std::vector<HexAnswer*>& answers, OperatorArguments& parameters) throw (OperatorException);
			};
		}
	}
}


/*! \fn std::string findUniqueAtomName(const std::string prefix, std::set<std::string>& usedPredNames)
 * Finds an predicate name that was not used yet and appends it to the list of used predicate names
 * \param prefix The desired prefix of the predicate
 * \param usedPredNames Reference to the list of predicate names used so far
 */

/*! \fn void parseParameters(int arity, OperatorArguments& parameters, std::string optAtom, std::string costSum, std::vector<int>& weights, int& maxint, std::set<std::string>& usedAtoms, std::set<std::string>& ignoredPredicates, dlvhex::Program& program, dlvhex::AtomSet& facts, std::string& aggregation, float penalize[4][4]);
 * Parses the following parameters:
 * 	-) constraint: "some constraint"
 * 	-) constraintfile: "some filename"
 * 	-) weights: "w1,...,wn"
 * 	-) penalize "individual,aggregated,factor"
 * 	-) maxint: "integer"
 * 	-) ignore: "pred1,...,predn"
 * \param arity The number of answer arguments
 * \param optAtom The unique name of the atom upon which optimization occurs
 * \param costSum The unique name of the atom where sourcewise cost aggregation occurs
 * \param weights Reference to the vector where the weights shall be written to
 * \param usedAtoms The list of atoms occurring in any source program
 * \param maxint Reference to the integer where the maximum int value shall be written to
 * \param ignoredPredicates Reference to the set where the ignored predicates shall be written to
 * \param program Reference to the program where constraints shall be appended
 * \param facts Reference to the facts part of the program where constraints shall be appended
 * \param aggregation Reference to the string where the aggregation function shall be written to
 */

/*! \fn void parsePenalize(std::string& rule, float penalize[4][4]);
 * Parses a penalize rule.
 * \param rule Some rule (either shortcuts "ignoring", "unfounded", "aberration" or triples of kind "{+,not,-,not-},{+,not,-,not-},int")
 * \param penalize Pointer to a 4x4 float matrix where the new rule is added (where only elements that are explicitly reset will be overwritten)
 */

/*! \fn void buildAggregateFunction(const int arity, const std::string aggregation, const std::string optAtom, const std::string costSum, dlvhex::Program& program, dlvhex::AtomSet& facts);
 * Writes code for computing the aggregation function
 * Either the computation rule is directly passed in argument "std::string aggregation", or a (supported) shortcut like "sum" or "max" is used; in the latter case
 * the function will generate the appropriate code automatically
 * \param aggregation Either user-defined rules of one of the strings "sum", "max"
 * \param optAtom The name of the atom upon which to minimize
 * \param costSum The name of the atom where sourcewise costs are sumed up
 * \param usedAtoms The list of atoms occurring somewhere in the sources
 * \param program Reference to the program where rules shall be added
 * \param facts Reference to the facts part of the program where rules shall be added
 */

/*! \fn void writeAtomSelectionRule(const Atom* atom, dlvhex::Program& program);
 * Writes a disjunctive rule that either selects an atom or it's negated version
 * \param atom Some (positive) atom
 * \param program The program where the rule shall be appended
 */

/*! \fn void writeAtomSelectionRule(const std::set<Atom>& usedAtoms, dlvhex::Program& program);
 * Writes the atom selection rule for each atom in the given list (see overloaded version of this method).
 * \param list A reference to the list of a atoms for which to write selection rules.
 * \param program A reference to the program to append the rules
 */

/*! \fn void void createAtomList(const std::vector<HexAnswer*>& arguments, const std::set<std::string>& ignoredPredicates, std::set<Atom>& usedAtoms);
 * Extracts all atoms from the sources and writes it into a list.
 * Additionally it writes the rules that make sure that each atom is either accepted or denied (strong negation)
 * \param arguments The vector of answers passed to the operator
 * \param usedAtoms A reference to the list where atoms shall be written to
 * \param ignoredPredicates A reference to the list of atoms that shall be ignored
 * \param program A reference to the program where rules shall be appended to
 */

/*! \fn writeCostComputation(const AtomPtr costAtom, Literal* lit_individual, Literal* lit_agg, dlvhex::Program& program);
 * Adds rules that bind the new belief base specific atom to the final one.
 * If possible, the belief base specific (renamed) attribute should be equal to the final one.
 * i.e.:
 *	cost(BB, RR, c) :- a123, not a.
 *	cost(BB, RR, c) :- -a123, not -a.
 * where BB is a unique number for each belief base, RR is a unique index for each atom
 * and c the weight for this belief base.
 * \param costAtom The rule's head atom
 * \param lit_individual The first literal of the rule's body
 * \param lit_agg The second literal of the rule's body
 * \param program Reference to the program where the rules shall be appended
 */

/*! \fn std::map<std::string, std::string> renameSourceAtoms(const HexAnswer* source, const std::set<Atom>& sourceAtoms, std::set<std::string>& usedPredNames);
 * Renames the atoms of a given source such that they are unique within the program
 * \param source Pointer to the source to
 * \param sourceAtoms Vector of ALL atoms in ANY source (not only those that occur in this one); we even need unique names for atoms that never occur in this source for cost model "unfounded"
 * \param usedPredNames Reference to the set of already used predicate names (is expanded by this method)
 * \return std::map<std::string, std::string> A mapping of each name occurring in the source atoms onto a unique name
 */

/*! \fn void writeAnswerSetSelectionRules(const HexAnswer* source, const std::map<std::string, std::string>& localAtomMapping, const std::set<std::string>& ignoredPredicates, std::set<std::string>& usedPredNames, dlvhex::Program& program);
 * Writes rules that select exactly one of a certain source's answer-sets and derive all the atoms in this set in case that it is selected.
 * i.e. if the source has answer-sets {a},{b}, it will generate the rules:
 *   as0 v as1.
 *   a :- as0.
 *   b :- as1.
 * \param source A pointer to the source
 * \param localAtomMapping A reference to the mapping of global atom names onto the source's
 * \param ignoredPredicates A reference to the set of irrelevant predicate names
 */

/*! \fn void writeCostComputationRules(const HexAnswer* source, const std::map<std::string, std::string>& localAtomMapping, const std::set<Atom>& sourceAtoms, const std::set<std::string>& ignoredPredicates, const std::set<std::string>& usedPredNames, const std::string costAtom, const std::string costSumAtom, const int weight, const float penalize[4][4], int& maxint, dlvhex::Program& program);
 * Given a certain source. This method writes all the rules that compute the costs for derivations of the aggregated decision from this individual's one.
 * \param source A pointer to the source
 * \param localAtomMapping A reference to the mapping of global atom names onto the source's
 * \param sourceAtoms The atoms occurring in any of the sources
 * \param ignoredPredicates A reference to the set of irrelevant predicate names
 * \param usedPredNames The names of the predicate names used so far
 * \param costAtom The name of the atom where costs of individual constraint violations are summed up
 * \param costSumAtom The name of the atom where costs of individuals are summed up
 * \param weight Weight of this source
 * \param penalize The cost model
 * \param maxint Reference to the maximum integer value occurring in the program or in an intermediate result (will be raised by this function if necessary)
 * \param program Reference to the program where rules shall be appended
 */

/*! \fn void writeSumRule(const int sourceNr, const std::string costAtom, const std::string costSumAtom, dlvhex::Program& program);
 * Writes a rule of kind
 *   costSum(SRC, S) :- #int(S), S=#sum{ Cost,Constraint : cost(SRC, Constraint, Cost) }.
 * to sum up the costs for constraint violations for a certain source
 * \param sourceNr The unique number of the source
 * \param program The program where rules shall be appended
 */

/*! \fn void optimize(HexAnswer& result, std::string optAtom);
 * Keeps only the minimum-costs answer-sets in "result" and removes the others
 * This is normally already done by DLV. But due to the fact that DLVresultParserDriver does not support parsing outputs with cost values, we need to
 * replace weak constraints by normal rules that derive their penalty in the head. This method sums up the penalties and computes the optimal answer-sets.
 * \param optAtom Tells the function upon which (unary) predicate to minimize
 */

#endif
