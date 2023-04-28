#include "TrieformProverK4.h"

shared_ptr<Cache> TrieformProverK4::persistentCache = make_shared<PrefixCache>("P");

shared_ptr<Trieform>
TrieformFactory::makeTrieK4(const shared_ptr<Formula> &formula,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverK4());
  trie->initialise(formula, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieK4(const shared_ptr<Formula> &formula,
                            const vector<int> &newModality,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverK4());
  trie->initialise(formula, newModality, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieK4(const vector<int> &newModality,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverK4());
  trie->initialise(newModality, trieParent);
  return trie;
}

TrieformProverK4::TrieformProverK4() {}
TrieformProverK4::~TrieformProverK4() {}

shared_ptr<Trieform>
TrieformProverK4::create(const shared_ptr<Formula> &formula) {
  return TrieformFactory::makeTrieK4(formula, shared_from_this());
}
shared_ptr<Trieform>
TrieformProverK4::create(const shared_ptr<Formula> &formula,
                         const vector<int> &newModality) {
  return TrieformFactory::makeTrieK4(formula, newModality, shared_from_this());
}
shared_ptr<Trieform> TrieformProverK4::create(const vector<int> &newModality) {
  shared_ptr<Trieform> test = shared_from_this();
  return TrieformFactory::makeTrieK4(newModality, shared_from_this());
}

shared_ptr<Bitset>
TrieformProverK4::convertAssumptionsToBitset(literal_set literals) {
  shared_ptr<Bitset> bitset =
      shared_ptr<Bitset>(new Bitset(2 * assumptionsSize));
  for (Literal literal : literals) {
    bitset->set(2 * idMap[literal.getName()] + literal.getPolarity());
  }
  return bitset;
}

void TrieformProverK4::updateSolutionMemo(const shared_ptr<Bitset> &assumptions,
                                          Solution solution) {
  if (solution.satisfiable) {
    localMemo.insertSat(assumptions);
  } else {
    localMemo.insertUnsat(assumptions, solution.conflict);
  }
}

bool TrieformProverK4::isInHistory(vector<shared_ptr<Bitset>> history,
                                   shared_ptr<Bitset> bitset) {
  for (shared_ptr<Bitset> assump : history) {
    if (assump->contains(*bitset)) {
      return true;
    }
  }
  return false;
}

void TrieformProverK4::makePersistence() {
  for (auto modalSubtrie : subtrieMap) {
    dynamic_cast<TrieformProverK4 *>(modalSubtrie.second.get())
        ->makePersistence();
  }

  modal_clause_set persistentBoxes;
  for (ModalClause boxClause : clauses.getBoxClauses()) {
    // For a=>[]b in our box clauses add 
    // a => [] a

    // Make persistence
    /*
    shared_ptr<Formula> persistent =
        persistentCache->getVariableOrCreate(boxClause.right);
    persistentBoxes.insert({boxClause.modality, persistent, persistent});

    formula_set leftSet;
    leftSet.insert(Not::create(boxClause.left)->negatedNormalForm());
    leftSet.insert(persistent);
    clauses.addClause(Or::create(leftSet));

    formula_set rightSet;
    rightSet.insert(Not::create(persistent)->negatedNormalForm());
    rightSet.insert(boxClause.right);
    propagateClauses(Box::create(boxClause.modality, 1, Or::create(rightSet)));
    */
    persistentBoxes.insert({boxClause.modality, boxClause.left, boxClause.left});
  }
  clauses.extendBoxClauses(persistentBoxes);

  for (ModalClause persistentBox : persistentBoxes) {
    subtrieMap[persistentBox.modality]->clauses.addBoxClause(persistentBox);
  }
}

void TrieformProverK4::propagateLevels() {
  for (auto modalSubtrie : subtrieMap) {
    if (modalSubtrie.second->hasSubtrie(modalSubtrie.first)) {
      modalSubtrie.second->getSubtrie(modalSubtrie.first)
          ->overShadow(modalSubtrie.second, modalSubtrie.first);
    }
    dynamic_cast<TrieformProverK4 *>(modalSubtrie.second.get())
        ->propagateLevels();
  }
}

void TrieformProverK4::preprocess() {
  makePersistence();
  propagateLevels();
}

void TrieformProverK4::prepareSAT(name_set extra) {
  for (string name : extra) {
    idMap[name] = assumptionsSize++;
  }
  modal_names_map modalExtras = prover->prepareSAT(clauses, extra);
  for (auto modalSubtrie : subtrieMap) {
    modalSubtrie.second->prepareSAT(modalExtras[modalSubtrie.first]);
  }
}

Solution TrieformProverK4::prove(vector<shared_ptr<Bitset>> history,
                                 literal_set assumptions) {
  // Check solution memo
  shared_ptr<Bitset> assumptionsBitset =
      convertAssumptionsToBitset(assumptions);
  LocalSolutionMemoResult memoResult = localMemo.getFromMemo(assumptionsBitset);

  if (memoResult.inSatMemo) {
    return memoResult.result;
  }

  // Ancestor loop check
  if (isInHistory(history, assumptionsBitset)) {
    return {true, literal_set()};
  }

  // Solve locally
  Solution solution = prover->solve(assumptions);

  if (!solution.satisfiable) {
    updateSolutionMemo(assumptionsBitset, solution);
    return solution;
  }

  prover->calculateTriggeredDiamondsClauses();
  modal_literal_map triggeredDiamonds = prover->getTriggeredDiamondClauses();

  // If there are no fired diamonds, it is satisfiable
  if (triggeredDiamonds.size() == 0) {
    updateSolutionMemo(assumptionsBitset, solution);
    return solution;
  }

  prover->calculateTriggeredBoxClauses();
  modal_literal_map triggeredBoxes = prover->getTriggeredBoxClauses();

  for (auto modalityDiamonds : triggeredDiamonds) {
    // Handle each modality normally
    if (isSubsetOf(modalityDiamonds.second,
                   triggeredBoxes[modalityDiamonds.first])) {
      // The fired diamonds are a subset of the boxes - we thus can create one
      // world.

      Solution childSolution;
      if (hasSubtrie(modalityDiamonds.first)) {
        childSolution = subtrieMap[modalityDiamonds.first]->prove(
            triggeredBoxes[modalityDiamonds.first]);
      } else {
        history.push_back(assumptionsBitset);
        childSolution = prove(history, triggeredBoxes[modalityDiamonds.first]);
        history.pop_back();
      }
      // If the one world solution is satisfiable, then we're all good
      if (childSolution.satisfiable) {
        continue;
      }
      // Otherwise, as the diamonds are a subset of the boxes, the left
      // implications of the problem box clauses cannot be true with any diamond
      // clause of this modality
      vector<literal_set> badImplications = prover->getNotProblemBoxClauses(
          modalityDiamonds.first, childSolution.conflict);
      badImplications.push_back(
          prover->getNotAllDiamondLeft(modalityDiamonds.first));
      // Add ~leftDiamond=>\/~leftProbemBox
      for (literal_set learnClause : generateClauses(badImplications)) {
        prover->addClause(learnClause);
      }
      // Find new result
      return prove(assumptions);
    } else {
      // The fired diamonds are not a subset of the fired boxes, we need to
      // create one world for each diamond clause
      diamond_queue diamondPriority =
          prover->getPrioritisedTriggeredDiamonds(modalityDiamonds.first);

      while (!diamondPriority.empty()) {
        // Create a world for each diamond
        Literal diamond = diamondPriority.top().literal;
        diamondPriority.pop();

        literal_set childAssumptions =
            literal_set(triggeredBoxes[modalityDiamonds.first]);
        childAssumptions.insert(diamond);

        // Run the solver for the next level
        Solution childSolution;
        if (hasSubtrie(modalityDiamonds.first)) {
          childSolution =
              subtrieMap[modalityDiamonds.first]->prove(childAssumptions);
        } else {
          history.push_back(assumptionsBitset);
          childSolution = prove(history, childAssumptions);
          history.pop_back();
        }

        // If it is satisfiable create the next world
        if (childSolution.satisfiable) {
          continue;
        }

        // Otherwise there must have been a conflict
        vector<literal_set> badImplications = prover->getNotProblemBoxClauses(
            modalityDiamonds.first, childSolution.conflict);

        if (childSolution.conflict.find(diamond) !=
            childSolution.conflict.end()) {
          // The diamond clause, either on its own or together with box clauses,
          // caused a conflict. We must add diamond implies OR NOT problem
          // box clauses.
          prover->updateLastFail(diamond);
          badImplications.push_back(
              prover->getNotDiamondLeft(modalityDiamonds.first, diamond));

          for (literal_set learnClause : generateClauses(badImplications)) {
            prover->addClause(learnClause);
          }

          // Find new result
          return prove(assumptions);
        } else {
          // Only the box clauses caused a conflict, so we must add each diamond
          // clause implies OR NOT problem box lefts
          badImplications.push_back(
              prover->getNotAllDiamondLeft(modalityDiamonds.first));
          // Add ~leftDiamond=>\/~leftProbemBox
          for (literal_set learnClause : generateClauses(badImplications)) {
            prover->addClause(learnClause);
          }
          // Find new result
          return prove(assumptions);
        }
      }
    }
  }
  // If we reached here the solution is satisfiable under all modalities
  updateSolutionMemo(assumptionsBitset, solution);
  return solution;
}

Solution TrieformProverK4::prove(literal_set assumptions) {
  return prove(vector<shared_ptr<Bitset>>(), assumptions);
}
