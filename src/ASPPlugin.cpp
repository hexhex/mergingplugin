
//
// this include is necessary
//
#include <stdlib.h>

#include <iostream>
#include <HexExecution.h>
#include <Operators.h>

namespace dlvhex {
	namespace asp {

		//
		// A plugin must derive from PluginInterface
		//
		class ASPPlugin : public PluginInterface
		{
		private:
			// Cache for answer sets
			HexAnswerCache resultsetCache;

			// External atoms provided by this plugin
			HexAtom* hex_atom;
			HexFileAtom* hexfile_atom;
			AnswerSetsAtom* as_atom;
			TuplesAtom* tuples_atom;
			ArgumentsAtom* arguments_atom;
			OperatorAtom* operator_atom;

			// List of paths where operator libs are searched for
			std::vector<std::string> searchpaths;
		public:
			ASPPlugin(){
				hex_atom = NULL;
				hexfile_atom = NULL;
				as_atom = NULL;
				tuples_atom = NULL;
				arguments_atom = NULL;
				operator_atom = NULL;
			}

			virtual ~ASPPlugin(){
				delete hexfile_atom;
				delete hex_atom;
				delete as_atom;
				delete tuples_atom;
				delete arguments_atom;
				delete operator_atom;
			}

			//
			// register all atoms of this plugin:
			//
			virtual void
			getAtoms(AtomFunctionMap& a)
			{
				// Create external atoms
				hex_atom = new HexAtom(resultsetCache);
				hexfile_atom = new HexFileAtom(resultsetCache);
				as_atom = new AnswerSetsAtom(resultsetCache);
				tuples_atom = new TuplesAtom(resultsetCache);
				arguments_atom = new ArgumentsAtom(resultsetCache);
				operator_atom = new OperatorAtom(resultsetCache);
				operator_atom->setSearchPaths(searchpaths);

				boost::shared_ptr<PluginAtom> hex_atom_ptr(hex_atom);
				boost::shared_ptr<PluginAtom> hexfile_atom_ptr(hexfile_atom);
				boost::shared_ptr<PluginAtom> as_atom_ptr(as_atom);
				boost::shared_ptr<PluginAtom> tuples_atom_ptr(tuples_atom);
				boost::shared_ptr<PluginAtom> arguments_atom_ptr(arguments_atom);
				boost::shared_ptr<PluginAtom> operator_atom_ptr(operator_atom);

				// Register external atoms
				a["hex"] = hex_atom_ptr;
				a["hexfile"] = hexfile_atom_ptr;
				a["answersets"] = as_atom_ptr;
				a["tuples"] = tuples_atom_ptr;
				a["arguments"] = arguments_atom_ptr;
				a["operator"] = operator_atom_ptr;
			}

			virtual void
			setOptions(bool doHelp, std::vector<std::string>& argv, std::ostream& out)
			{
				for (std::vector<std::string>::iterator it = argv.begin(); it != argv.end(); it++){
					if (	it->substr(0, std::string("--operatorpath").size()) == std::string("--operatorpath") ||
						it->substr(0, std::string("--op").size()) == std::string("--op")){
						// Extract search paths for operator libs
						searchpaths.clear();
						std::string rest = it->substr(it->find_first_of('=', 0) + 1);
						while (rest.find_first_of(',') != std::string::npos){
							searchpaths.push_back(it->substr(0, it->find_first_of(',')));
							rest = rest.substr(it->find_first_of(',') + 1);
						}
						if (rest != std::string("")){
							searchpaths.push_back(rest);
						}
						std::cout << "ASPPlugin: Operator search paths:" << std::endl;
						for (int i = 0; i < searchpaths.size(); i++){
							std::cout << "   " << searchpaths[i] << std::endl;
						}
						// Argument was processed
						argv.erase(it);
						break;	// iterator is invalid now
					}
				}
				if (operator_atom != NULL) operator_atom->setSearchPaths(searchpaths);
			}

		};


		//
		// now instantiate the plugin
		//
		ASPPlugin theASPPlugin;

	} // namespace asp
} // namespace dlvhex

//
// and let it be loaded by dlvhex!
//
extern "C"
dlvhex::asp::ASPPlugin*
PLUGINIMPORTFUNCTION()
{

  dlvhex::asp::theASPPlugin.setPluginName("dlvhex-ASPPlugin");
  dlvhex::asp::theASPPlugin.setVersion(	0,
					0,
					1);
  return &dlvhex::asp::theASPPlugin;
}


