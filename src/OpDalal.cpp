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
#include <stdio.h>
#include <stdlib.h>

#include <boost/algorithm/string.hpp>

using namespace dlvhex;
using namespace dlvhex::merging;
using namespace dlvhex::merging::plugin;

std::string OpDalal::getName(){
	return "dalal";
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

// Adds weak constraints that binds the new belief base specific atom to the final one.
// If possible, the belief base specific (renamed) attribute should be equal to the final one.
// i.e.:
//	:~ a123, not a.
//	:~ -a123, not -a.
void bindAtoms(int weight, AtomPtr costAtom, AtomPtr atom_bb, AtomPtr atom_final, dlvhex::Program& program, dlvhex::AtomSet& facts){

	Literal* lit_bbPos = new Literal(AtomPtr(new Atom(*atom_bb))); if (atom_bb->isStronglyNegated()) lit_bbPos->getAtom()->negate();
	Literal* lit_bbNeg = new Literal(AtomPtr(new Atom(*atom_bb))); if (!atom_bb->isStronglyNegated()) lit_bbPos->getAtom()->negate();
	Literal* lit_finalPosWN = new Literal(AtomPtr(new Atom(*atom_final)), true); if (atom_final->isStronglyNegated()) lit_finalPosWN->getAtom()->negate();
	Literal* lit_finalNegWN = new Literal(AtomPtr(new Atom(*atom_final)), true); if (!atom_final->isStronglyNegated()) lit_finalPosWN->getAtom()->negate();
	Literal* lit_finalPos = new Literal(AtomPtr(new Atom(*atom_final))); if (atom_final->isStronglyNegated()) lit_finalPos->getAtom()->negate();
	Literal* lit_finalNeg = new Literal(AtomPtr(new Atom(*atom_final))); if (!atom_final->isStronglyNegated()) lit_finalPos->getAtom()->negate();
	Registry::Instance()->storeObject(lit_bbPos);
	Registry::Instance()->storeObject(lit_bbNeg);
	Registry::Instance()->storeObject(lit_finalPosWN);
	Registry::Instance()->storeObject(lit_finalNegWN);
	Registry::Instance()->storeObject(lit_finalPos);
	Registry::Instance()->storeObject(lit_finalNeg);

	// if possible, the final atom should have the same value than the one in the current source
	dlvhex::RuleBody_t body_bind1;
	dlvhex::RuleBody_t body_bind2;
	body_bind1.insert(lit_bbPos);
	body_bind1.insert(lit_finalPosWN);
	body_bind2.insert(lit_bbNeg);
	body_bind2.insert(lit_finalNegWN);

	dlvhex::RuleHead_t head_bind;
	head_bind.insert(costAtom);

	WeakConstraint* wc1 = new WeakConstraint(body_bind1, dlvhex::Term(weight), dlvhex::Term(1));	// level and weight of the weak constraint: 1
	WeakConstraint* wc2 = new WeakConstraint(body_bind2, dlvhex::Term(weight), dlvhex::Term(1));
	Registry::Instance()->storeObject(wc1);
	Registry::Instance()->storeObject(wc2);
	program.addWeakConstraint(wc1);
	program.addWeakConstraint(wc2);
}

// Adds rules that bind the new belief base specific atom to the final one.
// If possible, the belief base specific (renamed) attribute should be equal to the final one.
// i.e.:
//	cost(BB, RR, c) :- a123, not a.
//	cost(BB, RR, c) :- -a123, not -a.
// where BB is a unique number for each belief base, RR is a unique index for each atom
// and c the weight for this belief base.
void writeCostComputation(AtomPtr costAtom, AtomPtr atom_bb, AtomPtr atom_final, dlvhex::Program& program, dlvhex::AtomSet& facts){

	Literal* lit_bbPos = new Literal(AtomPtr(new Atom(*atom_bb))); if (atom_bb->isStronglyNegated()) lit_bbPos->getAtom()->negate();
	Literal* lit_bbNeg = new Literal(AtomPtr(new Atom(*atom_bb))); if (!atom_bb->isStronglyNegated()) lit_bbPos->getAtom()->negate();
	Literal* lit_finalPosWN = new Literal(AtomPtr(new Atom(*atom_final)), true); if (atom_final->isStronglyNegated()) lit_finalPosWN->getAtom()->negate();
	Literal* lit_finalNegWN = new Literal(AtomPtr(new Atom(*atom_final)), true); if (!atom_final->isStronglyNegated()) lit_finalPosWN->getAtom()->negate();
	Literal* lit_finalPos = new Literal(AtomPtr(new Atom(*atom_final))); if (atom_final->isStronglyNegated()) lit_finalPos->getAtom()->negate();
	Literal* lit_finalNeg = new Literal(AtomPtr(new Atom(*atom_final))); if (!atom_final->isStronglyNegated()) lit_finalPos->getAtom()->negate();
	Registry::Instance()->storeObject(lit_bbPos);
	Registry::Instance()->storeObject(lit_bbNeg);
	Registry::Instance()->storeObject(lit_finalPosWN);
	Registry::Instance()->storeObject(lit_finalNegWN);
	Registry::Instance()->storeObject(lit_finalPos);
	Registry::Instance()->storeObject(lit_finalNeg);

	// if possible, the final atom should have the same value than the one in the current source
	dlvhex::RuleBody_t body_bind1;
	dlvhex::RuleBody_t body_bind2;
	body_bind1.insert(lit_bbPos);
	body_bind1.insert(lit_finalPosWN);
	body_bind2.insert(lit_bbNeg);
	body_bind2.insert(lit_finalNegWN);

	// derive costs in case of constraint violation
	RuleHead_t head_costatom;
	head_costatom.insert(costAtom);
	Rule* cc1 = new Rule(head_costatom, body_bind1);
	Rule* cc2 = new Rule(head_costatom, body_bind2);
	Registry::Instance()->storeObject(cc1);
	Registry::Instance()->storeObject(cc2);
	program.addRule(cc1);
	program.addRule(cc2);

	// default costs are 0 (in case of no constraint violation)
	RuleHead_t head_defcosts;
	RuleBody_t body_nocostatom;
	AtomPtr costAtom0 = AtomPtr(new Atom(*costAtom));
	costAtom0->setArgument(3, Term(0));
	head_defcosts.insert(costAtom0);
	Literal* lit_nocostatom = new Literal(AtomPtr(new Atom(*costAtom)), true);
	body_nocostatom.insert(lit_nocostatom);
	Rule* rule_defcosts = new Rule(head_defcosts, body_nocostatom);
	Registry::Instance()->storeObject(lit_nocostatom);
	Registry::Instance()->storeObject(rule_defcosts);
	program.addRule(rule_defcosts);
}

// writes code for computing the aggregation function
// either the computation rule is directly passed in argument "std::string aggregation", or a (supported) shortcut like "sum" or "max" is used; in the latter case
// the function will generate the appropriate code automatically
void buildAggregateFunction(std::string aggregation, std::string costAtom, int arity, std::set<std::string>& usedAtoms, dlvhex::Program& program, dlvhex::AtomSet& facts){

	// create a unique optimization atom
	std::string optAtom = findUniqueAtomName("optimize", usedAtoms);
	usedAtoms.insert(optAtom);

	// create predefined aggregate function (if requested)
	std::stringstream aggregationss;
	if (aggregation == std::string("sum")){
		std::stringstream ss;
		ss << "optimize(S" << (arity - 1) << ") :- ";
		for (int i = 0; i < arity; i++){
			if (i > 0) ss << ", S" << i << "=S" << (i - 1) << "+V" << i << ", ";
			ss << "cost(" << i << ", sum, " << (i == 0 ? "S" : "V") << i << ")";
		}
		ss << ".";
		aggregation = ss.str();
	}else if (aggregation == std::string("max")){
		std::stringstream ss;
		for (int i = 0; i < arity; i++){
			ss << "optimize(M) :- cost(" << i << ", sum, M)";
			for (int j = 0; j < arity; j++) ss << ", cost(" << j << ", sum, V" << j << "), M >= V" << j;
			ss << ".";
		}
		aggregation = ss.str();
	}

	// make sure that the aggregate function uses unique predicate names
	boost::algorithm::replace_all(aggregation, "cost", costAtom);
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

// Adds an input source to the program and facts
HexAnswer OpDalal::addSource(HexAnswer* source, int weight, std::string costAtom, std::set<std::string>& usedatoms, std::set<std::string>& ignoredPredicates, dlvhex::Program& program, dlvhex::AtomSet& facts){

	// some atoms that make sure that exactly one of the answer-sets will be selected
	dlvhex::RuleHead_t head_asSelection;

	std::vector<AtomSet> newsource;	// will contain the answer-sets with the renamed literals

	// for all answer-sets of this source
	static int sourceNr = 0;
	int constraintNr = 0;
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
				AtomPtr atom_orig = AtomPtr(new Atom(*atIt));
				AtomPtr atom_new = AtomPtr(new Atom(*atIt));
				// find a unique name for the atom
				atom_new->setPredicate(dlvhex::Term(findUniqueAtomName(atom_new->getPredicate().getUnquotedString(), usedatoms)));
				usedatoms.insert(atom_new->getPredicate().getUnquotedString());	// name was consumed
				// add the renamed atom the the new source
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

				// add constraints that binds the new atom to the original one
				// if possible, the renamed attribute should be equal to the original one
				Tuple params;
				params.push_back(Term(sourceNr));	// unique name for the belief base
				params.push_back(Term(constraintNr));	// unique name for this constraint
				params.push_back(Term(weight));
				writeCostComputation(AtomPtr(new Atom(costAtom, params)), atom_new, atom_orig, program, facts);
				constraintNr++;
			}
		}
		// answer-set completed: add it to the source
		newsource.push_back(currentAS);
	}

	// sum up the costs for differrings to this source
	std::stringstream sum;
	sum << costAtom << "(" << sourceNr << ", sum, " << (constraintNr > 1 ? "S" : "V") << (constraintNr-1) << ") :- ";
	for (int i = 0; i < constraintNr; i++){
		if (i > 0) sum << ", ";

		sum << costAtom  << "(" << sourceNr << ", " << i << ", " << " V" << i << ")";
		if (i > 0) sum << ", S" << i << "=" <<((i - 1) > 0 ? "S" : "V") << (i - 1) << "+V" << i;
	}
	sum << ".";
	HexParserDriver hpd;
	hpd.parse(sum, program, facts);

	// make sure that one of the answer-sets of this source is selected
	// (at least one -> minimality of answer-sets will make sure that also at most one is selected)
	Rule* rule_asSelection = new Rule(head_asSelection, dlvhex::RuleBody_t());
	Registry::Instance()->storeObject(rule_asSelection);
	program.addRule(rule_asSelection);

	sourceNr++;
	return newsource;
}

