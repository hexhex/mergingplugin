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
	ss <<	"     dalal" << std::endl <<
		"     -----" << std::endl << std::endl <<
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

std::set<std::string> OpDalal::getRecognizedParameters(){
	std::set<std::string> list;
	list.insert("constraint");
	list.insert("constraintfile");
	list.insert("ignore");
	list.insert("weights");
	list.insert("aggregate");
	list.insert("penalize");
	list.insert("maxint");
	return list;
}

/**
 * Finds an predicate name that was not used yet and appends it to the list of used predicate names
 * \param prefix The desired prefix of the predicate
 * \param usedPredNames Reference to the list of predicate names used so far
 */
std::string OpDalal::findUniqueAtomName(const std::string prefix, std::set<std::string>& usedPredNames){
	static int ri = 0;
	std::string name;
	do{
		// repeat this until we find a unique name
		std::stringstream ss;
		ss << prefix;
		if (ri++ > 0) ss << ri;	// we want to avoid numbers if possible
		name = ss.str();
	}while (usedPredNames.find(name) != usedPredNames.end());

	// name was used now
	usedPredNames.insert(name);
	return name;
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
void OpDalal::parseParameters(int arity, OperatorArguments& parameters, std::string optAtom, std::string costSum, std::vector<int>& weights, int& maxint, std::set<std::string>& usedAtoms, std::set<std::string>& ignoredPredicates, dlvhex::Program& program, dlvhex::AtomSet& facts, std::string& aggregation, float penalize[4][4]){

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
			std::vector<std::string> w;
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
			std::vector<std::string> preds;
			boost::split(preds, argIt->second, boost::is_any_of(","));
			ignoredPredicates.insert(preds.begin(), preds.end());
		}
	}

	// built appropriate aggregate function
	buildAggregateFunction(arity, aggregation, optAtom, costSum, program, facts);

	// default value for penalize (if not used defined)
	if (!penalizeSet){
		penalize[0][0] = 0; penalize[0][1] = 1; penalize[0][2] = 0; penalize[0][3] = 0;
		penalize[1][0] = 0; penalize[1][1] = 0; penalize[1][2] = 0; penalize[1][3] = 0;
		penalize[2][0] = 0; penalize[2][1] = 0; penalize[2][2] = 0; penalize[2][3] = 1;
		penalize[3][0] = 0; penalize[3][1] = 0; penalize[3][2] = 0; penalize[3][3] = 0;
	}
}

/**
 * Parses a penalize rule.
 * \param rule Some rule (either shortcuts "ignoring", "unfounded", "aberration" or triples of kind "{+,not,-,not-},{+,not,-,not-},int")
 * \param penalize Pointer to a 4x4 float matrix where the new rule is added (where only elements that are explicitly reset will be overwritten)
 */
