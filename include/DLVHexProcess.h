#ifndef __DLVHexProcess_h_
#define __DLVHexProcess_h_

#include <dlvhex/Process.h>
//#include <dlvhex/ASPSolver.h>
#include <dlvhex/DLVProcess.h>
//#include <dlvhex/DLVresultParserDriver.h>
#include <dlvhex/globals.h>
#include <dlvhex/PrintVisitor.h>

#include <stdio.h>

DLVHEX_NAMESPACE_USE

class DLVHexProcess : public DLVProcess{
private:
	std::string filename;
	bool fromfile;

public:
	DLVHexProcess() : DLVProcess(), fromfile(false){
	}

	DLVHexProcess(std::string fn) : DLVProcess(), filename(fn), fromfile(true){
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
		tmp.insert(tmp.end(), argv.begin(), argv.end());
		// never show state information
		tmp.push_back("--silent");
		tmp.push_back(fromfile ? filename : "--"); // request stdin as last parameter!

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
};

#endif
