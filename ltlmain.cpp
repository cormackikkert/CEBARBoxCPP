#include <iostream>
#include <fstream>
#include "Clausifier/LtlFormulaTriple/LtlFormulaTriple.h"

#include "Prover/LtlProver/LineProver.h"
#include "Parser/ltlsnfLexer.h"
#include "Parser/ltlsnfParser.h"
#include "Parser/ltlsnfBaseListener.h"
#include "Prover/Literal/Literal.h"
#include <antlr4-runtime/antlr4-runtime.h>

using namespace std;

class LtlListener : public ltlsnfBaseListener {
private:
  shared_ptr<Literal> initialLiteral;
  LtlFormulaTriple formulaTriple;
  bool insideTemporalClause;
  int counter = 0;

public:
  shared_ptr<Literal> getInitialLiteral() {
    return initialLiteral;
  }

  Literal createLiteral(ltlsnfParser::LiteralContext *ctx) {
    std::string identifier = ctx->IDENTIFIER()->getText();
    bool negated = ctx->NOT() != nullptr;
    return Literal(identifier, !negated);
  }

  void enterTemporalClause(ltlsnfParser::TemporalClauseContext *ctx) override {
    insideTemporalClause = true;
  }

  void exitTemporalClause(ltlsnfParser::TemporalClauseContext *ctx) override {
    insideTemporalClause = false;
  }

  void enterPropositionalClause(ltlsnfParser::PropositionalClauseContext *ctx) override {
    auto clauseList = ctx->literalList()->literal();
    if (!insideTemporalClause) {
        assert (clauseList.size() == 1);
        initialLiteral = make_shared<Literal>(createLiteral(clauseList[0]));
        return;
    }
    
    literal_set literals;
    for (auto clause : clauseList) {
        literals.insert(createLiteral(clause));
    }
    formulaTriple.addClause(literals);
  }

  void enterStepClause(ltlsnfParser::StepClauseContext *ctx) override {
    auto literalList = ctx->literalList()->literal();
    literal_set left;
    for (auto literal : literalList) {
        left.insert(~createLiteral(literal));
    }

    auto nextLiteralList = ctx->nextLiteralList()->literal();
    literal_set right;
    for (auto literal : nextLiteralList) {
        right.insert(createLiteral(literal));
    }
    if (right.size() == 1) {
        formulaTriple.addStepClause(left, *right.begin());
    } else {
        string newX = "$x" + to_string(++counter);
        Literal newRight = Literal(newX, true);
        formulaTriple.addStepClause(left, newRight);
        right.insert(~newRight);
        formulaTriple.addClause(right);

    }
    
  }

  void enterSometimeClause(ltlsnfParser::SometimeClauseContext *ctx) override {
    auto literalList = ctx->literalList()->literal();
    literal_set left;
    for (auto literal : literalList) {
        left.insert(~createLiteral(literal));
    }
    Literal right = createLiteral(ctx->literal());
    formulaTriple.addEventualityClause(left, right);
  }

  LtlFormulaTriple getFormulaTriple() const {
    return formulaTriple;
  }
};

int main(int argc, char **argv) {
  // Read input formula from file
  if (argc != 3 || std::string(argv[1]) != "-f") {
    std::cerr << "Please provide a filename using the -f option." << std::endl;
    return 1;
  }

  const std::string filename = argv[2];

  // Get the path to the executable
  std::string programPath = argv[0];
  std::string::size_type lastSlashPos = programPath.find_last_of('/');
  std::string executablePath = programPath.substr(0, lastSlashPos + 1);

  // Construct the full path to ltl2snf executable
  std::string ltl2snfPath = executablePath + "ltl2snf";

  // Read input formula from file
  std::string command =  ltl2snfPath + " -aprenex -prenex -reuse_renaming -simp -ple -i " + filename + " -o simplified.out";
  int exitCode = std::system(command.c_str());
  if (exitCode != 0) {
    std::cerr << "Failed to execute ltl2snf program." << std::endl;
    return 1;
  }

  std::ifstream inputFile("simplified.out");
  if (!inputFile.is_open()) {
    std::cerr << "Failed to open input file." << std::endl;
    return 1;
  }

std::string inputFormula;
std::string line;
while (std::getline(inputFile, line)) {
  inputFormula += line; // Append the line to the formula string
}
inputFile.close();

  // Create ANTLR input stream
  antlr4::ANTLRInputStream input(inputFormula);

  // Create lexer
  ltlsnfLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);

  // Create parser
  ltlsnfParser parser(&tokens);

  // Parse the formula using MyListener
  LtlListener listener;
  antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, parser.formula());
  cout << listener.getFormulaTriple().toString() << endl;

  // Get the parsed formula triple
  /*
  LtlFormulaTriple formulaTriple = listener.getFormulaTriple();

  // Access and manipulate the data structure as needed
  const clause_set& clauses = formulaTriple.getClauses();
  const ltl_clause_set& stepClauses = formulaTriple.getStepClauses();
  const ltl_clause_set& eventualityClauses = formulaTriple.getEventualityClauses();
  */


  LineProver lineProver (listener.getFormulaTriple(), *listener.getInitialLiteral());
  if (lineProver.isSat()) {
      cout << "SAT" << endl;
  } else {
      cout << "UNSAT" << endl;
  }
  return 0;
}
