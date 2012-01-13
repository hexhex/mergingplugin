//
// this include is necessary
//
#include <stdlib.h>

#include <iostream>
#include <DLVHexProcess.h>
#include <HexExecution.h>
#include <Operators.h>
#include <ArbProcess.h>
#include <SpiritParser.h>
#include <ParseTreeNode.h>
#include <CodeGenerator.h>

#include <dlvhex/ProgramCtx.h>

#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <dirent.h>

std::stringstream filter;

namespace dlvhex {
	namespace merging {
		namespace plugin{
#if 0
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

			class MPCompiler : public PluginConverter
			{
			private:
				bool dumpmp;
			public:
				MPCompiler(bool dump) : dumpmp(dump){}

				virtual void
				convert(std::istream& i, std::ostream& o)
				{
					std::string line;
					std::stringstream ss;
					while(getline(i, line)) {
						ss << line << std::endl;
					}

					dlvhex::merging::tools::mpcompiler::SpiritParser parserInst(ss.str());

					// Try to parse the input
					// Note: This call has the side effect that stdin is read up to eof
					parserInst.parse();

					// If it was successful, generate output code
					// otherwise display an error message
					if (parserInst.succeeded()){
						// Retrieve parse tree
						dlvhex::merging::tools::mpcompiler::ParseTreeNode* parseTree = parserInst.getParseTree();

						// GenerateCode
						// On success, write the code to std::cout and "errors" (in this case warnings) to std::cerr
						dlvhex::merging::tools::mpcompiler::CodeGenerator cginst(parseTree);
						cginst.generateCode(dumpmp ? std::cout : o, std::cerr);

						if (!cginst.succeeded()){
							// Code generation failed
							std::cerr << "Code generation finished with errors:" << std::endl;
							std::cerr << "   " << cginst.getErrorCount() << " error" << (cginst.getErrorCount() == 0 || cginst.getErrorCount() > 1 ? "s" : "") << ", " << cginst.getWarningCount() << " warning" << (cginst.getWarningCount() == 0 || cginst.getWarningCount() > 1 ? "s" : "") << std::endl;
							delete parseTree;
							throw PluginError("Merging plan compilation failed");
						}

						delete parseTree;
					}else{
						std::cerr << "Parsing finished with errors:" << std::endl;
						std::cerr << "   " << parserInst.getErrorCount() << " error" << (parserInst.getErrorCount() == 0 || parserInst.getErrorCount() > 1 ? "s" : "") << ", " << parserInst.getWarningCount() << " warning" << (parserInst.getWarningCount() == 0 || parserInst.getWarningCount() > 1 ? "s" : "") << std::endl;
						throw PluginError("Merging plan parsing failed");
					}

					// send inconsistent program to avoid answer set output
					if (dumpmp) o << "p :- not p.";
				}
			};

			class DLV : public Rewriter
			{
			public:
				DLV(std::string c) : Rewriter(std::string("dlv -silent ") + c)
				{
				}

				std::string diagToAs(std::string s){
					// replace all blanks with ,
					for (int i = 0; i < s.length(); i++) if (s[i] == ' ') s[i] = ',';
					return s;
				}

				virtual void
				convert(std::istream& i, std::ostream& o)
				{
					std::stringstream dlvoutput;
					Rewriter::convert(i, dlvoutput);

					// strip off dlv's meta output concerning cost and diagnosis (since this is not valid dlvhex output format)
					std::string dlv = dlvoutput.str();

					while (dlv.length() > 0){
						if (dlv[0] == '{'){
							// print everything until '}'
							std::cout << dlv.substr(0, dlv.find_first_of("}") + 1) << std::endl;
							dlv = dlv.substr(dlv.find_first_of("}") + 1);
						}else{
							std::string diag("Diagnosis: ");
							if (dlv.substr(0, diag.length()) == diag){
								std::string firstDiag = std::string("{") + diagToAs(dlv.substr(diag.length(), dlv.find_first_of("\n") - diag.length())) + std::string("}");
								dlv = firstDiag + dlv.substr(dlv.find_first_of("\n") + 1);
							}
							// goto first '{' (if contained)
							if (dlv.find_first_of("{") == std::string::npos){
								// not contained: finished
								dlv = "";
							}else{
								dlv = dlv.substr(dlv.find_first_of("{"));
							}
						}
					}

					o << ":- not a.";
				}
			};
#endif

			//
			// A plugin must derive from PluginInterface
			//
			HexAnswerCache resultsetCache;
			class MergingPlugin : public PluginInterface
			{
			private:
				static const int CALL_MAX_ARITY = 32;

				bool silentMode;
				bool debugMode;

				// Cache for answer sets

			public:
				virtual std::vector<PluginAtomPtr> createAtoms(ProgramCtx& ctx) const
				{
					resultsetCache.setRegistry(ctx.registry());

					std::vector<PluginAtomPtr> ret;
			
					// return smart pointer with deleter (i.e., delete code compiled into this plugin)
					ret.push_back(PluginAtomPtr(new HexAtom(resultsetCache), PluginPtrDeleter<PluginAtom>()));
					ret.push_back(PluginAtomPtr(new HexFileAtom(resultsetCache), PluginPtrDeleter<PluginAtom>()));
					for (int i = 0; i < CALL_MAX_ARITY; ++i){
						ret.push_back(PluginAtomPtr(new CallHexAtom(resultsetCache, i), PluginPtrDeleter<PluginAtom>()));
						ret.push_back(PluginAtomPtr(new CallHexFileAtom(resultsetCache, i), PluginPtrDeleter<PluginAtom>()));
					}
					for (int in = 0; in <= 5; ++in){
						for (int out = 0; out <= 5; ++out){
							ret.push_back(PluginAtomPtr(new SimulatorAtom(in, out), PluginPtrDeleter<PluginAtom>()));
						}
					}
					ret.push_back(PluginAtomPtr(new AnswerSetsAtom(resultsetCache), PluginPtrDeleter<PluginAtom>()));
					ret.push_back(PluginAtomPtr(new PredicatesAtom(resultsetCache), PluginPtrDeleter<PluginAtom>()));
					ret.push_back(PluginAtomPtr(new ArgumentsAtom(resultsetCache), PluginPtrDeleter<PluginAtom>()));

					return ret;
				}
			};


			//
			// now instantiate the plugin
			//
			MergingPlugin theMergingPlugin;
		} // namespace plugin
	} // namespace merging
} // namespace dlvhex


//
// and let it be loaded by dlvhex!
//
extern "C"
dlvhex::merging::plugin::MergingPlugin*
PLUGINIMPORTFUNCTION()
{
//  dlvhex::merging::plugin::theMergingPlugin.setPluginName("dlvhex-mergingplugin");
//  dlvhex::merging::plugin::theMergingPlugin.setVersion(	2,
//							0,
//							0);
  return &dlvhex::merging::plugin::theMergingPlugin;
}
