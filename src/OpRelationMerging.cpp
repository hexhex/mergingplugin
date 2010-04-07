#include "OpRelationMerging.h"

#include <dlvhex/PrintVisitor.h>
#include <dlvhex/AggregateAtom.h>
#include <dlvhex/DLVProcess.h>
#include <dlvhex/ASPSolver.h>
#include <dlvhex/Registry.h>
#include <dlvhex/HexParserDriver.h>

#include <set>

#include <boost/algorithm/string.hpp>

using namespace dlvhex;
using namespace dlvhex::merging;
using namespace dlvhex::merging::plugin;

std::string OpRelationMerging::getName(){
	return "relationmerging";
}


std::string OpRelationMerging::getInfo(){
	std::stringstream ss;
	ss <<	"     relationmerging" << std::endl <<
		"     ---------------" << std::endl << std::endl <<
		"This operator is not thought to be used practically since it is buggy and only used for test purposes!" << std::endl <<
		 "A(H1), ..., A(Hn)	... handles to n answers" << std::endl <<
		 "			    each answer is expected to contain exactly one answer-set with data entries of kind" << std::endl <<
		 "			    	data(a1,...,an)" << std::endl <<
		 "			    where ai are the attribute values as well as a schema definition of kind" << std::endl <<
		 "			    	schema(n1,...,nn)" << std::endl <<
		 "			    where ni are the attribute names" << std::endl <<
		 "K(schema, s)		... A list of kind" << std::endl <<
		 "			    	s = \"a1,...,an\"" << std::endl <<
		 "			    defines the arity and the attribute names of the merged schema" << std::endl <<
		 "K(key, s)		... A list of kind" << std::endl <<
		 "			    	s = \"k1,...,km\"" << std::endl <<
		 "K(default, d)		... \"d\" is a string of kind \"a=v\" where \"a\" is some attribute and \"v\" some default value" << std::endl <<
		 "K(rule, r)		... \"r\" is a rule of kind" << std::endl <<
		 "			    	attribute(merged, PrimaryKey, Value) :- attribute(1, PrimaryKey, V1), ..., attribute(n, PrimaryKey, Vn)" << std::endl <<
		 "			    in order to overwrite the built-in rule that states that attribute values are copied into the result iff" << std::endl <<
		 "			    all sources agree upon it's value" << std::endl <<
		 "			    that defines the key attributes in the merged schema (they must also be contained in each source)" << std::endl <<
		 "A			... answer to the operator result";
	return ss.str();
}

std::set<std::string> OpRelationMerging::getRecognizedParameters(){
	std::set<std::string> list;
	list.insert("schema");
	list.insert("key");
	list.insert("default");
	list.insert("rule");
	return list;
}

// translates sources from
//	schema(a1,...,an), data(v1,...,vn)
// into
//	a1(m, v1), ..., an(m, vn)
void rewriteSource(int source, std::vector<HexAnswer*>& arguments, std::set<std::string>& key, std::vector<std::string>& key_ordered, std::vector<std::vector<std::string> >& schema_sources, AtomSet& facts){

	// find key attributes in this source
	std::set<int> keyIndices;
	std::vector<int> keyIndices_ordered;
	int i = 0;
	for (std::vector<std::string>::iterator keyIt = schema_sources[source].begin(); keyIt != schema_sources[source].end(); ++keyIt, ++i){
		if (key.find(*keyIt) != key.end()){
			keyIndices.insert(i);
			keyIndices_ordered.push_back(i);
		}
	}

	// sanity check: key of sources must coincide with key of merged data set
	if (keyIndices_ordered.size() != key_ordered.size()) throw IOperator::OperatorException("Number of key attributes in sources and in merged set differrs");
	for (int j = 0; j < keyIndices_ordered.size(); j++){
		if (schema_sources[source][keyIndices_ordered[j]].compare(key_ordered[j]) != 0){
			throw IOperator::OperatorException(std::string("Key attribute \"") + schema_sources[source][keyIndices_ordered[j]] + std::string("\" differrs from merged key attribute \"") + key_ordered[j] + std::string("\""));
		}
	}

	AtomSet currentSource;
	(*arguments[source])[0].matchPredicate("data", currentSource);
	// rewrite all tuples
	for (AtomSet::const_iterator atom = currentSource.begin(); atom != currentSource.end(); ++atom){
		// rewrite all attributes of this tuple
		for (int att = 1; att <= schema_sources[source].size(); att++){
			// only for non-key indices
			if (keyIndices.find(att) == keyIndices.end()){
				Tuple args;
				args.push_back(Term(source + 1));
				// add key attributes to each entry
				for (std::vector<int>::iterator keyIt = keyIndices_ordered.begin(); keyIt != keyIndices_ordered.end(); ++keyIt){
					args.push_back(atom->getArgument(*keyIt + 1));
				}
				args.push_back(atom->getArgument(att));
				facts.insert(AtomPtr(new Atom(schema_sources[source][att - 1], args)));
			}
		}
		// write individual
		Tuple args;
		for (std::vector<int>::iterator keyIt = keyIndices_ordered.begin(); keyIt != keyIndices_ordered.end(); ++keyIt){
			args.push_back(atom->getArgument(*keyIt + 1));
		}
		facts.insert(AtomPtr(new Atom("individuals", args)));
	}
}

