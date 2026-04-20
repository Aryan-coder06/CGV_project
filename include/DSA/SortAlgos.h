#pragma once
#include "../Visualizer/IAlgorithm.h"
#include <vector>
#include <string>
#include <random>

struct StepSnapshot {
    std::vector<int> array;
    int cmp_a = -1, cmp_b = -1;
    int swap_a = -1, swap_b = -1;
    std::vector<int> sorted_indices;
    int pivot_idx = -1;
    std::vector<int> subarray_left;
    std::vector<int> subarray_right;
    int comparisons = 0;
    int swaps = 0;
    std::string description;
    int pseudocode_line = -1;
};

class SortAlgoBase : public IAlgorithm {
protected:
    std::vector<int> data;
    std::vector<StepSnapshot> steps;
    int currentStepIdx = 0;
    int arraySize = 30;
    std::mt19937 rng;

    void generateRandomArray();
    StepSnapshot makeSnapshot(const std::vector<int>& arr, int cmps, int swps, const std::string& desc, int pline);

public:
    SortAlgoBase();
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
    int getArraySize() const override;
    void setArraySize(int n) override;
    void setCustomArray(const std::vector<int>& arr) override;
};

class BubbleSort : public SortAlgoBase {
public:
    void init() override;
    std::string getTheory() const override;
    std::string getName() const override;
    std::vector<std::string> getPseudoCode() const override;
    std::string getComplexity() const override;
};

class SelectionSort : public SortAlgoBase {
public:
    void init() override;
    std::string getTheory() const override;
    std::string getName() const override;
    std::vector<std::string> getPseudoCode() const override;
    std::string getComplexity() const override;
};

class InsertionSort : public SortAlgoBase {
public:
    void init() override;
    std::string getTheory() const override;
    std::string getName() const override;
    std::vector<std::string> getPseudoCode() const override;
    std::string getComplexity() const override;
};

class MergeSort : public SortAlgoBase {
private:
    void mergeSortHelper(std::vector<int>& arr, int left, int right, int& cmps, int& swps);
    void merge(std::vector<int>& arr, int left, int mid, int right, int& cmps, int& swps);
public:
    void init() override;
    std::string getTheory() const override;
    std::string getName() const override;
    std::vector<std::string> getPseudoCode() const override;
    std::string getComplexity() const override;
};

class QuickSort : public SortAlgoBase {
private:
    void quickSortHelper(std::vector<int>& arr, int low, int high, int& cmps, int& swps);
    int partition(std::vector<int>& arr, int low, int high, int& cmps, int& swps);
public:
    void init() override;
    std::string getTheory() const override;
    std::string getName() const override;
    std::vector<std::string> getPseudoCode() const override;
    std::string getComplexity() const override;
};

class HeapSort : public SortAlgoBase {
private:
    void heapify(std::vector<int>& arr, int n, int i, int& cmps, int& swps, std::vector<int>& sorted);
public:
    void init() override;
    std::string getTheory() const override;
    std::string getName() const override;
    std::vector<std::string> getPseudoCode() const override;
    std::string getComplexity() const override;
};

