#include "Prover.h"
#include <cassert>

Prover::Prover() {}
Prover::~Prover() {}

void Prover::createModalImplication(int modality, Literal left, Literal right,
        modal_lit_implication &modalLits,
        modal_lit_implication &modalFromRight) {
    modalLits[modality][left].insert(right);
    modalFromRight[modality][right].insert(left);
}

void Prover::refineOverApproximation(int modality, const literal_set &model, const literal_set &conflict) {
    cout << "CONFLICT: ";
    for (auto x : conflict) cout << x.toString() << " "; cout << endl;
    //refineBoxOverApproximation(modality, model, conflict);
    refineDiamondOverApproximation(modality, model, conflict);
}

void Prover::refineBoxOverApproximation(int modality, const literal_set &model, const literal_set &conflict) {
    // If an untriggered box clause contributed to a conflict, split it

    assert (boxGrouping[modality].has_value());
    
    unordered_set<int> groupsToSplit;
    unordered_set<int> triggeredGroups;
    for (auto literalAndGroup : boxGrouping[modality].value().groupIdentifier) {
        if (model.find(literalAndGroup.first) == model.end()) {
            for (Literal lit : boxLits[modality][literalAndGroup.first]) {
                if (conflict.find(lit) != conflict.end()) {
                    cout << "UNTRIGGERED CONF: " <<  literalAndGroup.first.toString() << endl;
                    groupsToSplit.insert(literalAndGroup.second);
                    break;
                }
            }
        } else {
            triggeredGroups.insert(literalAndGroup.second);
        }
    }

    for (int groupToSplit : groupsToSplit) if (triggeredGroups.find(groupToSplit) != triggeredGroups.end()) {
        int newGroup = boxGrouping[modality].value().modalGroups++;
        for (auto literalAndGroup : boxGrouping[modality].value().groupIdentifier) {
            if ((literalAndGroup.second == groupToSplit) && 
                    (model.find(literalAndGroup.first) == model.end())) {
                boxGrouping[modality].value().groupIdentifier[literalAndGroup.first] = newGroup;
            }
        }
    }
    cout << "Box Overapproximation size: " << boxGrouping[modality].value().modalGroups << " / " << boxGrouping[modality].value().groupIdentifier.size() << endl;
}
void Prover::refineDiamondOverApproximation(int modality, const literal_set &model, const literal_set &conflict) {
    assert (diamondGrouping[modality].has_value());
    // If two diamonds in the conflict set, split them up
    unordered_set<int> groupsToSplit;
    unordered_set<int> triggeredGroups;
    for (auto literalAndGroup : diamondGrouping[modality].value().groupIdentifier) {
        if (model.find(literalAndGroup.first) != model.end()) triggeredGroups.insert(literalAndGroup.second);
        for (Literal lit : diamondLits[modality][literalAndGroup.first]) {
            if (conflict.find(lit) != conflict.end()) {
                if (groupsToSplit.find(literalAndGroup.second) == groupsToSplit.end()) {
                    groupsToSplit.insert(literalAndGroup.second);
                } else {
                    int newGroup = diamondGrouping[modality].value().modalGroups++;
                    diamondGrouping[modality].value().groupIdentifier[literalAndGroup.first] = newGroup;
                    cout << "Dia Overapproximation size: " << diamondGrouping[modality].value().modalGroups << " / " << diamondGrouping[modality].value().groupIdentifier.size() << endl;
                    return;
                }
            }
        }
    }
    // OR if an unfired diamond is in the conflict set, split it up
    for (auto literalAndGroup : diamondGrouping[modality].value().groupIdentifier) {
        if ((model.find(literalAndGroup.first) == model.end()) && 
                (triggeredGroups.find(literalAndGroup.second) != triggeredGroups.end())) {
            for (auto lit : diamondLits[modality][literalAndGroup.first]) {
                if (conflict.find(lit) != conflict.end()) {
                    int newGroup = diamondGrouping[modality].value().modalGroups++;
                    diamondGrouping[modality].value().groupIdentifier[literalAndGroup.first] = newGroup;
                    cout << "Dia Overapproximation size: " << diamondGrouping[modality].value().modalGroups << " / " << diamondGrouping[modality].value().groupIdentifier.size() << endl;
                    return;

                }
            }
        }
    }

}

void Prover::createBoxOverApproximation() {
    for (auto implications : boxLits) {
        literal_set allLiterals;
        for (auto lit_implication : implications.second) {
            allLiterals.insert(lit_implication.first);
        }
        boxGrouping[implications.first] = ModalOverApproximation(allLiterals);
    }
}

void Prover::createDiamondOverApproximation() {
    for (auto implications : diamondLits) {
        literal_set allLiterals;
        for (auto lit_implication : implications.second) {
            allLiterals.insert(lit_implication.first);
        }
        diamondGrouping[implications.first] = ModalOverApproximation(allLiterals);
    }
}

