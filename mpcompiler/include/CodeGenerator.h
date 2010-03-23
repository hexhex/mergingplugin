#ifndef _CODEGENERATOR_h_
#define _CODEGENERATOR_h_

/**
 * \file CodeGenerator.h
 * 	Translates a parse tree into the final output
 */

#include <ParseTreeNode.h>

#include <iostream>

namespace dlvhex{
	namespace merging{
		namespace tools{
			namespace mpcompiler{
				class CodeGenerator{
					private:
						ParseTreeNode *parsetreeroot;
						bool codegenerated;
						int errorcount;
						int warningcount;

						void translateBeliefBase(ParseTreeNode *parsetree, std::ostream &os, std::ostream &err);
						std::string translateRevisionPlan(ParseTreeNode *parsetree, std::ostream &os, std::ostream &err);
						std::string translateRevisionPlan_composed(ParseTreeNode *parsetree, std::ostream &os, std::ostream &err);
						std::string translateRevisionPlan_beliefbase(ParseTreeNode *parsetree, std::ostream &os, std::ostream &err);
						void writeAnswerSetExtraction(ParseTreeNode *parsetree, std::ostream &os, std::ostream &err);
						std::string quote(std::string code);
						std::string unquote(std::string code);

					public:
						CodeGenerator (ParseTreeNode *parsetree);
						int getErrorCount();
						int getWarningCount();
						bool succeeded();
						bool codeGenerated();
						void generateCode(std::ostream &os, std::ostream &err);
				};
			}
		}
	}
}

#endif


/*! \fn dlvhex::merging::tools::rpcompiler::CodeGenerator::CodeGenerator(ParseTreeNode *parsetree)
 *  \brief Initializes the code generator for a certain parse tree.
 *  \param parsetree A pointer to a parsetree created by an IParser instance.
 */

/*! \fn static std::string dlvhex::merging::tools::rpcompiler::CodeGenerator::quote(std::string code)
 *  \brief Quotes dlv progarm code for &hex and &hexfile calls s.t. the character " is avoided.
 *  \param code dlv program code
 *  \return std::string dlv program code without the character ". \ is escaped as \\, " is escaped as \'
 */

/*! \fn static std::string dlvhex::merging::tools::rpcompiler::CodeGenerator::unquote(std::string code)
 *  \brief Unquotes dlv progarm code for &hex and &hexfile calls.
 *  \param code dlv program code
 *  \return std::string unquoted dlv program code
 */

/*! \fn void dlvhex::merging::tools::rpcompiler::CodeGenerator::translateBeliefBase(ParseTreeNode *parsetree, std::ostream &os, std::ostream &os)
 *  \brief Translates the definitions for one belief base.
 *  \param parsetree Pointer to the belief base root node.
 *  \param os An output stream to write the code to
 *  \param err An output stream to write error messages to
 */

/*! \fn std::string dlvhex::merging::tools::rpcompiler::CodeGenerator::translateRevisionPlan(ParseTreeNode *parsetree, std::ostream &os, std::ostream &os)
 *  \brief Translates one hierarchie level of the revision plan
 *  \param parsetree Pointer to the current node in the revision plan.
 *  \param os An output stream to write the code to
 *  \param err An output stream to write error messages to
 *  \return std::string Name of this result or 	intermediate result consisting of the belief base names delimited by underscores
 */

/*! \fn std::string dlvhex::merging::tools::rpcompiler::CodeGenerator::translateRevisionPlan_composed(ParseTreeNode *parsetree, std::ostream &os, std::ostream &os)
 *  \brief Translates one hierarchie level of a composed revision plan (operator application)
 *  \param parsetree Pointer to the root node of a composed revision plan.
 *  \param os An output stream to write the code to
 *  \return std::string Name of this intermediate result consisting of the belief base names delimited by underscores
 *  \param err An output stream to write error messages to
 */

/*! \fn std::string dlvhex::merging::tools::rpcompiler::CodeGenerator::translateRevisionPlan_beliefbase(ParseTreeNode *parsetree, std::ostream &os, std::ostream &os)
 *  \brief Translates an access to a belief base as used in a revision plan
 *  \param parsetree Pointer to the root node of a belief base usage
 *  \param os An output stream to write the code to
 *  \param err An output stream to write error messages to
 *  \return std::string Name of this result or 	intermediate consisting of the belief base name prefixed by "_"
 */

/*! \fn void dlvhex::merging::tools::rpcompiler::CodeGenerator::writeAnswerSetExtraction(ParseTreeNode *parsetree, std::ostream &os, std::ostream &err)
 *  \brief Writes hex code which extracts the answer sets from the sub programs and transfers their content into the real answer sets of this program. This step requires the common signature from the parse tree in order to determine the public predicates.
 *  \param parsetree Pointer to the current node in the revision plan.
 *  \param os An output stream to write the code to
 *  \param err An output stream to write error messages to
 */

/*! \fn void dlvhex::merging::tools::rpcompiler::CodeGenerator::ParseTreeNode *parsetree)
 *  \brief Prepares this instance for code generation for a given parse tree.
 *  \param parsetree Pointer to the root of the parse tree.
 */

/*! \fn int dlvhex::merging::tools::rpcompiler::CodeGenerator::getErrorCount()
 *  \brief Returns the number of errors which occurred during the last code generation (last call of generateCode). If no code has been generated so far, error count will always trivially be 0. Call codeGenerated() to check if code has been generated.
 *  \return int The number of errors which occurred during code generation
 */

/*! \fn int dlvhex::merging::tools::rpcompiler::CodeGenerator::getWarningCount()
 *  \brief Returns the number of warnings which occurred during code generation. If no code has been generated so far, warning count will always trivially be 0. Call codeGenerated() to check if code has been generated.
 *  \return int The number of warnings which occurred during code generation
 */

/*! \fn bool dlvhex::merging::tools::rpcompiler::CodeGenerator::succeeded()
 *  \brief Returns true iff the last code generation (last call of generateCode) finished without errors _and_ code has been generated so far.
 *  \return bool True if the last code generation (last call of generateCode) finished without errors _and_ code has been generated so far, otherwise false.
 */

/*! \fn bool dlvhex::merging::tools::rpcompiler::CodeGenerator::codeGenerated()
 *  \brief Returns true iff code has been generated so far (i.e. at least one call of generateCode() occurred)
 *  \return bool True if code has been generated so far, otherwise false
 */

/*! \fn void dlvhex::merging::tools::rpcompiler::CodeGenerator::generateCode(std::ostream &os, std::ostream &err)
 *  \brief Generates the output code for a given parsetree.
 *  \param parsetree Pointer to the root of the parse tree.
 *  \param os An output stream to write the code to
 *  \param err An output stream to write error messages to
 *  \return int The number of errors which occurred during code generation
 */
