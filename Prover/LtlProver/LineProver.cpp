#include "LineProver.h"

LineProver::LineProver(LtlFormulaTriple formulaTriple, Literal initialLiteral)
    : formulaTriple(formulaTriple), initialLiteral(initialLiteral) {}

LineProver::~LineProver() {}

bool LineProver::isSat() {
    while (pq.size()) pq.pop();
    ;
    for (int k = 0;; ++k) {
        conflictStore.clear();
        cout << "LEVEL: " << k << endl;
        pq.push({{literal_set{initialLiteral}, {}}, k});
        while (!pq.empty()) {
            auto state = pq.top();
            pq.pop();

            // get the last state
            // auto state = states.back();
            // states.pop_back();
            cout << "TESTING OBLIGATION: " << state.toString() << endl;
            StateResult result = solveState(state, k);

            // cout << endl;
            if (result.success) {
                cout << "SUCCESS" << endl;
                if (state.distance == 0) {
                    return true;
                }
                pq.push({state.assumptions, state.distance});
                pq.push({result.successor, state.distance - 1});
            } else {
                if (result.conflict.size() == 0) {
                    cout << "UNSAT FROM EMPTY CLAUSE" << endl;
                    return false;
                }
                addReason(result.conflict, state.distance);
                // obligation rescheduling
                if (state.distance + 1 < k)
                    pq.push({state.assumptions, state.distance + 1});
            }
        }
        for (int i = 2; i <= k + 1; ++i) {
            // Clause push from reasons[i-1] => reason[i]
            for (vector<int> reason : reasons[i - 1].get_all()) {
                cout << "BEFORE REASON: " << reason.size() << endl;
                literal_set conflict = indicesToLiteralSet(reason);
                Solution sol = getSolver(i)->solve(conflict);
                if (!sol.satisfiable) {
                    cout << "REASON: " << reason.size() << endl;
                    assert (reasons[i-1].contains_remove(reason));
                    if (i == k+1) {
                        cout << "ADDING GLOBAL REASON" << endl;
                        addGlobalReason(conflict);
                    } else {
                        reasons[i].insert(reason);
                    }
                }
                solvers[i]->addClause(indicesToLiteralSet(reason));
            }
            // Check if two layers are equal
            // check if reason is the same as previous
            // reasons[i-1] is empty => reasons[i] = reasons[i-1]
            // reasons[0] isn't initialised, so start with i=2
            if (reasons[i - 1].size() == 0) {
                cout << "UNSAT BY INVARIANT" << endl;
                return false;
            }
        }
    }
}

void LineProver::addGlobalReason(const literal_set& reason) {
    formulaTriple.addClause(reason);
    for (int i = 1; i < solvers.size(); ++i) {
        solvers[i]->addClause(reason);
    }
}

void LineProver::removeObligationsMatchingConflict(const literal_set& conflict,
                                                   int distance) {
    priority_queue<Obligation, vector<Obligation>, ObligationComparator>
        updatedPq;

    while (!pq.empty()) {
        Obligation obligation = pq.top();
        pq.pop();

        literal_set combined = obligation.assumptions.assumptions;
        combined.insert(obligation.assumptions.eventualities.begin(),
                        obligation.assumptions.eventualities.end());

        bool isSuperset = true;
        for (auto literal : conflict) {
            if (combined.find(literal) == combined.end()) {
                isSuperset = false;
                break;
            }
        }

        if (!isSuperset || (obligation.distance > distance)) {
            updatedPq.push(obligation);
        }
    }

    pq = updatedPq;  // Update pq with the filtered obligations
}
/*
void LineProver::removeObligationsMatchingConflict(const literal_set& conflict)
{ std::vector<Obligation>::iterator it = states.begin(); while (it !=
states.end()) { literal_set combined = it->assumptions.assumptions;
       combined.insert(it->assumptions.eventualities.begin(),
it->assumptions.eventualities.end());

       bool isSuperset = true;
       for (auto literal : conflict) {
           if (combined.find(literal) == combined.end()) {
               isSuperset = false;
               break;
           }
       }

       if (isSuperset) {
           it = states.erase(it);
       } else {
           ++it;
       }
   }
}
*/

