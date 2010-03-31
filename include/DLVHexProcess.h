#ifndef __DLVHexProcess_h_
#define __DLVHexProcess_h_

#include <dlvhex/Process.h>
#include <dlvhex/DLVProcess.h>
#include <dlvhex/globals.h>
#include <dlvhex/PrintVisitor.h>

#include <dlvhex/ASPSolver.h>
//#include <dlvhex/ASPSolver.tcc>
#include <dlvhex/DLVresultParserDriver.h>

#include "dlvhex/Program.h"
#include "dlvhex/AtomSet.h"
#include "dlvhex/DLVresultParserDriver.h"
#include "dlvhex/PrintVisitor.h"
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
namespace merging{
namespace plugin{


//	We need a mixture of RawPrintVisitor and DLVPrintVisitor. For usual atoms, DLVPrintVisitor is exactly what we need since DLV's and dlvhex' syntax is equivalent.
//	But for external atoms we need the RawPrintVisitor. However, the RawPrintVisitor uses a rather strange syntax (set braces) for AtomSets.
//	Thus we need some methods of both visitors. Multiple inheritance would be nice here, but it cannot be used since the Raw- and DLVPrintVisitor do not inherit
//	from their base class with keyword "virtual".
//
class HexPrintVisitor : public virtual PrintVisitor{
private:
	RawPrintVisitor raw;
	DLVPrintVisitor dlv;
public:
	explicit HexPrintVisitor(std::ostream& os) : PrintVisitor(os), raw(os), dlv(os){} // : RawPrintVisitor::RawPrintVisitor(os), DLVPrintVisitor::DLVPrintVisitor(os){}

	// A nicer way to do this is multiple inheritance from RawPrintVisitor and DLVPrintVisitor
	// However, this does not work yet since the two classes must inherit "virtual" from their base class PrintVisitor, i.e.
	// 	class DLVHEX_EXPORT RawPrintVisitor : public virtual PrintVisitor
	// and
	// 	class DLVHEX_EXPORT DLVPrintVisitor : public virtual PrintVisitor
	//
	/*
	virtual void visit(Program* const p){ PrintVisitor::visit(p); }
	virtual void visit(Rule* const r){ RawPrintVisitor::visit(r); }
	virtual void visit(AtomSet* const as){ DLVPrintVisitor::visit(as); }
	virtual void visit(WeakConstraint* const wc){ RawPrintVisitor::visit(wc); }
	virtual void visit(Literal* const l){ PrintVisitor::visit(l); }
	virtual void visit(Atom* const a){ DLVPrintVisitor::visit(a); }
	virtual void visit(BuiltinPredicate* const bp){ PrintVisitor::visit(bp); }
	virtual void visit(AggregateAtom* const aa){ PrintVisitor::visit(aa); }
	virtual void visit(ExternalAtom* const ea){ RawPrintVisitor::visit(ea); }
	*/

