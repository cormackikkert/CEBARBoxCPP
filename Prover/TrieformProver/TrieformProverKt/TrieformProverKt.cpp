#include "TrieformProverKt.h"

map<vector<int>, shared_ptr<TrieformProverKt>> TrieformProverKt::all_trieforms;
ProbationSolutionMemo TrieformProverKt::probationMemo;
int TrieformProverKt::restartUntil = -1;

shared_ptr<Trieform>
TrieformFactory::makeTrieKt(const shared_ptr<Formula> &formula,
        shared_ptr<Trieform> trieParent) {
    shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverKt());
    trie->initialise(formula, trieParent);
    return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieKt(const shared_ptr<Formula> &formula,
        const vector<int> &newModality,
        shared_ptr<Trieform> trieParent) {
    shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverKt());
    trie->initialise(formula, newModality, trieParent);
    return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieKt(const vector<int> &newModality,
        shared_ptr<Trieform> trieParent) {
    shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverKt());
    trie->initialise(newModality, trieParent);
    return trie;
}

TrieformProverKt::TrieformProverKt() {
    stringModalContexts=true;
}
TrieformProverKt::~TrieformProverKt() {}

shared_ptr<Trieform>
TrieformProverKt::create(const shared_ptr<Formula> &formula) {
    return TrieformFactory::makeTrieKt(formula, shared_from_this());
}
shared_ptr<Trieform> TrieformProverKt::create(const shared_ptr<Formula> &formula,
        const vector<int> &newModality) {
    return TrieformFactory::makeTrieKt(formula, newModality, shared_from_this());
}
shared_ptr<Trieform> TrieformProverKt::create(const vector<int> &newModality) {
    shared_ptr<Trieform> test = shared_from_this();
    return TrieformFactory::makeTrieKt(newModality, shared_from_this());
}

shared_ptr<Bitset>
TrieformProverKt::convertAssumptionsToBitset(literal_set literals) {
    shared_ptr<Bitset> bitset =
        shared_ptr<Bitset>(new Bitset(2 * assumptionsSize));
    for (Literal literal : literals) {
        bitset->set(2 * idMap[literal.getName()] + literal.getPolarity());
    }
    return bitset;
}

shared_ptr<Bitset>
TrieformProverKt::fleshedOutAssumptionBitset(literal_set model) {
    shared_ptr<Bitset> bitset =
        shared_ptr<Bitset>(new Bitset(2 * assumptionsSize)); 
    for (auto x : idMap) {
        Literal lit = Literal{x.first, 0};
        bitset->set(2 * x.second + (model.find(lit) == model.end()));
    }
    return bitset;
}

void TrieformProverKt::updateSolutionMemo(const shared_ptr<Bitset> &assumptions,
        Solution solution) {
    if (solution.satisfiable) {
        localMemo.insertSat(assumptions);
    } else {
        localMemo.insertUnsat(assumptions, solution.conflict);
    }
}

void TrieformProverKt::getStats() {

    for (auto modalSubtrie : subtrieMap) {
        auto subtrie = dynamic_cast<TrieformProverKt*>(modalSubtrie.second.get());
        subtrie->getStats();
        numRelations = max(abs(modalSubtrie.first),subtrie->numRelations);
    }
}

shared_ptr<TrieformProverKt> TrieformProverKt::convertToGrid(vector<int>& modal_context) {
    if (all_trieforms.find(modal_context) == all_trieforms.end()) {
        all_trieforms[modal_context] = shared_ptr<TrieformProverKt>(new TrieformProverKt);
        all_trieforms[modal_context]->modalContext = make_shared<vector<int>>(modal_context);
    }     

    shared_ptr<TrieformProverKt> gridTrieform = all_trieforms[modal_context];

    gridTrieform->clauses.extendClauses(getClauses());

    for (auto modalSubtrie: subtrieMap) {

        modal_context[abs(modalSubtrie.first)] += (modalSubtrie.first > 0) ? 1 : -1;
        shared_ptr<TrieformProverKt> gridTrieformSuccessor = dynamic_cast<TrieformProverKt*>(modalSubtrie.second.get())->convertToGrid(modal_context); 
        modal_context[abs(modalSubtrie.first)] -= (modalSubtrie.first > 0) ? 1 : -1;

        gridTrieform->subtrieMap[modalSubtrie.first] = gridTrieformSuccessor;
        gridTrieformSuccessor->subtrieMap[-modalSubtrie.first] = gridTrieform;

    }

    return gridTrieform;
}