string Prover::getPrimitiveName(shared_ptr<Formula> formula) {
    if (formula->getType() == FAtom) {
        return dynamic_cast<Atom *>(formula.get())->getName();
    } else if (formula->getType() == FNot) {
        return dynamic_cast<Atom *>(
                dynamic_cast<Not *>(formula.get())->getSubformula().get())
            ->getName();
    }
    throw invalid_argument("Expected Atom or Not but got " + formula->toString());
}

Literal Prover::toLiteral(shared_ptr<Formula> formula) {
    if (formula->getType() == FAtom) {
        return Literal(dynamic_cast<Atom *>(formula.get())->getName(), true);
    } else if (formula->getType() == FNot) {
        return Literal(
                dynamic_cast<Atom *>(
                    dynamic_cast<Not *>(formula.get())->getSubformula().get())
                ->getName(),
                false);
    }
    throw invalid_argument("Expected Atom or Not but got " + formula->toString());
}

void Prover::calculateTriggeredModalClauses(modal_lit_implication &modalLits,
        modal_literal_map &triggered) {
    triggered.clear();
    for (auto modalityLitImplication : modalLits) {
        for (auto literalImplication : modalityLitImplication.second) {
            if (modelSatisfiesAssump(literalImplication.first)) {
                triggered[modalityLitImplication.first].insert(
                        literalImplication.second.begin(), literalImplication.second.end());
            }
        }
    }
}

void Prover::calculateTriggeredModalClauses(modal_lit_implication &modalLits,
        modal_literal_map &triggered, modal_grouping &overApproximation) {

    triggered.clear();

    for (auto modalityLitImplication : modalLits) {
        // Triggered groups are unique for each modality
        unordered_set<int> triggeredGroups; 
        auto modalityOverApproximation = overApproximation[modalityLitImplication.first];
        literal_set pretendTriggeredLiterals;

        for (auto literalImplication : modalityLitImplication.second) {
            if (modelSatisfiesAssump(literalImplication.first)) {
                if (modalityOverApproximation.has_value()) {
                    triggeredGroups.insert(overApproximation[modalityLitImplication.first]->groupIdentifier[literalImplication.first]);
                } else {
                    //pretendTriggeredLiterals.insert(literalImplication.first);
                    triggered[modalityLitImplication.first].insert(literalImplication.second.begin(), literalImplication.second.end());
                }
            }
        }

        if (modalityOverApproximation.has_value()) {
            for (auto literalAndGroup : modalityOverApproximation.value().groupIdentifier) {
                if (triggeredGroups.find(literalAndGroup.second) != triggeredGroups.end()) {
                    triggered[modalityLitImplication.first].insert(
                        modalityLitImplication.second[literalAndGroup.first].begin(),
                        modalityLitImplication.second[literalAndGroup.first].end()
                    );

                }
            }
        }
    }
}

bool Prover::modelSatisfiesAssumps(literal_set assumptions) {
    for (Literal assump : assumptions) {
        if (!modelSatisfiesAssump(assump)) {
            return false;
        }
    }
    return true;
}
modal_literal_map
Prover::getTriggeredModalClauses(modal_lit_implication &modalLits) {
    modal_literal_map triggered;
    for (auto modalityLitImplication : modalLits) {
        for (auto literalImplication : modalityLitImplication.second) {
            if (modelSatisfiesAssump(literalImplication.first)) {
                triggered[modalityLitImplication.first].insert(
                        literalImplication.second.begin(), literalImplication.second.end());
            }
        }
    }
    return triggered;
}

void Prover::calculateTriggeredBoxClauses() {
    calculateTriggeredModalClauses(boxLits, triggeredBoxes);//, boxGrouping);
}

void Prover::calculateTriggeredDiamondsClauses() {
    calculateTriggeredModalClauses(diamondLits, triggeredDiamonds);
}

modal_literal_map Prover::getTriggeredBoxClauses() { return triggeredBoxes; }
modal_literal_map Prover::getTriggeredDiamondClauses() {
    return triggeredDiamonds;
}
literal_set Prover::getNotAllDiamondLeft(int modality) {
    literal_set notAllDiamondLeft;
    for (auto literalImplication : diamondLits[modality]) {
        notAllDiamondLeft.insert(~literalImplication.first);
    }
    return notAllDiamondLeft;
}


literal_set Prover::rememberTriggeredImplications() {
    // Required for Tense where we might reuse SAT solvers inbetween calculating the conflict
    literal_set triggeredImplications;
    for (auto x : boxLits) {
        for (auto literalImplication : x.second) {
            if (modelSatisfiesAssump(literalImplication.first)) {
                triggeredImplications.insert(literalImplication.first);
            }
        }
    }
    for (auto x : diamondLits) {
        for (auto literalImplication : x.second) {
            if (modelSatisfiesAssump(literalImplication.first)) {
                triggeredImplications.insert(literalImplication.first);
            }
        }
    }
    return triggeredImplications;
}