// translates sources from
//	schema(a1,...,an), data(v1,...,vn)
// into
//	a1(m, v1), ..., an(m, vn)
void rewriteSources(std::vector<HexAnswer*>& arguments, std::set<std::string>& key, std::vector<std::string>& key_ordered, std::vector<std::vector<std::string> >& schema_sources, AtomSet& facts){

	// extract individual schemas for the sources
	for (int source = 0; source < arguments.size(); source++){
		if (arguments[source]->size() != 1){
			throw IOperator::OperatorException("Each source is expected to contain exactly one answer-set");
		}
		AtomSet currentSchemaAtom;
		(*arguments[source])[0].matchPredicate("schema", currentSchemaAtom);
		// sanity check
		if (currentSchemaAtom.size() != 1){
			throw IOperator::OperatorException("Each source is expected to contain exactly one atom upon the predicate \"schema\".");
		}else{
			Tuple s = currentSchemaAtom.begin()->getArguments();
			std::vector<std::string> currentSchema;
			for (Tuple::iterator it = s.begin(); it != s.end(); it++){
				currentSchema.push_back(it->getUnquotedString());
			}
			schema_sources.push_back(currentSchema);
		}
	}

	// source m: schema(a1,...,an), data(v1,...,vn) --> a1(m, v1), ..., an(m, vn)
	for (int source = 0; source < arguments.size(); source++){
		rewriteSource(source, arguments, key, key_ordered, schema_sources, facts);
	}
}

// merges output of kind
//	a1(m, v1), ..., an(m, vn)
// into single atoms of kind
//	data(v1,...,vn)
void resultExtraction(Program& program, std::set<std::string>& key, std::vector<std::string>& key_ordered, std::vector<std::string>& schema_merged){
	AtomSet facts; // empty

	std::stringstream assembling;
	std::stringstream query;
	std::stringstream keyquery;
	int i = 1;
	for (std::vector<std::string>::iterator keyIt = key_ordered.begin(); keyIt != key_ordered.end(); ++keyIt){
		keyquery << (keyIt == key_ordered.begin() ? "" : ", ") << "K" << (i++);
	}
	assembling << "data(" << keyquery.str();
	i = 1;
	for (std::vector<std::string>::iterator attIt = schema_merged.begin(); attIt != schema_merged.end(); ++attIt){
		// only for non-key attributes
		if (key.find(*attIt) == key.end()){
			assembling << ", " << "V" << i;
			query << (query.str().size() == 0 ? "" : ", ") << "fin_" << (*attIt) << "(merged, " << keyquery.str() << ", V" << (i++) << ")";
		}
	}
	assembling << ") :- " << query.str() << ".";


	// assemble subprogram
	HexParserDriver hpd;
	try{
		hpd.parse(assembling, program, facts);
	}catch(...){
		throw IOperator::OperatorException("Error while assembling unification rules");
	}
}