void TrieformProverKt::preprocess() {
    doResiduation();
}

shared_ptr<TrieformProverKt> TrieformProverKt::createGridTrie() {
    getStats();
    if (numRelations == -1) numRelations = 0;
    vector<int> modal_context (numRelations+1);
    auto replacement = convertToGrid(modal_context);
    replacement->numRelations = numRelations;
    return replacement;
    //replacement.buildConnections();
}

bool TrieformProverKt::isSatisfiable() {
    literal_set assumps;
    assumps.insert(Literal("$root", true));
    return prove(assumps).satisfiable;
}

Solution TrieformProverKt::prove(literal_set assumptions) {
    return prove(0, assumptions);
}

Solution  TrieformProverKt::prove(int depth, literal_set assumptions) {

    //cout << "Depth :" << depth << " " << endl;
    //cout << "Depth :" << depth << " " << endl << "SIZES: " << pastModels.size() << " " << history.size() << endl;
    //if (pastModels.size() != history.size()) {
    //    cout << "Depth :" << depth << " " << "WTF" << endl;
    //}
    ProbationSolutionMemoState probationState = probationMemo.getState();
                                             /*
       cout << endl;
       cout << "Depth :" << depth << " " << "PROVE CONTEXT\n" << endl;
       for  (auto x : assumptions) {
       cout <<  x.toString() << " ";
       }
       cout << endl;
       */


    for (auto x : all_trieforms) {
        if (this == x.second.get()) {
            //cout << "Depth :" << depth << " " << "At sum: " << x.first[1] << endl;
        }
    }
    //cout << "Depth :" << depth << " " << "At depth: " << depth << endl;
    //cout << "Depth :" << depth << " " << "History size and depths: " << history.size() << endl;
    for (auto x : history) {
        //cout <<  x.first << " ";
    }
    //cout << endl;

    // Check solution memo
    shared_ptr<Bitset> assumptionsBitset =
        convertAssumptionsToBitset(assumptions);
    LocalSolutionMemoResult memoResult = localMemo.getFromMemo(assumptionsBitset);
    if (memoResult.inSatMemo) {
        //cout << "Depth :" << depth << " " << "sat from actual memo" << endl;
        return memoResult.result;
    }

    ProbationSolutionMemoResult probationMemoResult = probationMemo.getFromMemo(modalContext, assumptionsBitset);

    if (probationMemoResult.inSatMemo) {
        //cout << "Depth :" << depth << " " << "sat from probation: " << probationMemo.minimalRoot << endl;
        return probationMemoResult.result;
    }

    //cout << "WTF: ";
    int inHistory = isInHistory(history, assumptionsBitset);
    if (inHistory != -1) {
        probationMemo.updateMinimalRoot(inHistory);
        //cout << "Depth :" << depth << " " << "Using history: " << inHistory << endl;
        return {true, literal_set()};
    }

    // Solve locally
    Solution solution = prover->solve(assumptions);

    if (!solution.satisfiable) {
        // Clear probationCache
        probationMemo.setState(probationState);
        return solution;
    }
    literal_set currentModel = prover->getModel();
    assumptionsBitset = fleshedOutAssumptionBitset(currentModel);
    prover->calculateTriggeredDiamondsClauses();
    modal_literal_map triggeredDiamonds = prover->getTriggeredDiamondClauses();



    literal_set triggeredImplications = prover->rememberTriggeredImplications();
    prover->calculateTriggeredBoxClauses();
    modal_literal_map triggeredBoxes = prover->getTriggeredBoxClauses();


    for (auto modalitySubtrie : subtrieMap) {
        // Handle each modality

        if (triggeredDiamonds[modalitySubtrie.first].size() == 0) {
            // If there are no triggered diamonds of a certain modality we can skip it
            continue;
        }

        
        diamond_queue diamondPriority =
            prover->getPrioritisedTriggeredDiamonds(modalitySubtrie.first, triggeredBoxes[modalitySubtrie.first], triggeredDiamonds[modalitySubtrie.first]);

        while (!diamondPriority.empty()) {
            // Create a world for each diamond
            Literal diamond = diamondPriority.top().literal;
            literal_set diamondGroup = prover->getDiamondOverApproximation(modalitySubtrie.first, diamond, currentModel);
            assert (diamondGroup.find(diamond) != diamondGroup.end());

            diamondPriority.pop();
            literal_set childAssumptions =
                literal_set(triggeredBoxes[modalitySubtrie.first]);
            childAssumptions.insert(diamondGroup.begin(), diamondGroup.end());

            history.push_back({depth, assumptionsBitset});
            pastModels.push_back({depth, currentModel});
            // Run the solver for the next level

            TrieformProverKt* childNode = dynamic_cast<TrieformProverKt*>(modalitySubtrie.second.get());
            Solution childSolution = childNode->prove(depth+1, childAssumptions);
            history.pop_back();

            // Clause propagation (learn clauses from the child)
            for (literal_set learnClause : prover->getClauses(modalitySubtrie.first, prover->negatedClauses(childNode->allConflicts))) {
                allConflicts.push_back(learnClause);
                prover->addClause(learnClause);
                restartUntil = checkClauseAgainstPastModels(restartUntil, learnClause);
            }
            childNode->allConflicts.clear();

            if (!childSolution.satisfiable) {
                for (literal_set learnClause : prover->getClauses(modalitySubtrie.first, childSolution.conflict)) {
                    allConflicts.push_back(learnClause);
                    prover->addClause(learnClause);
                    restartUntil = checkClauseAgainstPastModels(restartUntil, learnClause);
                }
            }
            pastModels.pop_back();

            // Start to backtrack if we learnt a conflict against an ancestor
            // Note this handles the restart rule
            if (restartUntil != -1) {
                probationMemo.setState(probationState);
                if (restartUntil == depth) {
                    // restart current node
                    restartUntil = -1;
                    return prove(depth, assumptions);
                } else {
                    // Keep backtracking until we should restart
                    return childSolution;
                }
            }

            if (!childSolution.satisfiable) {
                // We have not restarted, meaning our learnt clauses didn't 
                // conflict with the current model.
                // This means we got a conflict from our overapproximation, so we 
                // must refine the overapproximation
                prover->refineOverApproximation(modalitySubtrie.first, currentModel, childSolution.conflict);
                return prove(depth, assumptions);

            }
            // If the fired diamonds are a subset of the fired boxes we only
            // needed to create one successor
            if (isSubsetOf(triggeredDiamonds[modalitySubtrie.first], triggeredBoxes[modalitySubtrie.first])) {
                break;
            }
        }
    }

    // If we reached here the solution is satisfiable under all modalities
    if (probationMemo.minimalRoot == -1) {
        updateSolutionMemo(assumptionsBitset, solution);
    } else if (depth == probationMemo.minimalRoot) {
        // Move probation cache to actual cache
        for (auto x : probationMemo.getSatSols()) {
            auto trieform = all_trieforms[*x.first];
            trieform->localMemo.insertSat(x.second);
        }
        probationMemo.setState({-1, 0});
    } else if (depth > probationMemo.minimalRoot) {
        probationMemo.insertSat(modalContext, assumptionsBitset);
    } else {
        // Shouldn't get here
        assert (false);
    }
    return solution;
}

