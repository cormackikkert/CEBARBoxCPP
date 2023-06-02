#include "MinisatProver.h"

shared_ptr<Minisat::SimpSolver> MinisatProver::completeSolver = shared_ptr<Minisat::SimpSolver>(new Minisat::SimpSolver());


MinisatProver::MinisatProver(bool onesat) {
  // solver->random_var_freq = 0;
  // solver->rnd_init_act = true;
  // solver->ccmin_mode = 1;
  // solver->clause_decay = 0;
  // solver->var_decay = 0.5;
  // solver->luby_restart = false;
  // solver->

    onesat = false;
  if (onesat) {
        completeSolver->eliminate(true);
        //completeSolver->verbosity=2;
    calcSolver = completeSolver;
    //calcSolver = solver;

  } else {
        solver->eliminate(true);
        //solver->verbosity=2;
      calcSolver = solver;
  }
 // calcSolver->verbosity = 2;
}
MinisatProver::~MinisatProver() {}

modal_names_map MinisatProver::prepareSAT(FormulaTriple clauses,
                                          name_set extra) {
  prepareExtras(extra);

  modal_names_map newExtra;
  prepareModalClauses(clauses.getBoxClauses(), newExtra, boxLits, boxFromRight);
  prepareModalClauses(clauses.getDiamondClauses(), newExtra, diamondLits,
                      diamondFromRight);

  prepareFalse();
  prepareClauses(clauses.getClauses());

  return newExtra;
}

void MinisatProver::prepareFalse() {
    calcSolver->addClause(~Minisat::mkLit(createOrGetVariable("$false")));
}

void MinisatProver::prepareExtras(name_set extra) {
  for (string name : extra) {
    createOrGetVariable(name);//, Minisat::lbool((uint8_t)0));
  }
}

void MinisatProver::prepareClauses(clause_set clauses) {
  for (Clause clause : clauses) {
    if (clause.formula->getType() == FOr) {
      Minisat::vec<Minisat::Lit> literals;
      for (shared_ptr<Formula> subformula :
           dynamic_cast<Or *>(clause.formula.get())->getSubformulas()) {
        literals.push(makeLiteral(subformula));
      }

      calcSolver->addClause(literals);

    } else {

        calcSolver->addClause(makeLiteral(clause.formula));

    }
  }
}

void MinisatProver::prepareModalClauses(modal_clause_set modal_clauses,
                                        modal_names_map &newExtra,
                                        modal_lit_implication &modalLits,
                                        modal_lit_implication &modalFromRight) {
  for (ModalClause clause : modal_clauses) {
    if (clause.left->getType() == FAtom) {
      createOrGetVariable(getPrimitiveName(clause.left),
                          Minisat::lbool((uint8_t)0));
    } else if (clause.left->getType() == FNot) {
      createOrGetVariable(getPrimitiveName(clause.left),
                          Minisat::lbool((uint8_t)1));
    }
    newExtra[clause.modality].insert(getPrimitiveName(clause.right));

    createModalImplication(clause.modality, toLiteral(clause.left),
                           toLiteral(clause.right), modalLits, modalFromRight);
  }
}

Minisat::Var MinisatProver::createOrGetVariable(string name,
                                                Minisat::lbool polarity) {
  if (variableMap.find(name) == variableMap.end()) {
    Minisat::Var newVar =calcSolver->newVar(polarity);  
    variableMap[name] = newVar;
    nameMap[variableMap[name]] = name;
  }
  return variableMap[name];
}

Minisat::Lit MinisatProver::makeLiteral(shared_ptr<Formula> formula) {
  if (formula->getType() == FAtom) {
    string name = dynamic_cast<Atom *>(formula.get())->getName();
    return Minisat::mkLit(createOrGetVariable(name));
  } else if (formula->getType() == FNot) {
    string name = dynamic_cast<Atom *>(
                      dynamic_cast<Not *>(formula.get())->getSubformula().get())
                      ->getName();
    return ~Minisat::mkLit(createOrGetVariable(name));
  }
  throw invalid_argument("Expected Atom or Not but got " + formula->toString());
}

shared_ptr<Minisat::vec<Minisat::Lit>>
MinisatProver::convertAssumptions(literal_set assumptions) {
  shared_ptr<Minisat::vec<Minisat::Lit>> literals =
      shared_ptr<Minisat::vec<Minisat::Lit>>(new Minisat::vec<Minisat::Lit>());

  for (Literal assumption : assumptions) {
    Minisat::Var variable = variableMap[assumption.getName()];
    literals->push(assumption.getPolarity() ? Minisat::mkLit(variable)
                                            : ~Minisat::mkLit(variable));

  } 
  return literals;
}

bool MinisatProver::modelSatisfiesAssump(Literal assumption) {
  if (variableMap.find(assumption.getName()) == variableMap.end()) {
    return false;
  }
  int lbool =
      Minisat::toInt(calcSolver->modelValue(variableMap[assumption.getName()]));
  if (lbool == 2) {
    throw runtime_error("Model value for " + assumption.getName() +
                        " is undefined");
  }
  return (lbool == 0 && assumption.getPolarity()) ||
         (lbool == 1 && !assumption.getPolarity());
}

literal_set MinisatProver::convertConflictToAssumps(
    Minisat::LSet &conflictLits) {
  literal_set conflict;
  for (int i = 0; i < conflictLits.size(); i++) {
    conflict.insert(Literal(nameMap[Minisat::var(conflictLits[i])],
                            Minisat::sign(conflictLits[i])));
  }
  return conflict;
}

Solution MinisatProver::solve(const literal_set &assumptions) {
  Solution solution;
  shared_ptr<Minisat::vec<Minisat::Lit>> vecAssumps =
      convertAssumptions(assumptions);
  solution.satisfiable = calcSolver->solve(*vecAssumps);
  if (!solution.satisfiable) {
    solution.conflict = convertConflictToAssumps(calcSolver->conflict);
  }
  return solution;
}

void MinisatProver::reduce_conflict(literal_set& conflict) {
    literal_set all_lits = conflict;
    for (auto lit_to_remove : all_lits) {
        literal_set new_conflict;
        for (auto x : conflict) if (x != lit_to_remove) new_conflict.insert(x);
        if (new_conflict.size() < conflict.size()) {
            Solution sol = solve(new_conflict);
            if (!sol.satisfiable) {
                cout << "REMOVE SUCCESS\n";
                conflict = sol.conflict;
            }
        }
    }
}

void MinisatProver::addClause(literal_set clause) {
  calcSolver->addClause(*convertAssumptions(clause));
}

void MinisatProver::printModel() {
  for (auto varName : nameMap) {
    cout << varName.second << " is "
         << Minisat::toInt(calcSolver->modelValue(varName.first)) << endl;
  }
}

int MinisatProver::getLiteralId(Literal literal) {
  return variableMap[literal.getName()];
}

literal_set MinisatProver::getModel() {
    literal_set model;
    for (auto varName : nameMap) {
        model.insert(
            Literal(varName.second, 1 - Minisat::toInt(calcSolver->modelValue(varName.first)))
        );
    }
    return model;
                            
}