// Keeps only the minimum-costs answer-sets in "result" and removes the others
// This is normally already done by DLV. But due to the fact that DLVresultParserDriver does not support parsing outputs with cost values, we need to
// replace weak constraints by normal rules that derive their penalty in the head. This method sums up the penalties and computes the optimal answer-sets.
// The atoms that were used in the source program need to be passed because the function needs to distinct between usual atoms and special "cost atoms" that
// are introduced as part of the replacement of weak constraints by normal ones.
void OpDalal::optimize(HexAnswer& result, std::set<std::string>& usedAtoms){
	// answer cleaning: only keep those answer-sets that have to pay the minimum penalty for weak constraint violation
	// unfortuanately we need to compute the costs ourselves since the DLVresultParserDriver is not able to parse DLV's cost output
	std::map<AtomSet*, int> weights;
	int maxLevel = 0;
	for (HexAnswer::iterator argIt = result.begin(); argIt != result.end(); argIt++){
		for (AtomSet::const_iterator atIt = argIt->begin(); atIt != argIt->end(); atIt++){
			if (atIt->getPredicate().getUnquotedString().substr(0, 5) == std::string("wch__") && usedAtoms.find(atIt->getPredicate().getUnquotedString()) == usedAtoms.end()){
				assert(atIt->getArgument(atIt->getArity()).isInt());	// level
				maxLevel = atIt->getArgument(atIt->getArity()).getInt() > maxLevel ? atIt->getArgument(atIt->getArity()).getInt() : maxLevel;
			}
		}
	}

	// for all levels
	for (int level = 0; level <= maxLevel; level++){
		int bestWeight = -1;	// none found so far
		for (HexAnswer::iterator argIt = result.begin(); argIt != result.end(); argIt++){
			// take only those answer-sets with the least penalty for weak constraint violation
			int weight = 0;
			for (AtomSet::const_iterator atIt = argIt->begin(); atIt != argIt->end(); atIt++){
				if (atIt->getPredicate().getUnquotedString().substr(0, 5) == std::string("wch__") && usedAtoms.find(atIt->getPredicate().getUnquotedString()) == usedAtoms.end()){
					// sum up the second argument (weight)
					assert(atIt->getArgument(atIt->getArity()).isInt());				// level
					assert(atIt->getArgument(atIt->getArity() - 1).isInt());			// weight
					if (atIt->getArgument(atIt->getArity()).getInt() == level){
						weight += atIt->getArgument(atIt->getArity() - 1).getInt();
					}
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
	}
}

// writes a disjunctive rule that either selects an atom or it's negated version
void writeAtomSelectionRule(const Atom* atom, dlvhex::Program& program, dlvhex::AtomSet& facts){

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

void parseParameters(int arity, OperatorArguments& parameters, std::vector<int>& weights, int& maxint, std::set<std::string>& ignoredPredicates, dlvhex::Program& program, dlvhex::AtomSet& facts, std::string& aggregation){
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
}

HexAnswer OpDalal::apply(int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters) throw (OperatorException){

	// default values
	std::string aggregation = "sum";
	int maxint = 0;

	dlvhex::Program program;
	dlvhex::AtomSet facts;
	std::set<std::string> ignoredPredicates;
	std::vector<int> weights;
	for (int i = 0; i < arity; i++)	// default value
		weights.push_back(1);

	// parse parameters (may throws an exception)
	parseParameters(arity, parameters, weights, maxint, ignoredPredicates, program, facts, aggregation);

	// construct a logic program that computes the answer as follows:
	//	- rename all atoms such that they are unique for each belief base, i.e. no atom occurs in multiple sources
	//	- guess final atoms: each one can be true or false
	//	- add weak constraints, such that the final atoms are as similar to the sources as possible

	// create a collection of all occurring atoms
	std::set<std::string> usedAtoms;
	for (std::vector<HexAnswer*>::iterator argIt = arguments.begin(); argIt != arguments.end(); argIt++){
		// look into each answer-set
		for (std::vector<AtomSet>::iterator asIt = (*argIt)->begin(); asIt != (*argIt)->end(); asIt++){
			// extract atoms and arities
			for (AtomSet::const_iterator atIt = asIt->begin(); atIt != asIt->end(); atIt++){
				// check if this atom is relevant
				if (ignoredPredicates.find(atIt->getPredicate().getUnquotedString()) == ignoredPredicates.end()){
					usedAtoms.insert(atIt->getPredicate().getUnquotedString());
					writeAtomSelectionRule(&(*atIt), program, facts);
				}
			}
		}
	}

	// create a unique costs atom
	std::string costAtom = findUniqueAtomName("cost", usedAtoms);
	usedAtoms.insert(costAtom);
	// write code for computing the aggregation function (may throws an exception)
	buildAggregateFunction(aggregation, costAtom, arity, usedAtoms, program, facts);
	// by default, maxint= sum_{bb} (weight[bb] * atom_count * 2)
	// this value is sufficient for maximum aggregate function
	if (maxint == 0){
		int sourceIndex = 0;
		for (std::vector<HexAnswer*>::iterator argIt = arguments.begin(); argIt != arguments.end(); argIt++){
			maxint += weights[sourceIndex++] * (usedAtoms.size() - ignoredPredicates.size()) * 2;
		}
	}

	// add the information from all sources (may throws an exception)
	std::vector<HexAnswer> sources;
	int sourceIndex = 0;
	for (std::vector<HexAnswer*>::iterator argIt = arguments.begin(); argIt != arguments.end(); argIt++){
		HexAnswer currentSource = addSource(*argIt, weights[sourceIndex++], costAtom, usedAtoms, ignoredPredicates, program, facts);

		// source completed
		//	the names used in this source must not be used in others (all atoms of all atom sets were used)
		for (std::vector<AtomSet>::iterator asIt = currentSource.begin(); asIt != currentSource.end(); asIt++){
			for (AtomSet::const_iterator atIt = asIt->begin(); atIt != asIt->end(); atIt++){
				usedAtoms.insert(atIt->getPredicate().getUnquotedString());
			}
		}
		sources.push_back(currentSource);
	}

/*
DLVPrintVisitor pv(std::cout);
std::cout << "!Prog!" << maxint;
pv.PrintVisitor::visit(&program);
pv.PrintVisitor::visit(&facts);
*/

	// build the resulting program and execute it
	DLVProcess proc;
	std::stringstream maxint_str;
	maxint_str << "-N=" << maxint;
	proc.addOption(maxint_str.str());
	BaseASPSolver* solver = proc.createSolver();
	std::vector<AtomSet> result;
	solver->solve(program, facts, result);

	// finally, keep only the answer-sets with minimal costs
	optimize(result, usedAtoms);

/*
std::cout << "!AS!";
for (std::vector<AtomSet>::iterator it = result.begin(); it != result.end(); it++){
	pv.PrintVisitor::visit(&(*it));
}
*/

	return result;
}
