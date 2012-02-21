#ifndef __DlvhexSolver_h_
#define __DlvhexSolver_h_

#include <dlvhex/ASPSolverManager.h>

#include "dlvhex2/Process.h"
#include "dlvhex2/ComfortPluginInterface.h"
#include "dlvhex2/DLVresultParserDriver.h"
#include "dlvhex2/PrintVisitor.h"

#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>

using namespace dlvhex;

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
	virtual void visit(const Program* const p){ PrintVisitor::visit(p); }
	virtual void visit(const Rule* const r){ RawPrintVisitor::visit(r); }
	virtual void visit(const AtomSet* const as){ DLVPrintVisitor::visit(as); }
	virtual void visit(const WeakConstraint* const wc){ RawPrintVisitor::visit(wc); }
	virtual void visit(const Literal* const l){ PrintVisitor::visit(l); }
	virtual void visit(const Atom* const a){ DLVPrintVisitor::visit(a); }
	virtual void visit(const BuiltinPredicate* const bp){ PrintVisitor::visit(bp); }
	virtual void visit(const AggregateAtom* const aa){ PrintVisitor::visit(aa); }
	virtual void visit(const ExternalAtom* const ea){ RawPrintVisitor::visit(ea); }
	*/

	virtual void visit(const Program* const p){ PrintVisitor::visit(p); }
	virtual void visit(const Rule* const r){ raw.visit(r); }
	virtual void visit(const AtomSet* const as){ dlv.visit(as); }
	virtual void visit(const WeakConstraint* const wc){ raw.visit(wc); }
	virtual void visit(const Literal* const l){ PrintVisitor::visit(l); }
	virtual void visit(const Atom* const a){ PrintVisitor::visit(a); }
	virtual void visit(const BuiltinPredicate* const bp){ PrintVisitor::visit(bp); }
	virtual void visit(const AggregateAtom* const aa){ PrintVisitor::visit(aa); }
	virtual void visit(const ExternalAtom* const ea){ raw.visit(ea); }
};

// Just forces the DLVresultParserDriver to parse in first-order mode
class HexResultParserDriver : public DLVresultParserDriver{
public:
	HexResultParserDriver() : DLVresultParserDriver(DLVresultParserDriver::FirstOrder){}
};

// specific solver software dlvhex
struct DlvhexSolver : public ASPSolverManager::SoftwareBase {

	struct Options : public ASPSolverManager::GenericOptions {
		Options();
		virtual ~Options();

		// commandline arguments to add
		std::vector<std::string> arguments;
	};

	// inherit base delegate
	struct Delegate : public ASPSolverManager::DelegateInterface {
    typedef DlvhexSolver::Options Options;

		Delegate(const Options& options);
		virtual ~Delegate();

    Options options;
		Process* proc;

		void useASTInput(const Program& idb, const AtomSet& edb);
		void useStringInput(const std::string& program);
		void useFileInput(const std::string& fileName);
		void getOutput(std::vector<AtomSet>& result);
	};
};

}
}
}

#endif
