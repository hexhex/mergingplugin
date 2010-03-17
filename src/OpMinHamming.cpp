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

HexAnswer OpMinHamming::addSource(HexAnswer* source, std::set<std::string>& atoms, dlvhex::Program& program, dlvhex::AtomSet& facts){

	// some atoms that make sure that one of the answer-sets will be selected
	dlvhex::RuleHead_t asSelection;

	// all answer-sets of this source
	std::vector<AtomSet> newsource;
	int asIndex = 1;
	for (std::vector<AtomSet>::iterator asIt = source->begin(); asIt != source->end(); asIt++, asIndex++){

		// make a unique prop. atom that decides if this answer-set is selected or not
		AtomPtr asSelectionAtom = AtomPtr(new Atom(""));
		int ri = 0;
		do{
			std::stringstream ss;
			ss << "as_" << asIndex << "_" << (ri++);
			asSelectionAtom->setPredicate(dlvhex::Term(ss.str().c_str()));
		}while (atoms.find(asSelectionAtom->getPredicate().getUnquotedString()) != atoms.end());
		asSelection.insert(dlvhex::AtomPtr(asSelectionAtom));
		atoms.insert(asSelectionAtom->getPredicate().getUnquotedString());

		AtomSet currentAS;
		// all atoms of this answer-set
		for (AtomSet::const_iterator atIt = asIt->begin(); atIt != asIt->end(); atIt++){
			// find a unique name for the atom
			AtomPtr origAtom = AtomPtr(new Atom(*atIt));
			AtomPtr newAtom = AtomPtr(new Atom(*atIt));
			int nr = 1;
			while (atoms.find(newAtom->getPredicate().getUnquotedString()) != atoms.end()){
				std::stringstream nrss;
				nrss << nr++;
				newAtom->setPredicate(dlvhex::Term(newAtom->getPredicate().getUnquotedString() + nrss.str()));
			}
			// unique name was found
			//	add the atom the the new source
			currentAS.insert(newAtom);

			// if the current answer-set is selected, then the contained atoms can be derived
			RuleHead_t headSelectAS;
			headSelectAS.insert(newAtom);
			RuleBody_t bodySelectAS;
			Literal* lit = new Literal(asSelectionAtom);
			bodySelectAS.insert(lit);
			Rule* asSelectedRule = new Rule(headSelectAS, bodySelectAS);
			Registry::Instance()->storeObject(asSelectedRule);
			Registry::Instance()->storeObject(lit);
			program.addRule(asSelectedRule);

			// add a weak constraint that binds the new atom to the original one
			// if possible, the renamed attribute should be equal to the original one
			Literal* lit_newAtomPos = new Literal(AtomPtr(new Atom(*newAtom))); if (newAtom->isStronglyNegated()) lit_newAtomPos->getAtom()->negate();
			Literal* lit_newAtomNeg = new Literal(AtomPtr(new Atom(*newAtom))); if (!newAtom->isStronglyNegated()) lit_newAtomPos->getAtom()->negate();
			Literal* lit_origAtomPosWN = new Literal(AtomPtr(new Atom(*origAtom)), true); if (origAtom->isStronglyNegated()) lit_origAtomPosWN->getAtom()->negate();
			Literal* lit_origAtomNegWN = new Literal(AtomPtr(new Atom(*origAtom)), true); if (!origAtom->isStronglyNegated()) lit_origAtomPosWN->getAtom()->negate();
			Literal* lit_origAtomPos = new Literal(AtomPtr(new Atom(*origAtom))); if (origAtom->isStronglyNegated()) lit_origAtomPos->getAtom()->negate();
			Literal* lit_origAtomNeg = new Literal(AtomPtr(new Atom(*origAtom))); if (!origAtom->isStronglyNegated()) lit_origAtomPos->getAtom()->negate();
			Registry::Instance()->storeObject(lit_newAtomPos);
			Registry::Instance()->storeObject(lit_newAtomNeg);
			Registry::Instance()->storeObject(lit_origAtomPosWN);
			Registry::Instance()->storeObject(lit_origAtomNegWN);
			Registry::Instance()->storeObject(lit_origAtomPos);
			Registry::Instance()->storeObject(lit_origAtomNeg);

			// make sure that the new atom is either positive or negative
			RuleHead_t ruleSetNewAtom_head;
			ruleSetNewAtom_head.insert(lit_origAtomPos->getAtom());
			ruleSetNewAtom_head.insert(lit_origAtomNeg->getAtom());
			Rule* ruleSetNewAtom = new Rule(ruleSetNewAtom_head, RuleBody_t());
			Registry::Instance()->storeObject(ruleSetNewAtom);
			program.addRule(ruleSetNewAtom);


			

			dlvhex::RuleBody_t bindRuleBody1;
			dlvhex::RuleBody_t bindRuleBody2;
			bindRuleBody1.insert(lit_newAtomPos);
			bindRuleBody1.insert(lit_origAtomPosWN);
			bindRuleBody2.insert(lit_newAtomNeg);
			bindRuleBody2.insert(lit_origAtomNegWN);
//			Rule* bindRule1 = new Rule(RuleHead_t(), b1);
//			Rule* bindRule2 = new Rule(RuleHead_t(), b2);
//			Registry::Instance()->storeObject(bindRuleBody2);
			WeakConstraint* wc1 = new WeakConstraint(bindRuleBody1, dlvhex::Term(1), dlvhex::Term(1));
			WeakConstraint* wc2 = new WeakConstraint(bindRuleBody2, dlvhex::Term(1), dlvhex::Term(1));
			Registry::Instance()->storeObject(wc1);
			Registry::Instance()->storeObject(wc2);
			program.addWeakConstraint(wc1);
			program.addWeakConstraint(wc2);
//			if (!newAtom.isStronglyNegated()) newAtom.negate(); b2.insert(newAtom);
//			if (newAtom.isStronglyNegated()) newAtom.negate(); b2.insert(origAtom);
//			program.addWeakConstraint(dlvhex::WeakConstraint(b2, dlvhex::Term(1), dlvhex::Term(1)));
		}
		// answer-set completed: add it to the source
		newsource.push_back(currentAS);
	}

	// make sure that one of the answer-sets of this source is selected
	// (at least one -> minimality of answer-sets will make sure that also at most one is selected)
	Rule* asSelectionRule = new Rule(asSelection, dlvhex::RuleBody_t());
	Registry::Instance()->storeObject(asSelectionRule);
	program.addRule(asSelectionRule);


	return newsource;
}

