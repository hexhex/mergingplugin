#include <HexExecution.h>
#include <DLVHexProcess.h>
#include <fstream>
#include <string>
#include <sstream>

using namespace dlvhex::merging;

// -------------------- Util --------------------
/*
std::string Util::rewrite(std::string rewriterpath, std::string input){
std::cout << "CALLING" << std::endl;
	ArbProcess rewriter(rewriterpath);
	rewriter.spawn();

	std::istream& pi = rewriter.getInput();
	std::ostream& po = rewriter.getOutput();
	std::string line;

	// Send program to input rewriter
	po << input;
	rewriter.endoffile();

	// Read output from input rewriter and overwrite the input program with modified one
	std::stringstream routput;
	while(getline(pi, line)) {
		routput << line << std::endl;
	}
	std::string result = routput.str();

	// On errors throw a PluginError
	int errcode;
	if((errcode = rewriter.close()) != 0){
		throw PluginError(std::string("Error from rewriter \"") + rewriterpath + std::string("\""));
	}

	return result;
}
*/

// Splits the command line arguments at blanks (if they are not part of a string literal)
std::vector<std::string> splitArguments(std::string argsstring){
	std::vector<std::string> args;

	bool stringlit = false;
	int argstart = 0;
	for (int i = 0; i <= argsstring.size(); i++){
		switch (argsstring[i]){
			case ' ':
			case '\0':
				if (!stringlit){
					if (i == argstart){ argstart++;
					}else{
						args.push_back(argsstring.substr(argstart, i - argstart));
						argstart = i + 1;
					}
				}
				break;
			case '\"':
				stringlit = !stringlit;
				break;
			default:
				break;
		}
	}

	return args;
}


// -------------------- HexAtom --------------------

HexAtom::HexAtom(HexAnswerCache &rsCache) : resultsetCache(rsCache)
{
	addInputConstant();	// program
	addInputConstant();	// command line arguments
	setOutputArity(1);	// list of answer set handles
}

HexAtom::~HexAtom()
{
}

void
HexAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	std::string program("");
	std::string cmdargs("");
	try{
		// resolve escape sequences
		std::string srcprogram = query.getInputTuple()[0].getUnquotedString();
		//    \\ --> \ 
		//    \' --> "
		bool escaped = false;
		for (std::string::iterator it = srcprogram.begin(); it != srcprogram.end(); it++){
			if (escaped){
				switch (*it){
					case '\\':
						program += '\\';
						break;
					case '\'':
						program += '\"';
						break;
					default:
						program += *it;
						break;
				}
				escaped = false;
			}else{
				switch (*it){
					case '\\':
						escaped = true;
						break;
					default:
						program += *it;
						break;
				}
			}
		}

		// Retrieve command line arguments
		cmdargs = query.getInputTuple()[1].getUnquotedString();

		// Build hex call identifier
		HexCall hc(HexCall::HexProgram, program, cmdargs);

		// Check if result is already cached
		int i = 0;
		for (HexAnswerCache::iterator it = resultsetCache.begin(); it != resultsetCache.end(); it++){
			if (it->first == hc){
				// Reuse answer
				Tuple out;
				out.push_back(Term(i));
				answer.addTuple(out);
				return;
			}
			i++;
		}


		// Not in cache: Execute program and cache result

		// parse and assemble hex program (parameter 0)
		std::stringstream ss(program);
		Program prog;
		AtomSet facts;
		HexParserDriver hpd;
		hpd.parse(ss, prog, facts);

		// solve hex program
		DLVHexProcess proc;
		//DLVProcess proc;

		// split command line arguments
		std::vector<std::string> cmdargsSplit = splitArguments(cmdargs);
		for (int i = 0; i < cmdargsSplit.size(); i++) proc.addOption(cmdargsSplit[i]);

		std::vector<AtomSet> as;
		BaseASPSolver* solver = proc.createSolver();
		solver->solve(prog, facts, as);
	//	proc.solve(program, prog, facts, as);

		// store output (answer sets) in cache
		int rn = resultsetCache.size();
		resultsetCache.push_back(HexAnswerCacheEntry(hc, as));

		// return answer handle
		Tuple out;
		out.push_back(Term(rn));
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
	addInputConstant();	// command line arguments
	setOutputArity(1);	// list of answer set handles
}

HexFileAtom::~HexFileAtom()
{
}