	virtual void visit(Program* const p){ PrintVisitor::visit(p); }
	virtual void visit(Rule* const r){ raw.visit(r); }
	virtual void visit(AtomSet* const as){ dlv.visit(as); }
	virtual void visit(WeakConstraint* const wc){ raw.visit(wc); }
	virtual void visit(Literal* const l){ PrintVisitor::visit(l); }
	virtual void visit(Atom* const a){ PrintVisitor::visit(a); }
	virtual void visit(BuiltinPredicate* const bp){ PrintVisitor::visit(bp); }
	virtual void visit(AggregateAtom* const aa){ PrintVisitor::visit(aa); }
	virtual void visit(ExternalAtom* const ea){ raw.visit(ea); }
};

//	This class is an awesome hack.
//
//	====> 2010-03-12 The problem in dlvhex was fixed!
//
//	dlvhex contains classes for calling DLV for subprograms. We need a similar mechanism but for hex subprograms.
//	The DLVresultParserDriver has an internal assertion that tests if DLV's result contains propositional atoms. If dlvhex is started in higher-order mode, this is prohibited since
//	all atoms are passed to DLV in higher-order style by DLVProcess. In this case the assertion is senseful, since DLV will never return propositional facts if the input program
//	does not contain any.
//	However, this is only true for DLVProcess. DLVHexProcess passes the program as it is, even if dlvhex is run in higher-order mode. This is necessary since external atoms
//	can not be converted into normal first-order atoms so easily. Also note that we call a further instance of dlvhex rather than DLV.
//	In summary, we pass a program to dlvhex that possibly contains propositional atoms. However, DLVresultParserDriver contains this assertion. Since all of it's methods are private
//	is was not possible to derive a HexResultParserDriver. Rewriting a new one from the scratch is also a bad idea.
//	Thus we have to make the DLVresultParserDriver believe that we always run in first-order mode. This makes him happy any everything is fine... However we have to restore the original
//	flag afterwards.
/*
template <typename Builder, typename Parser> class HexSolver : public ASPSolver<Builder, Parser>{
public:

	HexSolver(Process& p) : ASPSolver<Builder, Parser>(p) {}

	void solve(const Program& prg, const AtomSet& facts, std::vector<AtomSet>& as) throw (FatalError){
		// Remember the setting of "NoPredicate"
		unsigned noPred = Globals::Instance()->getOption("NoPredicate");
		Globals::Instance()->setOption("NoPredicate", 0);

		// solve the hex program
		ASPSolver<Builder, Parser>::solve(prg, facts, as);

		// Restore the old option
		Globals::Instance()->setOption("NoPredicate", noPred);
	}
};
*/
#include <iostream>
// Just forces the DLVresultParserDriver to parse in first-order mode
class HexResultParserDriver : public DLVresultParserDriver{
public:
	HexResultParserDriver() : DLVresultParserDriver(DLVresultParserDriver::FirstOrder){}
};

class DLVHexProcess : public DLVProcess{
private:
	std::vector<std::string> filename;
	bool fromfile;

public:
	DLVHexProcess() : DLVProcess(), fromfile(false){
	}

	DLVHexProcess(std::vector<std::string> fn) : DLVProcess(), filename(fn), fromfile(true){
	}

	DLVHexProcess(std::vector<std::string> fn, std::vector<std::string> o) : DLVProcess(), filename(fn), fromfile(true){
		argv = o;
	}

	virtual std::string
	path() const
	{
		return std::string("dlvhex");
	}

	virtual std::vector<std::string>
	commandline() const
	{
		std::vector<std::string> tmp;
		tmp.push_back(path());
		// never show state information
		tmp.push_back("--silent");
		tmp.push_back("--firstorder");
		tmp.insert(tmp.end(), argv.begin(), argv.end());
		if (fromfile){
			tmp.insert(tmp.end(), filename.begin(), filename.end());
		}else{
			tmp.push_back("--"); // request stdin as last parameter!
		}
		return tmp;
	}

	virtual void
	spawn()
	{
		setupStreams();
		proc.open(commandline());

		// workaround: dlvhex crashes on empty input
		this->getOutput() << "%" << std::endl;
	}

	virtual BaseASPSolver* createSolver()
	{
		// RawPrintVisitor makes sure that external atoms remain in HEX syntax.
		// DLVPrintVisitor would replace them and thus prevent arbitrarily nested HEX programs.
		return new ASPSolver<HexPrintVisitor, HexResultParserDriver>(*this);
	}

	// THIS METHOD IS NEVER CALLED, but it may is useful for testing
	void solve(std::string program, const Program& prg, const AtomSet& facts, std::vector<AtomSet>& as){

		spawn();
		try{
			// Write program to dlvhex
			getOutput() << program << std::endl;

			/*
			HexPrintVisitor builder(std::cerr);
			 if( !Globals::Instance()->maxint.empty() )
			 	getOutput() << Globals::Instance()->maxint << std::endl;
			 const_cast<Program&>(prg).accept(builder);
			 const_cast<AtomSet&>(facts).accept(builder);
			*/

			// send EOF to process
			endoffile();

			
//			std::cerr << "OUTPUT IS " << std::endl;
//			std::istream& inp = getInput();
//			std::string s;

//			while (inp.good() && std::getline(inp, s))
//			{
//				std::cerr << s << std::endl;
//			}
			

			// parse result
//			Globals::Instance()->setOption("NoPredicate", 0);
			DLVresultParserDriver parser;
			parser.parse(getInput(), as);

//			std::cerr << "IN AS-FORMAT " << std::endl;
			for (std::vector<AtomSet>::iterator it = as.begin(); it != as.end(); it++){
				DLVPrintVisitor pv(std::cerr);
//				pv.PrintVisitor::visit(&(*it));
//std::cerr << it->size() << std::endl;
			}

			// get exit code of process
			int retcode = close();
		}catch (std::ios_base::failure& e){
		}
	}
};

}
}
}

#endif
