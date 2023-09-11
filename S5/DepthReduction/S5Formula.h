#ifndef S5FORMULA
#define S5FORMULA

#include "../../Formula/And/And.h"
#include "../../Formula/Atom/Atom.h"
#include "../../Formula/Box/Box.h"
#include "../../Formula/Diamond/Diamond.h"
#include "../../Formula/FEnum/FEnum.h"
#include "../../Formula/Formula/Formula.h"
#include "../../Formula/Or/Or.h"
#include "../../Clausifier/FormulaTriple/FormulaTriple.h"

typedef vector<formula_set> CNF_form;

class NewVariableGenerator {
    string prepend;
    int current;
public:
    NewVariableGenerator(const string prepend_);
    shared_ptr<Formula> operator()();
};

CNF_form convert_to_CNF(shared_ptr<Formula> inp_formula, NewVariableGenerator& GetNewVar);

CNF_form DepthReduce(shared_ptr<Formula> inp_formula, NewVariableGenerator& GetNewVar);

std::vector<FormulaTriple> form_triples(CNF_form &cnf_formula);
string cnfToString(CNF_form cnf_formula);

CNF_form DepthReduceBoxFromCNF(CNF_form cnf_subf, NewVariableGenerator& GetNewVar, int modality);
CNF_form DepthReduceBox(shared_ptr<Formula> inp_formula, NewVariableGenerator& GetNewVar);
CNF_form DepthReduceDiamond(shared_ptr<Formula> inp_formula, NewVariableGenerator& GetNewVar);
CNF_form DepthReduceAnd(shared_ptr<Formula> inp_formula, NewVariableGenerator& GetNewVar);
CNF_form DepthReduceOr(shared_ptr<Formula> inp_formula, NewVariableGenerator& GetNewVar);

bool isPropLiteral(shared_ptr<Formula> inp_formula);
bool isS5Literal(shared_ptr<Formula> inp_formula);

#endif