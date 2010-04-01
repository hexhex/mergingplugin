#include <OpDalal.h>

#include <dlvhex/AggregateAtom.h>
#include <dlvhex/DLVProcess.h>
#include <dlvhex/ASPSolver.h>
#include <dlvhex/Registry.h>
#include <dlvhex/HexParserDriver.h>

#include <dlvhex/PrintVisitor.h>
#include <DLVHexProcess.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

#include <boost/algorithm/string.hpp>

using namespace dlvhex;
using namespace dlvhex::merging;
using namespace dlvhex::merging::plugin;

std::string OpDalal::getName(){
	return "dalal";
}

std::string OpDalal::getInfo(){
	std::stringstream ss;
	ss <<	"dalal" << std::endl <<
		"-----" << std::endl << std::endl <<
		 " &operator[\"dalal\", A, K](A)" << std::endl <<
		 "	A(H1), ..., A(Hn)	... Handles to n answers" << std::endl <<
		 "	K(constraint, c)	... c = arbitrary constraints of kind \":-list-of-literals.\"" << std::endl <<
		 "	K(constraintfile, f)	... f = the name of a file that contains additional constraints for the group decision" << std::endl <<
		 "	K(ignore, c)		... c=arbitrary list of predicate names of kind \"pred1,pred2,...,predn\" that shall be ignored, i.e." << std::endl <<
		 "				    it is irrelevant if the truth value of these predicates in the result coincides with the sources" << std::endl <<
		 "	K(weights, c)		... assigns weight values to the belief bases. c is a string of form \"w1,w2,..,wn\", where n is" << std::endl <<
		 "				    the number of knowledge bases and wi are integer value. default weight is 1 for all bases, higher values" << std::endl <<
		 "				    denote higher impact of this source" << std::endl <<
		 "	K(aggregate, a)		... \"a\" is either a program that compuates the value to be optimized or the name of a built-in aggregate function" << std::endl <<
		 "" << std::endl <<
		 "				    In case that a program is passed, it may uses" << std::endl <<
		 "					cost(B,sum,C)" << std::endl <<
		 "				    to access the total cost C for a certain belief base B" << std::endl <<
		 "				    	(cost(B,I,C) delivers costs for individual atoms)" << std::endl <<
		 "				... The program is expected to derive an atom" << std::endl <<
		 "				    	optimize(C)" << std::endl <<
		 "				    where C denotes the total costs to be minimized" << std::endl <<
		 "" << std::endl <<
		 "	K(penalize, p)		... Sets the costs for a certain kind of difference between an individual's opinion and the aggregated one." << std::endl <<
		 "	                            Arbitrary many of penalize entries may occur, where each is a triplet of kind:" << std::endl <<
		 "	                            	{+,not,-,not-},{+,not,-,not-},int" << std::endl <<
		 "				    " << std::endl <<
		 "	                            Example: +,-,10" << std::endl <<
		 "" << std::endl <<				    
		 "	                            The first entry refers to an atom in an individual result set, the second one to an entry in the aggregated" << std::endl <<
		 "	                            decision, the integer is the cost factor for a violation of this constraint (multiplied with the weight of" << std::endl <<
		 "				    the source)" << std::endl <<
		 "				    + = positive, e.g. { ab(heater) }" << std::endl <<
		 "				    not = positive atom not contained, e.g. { }" << std::endl <<
		 "				    - = strongly negated atom contained, e.g. { -ab(heater) }" << std::endl <<
		 "				    not- = strongly negated atom not contained, e.g. { }" << std::endl <<
		 "				    " << std::endl <<
		 "				    Example: \"+,-,10\" means, if the individual votes for a positive atom and the group decision is the strongly" << std::endl <<
		 "				    negated version of the proposition, then the penalty is 10 times the weight of the individual" << std::endl <<
		 "" << std::endl <<				    
		 "				    Supported shortcuts: \"ignoring\" for penaltizing individual's beliefs that are not in the aggregated decision," << std::endl <<
		 "								i.e.\"+,not,1\" and \"-,not-,1\"" << std::endl <<
		 "				                         \"unfounded\" for penaltizing aggregated beliefs that are not in the individual's" <<
		 "								i.e. \"not,+,1\" and \"not-,-,1\"" << std::endl <<
		 "				                         \"aberration\" for penaltizing both ignoring and unfounded beliefs" << std::endl <<
		 "								i.e. \"not,+,1\", \"not-,-,1\", \"not,+,1\" and \"not-,-,1\"" << std::endl <<
		 "" << std::endl <<
		 "				    Built-In aggregate functions are \"sum\", \"max\"" << std::endl <<
		 "	K(maxint, i)		... Defines the maximum integer that may occurrs in the computation of the aggregate function" << std::endl <<
		 "	                            The operator provides a default value that is high enough for sum aggregate function" << std::endl <<
		 "	A			... Handle to the answer of the operator result" << std::endl;
	return ss.str();
}

