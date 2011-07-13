#include <HexExecution.h>
#include <DLVHexProcess.h>
#include <fstream>
#include <string>
#include <sstream>

using namespace dlvhex::merging::plugin;

// -------------------- HexAtom --------------------

HexAtom::HexAtom(HexAnswerCache &rsCache) : resultsetCache(rsCache)
{
	addInputConstant();	// program
	addInputTuple();	// command line arguments (optional)
	setOutputArity(1);	// list of answer-set handles
}

HexAtom::~HexAtom()
{
}

void
HexAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	std::string program;
	std::string cmdargs;
	AtomSet inputfacts;
	try{
		Tuple params = query.getInputTuple();

		// resolve escape sequences
		program = params[0].getUnquotedString();

		// Retrieve command line arguments
		if (params.size() > 1){
			cmdargs = params[1].getUnquotedString();
		}

		// Build hex call identifier
		AtomSet inputfacts;
		HexCall hc(HexCall::HexProgram, program, cmdargs, inputfacts);

		// request entry from cache (this will automatically add it if it's not contained yet)
		Tuple out;
		out.push_back(Term(resultsetCache[hc]));
		answer.addTuple(out);
	}catch(...){
		std::stringstream msg;
		msg << "Nested Hex program \"" << program << "\" failed. Command line arguments were: " << cmdargs;
		throw PluginError(msg.str());
	}
}


// -------------------- HexFileAtom --------------------

HexFileAtom::HexFileAtom(HexAnswerCache &rsCache) : resultsetCache(rsCache)
{
	addInputConstant();	// program path
	addInputTuple();	// command line arguments (optional)
	setOutputArity(1);	// list of answer-set handles
}

HexFileAtom::~HexFileAtom()
{
}

void
HexFileAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	std::string programpath;
	std::string cmdargs;
	AtomSet inputfacts;
	try{
		Tuple params = query.getInputTuple();

		// load program
		programpath = params[0].getUnquotedString();

		// Retrieve command line arguments
		if (params.size() > 1){
			cmdargs = params[1].getUnquotedString();
		}

		// Build hex call identifier
		HexCall hc(HexCall::HexFile, programpath, cmdargs, inputfacts);

		// request entry from cache (this will automatically add it if it's not contained yet)
		Tuple out;
		out.push_back(Term(resultsetCache[hc]));
		answer.addTuple(out);
	}catch(...){
		std::stringstream msg;
		msg << "Nested Hex program \"" << programpath << "\" failed. Command line arguments were: " << cmdargs;
		throw PluginError(msg.str());
	}
}


// -------------------- CallHexAtom --------------------

CallHexAtom::CallHexAtom(HexAnswerCache &rsCache) : resultsetCache(rsCache)
{
	addInputConstant();	// program
	addInputPredicate();	// input facts
	addInputTuple();	// command line arguments (optional)
	setOutputArity(1);	// list of answer-set handles
}

CallHexAtom::~CallHexAtom()
{
}