vector<literal_set> Prover::getNotProblemBoxClauses(int modality,
        literal_set conflicts, literal_set& triggeredImplications) {
    vector<literal_set> notProblemBoxClauses;
    for (Literal conflict : conflicts) {
        literal_set problemLeft;
        for (auto literalImplication : boxLits[modality]) {
            if ((triggeredImplications.find(literalImplication.first) != triggeredImplications.end()) &&
                    (literalImplication.second.find(conflict) !=
                     literalImplication.second.end())) {
                problemLeft.insert(~literalImplication.first);
            }
        }
        if (problemLeft.size() > 0) {
            notProblemBoxClauses.push_back(problemLeft);
        }
    }
    return notProblemBoxClauses;
};
vector<literal_set> Prover::getNotProblemBoxClauses(int modality,
        literal_set conflicts) {
    vector<literal_set> notProblemBoxClauses;
    for (Literal conflict : conflicts) {
        literal_set problemLeft;
        for (auto literalImplication : boxLits[modality]) {
            if (modelSatisfiesAssump(literalImplication.first) &&
                    literalImplication.second.find(conflict) !=
                    literalImplication.second.end()) {
                problemLeft.insert(~literalImplication.first);
            }
        }
        if (problemLeft.size() > 0) {
            notProblemBoxClauses.push_back(problemLeft);
        }
    }
    return notProblemBoxClauses;
};

literal_set Prover::getNotDiamondLeft(int modality, Literal diamond, literal_set& triggeredImplications) {
    literal_set notDiamondLeft;
    for (auto literalImplication : diamondLits[modality]) {
        if ((triggeredImplications.find(literalImplication.first) != triggeredImplications.end()) &&
                (literalImplication.second.find(diamond) !=
                 literalImplication.second.end())) {
            notDiamondLeft.insert(~literalImplication.first);
        }
    }
    return notDiamondLeft;
}
literal_set Prover::getNotDiamondLeft(int modality, Literal diamond) {
    literal_set notDiamondLeft;
    for (auto literalImplication : diamondLits[modality]) {
        if (modelSatisfiesAssump(literalImplication.first) &&
                literalImplication.second.find(diamond) !=
                literalImplication.second.end()) {
            notDiamondLeft.insert(~literalImplication.first);
        }
    }
    return notDiamondLeft;
}

void Prover::updateLastFail(Literal clause) { lastFail[clause] = ++failCount; }

diamond_queue Prover::getPrioritisedTriggeredDiamonds(int modality) {
    // Note MUST avoid box clauses
    diamond_queue prioritisedTriggeredDiamonds;
    literal_set triggeredBoxes = getTriggeredBoxClauses()[modality];
    literal_set triggeredDiamonds = getTriggeredDiamondClauses()[modality];
    for (Literal diamond : triggeredDiamonds) {
        if (triggeredBoxes.find(diamond) == triggeredBoxes.end()) {
            prioritisedTriggeredDiamonds.push({diamond, lastFail[diamond]});
        }
    }
    return prioritisedTriggeredDiamonds;
}


diamond_queue Prover::getPrioritisedTriggeredDiamonds(int modality, literal_set& triggeredBoxes, literal_set& triggeredDiamonds) {
    // Note MUST avoid box clauses
    diamond_queue prioritisedTriggeredDiamonds;
    for (Literal diamond : triggeredDiamonds) {
        if (triggeredBoxes.find(diamond) == triggeredBoxes.end()) {
            prioritisedTriggeredDiamonds.push({diamond, lastFail[diamond]});
        }
    }
    return prioritisedTriggeredDiamonds;
}

diamond_queue Prover::getPrioritisedTriggeredDiamondsOverApproximation(int modality, literal_set& triggeredBoxes, literal_set& triggeredDiamonds) {

    unordered_set<int> triggeredGroups;

    // Note MUST avoid box clauses
    diamond_queue prioritisedTriggeredDiamonds;
    return prioritisedTriggeredDiamonds;
}

literal_set Prover::getDiamondOverApproximation(int modality, Literal diamondRepresentative, literal_set model) {
    literal_set ans;
    int triggeredGroup = -1;
    for (auto x : diamondLits[modality]) {
        if (x.second.find(diamondRepresentative) != x.second.end()) {
            triggeredGroup = diamondGrouping[modality].value().groupIdentifier[x.first];
        }
    }

    for (auto x : diamondLits[modality]) {
        if (model.find(x.first) == model.end()) continue;
        if (triggeredGroup == diamondGrouping[modality].value().groupIdentifier[x.first]) {
            ans.insert(x.second.begin(), x.second.end());
        }
    }
    return ans;
}

