#include <DlvhexSolver.h>
#include <DLVHexProcess.h>
#include <fstream>
#include <string>
#include <sstream>

using namespace dlvhex::merging::plugin;

DlvhexSolver::Options::Options() : ASPSolverManager::GenericOptions::GenericOptions(){
	arguments.push_back("--silent");
}

DlvhexSolver::Options::~Options(){
	this->GenericOptions::~GenericOptions();
}

DlvhexSolver::Delegate::Delegate(const Options& opt) : ASPSolverManager::DelegateBase<Options>::DelegateBase(opt) {
	proc = new DLVHexProcess();

	for (int i = 0; i < options.arguments.size(); i++)
		proc->addOption(options.arguments[i]);
/*
std::vector<std::string> o = proc->commandline();
for (int i = 0; i < o.size(); i++)
std::cerr << o[i] << " " << std::endl; 
*/
}

DlvhexSolver::Delegate::~Delegate() {
	int retcode = proc->close();
	delete proc;
}

void DlvhexSolver::Delegate::useASTInput(const Program& idb, const AtomSet& edb) {
	proc->addOption("--");

	// fork dlvhex process
	proc->spawn();

	std::ostream& programStream = proc->getOutput();

	///@todo: this is marked as "temporary hack" in globals.h -> move this info into ProgramCtx and allow ProgramCtx to contribute to the solving process
	if( !Globals::Instance()->maxint.empty() )
	programStream << Globals::Instance()->maxint << std::endl;

	// output program
	typedef boost::shared_ptr<HexPrintVisitor> PrinterPtr;
	PrinterPtr printer;
	printer = PrinterPtr(new HexPrintVisitor(programStream));

	idb.accept(*printer);
	edb.accept(*printer);

	proc->endoffile();
}

void DlvhexSolver::Delegate::useStringInput(const std::string& program){
	proc->addOption("--");
	proc->spawn();
	proc->getOutput() << program << std::endl;
	proc->endoffile();
}

void DlvhexSolver::Delegate::useFileInput(const std::string& fileName){
	proc->addOption(fileName);
	proc->spawn();
	proc->endoffile();
}

void DlvhexSolver::Delegate::getOutput(std::vector<AtomSet>& result){
/*
std::cerr << "OUTPUT IS " << std::endl;
std::istream& inp = proc->getInput();
std::string s;

while (inp.good() && std::getline(inp, s))
{
	std::cerr << s << std::endl;
}
*/

	// parse result
	HexResultParserDriver parser;
	parser.parse(proc->getInput(), result);
}