void
CallHexAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	std::string program;
	std::string cmdargs;
	AtomSet inputfacts;
	try{
		Tuple params = query.getInputTuple();

		// resolve escape sequences
		program = params[0].getUnquotedString();

		// Retrieve command line arguments
		std::string inputfactspred = params[1].getUnquotedString();

/*
	// Get predicate which specifies the input predicates:
	AtomSet inputpreds;
	query.getInterpretation().matchPredicate(inputfactspred, inputpreds);
	for (AtomSet::const_iterator it = inputpreds.begin(); it != inputpreds.end(); ++it){
		if (it->getArguments().size() != 1) throw PluginError("Input predicates must be unary");
		// Read next input predicate name
		std::string inputpredname = it->getArguments()[0].getUnquotedString();
		// Read inputs over this predicate name
		AtomSet facts;
std::cout << "adding " << inputpredname << std::endl;
		query.getInterpretation().matchPredicate(inputpredname, facts);
		// convert from higher-order notation to first-order notation
		for (AtomSet::const_iterator it = facts.begin(); it != facts.end(); ++it){
			AtomPtr atom = AtomPtr(new Atom(it->getArguments()));
			inputfacts.insert(atom);
std::cout << "adding atom over " << inputpredname << std::endl;
		}
	}
*/

		// Read input facts
		AtomSet f;
		query.getInterpretation().matchPredicate(inputfactspred, f);

		// convert from higher-order notation to first-order notation
		for (AtomSet::const_iterator it = f.begin(); it != f.end(); ++it){
			Tuple args = it->getArguments();
			// read arity
			int arity = it->getArguments()[0].getInt();
			// delete superfluous parameters
			args.erase(args.begin(), args.begin() + 1); // arity information
			args.erase(args.end() - (args.size() - 1 - arity), args.end()); // number of existing args minus number of needed args			
			AtomPtr atom = AtomPtr(new Atom(args));
			inputfacts.insert(atom);
		}

		// Retrieve command line arguments (if specified)
		if (params.size() > 2){
			cmdargs = params[2].getUnquotedString();
		}

		// Build hex call identifier
		HexCall hc(HexCall::HexProgram, program, cmdargs, inputfacts);

		// request entry from cache (this will automatically add it if it's not contained yet)
		Tuple out;
		out.push_back(Term(resultsetCache[hc]));
		answer.addTuple(out);
	}catch(...){
		std::stringstream msg;
		msg << "Nested Hex program \"" << program << "\" failed. Command line arguments were: " << cmdargs;
		throw PluginError(msg.str());
	}
}


// -------------------- CallHexFileAtom --------------------

CallHexFileAtom::CallHexFileAtom(HexAnswerCache &rsCache) : resultsetCache(rsCache)
{
	addInputConstant();	// program path
	addInputPredicate();	// predicate with input facts
	addInputTuple();	// command line arguments (optional)
	setOutputArity(1);	// list of answer-set handles
}

CallHexFileAtom::~CallHexFileAtom()
{
}

void
CallHexFileAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	std::string programpath;
	std::string cmdargs;
	AtomSet inputfacts;
	try{
		Tuple params = query.getInputTuple();

		// load program
		programpath = params[0].getUnquotedString();

		// Retrieve command line arguments
		std::string inputfactspred = params[1].getUnquotedString();

		// retrieve the facts over the specified predicate and pass them to the subprogram
		AtomSet f;
		query.getInterpretation().matchPredicate(inputfactspred, f);

		// convert from higher-order notation to first-order notation
		for (AtomSet::const_iterator it = f.begin(); it != f.end(); ++it){
			Tuple args = it->getArguments();
			// read arity
			int arity = it->getArguments()[0].getInt();
			// delete superfluous parameters
			args.erase(args.begin(), args.begin() + 1); // arity information
			args.erase(args.end() - (args.size() - 1 - arity), args.end()); // number of existing args minus number of needed args			
			AtomPtr atom = AtomPtr(new Atom(args));
			inputfacts.insert(atom);
		}

		// Retrieve command line arguments
		if (params.size() > 1){
			cmdargs = params[2].getUnquotedString();
		}

		// Build hex call identifier
		HexCall hc(HexCall::HexFile, programpath, cmdargs, inputfacts);

		// request entry from cache (this will automatically add it if it's not contained yet)
		Tuple out;
		out.push_back(Term(resultsetCache[hc]));
		answer.addTuple(out);
	}catch(...){
		std::stringstream msg;
		msg << "Nested Hex program \"" << programpath << "\" failed. Command line arguments were: " << cmdargs;
		throw PluginError(msg.str());
	}
}


// -------------------- AnswerSetsAtom --------------------

AnswerSetsAtom::AnswerSetsAtom(HexAnswerCache &rsCache) : resultsetCache(rsCache)
{
	addInputConstant();	// answer index
	setOutputArity(1);	// list of answer-set handles
}

AnswerSetsAtom::~AnswerSetsAtom()
{
}