void
HexFileAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	std::string programpath("");
	std::string cmdargs("");
	try{
		// load program
		programpath = query.getInputTuple()[0].getUnquotedString();

		// Retrieve command line arguments
		cmdargs = query.getInputTuple()[1].getUnquotedString();

		// Build hex call identifier
		HexCall hc(HexCall::HexFile, programpath, cmdargs);

		// Check if result is already cached
		int i = 0;
		for (HexAnswerCache::iterator it = resultsetCache.begin(); it != resultsetCache.end(); it++){
			if (it->first == hc){
				// Reuse answer
				Tuple out;
				out.push_back(Term(i));
				answer.addTuple(out);
				return;
			}
			i++;
		}

		// Not in cache: Execute program and cache result
/*
		OLD VERSION SINCE THE CODE BELOW DID NOT WORK (but now it does)

		// read source file
		std::ifstream inFile;
		inFile.open(programpath.c_str());
		std::string program;
		std::string line;
		while (inFile >> line) {
			program = program + line;
		}
		inFile.close();

		// parse and assemble hex program (parameter 0)
		std::stringstream ss(program);
		Program prog;
		AtomSet facts;
		HexParserDriver hpd;
		hpd.parse(ss, prog, facts);

		// solve hex program
		DLVHexProcess proc;

		// split command line arguments
		std::vector<std::string> cmdargsSplit = splitArguments(cmdargs);
		for (int i = 0; i < cmdargsSplit.size(); i++) proc.addOption(cmdargsSplit[i]);

		std::vector<AtomSet> as;
		BaseASPSolver* solver = proc.createSolver();
		solver->solve(prog, facts, as);
*/

		// solve hex program
		DLVHexProcess proc(programpath);
		Program prog;
		AtomSet facts;

		std::vector<AtomSet> as;
		BaseASPSolver* solver = proc.createSolver();
		solver->solve(prog, facts, as);
		//proc.solve("", prog, facts, as);

		// store output (answer sets)
		int rn = resultsetCache.size();
		resultsetCache.push_back(HexAnswerCacheEntry(hc, as));

		// return answer handle
		Tuple out;
		out.push_back(Term(rn));
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
	setOutputArity(1);	// list of answer set handles
}

AnswerSetsAtom::~AnswerSetsAtom()
{
}

void
AnswerSetsAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	// Retrieve answer index
	int answerindex = query.getInputTuple()[0].getInt();

	// Return handles to all answer sets of the given answer (all integers from 0 to the number of answer sets-1)
	int i = 0;
	for (HexAnswer::iterator it = resultsetCache[answerindex].second.begin(); it != resultsetCache[answerindex].second.end(); it++){
		Tuple out;
		out.push_back(Term(i++));
		answer.addTuple(out);
	}
}

// -------------------- PredicatesAtom --------------------

PredicatesAtom::PredicatesAtom(HexAnswerCache &rsCache) : resultsetCache(rsCache)
{
	addInputConstant();	// answer index
	addInputConstant();	// answerset index
	setOutputArity(2);	// list of answer set handles
}

PredicatesAtom::~PredicatesAtom()
{
}

void
PredicatesAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	// Retrieve answer index and answer set index
	int answerindex = query.getInputTuple()[0].getInt();
	int answersetindex = query.getInputTuple()[1].getInt();

	// Go through all atoms of the given answer set
	int i = 0;
	for (AtomSet::const_iterator it = resultsetCache[answerindex].second[answersetindex].begin(); it != resultsetCache[answerindex].second[answersetindex].end(); it++){
		// Return predicate name and arity of the atom
		Tuple out;
		out.push_back(Term(it->getPredicate().getString()));
		out.push_back(Term(it->getArity()));
		answer.addTuple(out);
	}
}

// -------------------- PredicatesAtom --------------------

ArgumentsAtom::ArgumentsAtom(HexAnswerCache &rsCache) : resultsetCache(rsCache)
{
	addInputConstant();	// answer index
	addInputConstant();	// answerset index
	addInputConstant();	// predicate name
	setOutputArity(3);	// list of answer set handles
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

	// Go through all atoms of the given answer set
	int runningindex = 0;
	for (AtomSet::const_iterator it = resultsetCache[answerindex].second[answersetindex].begin(); it != resultsetCache[answerindex].second[answersetindex].end(); it++){
		// If the atom is built upon the given predicate, return it's parameters
		if (it->getPredicate() == predicate){
			// special case of index -1: positive or strongly negated
			Tuple outsign;
			outsign.push_back(Term(runningindex));
			outsign.push_back(Term("s"));
			outsign.push_back(Term(it->isStronglyNegated() ? 1 : 0));
			answer.addTuple(outsign);

			// Go through all parameters
			for (int argindex = 0; argindex < it->getArity(); argindex++){
				// For each: Return a running index (since the same predicate can occur arbitrary often within an answer set), the argument index and the value
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
