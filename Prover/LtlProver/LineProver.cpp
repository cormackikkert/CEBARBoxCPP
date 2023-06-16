#include "LineProver.h"

LineProver::LineProver(LtlFormulaTriple formulaTriple, Literal initialLiteral)
    : formulaTriple(formulaTriple), initialLiteral(initialLiteral) {}

    LineProver::~LineProver() {}

    bool LineProver::isSat() {
        while (pq.size()) pq.pop();;
        for (int k = 0; ; ++k) {
            conflictStore.clear();
            cout << "LEVEL: " << k << endl;
            pq.push({{literal_set{initialLiteral}, {}}, k});
            while (!pq.empty()) {
                auto state = pq.top();
                pq.pop();

                // get the last state
                //auto state = states.back();
                //states.pop_back();
                //cout << "TESTING OBLIGATION: " << state.toString() << endl;
                StateResult result = solveState(state, k);


                //cout << endl;
                if (result.success) {
                    //cout << "SUCCESS" << endl;
                    if (state.distance == 0) {
                        return true;
                    }
                    pq.push({state.assumptions, state.distance});
                    pq.push({result.successor, state.distance-1});
                } else {
                    string conflict = litsetString(result.conflict);
                    // FOR SOME REASON THIS IF STATEMENT AFFECTS THE RUNNING TIME OF TRP
                    /*
                    if (false && (conflictStore.find(conflict) != conflictStore.end()) &&
                            (conflictStore[conflict] >= state.distance)) {
                    cout << "SEEN: " << conflictStore[conflict] << " " << state.distance << endl;
                    */

                conflictStore[conflict] = state.distance;
                removeObligationsMatchingConflict(result.conflict, state.distance);
                for (literal_set reason : solvers[0]->createLtlReasons(result.conflict)) {
                    removeObligationsMatchingConflict(reason, state.distance+1);

                    // Empty reason -> unsat
                    if (reason.size() == 0) {
                        return false;
                    }
                    
                    if (state.distance > k) {
                        // this is a global reason
                        //addReason(reason, state.distance);
                        cout << "GLOBAL REASONS" << endl;
                        addGlobalReason(reason);
                    } else {
                        // This is a normal reason
                        addReason(reason, state.distance+1);
                    }
                }
                // obligation rescheduling
                if (state.distance + 1 < k)
                    pq.push({state.assumptions, state.distance+1});
                
            }
        }
        for (int i = 2; i <= k+1; ++i) {

            // Check if two layers are equal
            // check if reason is the same as previous
            // reasons[i-1] is empty => reasons[i] = reasons[i-1]
            // reasons[0] isn't initialised, so start with i=2
            if (reasons[i-1].size() == 0) {
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

void LineProver::removeObligationsMatchingConflict(const literal_set& conflict, int distance) {
    priority_queue<Obligation, vector<Obligation>, ObligationComparator> updatedPq;

    while (!pq.empty()) {
        Obligation obligation = pq.top();
        pq.pop();

        literal_set combined = obligation.assumptions.assumptions;
        combined.insert(obligation.assumptions.eventualities.begin(), obligation.assumptions.eventualities.end());

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

    pq = updatedPq; // Update pq with the filtered obligations
}
 /*
 void LineProver::removeObligationsMatchingConflict(const literal_set& conflict) {
    std::vector<Obligation>::iterator it = states.begin();
    while (it != states.end()) {
        literal_set combined = it->assumptions.assumptions;
        combined.insert(it->assumptions.eventualities.begin(), it->assumptions.eventualities.end());

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
        if (literalNameToIndex.find(literal.getName()) == literalNameToIndex.end()) {
            literalNameToIndex[literal.getName()] = literalNameToIndex.size() + 1;
            /*indexToLiteral[literalNameToIndex.size()] = 
                Literal(literal.getName(), true);
            indexToLiteral[-literalNameToIndex.size()] = 
                Literal(literal.getName(), false);
                */
        }
        int idx = literalNameToIndex[literal.getName()];
        result.push_back(literal.getPolarity() ? idx : -idx);
    }
    // sort by absolute value
    sort(result.begin(), result.end(), [](int a, int b) { return abs(a) < abs(b); });
    return result;
}

void LineProver::addReason(const literal_set& reason, int distance) {
    //cout << "ADDING REASON: ";
    //for (auto literal : reason) {
        //cout << literal.toString() << " ";
    //}
    //cout << endl;
    vector<int> reasonRepresentation = literalSetToIndices(reason);
    // Check if adding a stronger reason
    // Needed to preserve monotonicity
    for (int i = 0; i <= distance; ++i) {
        if (reasons[i].contains_remove(reasonRepresentation)) {
            reasons[distance].insert(reasonRepresentation);
            solvers[distance]->addClause(reason);
            return;
            
        }
    }

    // Otherwise add the clause normally
    reasons[distance].insert(reasonRepresentation);
    solvers[distance]->addClause(reason);
}

StateResult LineProver::solveState(Obligation &obligation, int k) {
    shared_ptr<Prover> solver = getSolver(obligation.distance); 

    literal_set assumptions = obligation.assumptions.assumptions;
    assumptions.insert(obligation.assumptions.eventualities.begin(), obligation.assumptions.eventualities.end());

    Solution solution = solver->solve(assumptions);
    if (!solution.satisfiable) {
        //cout << "UNSAT: " << litsetString(solution.conflict) << endl;
        //solver->reduce_conflict(solution.conflict);

        // Clause pushing
        
        int newDistance = obligation.distance;
        while (newDistance == k) {
            solver = getSolver(newDistance+1);
            Solution newSolution = solver->solve(solution.conflict);
            if (newSolution.satisfiable) {
                break;
            }
            newDistance++;
            //obligation.distance;
        }
        if (true) {
            // This is a global conflict, bit hacky
            obligation.distance = newDistance;
        }
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
        //cout << "MODEL: " << litsetString(solver->getModel()) << endl;
        auto successorAssumps = solver->getLtlSuccessorAssumps(obligation.assumptions.eventualities);
        return {true, {successorAssumps.first, successorAssumps.second}, {}};
    }
}

void LineProver::addReason(int distance, literal_set reason) {
    shared_ptr<Prover> solver = getSolver(distance);
    solver->addClause(reason);
}

shared_ptr<Prover> LineProver::getSolver(int distance) {
    if (distance >= (uint8_t) solvers.size()) {
        //cout << "SPAWNING NEW: " << distance << endl;
        solvers.push_back(shared_ptr<Prover>(new MinisatProver(false)));
        solvers[distance]->prepareLtlfSat(formulaTriple, initialLiteral, succInSat);
        if (distance == 0) {
            // Must have all eventualities fulfilled
            solvers[distance]->makeLtlTail();
        }
    }
    return solvers[distance];
}
