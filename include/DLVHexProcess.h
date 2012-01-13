#ifndef __DLVHexProcess_h_
#define __DLVHexProcess_h_

#include <dlvhex/Process.h>
#include <dlvhex/DLVProcess.h>

#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
namespace merging{
namespace plugin{

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
		tmp.insert(tmp.end(), argv.begin(), argv.end());
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

}
}
}

#endif
