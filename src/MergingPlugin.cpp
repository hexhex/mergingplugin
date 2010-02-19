
//
// this include is necessary
//
#include <stdlib.h>

#include <iostream>
#include <HexExecution.h>
#include <Operators.h>
#include <ArbProcess.h>

#include "dlvhex/globals.h"

#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <dirent.h>



namespace dlvhex {
	namespace merging {

		class Rewriter : public PluginConverter
		{
		private:
			std::string command;
		public:
			Rewriter(std::string c) : command(c)
			{
			}

			virtual void
			convert(std::istream& i, std::ostream& o)
			{
				ArbProcess rewriter(command);
				rewriter.spawn();

				std::istream& pi = rewriter.getInput();
				std::ostream& po = rewriter.getOutput();
				std::string line;

				// Send program to input rewriter
				while(getline(i, line)) {
					po << line << std::endl;
				}
				rewriter.endoffile();

				// Read output from input rewriter and overwrite the input program with modified one
				std::stringstream routput;
				while(getline(pi, line)) {
					routput << line << std::endl;
				}
				o << routput.str();

				// On errors throw a PluginError
				int errcode;
				if((errcode = rewriter.close()) != 0){
					throw PluginError(std::string("Error from rewriter \"") + command + std::string("\""));
				}

			}
		};

		//
		// A plugin must derive from PluginInterface
		//
		class MergingPlugin : public PluginInterface
		{
		private:
			bool silentMode;
			bool debugMode;

			// Cache for answer sets
			HexAnswerCache resultsetCache;

			// External atoms provided by this plugin
			HexAtom* hex_atom;
			HexFileAtom* hexfile_atom;
			AnswerSetsAtom* as_atom;
			PredicatesAtom* predicates_atom;
			ArgumentsAtom* arguments_atom;
			OperatorAtom* operator_atom;

			// List of paths where operator libs are searched for
			std::vector<std::string> searchpaths;

			// Strings containing the paths and/or program names of input and output rewriters
			Rewriter *inputrewriter;
			Rewriter *outputrewriter;

			std::string removeQuotes(std::string arg){
				if (arg[0] == '\"' && arg[arg.length() - 1] == '\"'){
					return arg.substr(1, arg.length() - 2);
				}else{
					return arg;
				}
			}
		public:
			MergingPlugin(){
				hex_atom = NULL;
				hexfile_atom = NULL;
				as_atom = NULL;
				predicates_atom = NULL;
				arguments_atom = NULL;
				operator_atom = NULL;
				inputrewriter = NULL;
				outputrewriter = NULL;

				// Create external atoms
				hex_atom = new HexAtom(resultsetCache);
				hexfile_atom = new HexFileAtom(resultsetCache);
				as_atom = new AnswerSetsAtom(resultsetCache);
				predicates_atom = new PredicatesAtom(resultsetCache);
				arguments_atom = new ArgumentsAtom(resultsetCache);
				operator_atom = new OperatorAtom(resultsetCache);
			}

			virtual ~MergingPlugin(){
				if (hexfile_atom) delete hexfile_atom;
				if (hex_atom) delete hex_atom;
				if (as_atom) delete as_atom;
				if (predicates_atom) delete predicates_atom;
				if (arguments_atom) delete arguments_atom;
				if (operator_atom) delete operator_atom;
				if (inputrewriter) delete inputrewriter;
				if (outputrewriter) delete outputrewriter;
			}

			virtual PluginConverter*
			createConverter()
			{
				return inputrewriter;
			}

			//
			// register all atoms of this plugin:
			//
			virtual void
			getAtoms(AtomFunctionMap& a)
			{
				boost::shared_ptr<PluginAtom> hex_atom_ptr(hex_atom);
				boost::shared_ptr<PluginAtom> hexfile_atom_ptr(hexfile_atom);
				boost::shared_ptr<PluginAtom> as_atom_ptr(as_atom);
				boost::shared_ptr<PluginAtom> predicates_atom_ptr(predicates_atom);
				boost::shared_ptr<PluginAtom> arguments_atom_ptr(arguments_atom);
				boost::shared_ptr<PluginAtom> operator_atom_ptr(operator_atom);

				// Register external atoms
				a["hex"] = hex_atom_ptr;
				a["hexfile"] = hexfile_atom_ptr;
				a["answersets"] = as_atom_ptr;
				a["predicates"] = predicates_atom_ptr;
				a["arguments"] = arguments_atom_ptr;
				a["operator"] = operator_atom_ptr;
			}