void TrieformProverKt::doResiduation() {
    map<vector<int>, modal_clause_set> newClauses;
    for (auto modTrie : all_trieforms) {
        auto modalContext = modTrie.first;
        for (ModalClause modalClause : modTrie.second->getClauses().getBoxClauses()) {
            modalContext[abs(modalClause.modality)] += (modalClause.modality>0) ? 1 : -1;
            newClauses[modalContext].insert({-modalClause.modality,
                    modalClause.right->negate(),
                    modalClause.left->negate()});
            modalContext[abs(modalClause.modality)] -= (modalClause.modality>0) ? 1 : -1;

        }

    }
    for (auto modTrie : all_trieforms) {
        modTrie.second->clauses.extendBoxClauses(newClauses[modTrie.first]);
    }
}

void TrieformProverKt::buildConnections() {
    for (auto modTrie : all_trieforms) {
        auto modalContext = modTrie.first;
        for (ModalClause modalClause : modTrie.second->getClauses().getDiamondClauses()) {
            modalContext[abs(modalClause.modality)] += (modalClause.modality>0) ? 1 : -1;

            modTrie.second->subtrieMap[modalClause.modality] = all_trieforms[modalContext]; 

            modalContext[abs(modalClause.modality)] -= (modalClause.modality>0) ? 1 : -1;

        }

    }
}
string TrieformProverKt::toString() {
    if (numRelations == -1) {
        return Trieform::toString();
    }
    string trieString = "";
    for (auto modTrie : all_trieforms) {
        string modalContext = "";
        for (unsigned int i = 1; i < modTrie.first.size(); ++i) {
            modalContext += "[" + to_string(i) + "](" 
                + to_string(modTrie.first[i])+")";
        }

        vector<string> clauseComponents = modTrie.second->clauses.toStringComponents();

        for (string component : clauseComponents) {
            trieString += modalContext + " " + component + '\n';
        }
    }
    return trieString; 
}