// finds an atom name that was not used yet
std::string findUniqueAtomName(std::string prefix, std::set<std::string>& usedatoms){
	static int ri = 0;
	std::string name;
	do{
		std::stringstream ss;
		ss << prefix << (ri++);
		name = ss.str();
	}while (usedatoms.find(name) != usedatoms.end());
	return name;
}


/**
 * Writes a disjunctive rule that either selects an atom or it's negated version
 * \param atom Some (positive) atom
 * \param program The program where the rule shall be appended
 */
void writeAtomSelectionRule(const Atom* atom, dlvhex::Program& program){

	// make sure that each atom is either positive or negative
	// note: this binary behaviour is reasonable since each atom occurrs in at least one answer-set. thus, the answer "unknown" is senseless because
	// we always have evidences for positive or negative

	// construct literals
	Literal* lit_Pos = new Literal(AtomPtr(new Atom(*atom))); if (lit_Pos->getAtom()->isStronglyNegated()) lit_Pos->getAtom()->negate();
	Literal* lit_Neg = new Literal(AtomPtr(new Atom(*atom))); if (!lit_Neg->getAtom()->isStronglyNegated()) lit_Neg->getAtom()->negate();
	Registry::Instance()->storeObject(lit_Pos);
	Registry::Instance()->storeObject(lit_Neg);

	// add rules of kind: "a v -a."
	RuleHead_t head_SetAtom;
	head_SetAtom.insert(lit_Pos->getAtom());
	head_SetAtom.insert(lit_Neg->getAtom());
	Rule* rule_SetAtom = new Rule(head_SetAtom, RuleBody_t());
	Registry::Instance()->storeObject(rule_SetAtom);
	program.addRule(rule_SetAtom);
}


/**
 * Extracts all atoms from the sources and writes it into a list.
 * Additionally it writes the rules that make sure that each atom is either accepted or denied (strong negation)
 * \param arguments The vector of answers passed to the operator
 * \param usedAtoms A reference to the list where atoms shall be written to
 * \param ignoredPredicates A reference to the list of atoms that shall be ignored
 * \param program A reference to the program where rules shall be appended to
 */
void createAtomList(std::vector<HexAnswer*>& arguments, std::set<Atom>& usedAtoms, std::set<std::string>& ignoredPredicates, dlvhex::Program& program){
	for (std::vector<HexAnswer*>::iterator argIt = arguments.begin(); argIt != arguments.end(); argIt++){
		// look into each answer-set
		for (std::vector<AtomSet>::iterator asIt = (*argIt)->begin(); asIt != (*argIt)->end(); asIt++){
			// extract atoms and arities
			for (AtomSet::const_iterator atIt = asIt->begin(); atIt != asIt->end(); atIt++){
				// check if this atom is relevant
				if (ignoredPredicates.find(atIt->getPredicate().getUnquotedString()) == ignoredPredicates.end()){
					Atom at(*atIt);
					if (at.isStronglyNegated()) at.negate();
					if (usedAtoms.find(at) == usedAtoms.end()) usedAtoms.insert(at);
					writeAtomSelectionRule(&(*atIt), program);
				}
			}
		}
	}
}


