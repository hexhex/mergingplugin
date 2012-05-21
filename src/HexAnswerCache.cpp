#include <HexAnswerCache.h>

#include <HexExecution.h>
#include "dlvhex2/HexParser.h"
#include "dlvhex2/InputProvider.h"
#include "dlvhex2/InternalGrounder.h"
#include "dlvhex2/InternalGroundDASPSolver.h"
#include "dlvhex2/ProgramCtx.h"
#include "dlvhex2/Registry.h"
#include "dlvhex2/PluginContainer.h"
#include "dlvhex2/ASPSolverManager.h"
#include "dlvhex2/ASPSolver.h"
#include "dlvhex2/State.h"
#include "dlvhex2/EvalGraphBuilder.h"
#include "dlvhex2/EvalHeuristicBase.h"
#include "dlvhex2/EvalHeuristicOldDlvhex.h"
#include "dlvhex2/EvalHeuristicTrivial.h"
#include "dlvhex2/EvalHeuristicEasy.h"
#include "dlvhex2/EvalHeuristicFromFile.h"
#include "dlvhex2/OnlineModelBuilder.h"
#include "dlvhex2/OfflineModelBuilder.h"

#include <fstream>
#include <iostream>

using namespace dlvhex;
using namespace merging;
using namespace dlvhex::merging::plugin;