int TrieformProverKt::isInHistory(vector<pair<int, shared_ptr<Bitset>>> history,
        shared_ptr<Bitset> bitset) {
    for (unsigned i = history.size(); i-- > 0; ) {
        if (history[i].second->contains(*bitset))
            return history[i].first;
    }
    return -1;
}

void TrieformProverKt::removeTrueAndFalse() {
    for (auto modTrie : all_trieforms) {
        modTrie.second->clauses.removeTrueAndFalse();
    }
}

void TrieformProverKt::prepareSAT(name_set extra_unused) {
    for (auto modTrie : all_trieforms) {
        vector<string> clauseComponents = modTrie.second->clauses.toStringComponents();

        vector<int> modalContext = modTrie.first;
        name_set extra;

        if (all_of(modalContext.begin(), modalContext.end(), [](int i) { return i==0; })) {
            extra.insert("$root");
        }

        for (int relation = 1; relation <= numRelations; ++relation) {
            for (int jump : {-1, 1}){
                modalContext[relation] += jump;

                if (all_trieforms.find(modalContext) != all_trieforms.end()) {
                    auto ancestorTrie = all_trieforms[modalContext];
                    for (ModalClause clause : ancestorTrie->getClauses().getBoxClauses()) {
                        if (clause.modality == -jump) {
                            extra.insert(ancestorTrie->getProver()->getPrimitiveName(clause.right));
                        }
                    }
                    for (ModalClause clause : ancestorTrie->getClauses().getDiamondClauses()) {
                        if (clause.modality == -jump) {
                            extra.insert(ancestorTrie->getProver()->getPrimitiveName(clause.right));
                        }
                    }
                }


                modalContext[relation] -= jump;

            }


        }
        for (string name : extra) {
            modTrie.second->idMap[name] = modTrie.second->assumptionsSize++;
        }
        modTrie.second->combineBoxLeft();
        modTrie.second->combineBoxRight();
        modTrie.second->combineDiamondRight();
        modTrie.second->prover->prepareSAT(modTrie.second->clauses, extra);
        //modTrie.second->prover->createBoxOverApproximation();
        modTrie.second->prover->createDiamondOverApproximation();
    }
}

unsigned int TrieformProverKt::checkClauseAgainstPastModels(int restartUntil, literal_set clause) {
    for (unsigned int i = 0; i < pastModels.size(); i++) {
        bool contains = false;
        for (Literal x : clause) {
            if (pastModels[i].second.find(x) != pastModels[i].second.end()) {
                contains = true;
                break;
            }
        }
        if (!contains) {
            if (restartUntil == -1) return pastModels[i].first;
            return min(pastModels[i].first, restartUntil);
        };
    }
    return restartUntil;
}