/**
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
void writeCostComputation(AtomPtr costAtom, Literal* lit_individual, Literal* lit_agg, dlvhex::Program& program){

	// if possible, the final atom should have the same value than the one in the current source
	dlvhex::RuleBody_t body_bind;
	body_bind.insert(lit_individual);
	body_bind.insert(lit_agg);

	// derive costs in case of constraint violation
	RuleHead_t head_costatom;
	head_costatom.insert(costAtom);
	Rule* cc = new Rule(head_costatom, body_bind);
	Registry::Instance()->storeObject(cc);
	program.addRule(cc);
}


/**
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
void buildAggregateFunction(std::string aggregation, std::string optAtom, std::string costSum, int arity, std::set<std::string>& usedAtoms, dlvhex::Program& program, dlvhex::AtomSet& facts){

	// create predefined aggregate function (if requested)
	std::stringstream aggregationss;
	if (aggregation == std::string("sum")){
		std::stringstream ss;
		ss << "optimize(S" << (arity - 1) << ") :- ";
		for (int i = 0; i < arity; i++){
			if (i > 0) ss << ", S" << i << "=S" << (i - 1) << "+V" << i << ", ";
			ss << "cost(" << i << ", " << (i == 0 ? "S" : "V") << i << ")";
		}
		ss << ".";
		aggregation = ss.str();
	}else if (aggregation == std::string("max")){
		std::stringstream ss;
		for (int i = 0; i < arity; i++){
			ss << "optimize(M) :- cost(" << i << ", M)";
			for (int j = 0; j < arity; j++) ss << ", cost(" << j << ", V" << j << "), M >= V" << j;
			ss << ".";
		}
		aggregation = ss.str();
	}

	// make sure that the aggregate function uses unique predicate names
	boost::algorithm::replace_all(aggregation, "cost", costSum);
	boost::algorithm::replace_all(aggregation, "optimize", optAtom);
	aggregationss << aggregation;

	// optimization criterion
	aggregationss << ":~ " << optAtom << "(C). [C:1]";

	// parse the aggregation function
	HexParserDriver hpd;
	dlvhex::Program cfprogram;
	dlvhex::AtomSet cffacts;
	try{
		hpd.parse(aggregationss, cfprogram, cffacts);
		// replace predicate name "cost" by costAtom
		for (Program::iterator rule = cfprogram.begin(); rule != cfprogram.end(); ++rule) program.addRule(*rule);
		facts.insert(cffacts);
	}catch(SyntaxError){
		throw IOperator::OperatorException(std::string("Could not parse aggregation function: \"") + aggregationss.str() + std::string("\""));
	}
}


/**
 * Adds an input source to the program and facts
 * \param source Pointer to the source to add
 * \param weight The numeric weight of this source
 * \param maxInt Reference to the maximum integer value we request from dlv; the source raises this value as needed
 * \param costSumAtom The unique name of the atom where costs per source are sumed up
 * \param costAtom The unique name of the atom where costs per constraint violation are computed
 * \param penalize The penalize model as 4x4 float matrix
 * \param usedAtoms Reference to the list of atoms occurring somewhere in the sources (is expanded by this function)
 * \param ignoredPredicates Reference to the list of predicates that are ignored since they are only used as intermediate predicates
 * \param program Reference to the program where rules for this source shall be written to
 * \param facts Reference to the facts part of the program where rules for this source shall be written to
 * \return HexAnswer The set of answer-sets from this source, renamed such that atoms are unique over all sources
 */
