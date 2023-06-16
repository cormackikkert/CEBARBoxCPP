#include "LtlFormulaTriple.h"

LtlFormulaTriple::LtlFormulaTriple() {}

LtlFormulaTriple::~LtlFormulaTriple() {}

//const Literal& LtlFormulaTriple::getInitialLiteral() const {
  //return initialLiteral;
//}

const clause_list& LtlFormulaTriple::getClauses() const {
  return clauses;
}

const ltl_clause_list& LtlFormulaTriple::getStepClauses() const {
  return stepClauses;
}

const ltl_clause_list& LtlFormulaTriple::getEventualityClauses() const {
  return eventualityClauses;
}

const literal_set& LtlFormulaTriple::getUnconditionalEventualities() const {
  return unconditionalEventualities;
}
//void LtlFormulaTriple::setInitialLiteral(Literal newInitialLiteral) {
  //initialLiteral = std::move(newInitialLiteral);
//}

void LtlFormulaTriple::setClauses(clause_list newClauses) {
  clauses = std::move(newClauses);
}

void LtlFormulaTriple::setStepClauses(ltl_clause_list newStepClauses) {
  stepClauses = std::move(newStepClauses);
}

void LtlFormulaTriple::setEventualityClauses(ltl_clause_list newEventualityClauses) {
  eventualityClauses = std::move(newEventualityClauses);
}

void LtlFormulaTriple::setUnconditionalEventualities(literal_set newEventualities) {
  unconditionalEventualities = std::move(newEventualities);
}

void LtlFormulaTriple::eraseClauses() {
  clauses.clear();
}

void LtlFormulaTriple::eraseStepClauses() {
  stepClauses.clear();
}

void LtlFormulaTriple::eraseEventualityClauses() {
  eventualityClauses.clear();
}

void LtlFormulaTriple::addClause(literal_set clause) {
  clauses.push_back(clause);
}

void LtlFormulaTriple::addUnconditionalEventuality(Literal literal) {
    unconditionalEventualities.insert(literal);

}
void LtlFormulaTriple::addStepClause(literal_set left, Literal right) {
    stepClauses.push_back(LtlClause(left, right));
}

void LtlFormulaTriple::addEventualityClause(literal_set left, Literal right) {
    eventualityClauses.push_back(LtlClause(left, right));
}

void LtlFormulaTriple::addStepClause(LtlClause clause) {
    stepClauses.push_back(clause);
}

void LtlFormulaTriple::addEventualityClause(LtlClause clause) {
    eventualityClauses.push_back(clause);
}


vector<string> LtlFormulaTriple::toStringComponents() const {
  vector<string> components;
  for (const auto& clause : clauses) {
    string clauseString = "";
    for (const auto& literal : clause) {
      clauseString += literal.toString() + " v ";
    }
    clauseString = clauseString.substr(0, clauseString.size() - 3);
    components.push_back(clauseString);
  }
  for (LtlClause stepClause : stepClauses) {
        string stepClauseString = "";
        for (const auto& literal : stepClause.left) {
          stepClauseString += literal.toString() + " & ";
        }
        stepClauseString = stepClauseString.substr(0, stepClauseString.size() - 3);
        stepClauseString += " -> X " + stepClause.right.toString();
        components.push_back(stepClauseString);
  }

  for (LtlClause eventualityClause : eventualityClauses) {
        string eventualityClauseString = "";
        for (const auto& literal : eventualityClause.left) {
          eventualityClauseString += literal.toString() + " & ";
        }
        eventualityClauseString = eventualityClauseString.substr(0, eventualityClauseString.size() - 3);
        eventualityClauseString += " -> <> " + eventualityClause.right.toString();
        components.push_back(eventualityClauseString);
  }
  return components;
}

string LtlFormulaTriple::toString() const {
    string formulaString = "";
    for (string component : toStringComponents()) {
        formulaString += "[] (" + component + ") \n";
    }
    return formulaString;
}