HexAnswer OpMinHamming::apply(int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters) throw (OperatorException){

	dlvhex::Program program;
	dlvhex::AtomSet facts;

	// construct a logic program that computes the answer as follows:
	//		- rename all atoms such that they are unique for each belief base, i.e. no atom occurs in multiple sources
	//		- guess final atoms: each one can be true or false
	//		- add weak constraints, such that the final atoms are as similar to the sources as possible

	// create a collection of all occurring atoms
	std::set<std::string> usedAtoms;
	for (std::vector<HexAnswer*>::iterator argIt = arguments.begin(); argIt != arguments.end(); argIt++){
		// look into each answer-set
		for (std::vector<AtomSet>::iterator asIt = (*argIt)->begin(); asIt != (*argIt)->end(); asIt++){
			// extract atoms and arities
			for (AtomSet::const_iterator atIt = asIt->begin(); atIt != asIt->end(); atIt++){
				usedAtoms.insert(atIt->getPredicate().getUnquotedString());
			}
		}
	}

	// rename source atoms
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
			HexParserDriver hpd;
			std::stringstream ss(constraint);
			hpd.parse(ss, program, facts);
		}
	}

	// build the resulting program and execute it
	DLVProcess proc;
	BaseASPSolver* solver = proc.createSolver();
	std::vector<AtomSet> result;
	solver->solve(program, facts, result);

//std::cout << "!Prog!";
//pv.PrintVisitor::visit(&program);

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

	return result;
}
