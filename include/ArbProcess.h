#ifndef __ARBPROCESS_h_
#define __ARBPROCESS_h_

#include <dlvhex2/Process.h>
#include <dlvhex2/DLVProcess.h>

#include <stdio.h>

DLVHEX_NAMESPACE_USE

namespace dlvhex{
namespace merging{
namespace plugin{

class ArbProcess : public DLVProcess{
private:
	std::string executionstring;

public:
	ArbProcess(std::string s) : DLVProcess(), executionstring(s){
	}

	virtual std::string
	path() const
	{
		return executionstring;
	}

	virtual std::vector<std::string>
	commandline() const
	{
		std::string es = executionstring + std::string(" ");
		std::vector<std::string> tmp;

		// Extract program name and parameters
		int start = 0;
		bool quotes = false;
		bool scanspaces = false;
		for (int j = 0; j < es.length(); j++){
			if (scanspaces && es[j] == ' '){
				// Skip spaces between arguments
				start = j + 1;
			}else{
				scanspaces = false;
				// In quoted strings, any characters are accepted
				if (es[j] == '\"') quotes = !quotes;
				// Otherwise, blanks separate arguments
				if (!quotes && es[j] == ' '){
					tmp.push_back(es.substr(start, j - start));
					start = j + 1;
					scanspaces = true;
				}

			}
		}

		return tmp;
	}

	virtual void
	spawn()
	{
		setupStreams();
		proc.open(commandline());
	}
};

}
}
}

#endif