			virtual void
			setOptions(bool doHelp, std::vector<std::string>& argv, std::ostream& out)
			{
				debugMode = false;
				silentMode = Globals::Instance()->getOption("Silent") ? true : false;

				if (!doHelp){
					bool argFound = true;
					while (argFound){
						argFound = false;
						for (std::vector<std::string>::iterator it = argv.begin(); it != argv.end(); it++){
							if (	it->substr(0, std::string("--operatorpath=").size()) == std::string("--operatorpath=") ||
								it->substr(0, std::string("--op=").size()) == std::string("--op=")){
								// Extract search paths for operator libs
								searchpaths.clear();
								std::string rest = it->substr(it->find_first_of('=', 0) + 1);
								while (rest.find_first_of(',') != std::string::npos){
									searchpaths.push_back(removeQuotes(it->substr(0, it->find_first_of(','))));
									rest = rest.substr(it->find_first_of(',') + 1);
								}
								if (rest != std::string("")){
									searchpaths.push_back(rest);
								}
								// Argument was processed
								argv.erase(it);
								argFound = true;
								break;	// iterator is invalid now
							}
							if (	it->substr(0, std::string("--inputrewriter=").size()) == std::string("--inputrewriter=") ||
								it->substr(0, std::string("--irw=").size()) == std::string("--irw=")){
								inputrewriter = new Rewriter(removeQuotes(it->substr(it->find_first_of('=', 0) + 1)));
								// Argument was processed
								argv.erase(it);
								argFound = true;
								break;
							}
							if (	(*it) == std::string("--operatordebug") ||
								(*it) == std::string("--od")){
								debugMode = true;

								// Argument was processed
								argv.erase(it);
								argFound = true;
								break;
							}

	/*
							if (	it->substr(0, std::string("--outputrewriter=").size()) == std::string("--outputrewriter=") ||
								it->substr(0, std::string("--orw=").size()) == std::string("--orw=")){
								outputrewriter = it->substr(it->find_first_of('=', 0) + 1);
								// Argument was processed
								argv.erase(it);
								argFound = true;
								break;
							}
	*/
						}
					}

					if (operator_atom != NULL){
						// Load all operators found in dlvhex' system plugin libraries
						std::stringstream sysplugindir;
						sysplugindir << SYS_PLUGIN_DIR;
						operator_atom->addOperators(sysplugindir.str(), silentMode, debugMode);

						// Load all operators found in dlvhex' user plugin libraries
						std::stringstream userplugindir;
						const char* homedir = ::getpwuid(::geteuid())->pw_dir;
						userplugindir << homedir << "/" << USER_PLUGIN_DIR;
						operator_atom->addOperators(userplugindir.str(), silentMode, debugMode);

						// Load additional operator directories
						for (std::vector<std::string>::iterator it = searchpaths.begin(); it != searchpaths.end(); it++){
							operator_atom->addOperators(*it, silentMode, debugMode);
						}
					}
				}else{
					out	<< "Merging-plugin" << std::endl
						<< "--------------" << std::endl
						<< " Provided external atoms:" << std::endl
						<< "   &hex[Prog, Args](A)   ... Will execture the hex program in Prog with the dlvhex" << std::endl
						<< "                             arguments given in Args. A will be a handle to the" << std::endl
						<< "                             answer of the program." << std::endl
						<< "   &hexfile[FN, Args](A) ... Will execture the hex program in the file named by" << std::endl
						<< "                             FN with the dlvhex arguments given in Args." << std::endl
						<< "                             A will be a handle to the answer of the program." << std::endl
						<< "   &answersets[A](AS)    ... A is a handle to the answer of a hex program" << std::endl
						<< "                             (generated by &hex or &hexfile)" << std::endl
						<< "                             AS are handles to the answer sets of answer A" << std::endl
						<< "   &predicates[A, AS]    ... A is a handle to the answer of a hex program" << std::endl
						<< "        (Pr, Ar)             AS is a handle to an answer set of answer A" << std::endl
						<< "                             The tuples (Pr, Ar) list all predicates in the" << std::endl
						<< "                             given answer set together with their arities" << std::endl
						<< "   &arguments[A, AS, Pr] ... A is a handle to the answer of a hex program" << std::endl
						<< "        (RI, AI, Arg)        AS is a handle to an answer set of answer A" << std::endl
						<< "                             Pr is a predicate occurring in the given answer set" << std::endl
						<< "                             The triples (RI, AI, Arg) list all atoms in the" << std::endl
						<< "                             answer set where the given predicate is involved in." << std::endl
						<< "                             RI is just a running index, which enables the user to" << std::endl
						<< "                             determine which arguments belong together (since the" << std::endl
						<< "                             same predicates may occur multiple times within an" << std::endl
						<< "                             answer set). AI is the 0-based index of the argument" << std::endl
						<< "                             and Arg is the argument value." << std::endl
						<< "   &operator[N, As, KV]  ... N is the name of an operator" << std::endl
						<< "        (A)                  As is a binary predicate defining all the answers" << std::endl
						<< "                               to be passed to the operator. The first element" << std::endl
						<< "                               of each tuple is the 0-based index, the second" << std::endl
						<< "                               one is a handle to the answer to be passed" << std::endl
						<< "                             KV is a binary predicate with key-value pairs to" << std::endl
						<< "                               be passed to the operator" << std::endl
						<< "                             A is a handle to the answer of the operator" << std::endl
						<< "   &operator[N, As, KV]  ... N is the name of an operator" << std::endl << std::endl
						<< "   &operator[N, As, KV]  ... N is the name of an operator" << std::endl << std::endl
						<< " Arguments:" << std::endl
						<< " --operatorpath  This option adds additional search paths for operator libraries." << std::endl
						<< " or        --op  It is necessary for the &operator predicate. A path can either" << std::endl
						<< "                 point to a directory containing .so libraries or directly to an" << std::endl
						<< "                 .so library. In case of directories, operators located in" << std::endl
						<< "                 subdirectories will _NOT_ be found!" << std::endl
						<< "                 By default, the system and user plugin directory will be added" << std::endl
						<< "" << std::endl
						<< " --inputrewriter This option causes ASP-plugin to exectute the given command line" << std::endl
						<< "                 string, pass the dlvhex input to this process and use the result" << std::endl
						<< "                 of the process as new dlvhex input." << std::endl
						<< "                 Example: dlvhex -irw=cat myfile.hex" << std::endl
						<< "                          will run cat on the content of myfile.hex, before dlvhex" << std::endl
						<< "                          is actually executed (however, cat has no effect of" << std::endl
						<< "                          course, since it just echos the standard input)" << std::endl
						<< " or        --irw" << std::endl
						<< " --operatordebug Adds more details during operator loading and is useful for operator." << std::endl
						<< "                 debugging. If --silent is passed, this flag is ignored." << std::endl
						<< "" << std::endl << std::endl;
				}
			}

		};


		//
		// now instantiate the plugin
		//
		MergingPlugin theMergingPlugin;

	} // namespace asp
} // namespace dlvhex

#include "OpUnion.h"

//
// and let it be loaded by dlvhex!
//
extern "C"
dlvhex::merging::MergingPlugin*
PLUGINIMPORTFUNCTION()
{
  dlvhex::merging::theMergingPlugin.setPluginName("dlvhex-mergingplugin");
  dlvhex::merging::theMergingPlugin.setVersion(	0,
						1,
						0);
  return &dlvhex::merging::theMergingPlugin;
}