HexAnswer OpDalal::addSource(HexAnswer* source, int weight, int& maxint, std::string costSumAtom, std::string costAtom, float penalize[4][4], std::set<Atom>& sourceAtoms, std::set<std::string>& usedatoms, std::set<std::string>& ignoredPredicates, dlvhex::Program& program, dlvhex::AtomSet& facts){

	// some atoms that make sure that exactly one of the answer-sets will be selected
	dlvhex::RuleHead_t head_asSelection;

	std::vector<AtomSet> newsource;	// will contain the answer-sets with the renamed literals

	// we need a unique name for each atom
	int constraintNr = 0;
	static int sourceNr = 0;
	std::map<std::string, std::string> localAtoms;
	for (std::set<Atom>::iterator it = sourceAtoms.begin(); it != sourceAtoms.end(); ++it){
		if (localAtoms.find(it->getPredicate().getUnquotedString()) == localAtoms.end()){
			localAtoms[it->getPredicate().getUnquotedString()] = findUniqueAtomName(it->getPredicate().getUnquotedString(), usedatoms);
			usedatoms.insert(localAtoms[it->getPredicate().getUnquotedString()]);
		}

		const Atom* atom_orig = &(*it);
		AtomPtr atom_new = AtomPtr(new Atom(*it));
		atom_new->setPredicate(Term(localAtoms[it->getPredicate().getUnquotedString()]));

		// add constraints that binds the new atom to the original one
		// if possible, the renamed attribute should be equal to the original one
		for (int individual = 0; individual < 4; individual++){
			for (int agg = 0; agg < 4; agg++){
				if (penalize[individual][agg] > 0){
					Tuple params;
					params.push_back(Term(sourceNr));	// unique name for the belief base
					params.push_back(Term(constraintNr));	// unique name for this constraint
					params.push_back(Term((int)(weight * penalize[individual][agg])));
					maxint += (int)(weight * penalize[individual][agg]);
					AtomPtr atom_individual = AtomPtr(new Atom(*atom_new));
					if (individual >= 2) atom_individual->negate();
					AtomPtr atom_agg = AtomPtr(new Atom(*atom_orig));
					if (agg >= 2) atom_agg->negate();
					Literal* lit_individual = new Literal(atom_individual, individual == 1 || individual == 3);
					Literal* lit_agg = new Literal(atom_agg, agg == 1 || agg == 3);
					writeCostComputation(AtomPtr(new Atom(costAtom, params)), lit_individual, lit_agg, program);
					Registry::Instance()->storeObject(lit_individual);
					Registry::Instance()->storeObject(lit_agg);
					constraintNr++;
				}
			}
		}
	}

	// for all answer-sets of this source
	for (std::vector<AtomSet>::iterator asIt = source->begin(); asIt != source->end(); asIt++){

		// make a unique prop. atom that decides if this answer-set is selected or not
		AtomPtr atom_asSelection = AtomPtr(new Atom(findUniqueAtomName("as_", usedatoms).c_str()));
		// a disjunction of all those atoms will select exactly one
		head_asSelection.insert(dlvhex::AtomPtr(atom_asSelection));
		usedatoms.insert(atom_asSelection->getPredicate().getUnquotedString());	// name was consumed

		AtomSet currentAS;
		// for all atoms of this answer-set
		for (AtomSet::const_iterator atIt = asIt->begin(); atIt != asIt->end(); atIt++){
			// check if this atom is relevant or ignored
			if (ignoredPredicates.find(atIt->getPredicate().getUnquotedString()) == ignoredPredicates.end()){
				AtomPtr atom_new = AtomPtr(new Atom(*atIt));
				atom_new->setPredicate(Term(localAtoms[atIt->getPredicate().getUnquotedString()]));

				// add the renamed atom to the new source
				currentAS.insert(atom_new);

				// if the current answer-set is selected, then the contained atoms can be derived
				RuleHead_t head_deriveAtom;
				head_deriveAtom.insert(atom_new);
				RuleBody_t body_deriveAtom;
				Literal* lit_asSelection = new Literal(atom_asSelection);
				body_deriveAtom.insert(lit_asSelection);
				Rule* rule_deriveAtom = new Rule(head_deriveAtom, body_deriveAtom);
				Registry::Instance()->storeObject(rule_deriveAtom);
				Registry::Instance()->storeObject(lit_asSelection);
				program.addRule(rule_deriveAtom);

			}
		}
		// answer-set completed: add it to the source
		newsource.push_back(currentAS);
	}


	// costSum(SRC, S) :- #int(S), S=#sum{ Cost,Constraint : cost(SRC, Constraint, Cost) }.
	//
	// make sure that we do not use too many variable per rule

	// Remember the setting of "NoPredicate"
	unsigned noPred = Globals::Instance()->getOption("NoPredicate");
	Globals::Instance()->setOption("NoPredicate", 0);

	Tuple sumvars;
	sumvars.push_back(Term("Cost"));
	sumvars.push_back(Term("Constraint"));

	// costsum(SRC, S)
	RuleHead_t head_sum;
	Tuple headArgs;
	headArgs.push_back(Term(sourceNr));
	headArgs.push_back(Term("S"));
	AtomPtr atom_costsum(new Atom(costSumAtom, headArgs));
	head_sum.insert(atom_costsum);

	// cost(SRC, Constraint, Cost)
	Tuple bodyArgs;
	bodyArgs.push_back(Term(sourceNr));
	bodyArgs.push_back(Term("Constraint"));
	bodyArgs.push_back(Term("Cost"));
	AtomPtr atom_costquery(new Atom(costAtom, bodyArgs));
	Literal* lit_costquery = new Literal(atom_costquery);
	Registry::Instance()->storeObject(lit_costquery);

	// : cost(SRC, Constraint, Cost
	RuleBody_t body_aggquery;
	body_aggquery.insert(lit_costquery);

	// S=#sum{ Cost,Constraint : cost(SRC, Constraint, Cost)
	AggregateAtom* agg = new AggregateAtom("#sum", sumvars, body_aggquery);
	agg->setLeftTerm(Term("S"));
	agg->setComp("=", "");
	AtomPtr atom_sum = AtomPtr(agg);
	Literal* lit_sum = new Literal(atom_sum);

	// #int(S)
	Tuple intArgs;
	intArgs.push_back(Term("S"));
	AtomPtr atom_int = AtomPtr(new Atom("#int", intArgs));
	Literal* lit_int = new Literal(atom_int);

	// :- #int(S), S=#sum{ Cost,Constraint : cost(SRC, Constraint, Cost)
	RuleBody_t body_sum;
	body_sum.insert(lit_sum);
	body_sum.insert(lit_int);
	Rule* rule_sum = new Rule(head_sum, body_sum);
	Registry::Instance()->storeObject(rule_sum);
	Registry::Instance()->storeObject(lit_sum);

	program.addRule(rule_sum);

	// make sure that one of the answer-sets of this source is selected
	// (at least one -> minimality of answer-sets will make sure that also at most one is selected)
	Rule* rule_asSelection = new Rule(head_asSelection, dlvhex::RuleBody_t());
	Registry::Instance()->storeObject(rule_asSelection);
	program.addRule(rule_asSelection);

	sourceNr++;
	return newsource;
}