void OpDalal::parsePenalize(std::string& rule, float penalize[4][4]){
	// shortcuts
	if (rule == "ignoring"){
		// +,not
		// -,not-
		penalize[0][1] = 1; penalize[2][3] = 1;
	}
	else if (rule == "unfounded"){
		// not,+
		penalize[1][0] = 1;
	}
	else if (rule == "aberration"){
		// +,not
		// -,not-
		// not,+
		penalize[0][1] = 1; penalize[2][3] = 1; penalize[1][0] = 1;
	}else{
		std::vector<std::string> p; //split(argIt->second, ',');
		boost::split(p, rule, boost::is_any_of(","));

		if (p.size() != 3) throw IOperator::OperatorException(std::string("Invalid penalize definition \"") + rule + std::string("\". Must be of kind \"{+,not,-,not-},{+,not,-,not-},int\""));
		int individual = -1;
		int agg = -1;
		if (p[0] == "+") individual = 0; 	if (p[0] == "not") individual = 1;	if (p[0] == "-") individual = 2;	if (p[0] == "not-") individual = 3;
		if (p[1] == "+") agg = 0; 		if (p[1] == "not") agg = 1;		if (p[1] == "-") agg = 2;		if (p[1] == "not-") agg = 3;
		float factor = atof(p[2].c_str());
		if (individual < 0 || agg < 0 || (factor == 0 && p[2] != "0")){
			throw IOperator::OperatorException(std::string("Invalid penalize definition \"") + rule + std::string("\". Must be of kind \"{+,not,-,not-},{+,not,-,not-},int\""));
		}else{
			penalize[individual][agg] = factor;
		}
	}
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
void OpDalal::buildAggregateFunction(const int arity, const std::string aggregation, const std::string optAtom, const std::string costSum, dlvhex::Program& program, dlvhex::AtomSet& facts){
	std::string aggregationcode;

	// create predefined aggregate function (if requested)
	std::stringstream aggregationss;
	if (aggregation == std::string("sum")){
		// sum up the results from the individuals
		std::stringstream ss;
		ss << "optimize(S" << (arity - 1) << ") :- ";
		for (int i = 0; i < arity; i++){
			if (i > 0) ss << ", S" << i << "=S" << (i - 1) << "+V" << i << ", ";
			ss << "cost(" << i << ", " << (i == 0 ? "S" : "V") << i << ")";
		}
		ss << ".";
		aggregationcode = ss.str();
	}else if (aggregation == std::string("max")){
		// take the maximum of the individual results
		std::stringstream ss;
		for (int i = 0; i < arity; i++){
			ss << "optimize(M) :- cost(" << i << ", M)";
			for (int j = 0; j < arity; j++) ss << ", cost(" << j << ", V" << j << "), M >= V" << j;
			ss << ".";
		}
		aggregationcode = ss.str();
	}else{
		// user has already implemented the aggregate function
		aggregationcode = aggregation;
	}

	// make sure that the aggregate function uses our unique predicate names
	boost::algorithm::replace_all(aggregationcode, "cost", costSum);
	boost::algorithm::replace_all(aggregationcode, "optimize", optAtom);
	aggregationss << aggregationcode;

	// optimization criterion
	aggregationss << ":~ " << optAtom << "(C). [C:1]";

	// parse the aggregation function
	HexParserDriver hpd;
	try{
		hpd.parse(aggregationss, program, facts);
	}catch(SyntaxError){
		throw IOperator::OperatorException(std::string("Could not parse aggregation function: \"") + aggregationss.str() + std::string("\""));
	}
}

/**
 * Writes a disjunctive rule that either selects an atom or it's negated version
 * \param atom Some (positive) atom
 * \param program The program where the rule shall be appended
 */
void OpDalal::writeAtomSelectionRule(const Atom* atom, dlvhex::Program& program){

	// Make sure that each atom is either positive or negative
	// Note: this binary behaviour is reasonable since each atom occurrs in at least one answer-set. Thus, the answer "unknown" is senseless anyway because
	// we always have evidences for positive or negative information.

	// construct literals from this atom
	Literal* lit_Pos = new Literal(AtomPtr(new Atom(*atom)));
	if (lit_Pos->getAtom()->isStronglyNegated())
		lit_Pos->getAtom()->negate();	// make sure that it is positive

	Literal* lit_Neg = new Literal(AtomPtr(new Atom(*atom)));
	if (!lit_Neg->getAtom()->isStronglyNegated())
		lit_Neg->getAtom()->negate();	// make sure that it is negative
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
 * Writes the atom selection rule for each atom in the given list (see overloaded version of this method).
 * \param list A reference to the list of a atoms for which to write selection rules.
 * \param program A reference to the program to append the rules
 */
void OpDalal::writeAtomSelectionRule(const std::set<Atom>& usedAtoms, dlvhex::Program& program){
	// we safely assume that the list does not contain duplicates (see createAtomList)
	for (std::set<Atom>::iterator it = usedAtoms.begin(); it != usedAtoms.end(); it++){
		writeAtomSelectionRule(&(*it), program);
	}
}

/**
 * Extracts all atoms from the sources and writes it into a list.
 * Additionally it writes the rules that make sure that each atom is either accepted or denied (strong negation)
 * \param arguments The vector of answers passed to the operator
 * \param usedAtoms A reference to the list where atoms shall be written to
 * \param ignoredPredicates A reference to the list of atoms that shall be ignored
 * \param program A reference to the program where rules shall be appended to
 */
void OpDalal::createAtomList(const std::vector<HexAnswer*>& arguments, const std::set<std::string>& ignoredPredicates, std::set<Atom>& usedAtoms){
	// for all answers
	for (std::vector<HexAnswer*>::const_iterator argIt = arguments.begin(); argIt != arguments.end(); argIt++){
		// look into each answer-set
		for (std::vector<AtomSet>::iterator asIt = (*argIt)->begin(); asIt != (*argIt)->end(); asIt++){
			// extract atoms and arities
			for (AtomSet::const_iterator atIt = asIt->begin(); atIt != asIt->end(); atIt++){
				// check if this atom is relevant
				if (ignoredPredicates.find(atIt->getPredicate().getUnquotedString()) == ignoredPredicates.end()){
					// yes: add it if it does not occur yet (we have to prevent duplicates)
					Atom at(*atIt);
					if (at.isStronglyNegated()) at.negate();
					if (usedAtoms.find(at) == usedAtoms.end()){
						usedAtoms.insert(at);
					}
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
void OpDalal::writeCostComputation(const AtomPtr costAtom, Literal* lit_individual, Literal* lit_agg, dlvhex::Program& program){

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

	// default costs are 0, but we do not have to derive them explicitly since we use aggregate function #sum anyway
}

/**
 * Renames the atoms of a given source such that they are unique within the program
 * \param source Pointer to the source to
 * \param sourceAtoms Vector of ALL atoms in ANY source (not only those that occur in this one); we even need unique names for atoms that never occur in this source for cost model "unfounded"
 * \param usedPredNames Reference to the set of already used predicate names (is expanded by this method)
 * \param localAtoms A reference to a map where the mapping of each name occurring in the source atoms onto a unique name shall be added
 */
void OpDalal::renameSourceAtoms(const HexAnswer* source, const std::set<Atom>& sourceAtoms, std::set<std::string>& usedPredNames, std::map<std::string, std::string>& localAtoms){

	for (std::set<Atom>::iterator it = sourceAtoms.begin(); it != sourceAtoms.end(); ++it){
		// do we have already a name for this atom?
		if (localAtoms.find(it->getPredicate().getUnquotedString()) == localAtoms.end()){
			// no: create one
			localAtoms[it->getPredicate().getUnquotedString()] = findUniqueAtomName(it->getPredicate().getUnquotedString(), usedPredNames);
		}
	}
}

/**
 * Writes rules that select exactly one of a certain source's answer-sets and derive all the atoms in this set in case that it is selected.
 * i.e. if the source has answer-sets {a},{b}, it will generate the rules:
 *   as0 v as1.
 *   a :- as0.
 *   b :- as1.
 * \param source A pointer to the source
 * \param localAtomMapping A reference to the mapping of global atom names onto the source's
 * \param ignoredPredicates A reference to the set of irrelevant predicate names
 */
void OpDalal::writeAnswerSetSelectionRules(const HexAnswer* source, const std::map<std::string, std::string>& localAtomMapping, const std::set<std::string>& ignoredPredicates, std::set<std::string>& usedPredNames, dlvhex::Program& program){

	// for all answer-sets of this source
	dlvhex::RuleHead_t head_asSelection;
	for (std::vector<AtomSet>::const_iterator asIt = source->begin(); asIt != source->end(); asIt++){

		// make a unique prop. atom that decides if this answer-set is selected or not
		AtomPtr atom_asSelection = AtomPtr(new Atom(findUniqueAtomName("as_", usedPredNames).c_str()));

		// a disjunction of all those atoms will select exactly one (due to minimality criterion of answer-sets)
		head_asSelection.insert(dlvhex::AtomPtr(atom_asSelection));

		// for all atoms of this answer-set
		for (AtomSet::const_iterator atIt = asIt->begin(); atIt != asIt->end(); atIt++){

			// check if this atom is relevant or ignored
			if (ignoredPredicates.find(atIt->getPredicate().getUnquotedString()) == ignoredPredicates.end()){
				// relevant: convert atom's name into local one
				AtomPtr atom_local = AtomPtr(new Atom(*atIt));
				assert(localAtomMapping.find(atIt->getPredicate().getUnquotedString()) != localAtomMapping.end());
				atom_local->setPredicate(Term(localAtomMapping.find(atIt->getPredicate().getUnquotedString())->second));

				// if the current answer-set is selected, then the contained atoms can be derived
				RuleHead_t head_deriveAtom;
				head_deriveAtom.insert(atom_local);
				RuleBody_t body_deriveAtom;
				Literal* lit_asSelection = new Literal(atom_asSelection);
				body_deriveAtom.insert(lit_asSelection);
				Rule* rule_deriveAtom = new Rule(head_deriveAtom, body_deriveAtom);
				Registry::Instance()->storeObject(rule_deriveAtom);
				Registry::Instance()->storeObject(lit_asSelection);
				program.addRule(rule_deriveAtom);

			}
		}
	}


	// make sure that one of the answer-sets of this source is selected
	// (at least one -> minimality of answer-sets will make sure that also at most one is selected)
	Rule* rule_asSelection = new Rule(head_asSelection, dlvhex::RuleBody_t());
	Registry::Instance()->storeObject(rule_asSelection);
	program.addRule(rule_asSelection);
}

/**
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
void OpDalal::writeCostComputationRules(const HexAnswer* source, const std::map<std::string, std::string>& localAtomMapping, const std::set<Atom>& sourceAtoms, const std::set<std::string>& ignoredPredicates, const std::set<std::string>& usedPredNames, const std::string costAtom, const std::string costSumAtom, const int weight, const float penalize[4][4], int& maxint, dlvhex::Program& program){

	// for all atoms (even those that do not occur in this source, since they can still be relevant depending on the cost model)
	int constraintNr = 0;
	static int sourceNr = 0;
	for (std::set<Atom>::const_iterator it = sourceAtoms.begin(); it != sourceAtoms.end(); ++it){

		// add constraints that binds the local atom to the original (group decision)
		// if possible, the renamed attribute should be equal to the original one
		const Atom* atom_group = &(*it);
		AtomPtr atom_individual = AtomPtr(new Atom(*it));	// is positive for sure
		atom_individual->setPredicate(Term(localAtomMapping.find(it->getPredicate().getUnquotedString())->second));

		// for all combinations of positive, strongly negated and default-negated literals
		for (int individual = 0; individual < 4; individual++){
			for (int agg = 0; agg < 4; agg++){
				if (penalize[individual][agg] > 0){
					Tuple params;
					params.push_back(Term(sourceNr));	// unique name for the belief base
					params.push_back(Term(constraintNr));	// unique name for this constraint
					params.push_back(Term((int)(weight * penalize[individual][agg])));

					// compute maximum integer needed
					maxint += (int)(weight * penalize[individual][agg]);

					// create literals with appropriate signs
					AtomPtr atom_individual_curRule = AtomPtr(new Atom(*atom_individual));
					if (individual >= 2) atom_individual_curRule->negate();
					AtomPtr atom_group_curRule = AtomPtr(new Atom(*atom_group));
					if (agg >= 2) atom_group_curRule->negate();

					Literal* lit_individual = new Literal(atom_individual_curRule, individual == 1 || individual == 3);
					Literal* lit_group = new Literal(atom_group_curRule, agg == 1 || agg == 3);

					// finally write the cost rules
					writeCostComputation(AtomPtr(new Atom(costAtom, params)), lit_individual, lit_group, program);
					Registry::Instance()->storeObject(lit_individual);
					Registry::Instance()->storeObject(lit_group);
					constraintNr++;
				}
			}
		}
	}

	// sum up the costs for all constraint violations
	writeSumRule(sourceNr, costAtom, costSumAtom, program);
	sourceNr++;
}

/**
 * Writes a rule of kind
 *   costSum(SRC, S) :- #int(S), S=#sum{ Cost,Constraint : cost(SRC, Constraint, Cost) }.
 * to sum up the costs for constraint violations for a certain source
 * \param sourceNr The unique number of the source
 * \param program The program where rules shall be appended
 */
void OpDalal::writeSumRule(const int sourceNr, const std::string costAtom, const std::string costSumAtom, dlvhex::Program& program){

	// costSum(SRC, S) :- #int(S), S=#sum{ Cost,Constraint : cost(SRC, Constraint, Cost) }.

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
		for (int j = i - 1; j >= 0; j--)
			if (result[i] == result[j])
				found = true;
		if (found){
			result.erase(result.begin() + i);
		}
	}
}

HexAnswer OpDalal::apply(bool debug, int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters) throw (OperatorException){

	// Make dlvhex believe we work in firstorder mode
	// This is a workaround due to a nasty assertion in dlvhex: otherwise we can not instanciate AggregateAtom
	unsigned noPred = Globals::Instance()->getOption("NoPredicate");
	Globals::Instance()->setOption("NoPredicate", 0);

	// create a subprogram that computes dalal's operator
	dlvhex::Program program;
	dlvhex::AtomSet facts;

	// default values
	std::string aggregation = "sum";
	int maxint = 0;

	std::set<std::string> ignoredPredicates;
	std::vector<int> weights;
	for (int i = 0; i < arity; i++)	// default value
		weights.push_back(1);


	// ---------- some information gathering ----------

	// construct a logic program that computes the answer as follows:
	//	- rename all atoms such that they are unique for each belief base, i.e. no atom occurs in multiple sources
	//	- guess final atoms: each one can be true or false
	//	- add weak constraints, such that the final atoms are as similar to the sources as possible

	// create a collection of all atoms occurring in some source and further extract all predicate names that were used
	std::set<Atom> sourceAtoms;
	createAtomList(arguments, ignoredPredicates, sourceAtoms);
	std::set<std::string> usedPredNames;
	for (std::set<Atom>::iterator it = sourceAtoms.begin(); it != sourceAtoms.end(); ++it){
		if (usedPredNames.find(it->getPredicate().getUnquotedString()) == usedPredNames.end()){
			usedPredNames.insert(it->getPredicate().getUnquotedString());
		}
	}

	// create a unique optimization atom
	std::string optAtom = findUniqueAtomName("optimize", usedPredNames);

	// create a unique cost atoms for intermedicate results and final sum (for each source)
	std::string costAtom = findUniqueAtomName("cost", usedPredNames);
	std::string costSum = findUniqueAtomName("costSum", usedPredNames);

	// parse parameters (may throws an exception)
	float penalize[4][4];	// 0=positive, 1=defneg, 2=strong neg, 3=def and strong neg
				// first dimension: individuals
				// second dimension: aggregated decision
	memset(penalize, 0, 16 * sizeof(float));
	parseParameters(arity, parameters, optAtom, costSum, weights, maxint, usedPredNames, ignoredPredicates, program, facts, aggregation, penalize);

	// filter the output to prevent double entries due to differences in intermediate atoms
	std::string filter = optAtom;
	for (std::set<Atom>::iterator it = sourceAtoms.begin(); it != sourceAtoms.end(); ++it){
		if (ignoredPredicates.find(it->getPredicate().getUnquotedString()) == ignoredPredicates.end()){
			filter += ","; //(it != usedAtoms.begin() ? "," : "");
			filter += it->getPredicate().getUnquotedString();
		}
	}


	// ---------- start building the program ----------

	// write selection rules for all atoms
	writeAtomSelectionRule(sourceAtoms, program);

	// by default, maxint is set by addSource (see below) such that it is high enough to compute the costs (using sum aggregation); if
	// the user uses additional integer rules or more expensive aggregate functions, the value can be overwritten in the parameters
	// this value is sufficient for maximum aggregate function
	int micomp = 0;

	// add the information from all sources (may throws an exception)
	std::vector<HexAnswer> sources;
	int sourceIndex = 0;
	for (std::vector<HexAnswer*>::iterator argIt = arguments.begin(); argIt != arguments.end(); argIt++){

		// make sure we use unique atom names in each source
		std::map<std::string, std::string> localAtomMapping;
		renameSourceAtoms(*argIt, sourceAtoms, usedPredNames, localAtomMapping);

		// select exactly one answer-set of this source and derive the atoms within
		writeAnswerSetSelectionRules(*argIt, localAtomMapping, ignoredPredicates, usedPredNames, program);

		// compute the costs for this source
		writeCostComputationRules(*argIt, localAtomMapping, sourceAtoms, ignoredPredicates, usedPredNames, costAtom, costSum, weights[sourceIndex++], penalize, micomp, program);
	}
	if (micomp > maxint)
		maxint = micomp;

/*
DLVPrintVisitor pv(std::cout);
pv.PrintVisitor::visit(&program);
pv.PrintVisitor::visit(&facts);
*/

	// build the resulting program and execute it
	try{
		ASPSolverManager& solver = ASPSolverManager::Instance();
		ASPSolverManager::DLVTypeSoftware::Options opt;
		std::stringstream maxint_str;
		maxint_str << "-N=" << maxint;
		opt.arguments.push_back(maxint_str.str());
		opt.arguments.push_back(std::string("-filter=") + filter);
		std::vector<AtomSet> result;
		solver.solve<ASPSolverManager::DLVSoftware>(program, facts, result, opt);

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