literal_set LineProver::indicesToLiteralSet(const vector<int>& indices) {
    literal_set result;
    for (auto index : indices) {
        result.insert(indexToLiteral.at(index));
    }
    return result;
}
vector<int> LineProver::literalSetToIndices(const literal_set& literals) {
    vector<int> result;
    for (auto literal : literals) {
        if (literalNameToIndex.find(literal.getName()) ==
            literalNameToIndex.end()) {
            literalNameToIndex[literal.getName()] =
                literalNameToIndex.size() + 1;
            indexToLiteral.emplace(literalNameToIndex.size(), 
                Literal(literal.getName(), true));
            indexToLiteral.emplace(-literalNameToIndex.size(),
                Literal(literal.getName(), false));
        }
        int idx = literalNameToIndex[literal.getName()];
        result.push_back(literal.getPolarity() ? idx : -idx);
    }
    // sort by absolute value
    sort(result.begin(), result.end(),
         [](int a, int b) { return abs(a) < abs(b); });
    return result;
}

void LineProver::addReason(const literal_set& conflict, int distance) {
    vector<int> reasonRepresentation = literalSetToIndices(conflict);
    // Check if adding a stronger reason
    // Needed to preserve monotonicity
    for (int i = 0; i <= distance; ++i) {
        if (reasons[i].contains_remove(reasonRepresentation)) break;
    }

    // Otherwise add the clause normally
    reasons[distance].insert(literalSetToIndices(conflict));
    for (literal_set learntClause : solvers[0]->createLtlReasons(conflict)) {
        cout << "LEARN REASON: " << litsetString(learntClause) << endl;
        for (int i = 1; i <= distance + 1; ++i) {
            solvers[i]->addClause(learntClause);
        }
    }
}

StateResult LineProver::solveState(Obligation& obligation, int k) {
    shared_ptr<Prover> solver = getSolver(obligation.distance);

    literal_set assumptions = obligation.assumptions.assumptions;
    assumptions.insert(obligation.assumptions.eventualities.begin(),
                       obligation.assumptions.eventualities.end());

    Solution solution = solver->solve(assumptions);
    if (!solution.satisfiable) {
        // cout << "UNSAT: " << litsetString(solution.conflict) << endl;
        // solver->reduce_conflict(solution.conflict);

        // Clause pushing

        /*
        int newDistance = obligation.distance;
        while (newDistance == k) {
            solver = getSolver(newDistance + 1);
            Solution newSolution = solver->solve(solution.conflict);
            if (newSolution.satisfiable) {
                break;
            }
            newDistance++;
            // obligation.distance;
        }
        if (true) {
            // This is a global conflict, bit hacky
            obligation.distance = newDistance;
        }
        */
        return {false, {}, solution.conflict};
    } else {
        // Create successor state
        /*
        auto model = solver->getModel();
        cout << "MODEL: " << endl;
        for (auto literal : model) {
            cout << literal.toString() << " ";
        }
        cout << endl;
        */
        // cout << "MODEL: " << litsetString(solver->getModel()) << endl;
        auto successorAssumps = solver->getLtlSuccessorAssumps(
            obligation.assumptions.eventualities);
        return {true, {successorAssumps.first, successorAssumps.second}, {}};
    }
}

void LineProver::addReason(int distance, literal_set reason) {
    shared_ptr<Prover> solver = getSolver(distance);
    solver->addClause(reason);
}

shared_ptr<Prover> LineProver::getSolver(int distance) {
    if (distance >= (uint8_t)solvers.size()) {
        // cout << "SPAWNING NEW: " << distance << endl;
        solvers.push_back(shared_ptr<Prover>(new MinisatProver(false)));
        solvers[distance]->prepareLtlfSat(formulaTriple, initialLiteral,
                                          succInSat);
        if (distance == 0) {
            // Must have all eventualities fulfilled
            solvers[distance]->makeLtlTail();
        }
    }
    return solvers[distance];
}