void
AnswerSetsAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	// Retrieve answer index
	int answerindex = query.getInputTuple()[0].getInt();

	// check index validity
	if (answerindex < 0 || answerindex >= resultsetCache.size()){
		throw PluginError("An invalid answer handle was passed to atom &answersets");
	}else{
		// Return handles to all answer-sets of the given answer (all integers from 0 to the number of answer-sets minus 1)
		int i = 0;
		for (HexAnswer::iterator it = resultsetCache[answerindex]->begin(); it != resultsetCache[answerindex]->end(); it++){
			Tuple out;
			out.push_back(Term(i++));
			answer.addTuple(out);
		}
	}
}


// -------------------- PredicatesAtom --------------------

PredicatesAtom::PredicatesAtom(HexAnswerCache &rsCache) : resultsetCache(rsCache)
{
	addInputConstant();	// answer index
	addInputConstant();	// answerset index
	setOutputArity(2);	// list of predicate/arity pairs
}

PredicatesAtom::~PredicatesAtom()
{
}

void
PredicatesAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	// Retrieve answer index and answer-set index
	int answerindex = query.getInputTuple()[0].getInt();
	int answersetindex = query.getInputTuple()[1].getInt();

	// check index validity
	if (answerindex < 0 || answerindex >= resultsetCache.size()){
		throw PluginError("An invalid answer handle was passed to atom &predicates");
	}else if(answersetindex < 0 || answersetindex >= resultsetCache[answerindex]->size()){
		throw PluginError("An invalid answer-set handle was passed to atom &predicates");
	}else{
		// Go through all atoms of the given answer_set
		int i = 0;
		for (AtomSet::const_iterator it = (*(resultsetCache[answerindex]))[answersetindex].begin(); it != (*(resultsetCache[answerindex]))[answersetindex].end(); it++){
			// Return predicate name and arity of the atom
			Tuple out;
			out.push_back(Term(it->getPredicate().getString()));
			out.push_back(Term(it->getArity()));
			answer.addTuple(out);
		}
	}
}


// -------------------- PredicatesAtom --------------------

ArgumentsAtom::ArgumentsAtom(HexAnswerCache &rsCache) : resultsetCache(rsCache)
{
	addInputConstant();	// answer index
	addInputConstant();	// answerset index
	addInputConstant();	// predicate name
	setOutputArity(3);	// list of triples: running-index/arg-index/arg-value
}

ArgumentsAtom::~ArgumentsAtom()
{
}

void
ArgumentsAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	// Extract answer index, answerset index and predicate to retrieve
	int answerindex = query.getInputTuple()[0].getInt();
	int answersetindex = query.getInputTuple()[1].getInt();
	std::string predicate = query.getInputTuple()[2].getUnquotedString();

	// check index validity
	if (answerindex < 0 || answerindex >= resultsetCache.size()){
		throw PluginError("An invalid answer handle was passed to atom &arguments");
	}else if(answersetindex < 0 || answersetindex >= resultsetCache[answerindex]->size()){
		throw PluginError("An invalid answer-set handle was passed to atom &arguments");
	}else{
		// Go through all atoms of the given answer_set
		int runningindex = 0;
		for (AtomSet::const_iterator it = (*(resultsetCache[answerindex]))[answersetindex].begin(); it != (*(resultsetCache[answerindex]))[answersetindex].end(); it++){
			// If the atom is built upon the given predicate, return it's parameters
			if (it->getPredicate() == predicate){
				// special case of index "s": positive or strongly negated
				Tuple outsign;
				outsign.push_back(Term(runningindex));
				outsign.push_back(Term("s"));
				outsign.push_back(Term(it->isStronglyNegated() ? 1 : 0));
				answer.addTuple(outsign);

				// Go through all parameters
				for (int argindex = 0; argindex < it->getArity(); argindex++){
					// For each: Return a running index (since the same predicate can occur arbitrary often within an answer-set), the argument index and the value
					Tuple out;
					out.push_back(Term(runningindex));
					out.push_back(Term(argindex));
					out.push_back(it->getArgument(argindex + 1));
					answer.addTuple(out);
				}
				runningindex++;
			}
		}
	}
}