/**
 * Keeps only the minimum-costs answer-sets in "result" and removes the others
 * This is normally already done by DLV. But due to the fact that DLVresultParserDriver does not support parsing outputs with cost values, we need to
 * replace weak constraints by normal rules that derive their penalty in the head. This method sums up the penalties and computes the optimal answer-sets.
 * \param optAtom Tells the function upon which (unary) predicate to minimize
 */
void OpDalal::optimize(HexAnswer& result, std::string optAtom){
	// answer cleaning: only keep those answer-sets that have to pay the minimum penalty for weak constraint violation
	// unfortuanately we need to compute the costs ourselves since the DLVresultParserDriver is not able to parse DLV's cost output

	std::map<AtomSet*, int> weights;
	int bestWeight = -1;	// none found so far
	for (HexAnswer::iterator argIt = result.begin(); argIt != result.end(); argIt++){
		// take only those answer-sets with the least penalty for weak constraint violation
		int weight = 0;
		for (AtomSet::const_iterator atIt = argIt->begin(); atIt != argIt->end(); atIt++){
			if (atIt->getPredicate().getUnquotedString() == optAtom){
				// sum up the weight
				assert(atIt->getArgument(atIt->getArity()).isInt());
				weight += atIt->getArgument(atIt->getArity()).getInt();
			}
		}
		weights[&(*argIt)] = weight;

		// check if this weight is better than the best previous one
		if (bestWeight == -1 || weight < bestWeight){
			bestWeight = weight;
		}
	}

	// now remove all answer-sets that have a weight higher than the best weight
	std::vector<HexAnswer::iterator> delIt;
	for (HexAnswer::iterator argIt = result.begin(); argIt != result.end(); argIt++){
		if (weights[&(*argIt)] > bestWeight){
			delIt.push_back(argIt);
		}
	}
	for (int i = delIt.size() - 1; i >= 0; i--){
		result.erase(delIt[i]);
	}

	// remove duplicates
	for (int i = result.size() - 1; i >= 0; --i){
		// further occurrence?
		bool found = false;
		for (int j = i - 1; j >= 0; j--) if (result[i] == result[j]) found = true;
		if (found){
			result.erase(result.begin() + i);
		}
	}
}