vector<literal_set> Prover::createConflictGroups(int modality, literal_set nextModalContextConflict) {
    vector<literal_set> conflictGroups;
    for (auto lit : nextModalContextConflict) {
        literal_set currentGroup;
        if (boxFromRight[modality].find(lit) != boxFromRight[modality].end()) {
            currentGroup.insert(boxFromRight[modality][lit].begin(), boxFromRight[modality][lit].end());

        }

        if (diamondFromRight[modality].find(lit) != diamondFromRight[modality].end()) {
            currentGroup.insert(diamondFromRight[modality][lit].begin(), diamondFromRight[modality][lit].end());

        }
        if (currentGroup.size() == 0) {
            return {};
        } else {
            conflictGroups.push_back(currentGroup);
        }
    }
    return conflictGroups;
}

vector<literal_set>
Prover::generateClauses(vector<literal_set> literalCombinations) {
    vector<literal_set> clauses;
    if (literalCombinations.size() == 0) {
        return clauses;
    }
    literal_set levelLiterals = literalCombinations.back();

    if (literalCombinations.size() == 1) {
        for (Literal literal : levelLiterals) {
            literal_set single;
            single.insert(literal);
            clauses.push_back(single);
        }
        return clauses;
    }

    literalCombinations.pop_back();
    vector<literal_set> futureClauses = generateClauses(literalCombinations);
    for (Literal literal : levelLiterals) {
        for (literal_set futureClause : futureClauses) {
            literal_set copy = literal_set(futureClause);
            copy.insert(literal);
            clauses.push_back(copy);
        }
    }
    return clauses;
}

bool contains(literal_set &superset, literal_set &subset) {
    for (auto lit : subset) {
        if (superset.find(lit) == superset.end()) return false;
    }
    return true;
}

vector<literal_set> Prover::checkClauses(int modality, vector<literal_set> clauses, literal_set badClause) {
    // return clauses which only have one diamond
    // or if they have none, duplicate that clause with all diamonds
    vector<literal_set> validClauses;
    for (auto clause : clauses) {
        literal_set boxLiterals;
        literal_set diamondLiterals;
        for (auto literal : clause) {
            for (Literal boxLit : boxLits[modality][literal]) boxLiterals.insert(boxLit);
            for (Literal diamondLit : diamondLits[modality][literal]) diamondLiterals.insert(diamondLit);
        }
        //cout << "Box lits: ";
        //for (auto x: boxLiterals) cout << x.toString() << " "; cout << endl;
        //for (auto x: diamondLiterals) cout << x.toString() << " "; cout << endl;
        for (auto diamondLiteral : diamondLiterals) {
            if (boxLiterals.find(diamondLiteral) == boxLiterals.end()) {
                boxLiterals.insert(diamondLiteral);
                if (contains(boxLiterals, badClause)) {
                    validClauses.push_back(clause);
                    boxLiterals.erase(boxLiterals.find(diamondLiteral));
                    break;
                }
                boxLiterals.erase(boxLiterals.find(diamondLiteral));
            }
        }
        if (contains(boxLiterals, badClause)) {
            vector<literal_set> newCombination;
            for (Literal literal : clause) {
                literal_set single;
                single.insert(literal);
                newCombination.push_back(single);
            }

            //cout << "Creating ALL BAD BOX" << endl;
            literal_set diamonds;
            for (auto diamond : diamondLits[modality]) {
                if (diamond.second.empty()) continue;
                diamonds.insert(diamond.first);
                //cout << diamond.first.toString() << " ";
            }
            //cout << endl;
            newCombination.push_back(diamonds);
            if (!diamonds.empty()) for (auto clause : generateClauses(newCombination)) validClauses.push_back(clause);

        }
    }
    return validClauses;
}

vector<literal_set> Prover::negatedClauses(vector<literal_set> clauses) {
    vector<literal_set> result;
    for (literal_set clause : clauses) {
        literal_set negatedClause;
        for (Literal lit : clause) {
            negatedClause.insert(~lit);
        }
        result.push_back(negatedClause);
    }
    return result;
}

vector<literal_set> Prover::getClauses(int modality, literal_set conflict) {
    return negatedClauses(checkClauses(modality, generateClauses(createConflictGroups(modality, conflict)), conflict));
}
vector<literal_set> Prover::getClauses(int modality, vector<literal_set> conflicts) {
    vector<literal_set> result;
    for (auto conflict : conflicts) {
        auto newConflicts = getClauses(modality, conflict);
        result.insert(result.end(), newConflicts.begin(), newConflicts.end());
    }
    return result;
}