// -------------------- Util (local functions!) --------------------

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
					if (i == argstart){
						argstart++;
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

// resolves escape sequences as follows:
//		\\ --> \ 
//		\' --> "
std::string unquote(std::string srcprogram){
	std::string program;
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
	return program;
}


// computes an 8-bytes salt string for MD5 hashing
std::string createSalt(){
	// compute random salt
	srand(time(NULL));
	char salt[8];
	for (int i = 0; i < 8; i++){
		salt[i] = 'a' + (rand() % 26);
	}
	return std::string(salt);
}

// computes an MD5 string over a certain input text and a given salt
std::string hash(std::string text){
	static std::string hashSalt = createSalt();	// create salt for MD5 hashing once
	return crypt(text.c_str(), (std::string("$1$") + hashSalt).c_str());
}


// ---------- HexCall ----------

HexCall::HexCall(CallType ct, std::string prog, std::string args, InterpretationConstPtr facts) : type(ct), program(prog), arguments(args), operatorImpl(NULL), inputfacts(facts){
	assert(ct == HexProgram || ct == HexFile);
	// compute hash value for the program
	//// (source or path) and input parameters
	std::stringstream h;
	h << prog;
	//h << std::endl;
	//for (AtomSet::const_iterator it = inputfacts.begin(); it != inputfacts.end(); ++it){
	//	h << (*it);
	//}
	hashcode = hash(h.str());
}

HexCall::HexCall(CallType ct, IOperator* op, bool deb, bool sil, std::vector<int> as, OperatorArguments kv) : type(ct), program(""), operatorImpl(op), debug(deb), silent(sil), asParams(as), kvParams(kv){
	assert(ct == OperatorCall);
}

const bool HexCall::operator==(const HexCall &other) const{
	if (type != other.type) return false;
	switch (type){
		case HexProgram:
		case HexFile:
			// Check if the programs (or the program paths), the command line arguments and the input facts are equivalent
			if (getHashCode() != other.getHashCode() || arguments != other.getArguments() || inputfacts->getStorage() != other.getFacts()->getStorage()) return false;
			return true;
			break;

		case OperatorCall:
			// Check if operator is the same
			if (other.operatorImpl != operatorImpl) return false;

			// Check if the answer set arguments are passed in the same order
			for (int i = 0; i < asParams.size(); i++){
				if (asParams[i] != other.asParams[i]) return false;
			}

			// Check if the sets of key-value arguments are equivalent (order does not matter)
			// One direction
			for (OperatorArguments::const_iterator it = kvParams.begin(); it != kvParams.end(); it++){
				bool found = false;
				for (OperatorArguments::const_iterator it2 = other.kvParams.begin(); it2 != other.kvParams.end(); it2++){
					if ((*it) == (*it2)){
						found = true;
						break;
					}
				}
				if (!found) return false;
			}

			// Other direction
			for (OperatorArguments::const_iterator it = other.kvParams.begin(); it != other.kvParams.end(); it++){
				bool found = false;
				for (OperatorArguments::const_iterator it2 = kvParams.begin(); it2 != kvParams.end(); it2++){
					if ((*it) == (*it2)){
						found = true;
						break;
					}
				}
				if (!found) return false;
			}
			return true;
			break;

		default:
			assert(0);
			break;
	}
}

const HexCall::CallType HexCall::getType() const{
	return type;
}

const std::string HexCall::getProgram() const{
	assert(getType() == HexProgram || getType() == HexFile);
	return program;
}

const std::string HexCall::getArguments() const{
	assert(getType() == HexProgram || getType() == HexFile);
	return arguments;
}

const InterpretationConstPtr HexCall::getFacts() const{
	assert(getType() == HexProgram || getType() == HexFile);
	return inputfacts;
}

const bool HexCall::hasInputFacts() const{
	assert(getType() == HexProgram || getType() == HexFile);
	if (inputfacts == InterpretationConstPtr()) return false;
	return inputfacts->getStorage().count() != 0;
}

const std::vector<int> HexCall::getAsParams() const{
	assert(getType() == OperatorCall);
	return asParams;
}

const OperatorArguments HexCall::getKvParams() const{
	assert(getType() == OperatorCall);
	return kvParams;
}

IOperator* HexCall::getOperator() const{
	assert(getType() == OperatorCall);
	return operatorImpl;
}

const std::string HexCall::getHashCode() const{
	assert(getType() == HexProgram || getType() == HexFile);
	return hashcode;
}

const bool HexCall::getDebug() const{
	assert(getType() == OperatorCall);
	return debug;
}

const bool HexCall::getSilent() const{
	assert(getType() == OperatorCall);
	return silent;
}



// ---------- HexAnswerCache ----------

HexAnswerCache::HexAnswerCache(){
	maxCacheEntries = -1;
	elementsInCache = 0;
}

HexAnswerCache::HexAnswerCache(int limit){
	maxCacheEntries = limit;
	elementsInCache = 0;
}

HexAnswerCache::~HexAnswerCache(){
	// cleanup
	for (int i = 0; i < cache.size(); i++)
		if (cache[i].second) delete cache[i].second;
}

HexAnswer* HexAnswerCache::loadHexProgram(const HexCall& call){
	assert(call.getType() == HexCall::HexProgram);

	HexAnswer* result = new HexAnswer();
	InputProviderPtr ip(new InputProvider());
	ip->addStringInput(unquote(call.getProgram()), "nestedprog");

	std::vector<InterpretationPtr> answer = ctx->evaluateSubprogram(ip, call.getFacts());
	BOOST_FOREACH (InterpretationPtr intr, answer){
		result->push_back(intr);
	}

	return result;
}

HexAnswer* HexAnswerCache::loadHexFile(const HexCall& call){
	assert(call.getType() == HexCall::HexFile);

	HexAnswer* result = new HexAnswer();

	InputProviderPtr ip(new InputProvider());
	ip->addFileInput(call.getProgram());

	std::vector<InterpretationPtr> answer = ctx->evaluateSubprogram(ip, call.getFacts());
	BOOST_FOREACH (InterpretationPtr intr, answer){
		result->push_back(intr);
	}

	return result;
}

HexAnswer* HexAnswerCache::loadOperatorCall(const HexCall& call){
	assert(call.getType() == HexCall::OperatorCall);

	HexAnswer opanswer;

	// make a list of pointers to all answers passed to this operator
	std::vector<int> answerIndices = call.getAsParams();
	std::vector<HexAnswer*> answers;
	for (std::vector<int>::iterator it = answerIndices.begin(); it != answerIndices.end(); ++it){
		// prevent the used cache entries from being removed
		locks[*it]++;
		if (cache[*it].second == NULL) load(*it);
		answers.push_back(cache[*it].second);
	}
	OperatorArguments oa = call.getKvParams();

	// check if all passed parameters are actually expected by the operator
	bool provided = false;
	try{
		std::set<std::string> params = call.getOperator()->getRecognizedParameters();
		provided = true;
		for (OperatorArguments::iterator it = oa.begin(); it != oa.end(); ++it){
			if (params.find(it->first) == params.end()) throw IOperator::OperatorException(std::string("Parameter \"") + it->first + std::string("\" is not recognized by this operator."));
		}
	}catch(...){
		if (provided == true){
			throw;
		}
	}

	// Finally call the operator
	opanswer = call.getOperator()->apply(!call.getSilent() && call.getDebug(), (int)call.getAsParams().size(), answers, oa);

	for (std::vector<int>::iterator it = answerIndices.begin(); it != answerIndices.end(); ++it){
		assert(locks[*it] > 0);
		locks[*it]--;
	}

	return new HexAnswer(opanswer);
}

void HexAnswerCache::load(const int index){
	assert(index >=0 && index < size());

	// this method must only be called for elements that are currently not in the cache
	assert(cache[index].second == NULL);

	// check type of the cache entry
	HexAnswer* result;
	switch(cache[index].first.getType()){
		case HexCall::HexProgram:
			result = loadHexProgram(cache[index].first);
			break;
		case HexCall::HexFile:
			result = loadHexFile(cache[index].first);
			break;
		case HexCall::OperatorCall:
			result = loadOperatorCall(cache[index].first);
			break;
		default:
			assert(0);
			break;
	}

	// store result in the cache
	cache[index].second = result;
//std::cout << "Have " << cache[index].size() << " answer sets" << std::endl;
	elementsInCache++;

	// make sure that the cache does not contain too many items
	reduceCache();
}

// resets the last access time for a certain entry to 0 and increments all others
void HexAnswerCache::access(const int index){
	assert(index >=0 && index < size());

	// element was accessed, all others were not
	accessCounter[index] = 0;
	for (int i = 0; i < cache.size(); i++)
		if (i != index) accessCounter[i]++;
}

// removes entries from the cache (if possible) such that no more than maxCacheEntries are contained
void HexAnswerCache::reduceCache(){
	// do we restrict the number of cache entries?
	if (maxCacheEntries < 0) return;

	// try to reduce the number of cache entries such that no more than maxCacheEntries are there
	while (elementsInCache > maxCacheEntries){
		// find element with oldes access timestamp
		int oldesAccessCounter = -1;
		for (int i = 0; i < cache.size(); i++){
			if (locks[i] == 0 && cache[i].second != NULL && (accessCounter[i] > oldesAccessCounter || oldesAccessCounter == -1))
				oldesAccessCounter = i;
		}
		if (oldesAccessCounter == -1)	// no element can be outsourced
			return;
		else{
			// remove oldest element
			delete cache[oldesAccessCounter].second;
			cache[oldesAccessCounter].second = NULL;
			accessCounter[oldesAccessCounter] = 0;
			elementsInCache--;
		}
	}
}

bool HexAnswerCache::SubprogramAnswerSetCallback::operator()(AnswerSetPtr model){
	answersets.push_back(model->interpretation);
	return true;
}

const int HexAnswerCache::operator[](const HexCall call){
	int index = 0;
	for (std::vector<HexAnswerCacheEntry>::iterator it = cache.begin(); it != cache.end(); ++it, index++){
		if (it->first == call){
			return index;
		}
	}
	// not in cache yet: add it
	cache.push_back(std::pair<HexCall, HexAnswer*>(call, NULL));
	accessCounter.push_back(0);
	locks.push_back(0);
	load(index);
	access(index);
	return index;
}

HexAnswer& HexAnswerCache::operator[](const int index){
	assert(index >=0 && index < size());

	// find the entry for this call
	// check if the result is in the cache
	if (cache[index].second == NULL){
		load(index);
	}
	access(index);
	// now it's in the cache for sure
	return *(cache[index].second);
}

const int HexAnswerCache::size(){
	return cache.size();
}

void HexAnswerCache::setProgramCtx(ProgramCtx& ctx){
	this->ctx = &ctx;
	this->reg = ctx.registry();
}
