#include "ProbationSolutionMemo.h"

ProbationSolutionMemo::ProbationSolutionMemo() {}
ProbationSolutionMemo::~ProbationSolutionMemo() {}

ProbationSolutionMemoResult
ProbationSolutionMemo::getFromMemo(const shared_ptr<vector<int>> &modalContext, const shared_ptr<Bitset> &assumptions) const {
    for (auto satisfiable : satSols) {
        if (satisfiable.first == modalContext) {
            if (satisfiable.second->contains(*assumptions)) {
                return {true, {true, literal_set()}};
            }
        }
    }
    return {false, {false, literal_set()}};
}

void ProbationSolutionMemo::insertSat(const shared_ptr<vector<int>> &modalContext, const shared_ptr<Bitset> &assumptions, literal_set model) {
    satSols.push_back({modalContext, assumptions});
}

void ProbationSolutionMemo::clear() {
    satSols.clear();
}
void ProbationSolutionMemo::resize(int n) {
    satSols.resize(n);
}
int ProbationSolutionMemo::size() {
    return satSols.size();
}

void ProbationSolutionMemo::updateMinimalRoot(int newMinimalRoot) {
    if (minimalRoot == -1) minimalRoot = newMinimalRoot;  
    else minimalRoot = min(minimalRoot, newMinimalRoot);
}

ProbationSolutionMemoState ProbationSolutionMemo::getState() {
    return {minimalRoot, satSols.size()};
}

void ProbationSolutionMemo::setState(ProbationSolutionMemoState pastInfo) {
    minimalRoot = pastInfo.first;
    satSols.resize(pastInfo.second);
    pastModels.resize(pastInfo.second);
}

 vector<pair<shared_ptr<vector<int>>, shared_ptr<Bitset>>> ProbationSolutionMemo::getSatSols() const {
    return satSols;
}
vector<pair<shared_ptr<vector<int>>, pair<shared_ptr<Bitset>, shared_ptr<literal_set>>>> ProbationSolutionMemo::getSatSolsAndModels() {
vector<pair<shared_ptr<vector<int>>, pair<shared_ptr<Bitset>, shared_ptr<literal_set>>>> res;

    for (int i = 0; i < satSols.size(); ++i) {
        res.push_back(
                {satSols[i].first, {satSols[i].second, pastModels[i]}}
        );
    }
     for (auto x : satSols) {

     }
    return res;
}

void ProbationSolutionMemo::addPastModel(shared_ptr<literal_set> model) {
    pastModels.push_back(model);
}