// writes rules that select all attributes that do not lead to contradictions
void writeDefaultMappings(Program& program, int arity, std::vector<std::string>& key_ordered, std::vector<std::string>& schema_merged, std::map<std::string, std::string>& defvalues){

	std::stringstream keyquery;
	int i = 1;
	for (std::vector<std::string>::iterator keyIt = key_ordered.begin(); keyIt != key_ordered.end(); ++keyIt){
		keyquery << (keyIt == key_ordered.begin() ? "" : ", ") << "K" << (i++);
	}

	std::stringstream defmappings;
	for (std::vector<std::string>::iterator att = schema_merged.begin(); att != schema_merged.end(); ++att){
		// if we have no contradicting values for some attribute, we overtake it as it is
		defmappings << "contradicting_" << (*att) << "(" << keyquery.str() << ") :- ";
		defmappings << (*att) << "(I1, " << keyquery.str() << ", V1)";
		defmappings << ", " << (*att) << "(I2, " << keyquery.str() << ", V2)";
		defmappings << ", V1 != V2";
		defmappings << ".";
		defmappings << "def_" << (*att) << "(merged, " << keyquery.str() << ", V) :- " << (*att) << "(I, " << keyquery.str() << ", V), not contradicting_" << (*att) << "(" << keyquery.str() << ").";
		// check if a user-defined rule for this attribute is defined
		defmappings << "userdefined_" << (*att) << "(merged, " << keyquery.str() << ") :- " << (*att) << "(merged, " << keyquery.str() << ", V).";
		// if this is the case, compute the final value using this rule
		defmappings << "fin_" << (*att) << "(merged, " << keyquery.str() << ", V) :- " << (*att) << "(merged, " << keyquery.str() << ", V).";
		// otherwise: take the default-value (exploiting nonmonotonic reasoning)
		defmappings << "fin_" << (*att) << "(merged, " << keyquery.str() << ", DV) :- def_" << (*att) << "(merged, " << keyquery.str() << ", DV), not userdefined_" << (*att) << "(merged, " << keyquery.str() << ").";

		// write existence predicates
		for (int i = 0; i < arity; i++){
			defmappings << "exists_" << (*att) << "(" << keyquery.str() << ") :- " << (*att) << "(" << (i + 1) << ", " << keyquery.str() << ", V).";
		}
	}

	HexParserDriver hpd;
	AtomSet facts;	// empty
	try{
		hpd.parse(defmappings, program, facts);
	}catch(...){
		throw IOperator::OperatorException("Error while parsing default mapping rules");
	}


	// write default values (if present)
	std::stringstream defvaluesrules;
	for (std::map<std::string, std::string>::iterator att = defvalues.begin(); att != defvalues.end(); ++att){
		defvaluesrules << att->first << "(merged, " << keyquery.str() << ", " << att->second << ") :- not exists_" << att->first << "(" << keyquery.str() << "), individuals(" << keyquery.str() << ").";
	}

	try{
		hpd.parse(defvaluesrules, program, facts);
	}catch(...){
		throw IOperator::OperatorException("Error while writing default value rules");
	}
}

HexAnswer OpRelationMerging::apply(int arity, std::vector<HexAnswer*>& arguments, OperatorArguments& parameters) throw (OperatorException){

	std::set<std::string> key;
	std::vector<std::string> key_ordered;
	std::vector<std::string> schema_merged;
	std::vector<std::vector<std::string> > schema_sources;
	std::map<std::string, std::string> defvalues;

	// extract merged schema
	std::stringstream mergingrules;
	for (OperatorArguments::iterator arg = parameters.begin(); arg != parameters.end(); ++arg){
		if (arg->first == std::string("schema")){
			boost::split(schema_merged, arg->second, boost::is_any_of(","));
		}
		else if (arg->first == std::string("key")){
			boost::split(key, arg->second, boost::is_any_of(","));
			boost::split(key_ordered, arg->second, boost::is_any_of(","));
		}
		else if (arg->first == std::string("rule")){
			mergingrules << arg->second;
		}

		else if (arg->first == std::string("default")){
			std::vector<std::string> defval;
			boost::split(defval, arg->second, boost::is_any_of("="));
			if (defval.size() != 2) throw OperatorException(std::string("Invalid default value specification: \"") + arg->second + std::string("\""));
			defvalues[defval[0]] = defval[1];
		}
	}

	// rewrite sources
	Program program;
	AtomSet facts;
	rewriteSources(arguments, key, key_ordered, schema_sources, facts);

	// map result onto atoms over predicate "data"
	resultExtraction(program, key, key_ordered, schema_merged);
	HexParserDriver hpd;
	try{
		hpd.parse(mergingrules, program, facts);
	}catch(...){
		throw IOperator::OperatorException("Error while assembling merging rules");
	}

	// write default mappings
	writeDefaultMappings(program, arity, key_ordered, schema_merged, defvalues);


/*
DLVPrintVisitor pv(std::cout);
std::cout << "!Prog!";
pv.PrintVisitor::visit(&program);
pv.PrintVisitor::visit(&facts);
*/

	// execute subprogram
	try{
		HexAnswer result;
		DLVProcess proc;
		BaseASPSolver* solver = proc.createSolver();
		solver->solve(program, facts, result);
//pv.PrintVisitor::visit(&(*(result.begin()));

		// add schema of final result
		Tuple t;
		for (std::vector<std::string>::iterator it = schema_merged.begin(); it != schema_merged.end(); ++it) t.push_back(Term(*it));
		result.begin()->insert(AtomPtr(new Atom("schema", t)));
		return result;
	}catch(...){
		throw OperatorException("Error while executing merging subprogram");
	}
}
