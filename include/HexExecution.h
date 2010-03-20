#ifndef __HEXEXECUTION_H_
#define __HEXEXECUTION_H_

#include <InternalTypes.h>
#include <dlvhex/PluginInterface.h>
#include <dlvhex/ASPSolver.h>
#include <dlvhex/HexParserDriver.h>
#include <stdlib.h>
#include <string>

namespace dlvhex {
	namespace merging {
		namespace plugin{
			/**
			 * This class implements an external atom which is capable of executing a hex program given as string.
			 * Usage:
			 * &hex[program, cmdargs](R)
			 *	program	... string containing a hex program as sourcecode
			 *	cmdargs	... string of command line arguments passed to dlvhex
			 *	R	... handle to the answer of the given program
			 */
			class HexAtom : public PluginAtom
			{
			private:
				HexAnswerCache &resultsetCache;

				std::string salt;	// salt for MD5 hashing
			public:

				HexAtom(HexAnswerCache &rsCache);
				virtual ~HexAtom();
				virtual void retrieve(const Query& query, Answer& answer) throw (PluginError);
			};

			/**
			 * This class implements an external atom which is capable of executing a hex program stored in a file.
			 * Usage:
			 * &hexfile[programpath, cmdargs](R)
			 *	programpath	... string containing the path to a hex program
			 *	cmdargs	... string of command line arguments passed to dlvhex
			 *	R		... handle to the answer of the given program
			 */
			class HexFileAtom : public PluginAtom
			{
			private:
				HexAnswerCache &resultsetCache;

				std::string salt;	// salt for MD5 hashing
			public:

				HexFileAtom(HexAnswerCache &rsCache);

				virtual ~HexFileAtom();
				virtual void retrieve(const Query& query, Answer& answer) throw (PluginError);
			};

			/**
			 * This class implements an external atom which can be used to access the answer sets of a hex program or hex file executed before.
			 * Usage:
			 * &answersets[R](AS)
			 *	R		... handle to the answer of a program or an operator application
			 *	AS		... list of handles to the answer sets (in general arbitrary many) within the given answer
			 */
			class AnswerSetsAtom : public PluginAtom
			{
			private:
				HexAnswerCache &resultsetCache;

			public:

			    AnswerSetsAtom(HexAnswerCache &rsCache);
			    virtual ~AnswerSetsAtom();
			    virtual void retrieve(const Query& query, Answer& answer) throw (PluginError);
			};

			/**
			 * This class implements an external atom which can be used to access the predicates and their arities that occur
			 * in a certain answer set.
			 * Usage:
			 * &predicates [R, AS](Pred, Arity)
			 *	R		... handle to the answer of a program or an operator application
			 *	AS		... handle to an answer set within answer R
			 *	Pred		... a string containing a predicate occurring in answer set AS within answer R
			 *	Arity		... arity of pred (0 for propositional atoms)
			 */
			class PredicatesAtom : public PluginAtom
			{
			private:
				HexAnswerCache &resultsetCache;

			public:

			    PredicatesAtom(HexAnswerCache &rsCache);
			    virtual ~PredicatesAtom();
			    virtual void retrieve(const Query& query, Answer& answer) throw (PluginError);
			};

			/**
			 * This class implements an external atom which can be used to access the arguments of a certain tuple of an answer set computed before.
			 * Usage:
			 * &arguments[R, AS, Pred](RunningNr, ArgIndex, Arg)
			 *	R		... handle to the answer of a program or an operator application
			 *	AS		... handle to an answer set within answer R
			 *	Pred		... a first order predicate occurring in AS within R
			 *	RunningNr	... a running integer since the same predicate can occur arbitraryly often within an answer set. the number has no inherent meaning, but may be used for distincting between several predicate occurrances
			 *	ArgIndex	... index of this argument value (index origin 0)
			 *	Arg		... argument value
			 */
			class ArgumentsAtom : public PluginAtom
			{
			private:
				HexAnswerCache &resultsetCache;

			public:

			    ArgumentsAtom(HexAnswerCache &rsCache);
			    virtual ~ArgumentsAtom();
			    virtual void retrieve(const Query& query, Answer& answer) throw (PluginError);
			};
		}
	}
}
#endif
