#ifndef __HEXANSWERCACHE_H_
#define __HEXANSWERCACHE_H_

#include <PublicTypes.h>
#include <IOperator.h>
#include <dlvhex/Registry.hpp>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
	namespace merging{
		namespace plugin{
			/**
			 * Unique identification of a certain hex or operator call
			 */
			class HexCall{
			public:
				/**
				 * HexProgram: Call of a hex program given as string which is directly embedded within another hex program
				 * HexFile: Call of an external hex program stored in a file
				 * OperatorCall: Application of a built-in or external operator on other hex program results
				 */
				enum CallType{
					HexProgram,
					HexFile,
					OperatorCall,
				};

			private:
				CallType type;
				std::string program;
				std::string arguments;
				InterpretationConstPtr inputfacts;
				std::string hashcode;

				std::vector<int> asParams;
				OperatorArguments kvParams;
				IOperator* operatorImpl;
				bool debug;
				bool silent;
			public:
				HexCall(CallType ct, std::string prog, std::string args, InterpretationConstPtr facts);
				HexCall(CallType ct, IOperator* op, bool debug, bool silent, std::vector<int> as, OperatorArguments kv);
				const bool operator==(const HexCall &other) const;

				const CallType getType() const;
				const std::string getProgram() const;
				const std::string getArguments() const;
				const InterpretationConstPtr getFacts() const;
				const bool hasInputFacts() const;
				const std::vector<int> getAsParams() const;
				const OperatorArguments getKvParams() const;
				IOperator* getOperator() const;
				const std::string getHashCode() const;
				const bool getDebug() const;
				const bool getSilent() const;
			};

			/*! \fn HexCall::HexCall(CallType ct, std::string prog, std::string args)
			 * \brief Constructs a new hash call identifier
			 * \param ct The type of the call: must be either HexProgram or HexFile; for operators use the overloaded constructor.
			 * \param prog The program source code or path to a program (depending on type)
			 * \param args The command line arguments for the program
			 */

			/*! \fn HexCall::HexCall(CallType ct, IOperator* op, bool debug, bool silent, std::vector<int> as, OperatorArguments kv)
			 * \brief Constructs a new hash call identifier
			 * \param ct The type of the call: must be either OperatorCall; for HexProgram or HexFile use the overloaded constructor.
			 * \param op A pointer to the operator in use
			 * \param debug Tells the operator if it's called in operator debug mode
			 * \param silent Tells the operator if dlvhex is called in silent mode
			 * \param as The indices of answers passed to the operator
			 * \param kv The list of key-value pairs passed to the operator
			 */

			/*! \fn bool HexCall::operator==(const HexCall &other)
			 * \brief Compares this hex call with another one
			 * \param \param other Reference to the hex call to compare this one with
			 * \param \return bool True in case that the calls are equivalent, otherwise false
			 */

			/*! \fn const std::string HexCall::getProgram() const
			 * \brief Returns the program or path (depending on type)
			 * \param std::string The program or path (depending on type)
			 */

			/*! \fn const std::string HexCall::getArguments() const
			 * \brief Returns the command line arguments for the program (only use this method for calls of type HexProgram or HexFile!)
			 * \param std::string The command line arguments
			 */

			/*! \fn const AtomSet HexCall::getFacts() const
			 * \brief Returns the facts passed to the called program.
			 * \param AtomSet Set of facts
			 */

			/*! \fn const bool HexCall::hasInputFacts() const
			 * \brief Returns true if the call passes at least one input fact to the subprogram.
			 * \param bool True if the call passes at least one input fact to the subprogram, otherwise false
			 */

			/*! \fn const std::vector<int> HexCall::getAsParams() const
			 * \brief Returns the list of indices passed to the operator (only use this method for calls of type OperatorCall!)
			 * \param std::vector<int> The list of indices passed to the operator
			 */

			/*! \fn const OperatorArguments HexCall::getKvParams() const
			 * \brief Returns the list of key-value pairs passed to the operator (only use this method for calls of type OperatorCall!)
			 * \param OperatorArguments The list of key-value pairs passed to the operator
			 */

			/*! \fn IOperator* OperatorArguments HexCall::getOperator() const
			 * \brief Returns a pointer to the called operator (only use this method for calls of type OperatorCall!)
			 * \param IOperator* A pointer to the called operator
			 */

			/*! \fn std::string OperatorArguments HexCall::getHashCode() const
			 * \brief Returns the hash value for this call (only use this method for calls of type HexProgram or HexFile!)
			 * \param std::string The hash value for this call
			 */

			/*! \fn const bool HexCall::getDebug() const
			 * \brief Returns if this operator is called in debug mode (only use this method for calls of type HexProgram or HexFile!)
			 * \param bool
			 */

			/*! \fn const bool HexCall::getSilent() const
			 * \brief Returns if this dlvhex instance is called in silent mode (only use this method for calls of type HexProgram or HexFile!)
			 * \param bool
			 */


			/**
			 * Manages the internal cache of hex answers. Removes old entries from the cache and reloads them in case of cache misses.
			 */
			class HexAnswerCache{
			private:
				ProgramCtx* ctx;
				RegistryPtr reg;
				typedef std::pair<HexCall, HexAnswer* > HexAnswerCacheEntry;
				std::vector<HexAnswerCacheEntry> cache;
				std::vector<int> locks;
				std::vector<long> accessCounter;
				int elementsInCache;
				int maxCacheEntries;

				void load(const int index);
				void access(const int index);
				void reduceCache();

				HexAnswer* loadHexProgram(const HexCall& call);
				HexAnswer* loadHexFile(const HexCall& call);
				HexAnswer* loadOperatorCall(const HexCall& call);
			public:
				class SubprogramAnswerSetCallback : public ModelCallback{
				public:
					std::vector<InterpretationPtr> answersets;
					virtual bool operator()(AnswerSetPtr model);
				};

				HexAnswerCache();
				HexAnswerCache(int limit);
				~HexAnswerCache();
				const int operator[](const HexCall call);
				HexAnswer& operator[](const int);
				const int size();
				void setProgramCtx(ProgramCtx& ctx);
			};

			/*! \fn HexAnswerCache::HexAnswerCache()
			 * \brief Constructs a new cache without limitation of the stored elements (only the physical available memory)
			 */

			/*! \fn HexAnswerCache::HexAnswerCache(int limit)
			 * \brief Constructs a new cache with limitation of the stored elements
			 * \param limit The maximum number of elements stored permanently in the cache; additionally the cache may contains an arbitrary number of elements temporarily if this is necessary for operator applications (maximum number of entries is bounded by the number of intermediate results during computation)
			 */

			/*! \fn HexAnswerCache::~HexAnswerCache()
			 * \brief Destructor: Frees all entries in cache
			 */

			/*! \fn const int HexAnswerCache::operator[](HexCall call)
			 * \brief Retrieves the index of a certain call in the cache; if it is not contained yet, a new entry will be added.
			 * \param call The hex call to look for
			 * \param int 0-based index to the entry
			 */

			/*! \fn const int HexAnswerCache::operator[](int index)
			 * \brief Retrieves the answer of a call with a certain index; to map calls to answers, call: cache[cache[call]]
			 * \param index The index of the desired hex call which's answer shall be retrieved
			 * \param HexAnswer* A pointer to the answer of the hex call with the given index
			 */

			/*! \fn const int size()
			 * \brief Returns the current size of the cache
			 * \param int The current size of the cache (including both elements that are actually in the cache and those that are currently outsourced but managed by the cache)
			 */
		}
	}
}
#endif