/**
 * Parses a penalize rule.
 * \param rule Some rule (either shortcuts "ignoring", "unfounded", "aberration" or triples of kind "{+,not,-,not-},{+,not,-,not-},int")
 * \param penalize Pointer to a 4x4 float matrix where the new rule is added (where only elements that are explicitly reset will be overwritten)
 */
void parsePenalize(std::string rule, float penalize[4][4]){
	// shortcuts
	if (rule == "ignoring"){
		// +,not
		// -,not-
		penalize[0][1] = 1; penalize[2][3] = 1;
	}
	else if (rule == "unfounded"){
		// not,+
		//// not-,-
		penalize[1][0] = 1; //penalize[3][2] = 1;
	}
	else if (rule == "aberration"){
		// +,not
		// -,not-
		// not,+
		//// not-,-
		penalize[0][1] = 1; penalize[2][3] = 1; penalize[1][0] = 1; //penalize[3][2] = 1;
	}else{
		std::vector<std::string> p; //split(argIt->second, ',');
		boost::split(p, rule, boost::is_any_of(","));

		if (p.size() != 3) throw IOperator::OperatorException(std::string("Invalid penalize definition \"") + rule + std::string("\". Must be of kind \"{+,not,-,not-},{+,not,-,not-},int\""));
		int individual = -1;
		int agg = -1;
		if (p[0] == std::string("+")) individual = 0; 	if (p[0] == std::string("not")) individual = 1;	if (p[0] == std::string("-")) individual = 2;	if (p[0] == std::string("not-")) individual = 3;
		if (p[1] == std::string("+")) agg = 0; 		if (p[1] == std::string("not")) agg = 1;	if (p[1] == std::string("-")) agg = 2;		if (p[1] == std::string("not-")) agg = 3;
		float factor = atof(p[2].c_str());
		if (individual < 0 || agg < 0 || (factor == 0 && p[2] != std::string("0"))){
			throw IOperator::OperatorException(std::string("Invalid penalize definition \"") + rule + std::string("\". Must be of kind \"{+,not,-,not-},{+,not,-,not-},int\""));
		}else{
			penalize[individual][agg] = factor;
		}
	}
}


