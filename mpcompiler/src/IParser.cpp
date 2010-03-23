#include <IParser.h>

#include <stdio.h>

using namespace dlvhex::merging::tools::mpcompiler;

// Private methods

std::string IParser::readFile(FILE* file){
	std::string filecontent;
	char buffer[1024];

	while (fgets(buffer, 1024, file) != NULL){
		filecontent = filecontent + std::string(buffer);
	}
	return filecontent;
}

FILE* IParser::getNextInputFile(){
	if (!parsefiles) return NULL;

	// Close current file
	if (currentInputFile != NULL) fclose(currentInputFile);

	// Go to next file
	inputFileCounter++;
	if (inputFileCounter < inputFiles.size()){
			if (inputFiles[inputFileCounter] != std::string("--")){
			currentInputFile = fopen(inputFiles[inputFileCounter].c_str(), "r");
			fseek(currentInputFile, SEEK_SET, 0);
			return currentInputFile;
		}else{
			currentInputFile = stdinFile;
			return currentInputFile;
		}
	}else{
		// No more files
		return NULL;
	}
}

int IParser::getCurrentInputFileIndex(){
	return inputFileCounter;
}

std::string IParser::getInputFileName(int index){
	return inputFiles[inputFileCounter];
}


// Public methods

IParser::IParser(std::string inp) : input(inp), parsefiles(false){
	reset();
}

IParser::IParser(std::vector<std::string> inFiles, FILE *stdin) : inputFiles(inFiles), stdinFile(stdin), parsefiles(true){
	reset();
}

void IParser::reset(){
	errorCount = 0;
	warningCount = 0;
	wasParsed = false;
	parseTree = NULL;
	inputFileCounter = -1;
	currentInputFile = NULL;
}

bool IParser::parsed(){
	return wasParsed;
}

bool IParser::succeeded(){
	return wasParsed && (errorCount == 0);
}

int IParser::getErrorCount(){
	return errorCount;
}

int IParser::getWarningCount(){
	return warningCount;
}

ParseTreeNode* IParser::getParseTree(){
	return parseTree;
}
