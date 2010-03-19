#include <OpMinHamming.h>

#include <dlvhex/DLVProcess.h>
#include <dlvhex/ASPSolver.h>
#include <dlvhex/Registry.h>
#include <dlvhex/HexParserDriver.h>

#include <dlvhex/PrintVisitor.h>
#include <DLVHexProcess.h>

#include <iostream>
#include <sstream>

using namespace dlvhex;
using namespace dlvhex::merging;

std::string OpMinHamming::getName(){
	return "minhamming";
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
void bindAtoms(AtomPtr atom_bb, AtomPtr atom_final, dlvhex::Program& program, dlvhex::AtomSet& facts){

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
	WeakConstraint* wc1 = new WeakConstraint(body_bind1, dlvhex::Term(1), dlvhex::Term(1));	// level and weight of the weak constraint: 1
	WeakConstraint* wc2 = new WeakConstraint(body_bind2, dlvhex::Term(1), dlvhex::Term(1));
	Registry::Instance()->storeObject(wc1);
	Registry::Instance()->storeObject(wc2);
	program.addWeakConstraint(wc1);
	program.addWeakConstraint(wc2);
}

// Adds an input source to the program and facts
HexAnswer OpMinHamming::addSource(HexAnswer* source, std::set<std::string>& usedatoms, dlvhex::Program& program, dlvhex::AtomSet& facts){

	// some atoms that make sure that exactly one of the answer-sets will be selected
	dlvhex::RuleHead_t head_asSelection;

	std::vector<AtomSet> newsource;	// will contain the answer-sets with the renamed literals

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

			// add weak constraints that binds the new atom to the original one
			// if possible, the renamed attribute should be equal to the original one
			// i.e.:
			//	:~ a123, not a.
			//	:~ -a123, not -a.
			bindAtoms(atom_new, atom_orig, program, facts);
		}
		// answer-set completed: add it to the source
		newsource.push_back(currentAS);
	}

	// make sure that one of the answer-sets of this source is selected
	// (at least one -> minimality of answer-sets will make sure that also at most one is selected)
	Rule* rule_asSelection = new Rule(head_asSelection, dlvhex::RuleBody_t());
	Registry::Instance()->storeObject(rule_asSelection);
	program.addRule(rule_asSelection);

	return newsource;
}

// Keeps only the minimum-costs answer-sets in "result" and removes the others
// This is normally already done by DLV. But due to the fact that DLVresultParserDriver does not support parsing outputs with cost values, we need to
// replace weak constraints by normal rules that derive their penalty in the head. This method sums up the penalties and computes the optimal answer-sets.
// The atoms that were used in the source program need to be passed because the function needs to distinct between usual atoms and special "cost atoms" that
// are introduced as part of the replacement of weak constraints by normal ones.
void OpMinHamming::optimize(HexAnswer& result, std::set<std::string>& usedAtoms){
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
					assert(atIt->getArgument(atIt->getArity()).isInt());		// level
					assert(atIt->getArgument(atIt->getArity() - 1).isInt());	// weight
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

HexAnswer OpMinHamming::apply(int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters) throw (OperatorException){

	dlvhex::Program program;
	dlvhex::AtomSet facts;

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
				usedAtoms.insert(atIt->getPredicate().getUnquotedString());
				writeAtomSelectionRule(&(*atIt), program, facts);
			}
		}
	}

	// add the information from all sources
	std::vector<HexAnswer> sources;
	for (std::vector<HexAnswer*>::iterator argIt = arguments.begin(); argIt != arguments.end(); argIt++){
		HexAnswer currentSource = addSource(*argIt, usedAtoms, program, facts);

		// source completed
		//	the names used in this source must not be used in others (all atoms of all atom sets were used)
		for (std::vector<AtomSet>::iterator asIt = currentSource.begin(); asIt != currentSource.end(); asIt++){
			for (AtomSet::const_iterator atIt = asIt->begin(); atIt != asIt->end(); atIt++){
				usedAtoms.insert(atIt->getPredicate().getUnquotedString());
			}
		}
		sources.push_back(currentSource);
	}

	// add side constraints
	for (OperatorArguments::iterator argIt = parameters.begin(); argIt != parameters.end(); argIt++){
		if (argIt->first == std::string("constraint")){
			std::string constraint = argIt->second;
			// parse it
			try{
				HexParserDriver hpd;
				std::stringstream ss(constraint);
				hpd.parse(ss, program, facts);
			}catch(SyntaxError){
				throw OperatorException(std::string("Could not parse constraint due to a syntax error: \"") + argIt->second + std::string("\""));
			}
		}
	}

	// build the resulting program and execute it
	DLVProcess proc;
	BaseASPSolver* solver = proc.createSolver();
	std::vector<AtomSet> result;
	solver->solve(program, facts, result);

//DLVPrintVisitor pv(std::cout);
//std::cout << "!Prog!";
//pv.PrintVisitor::visit(&program);

	// finally, keep only the answer-sets with minimal costs
	optimize(result, usedAtoms);

	return result;
}
