#pragma once
#include "../Visualizer/IAlgorithm.h"
#include <vector>
#include <string>
#include <random>

struct SearchStep {
    std::vector<int> array;
    int current_idx = -1;
    int left = -1, right = -1, mid = -1;
    int found_idx = -1;
    std::vector<bool> eliminated;
    std::vector<bool> checked;
    int comparisons = 0;
    std::string description;
    int pseudocode_line = -1;
};

class SearchAlgoBase : public IAlgorithm {
protected:
    std::vector<int> data;
    std::vector<SearchStep> steps;
    int currentStepIdx = 0;
    int arraySize = 20;
    int targetValue = 42;
    std::mt19937 rng;

    void generateSortedArray();

public:
    SearchAlgoBase();
    void step() override;
    void stepK(int k) override;
    void runToCompletion() override;
    void reset() override;
    void shuffle() override;
    bool isFinished() const override;
    AlgoState getCurrentState() const override;
    std::vector<Cell> getCells() const override;
    std::vector<Edge> getEdges() const override;
    VisMode getVisMode() const override;
    int getTargetValue() const override;
    int getArraySize() const override;
    void setArraySize(int n) override;
    void setCustomArray(const std::vector<int>& arr) override;
    void setTargetValue(int t) override;
};

class LinearSearch : public SearchAlgoBase {
public:
    void init() override;
    std::string getTheory() const override;
    std::string getName() const override;
    std::vector<std::string> getPseudoCode() const override;
    std::string getComplexity() const override;
};

class BinarySearch : public SearchAlgoBase {
public:
    void init() override;
    std::string getTheory() const override;
    std::string getName() const override;
    std::vector<std::string> getPseudoCode() const override;
    std::string getComplexity() const override;
};

