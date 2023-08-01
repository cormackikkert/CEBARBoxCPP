#ifndef PROBATION_SOLUTION_MEMO_H
#define PROBATION_SOLUTION_MEMO_H

#include "../../Bitset/Bitset.h"
#include "../Literal/Literal.h"
#include <memory>
#include <vector>
#include <map>

class TrieformProverKt;

using namespace std;

typedef pair<int, int> ProbationSolutionMemoState;

struct ProbationSolutionMemoResult {
    bool inSatMemo;
    Solution result;
};

class ProbationSolutionMemo {
    private:
        vector<pair<shared_ptr<vector<int>>, shared_ptr<Bitset>>> satSols;
        vector<shared_ptr<literal_set>> pastModels;

    public:
        int minimalRoot = -1;

        ProbationSolutionMemo();
        ~ProbationSolutionMemo();

        void addPastModel(shared_ptr<literal_set> model);

        ProbationSolutionMemoResult
            getFromMemo(const shared_ptr<vector<int>> &modalContext, const shared_ptr<Bitset> &assumptions) const;

        void insertSat(const shared_ptr<vector<int>> &modalContext, const shared_ptr<Bitset> &assumptions, literal_set model = {});

        void clear();
        void resize(int n);
        int size();
        
        void updateMinimalRoot(int newMinimalRoot);
        ProbationSolutionMemoState getState();
        void setState(ProbationSolutionMemoState pastInfo);
        
        vector<pair<shared_ptr<vector<int>>, shared_ptr<Bitset>>> getSatSols() const;
        vector<pair<shared_ptr<vector<int>>, pair<shared_ptr<Bitset>, shared_ptr<literal_set>>>> getSatSolsAndModels();
};

#endif