/**
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
void parseParameters(int arity, OperatorArguments& parameters, std::string optAtom, std::string costSum, std::vector<int>& weights, int& maxint, std::set<std::string>& usedAtoms, std::set<std::string>& ignoredPredicates, dlvhex::Program& program, dlvhex::AtomSet& facts, std::string& aggregation, float penalize[4][4]){

	bool penalizeSet = false;

	// process additional parameters
	for (OperatorArguments::iterator argIt = parameters.begin(); argIt != parameters.end(); argIt++){

		// add side constraints
		if (argIt->first == std::string("constraint")){
			std::string constraint = argIt->second;
			// parse it
			try{
				HexParserDriver hpd;
				std::stringstream ss(constraint);
				hpd.parse(ss, program, facts);
			}catch(SyntaxError){
				throw IOperator::OperatorException(std::string("Could not parse constraint due to a syntax error: \"") + argIt->second + std::string("\""));
			}
		}else if (argIt->first == std::string("constraintfile")){
			try{
				// read input
				std::ifstream inp;
				inp.open(argIt->second.c_str());
				std::string s;
				std::stringstream constraints;
				while (inp.good() && std::getline(inp, s)){
					constraints << s << std::endl;
				}

				// parse it
				std::stringstream ss(constraints.str());
				HexParserDriver hpd;
				hpd.parse(ss, program, facts);
			}catch(SyntaxError){
				throw IOperator::OperatorException(std::string("Could not parse constraint file due to a syntax error: \"") + argIt->second + std::string("\""));
			}

		// weight of knowledge bases
		}else if (argIt->first == std::string("weights")){
			std::vector<std::string> w; //split(argIt->second, ',');
			boost::split(w, argIt->second, boost::is_any_of(","));
			weights.clear();
			if (w.size() != arity) throw IOperator::OperatorException(std::string("Invalid number of weight values: \"") + argIt->second + std::string("\"; must be equal to arity!"));
			for (int i = 0; i < arity; i++){
				if (atoi(w[i].c_str()) <= 0) throw IOperator::OperatorException(std::string("Invalid weight value: \"") + w[i] + std::string("\"; must be a positive integer!"));
				weights.push_back(atoi(w[i].c_str()));
			}

		// penalize function
		}else if (argIt->first == std::string("penalize")){
			parsePenalize(argIt->second, penalize);
			penalizeSet = true;

		// extract aggregation function
		}else if (argIt->first == std::string("aggregate")){
			aggregation = argIt->second;

		// extract maxint
		}else if (argIt->first == std::string("maxint")){
			maxint = atoi(argIt->second.c_str());
			if (maxint <= 0) throw IOperator::OperatorException(std::string("maxint must be a positive integer. \"") + argIt->second + std::string("\" was passed"));

		// extract predicates to ignore
		}else if (argIt->first == std::string("ignore")){
			std::vector<std::string> preds; //split(argIt->second, ',');
			boost::split(preds, argIt->second, boost::is_any_of(","));
			ignoredPredicates.insert(preds.begin(), preds.end());
		}
	}

	// built appropriate aggregate function
	buildAggregateFunction(aggregation, optAtom, costSum, arity, usedAtoms, program, facts);
//void buildAggregateFunction(std::string aggregation, std::string optAtom, std::string costSum, int arity, std::set<std::string>& usedAtoms, dlvhex::Program& program, dlvhex::AtomSet& facts){
	// default value for penalize (if not used defined)
	if (!penalizeSet){
		penalize[0][0] = 0; penalize[0][1] = 1; penalize[0][2] = 0; penalize[0][3] = 0;
		penalize[1][0] = 0; penalize[1][1] = 0; penalize[1][2] = 0; penalize[1][3] = 0;
		penalize[2][0] = 0; penalize[2][1] = 0; penalize[2][2] = 0; penalize[2][3] = 1;
		penalize[3][0] = 0; penalize[3][1] = 0; penalize[3][2] = 0; penalize[3][3] = 0;
	}
}

HexAnswer OpDalal::apply(bool debug, int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters) throw (OperatorException){

	// make dlvhex believe we work in firstorder mode
	// (otherwise we can not instanciate AggregateAtom)
	unsigned noPred = Globals::Instance()->getOption("NoPredicate");
	Globals::Instance()->setOption("NoPredicate", 0);

	// default values
	std::string aggregation = "sum";
	int maxint = 0;

	dlvhex::Program program;
	dlvhex::AtomSet facts;
	std::set<std::string> ignoredPredicates;
	std::vector<int> weights;
	for (int i = 0; i < arity; i++)	// default value
		weights.push_back(1);

	// construct a logic program that computes the answer as follows:
	//	- rename all atoms such that they are unique for each belief base, i.e. no atom occurs in multiple sources
	//	- guess final atoms: each one can be true or false
	//	- add weak constraints, such that the final atoms are as similar to the sources as possible

	// create a collection of all occurring atoms
	std::set<std::string> usedAtoms;
	std::set<Atom> sourceAtoms;
	createAtomList(arguments, sourceAtoms, ignoredPredicates, program);
	for (std::set<Atom>::iterator it = sourceAtoms.begin(); it != sourceAtoms.end(); ++it) usedAtoms.insert(it->getPredicate().getUnquotedString());

	// create a unique optimization atom
	std::string optAtom = findUniqueAtomName("optimize", usedAtoms);
	usedAtoms.insert(optAtom);

	// create a unique cost atoms for intermedicate results and final sum (for each source)
	std::string costAtom = findUniqueAtomName("cost", usedAtoms);
	std::string costSum = findUniqueAtomName("costSum", usedAtoms);
	usedAtoms.insert(costAtom);
	usedAtoms.insert(costSum);



	// parse parameters (may throws an exception)
	float penalize[4][4];	// 0=positive, 1=defneg, 2=strong neg, 3=def and strong neg
				// first dimension: individuals
				// second dimension: aggregated decision
	memset(penalize, 0, 16 * sizeof(float));
	parseParameters(arity, parameters, optAtom, costSum, weights, maxint, usedAtoms, ignoredPredicates, program, facts, aggregation, penalize);

	// filter the output to prevent double entries due to differences in intermediate atoms
	std::string filter = optAtom;
	for (std::set<Atom>::iterator it = sourceAtoms.begin(); it != sourceAtoms.end(); ++it){
		if (ignoredPredicates.find(it->getPredicate().getUnquotedString()) == ignoredPredicates.end()){
			filter += ","; //(it != usedAtoms.begin() ? "," : "");
			filter += it->getPredicate().getUnquotedString();
		}
	}



	// by default, maxint is set by addSource (see below) such that it is high enough to compute the costs (using sum aggregation); if
	// the user uses additional integer rules or more expensive aggregate functions, the value can be overwritten in the parameters
	// this value is sufficient for maximum aggregate function
	int micomp = 0;

	// add the information from all sources (may throws an exception)
	std::vector<HexAnswer> sources;
	int sourceIndex = 0;
	for (std::vector<HexAnswer*>::iterator argIt = arguments.begin(); argIt != arguments.end(); argIt++){
		HexAnswer currentSource = addSource(*argIt, weights[sourceIndex++], micomp, costSum, costAtom, penalize, sourceAtoms, usedAtoms, ignoredPredicates, program, facts);

		// source completed
		//	the names used in this source must not be used in others (all atoms of all atom sets were used)
		for (std::vector<AtomSet>::iterator asIt = currentSource.begin(); asIt != currentSource.end(); asIt++){
			for (AtomSet::const_iterator atIt = asIt->begin(); atIt != asIt->end(); atIt++){
				usedAtoms.insert(atIt->getPredicate().getUnquotedString());
			}
		}
		sources.push_back(currentSource);
	}
	if (micomp > maxint) maxint = micomp;
/*
			DLVPrintVisitor pv(std::cout);
			pv.PrintVisitor::visit(&program);
			pv.PrintVisitor::visit(&facts);
*/
	// build the resulting program and execute it
	try{
		DLVProcess proc;
		std::stringstream maxint_str;
		maxint_str << "-N=" << maxint;
		proc.addOption(maxint_str.str());
//std::cerr << filter;
		proc.addOption("-filter=" + filter);
		BaseASPSolver* solver =  new ASPSolver<DLVPrintVisitor, DLVresultParserDriver>(proc);	// make sure that the result is parsed by DLVresultParserDriver even if we run in HO-mode
		std::vector<AtomSet> result;
		solver->solve(program, facts, result);

		// finally, keep only the answer-sets with minimal costs
		optimize(result, optAtom);

		/*
		DLVPrintVisitor pv(std::cout);
		std::cout << filter;
		for (int i = 0; i < result.size(); i++) pv.PrintVisitor::visit(&result[i]);
		*/

		// restore original setting of NoPredicate
		Globals::Instance()->setOption("NoPredicate", noPred);

		return result;
	}catch(...){
		// restore original setting of NoPredicate
		Globals::Instance()->setOption("NoPredicate", noPred);

		std::stringstream ss;
		if (debug){
			ss << ": " << std::endl;
			DLVPrintVisitor pv(ss);
			pv.PrintVisitor::visit(&program);
			pv.PrintVisitor::visit(&facts);
		}
		throw OperatorException("Error while building and executing the merging program" + ss.str());
	}
}
