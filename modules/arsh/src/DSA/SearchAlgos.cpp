#include "../../include/DSA/SearchAlgos.h"
#include <algorithm>
#include <chrono>
#include <sstream>

SearchAlgoBase::SearchAlgoBase()
    : rng(std::chrono::steady_clock::now().time_since_epoch().count()) {}

void SearchAlgoBase::generateSortedArray() {
    data.resize(arraySize);
    int val = 2 + rng() % 5;
    for (int i = 0; i < arraySize; i++) {
        data[i] = val;
        val += 2 + rng() % 6;
    }
    int idx = rng() % arraySize;
    targetValue = data[idx];
}

void SearchAlgoBase::step() {
    if (currentStepIdx < (int)steps.size() - 1)
        currentStepIdx++;
}

void SearchAlgoBase::stepK(int k) {
    for (int i = 0; i < k && currentStepIdx < (int)steps.size() - 1; i++)
        currentStepIdx++;
}

void SearchAlgoBase::runToCompletion() {
    currentStepIdx = (int)steps.size() - 1;
}

void SearchAlgoBase::reset() {
    currentStepIdx = 0;
}

void SearchAlgoBase::shuffle() {
    generateSortedArray();
    steps.clear();
    currentStepIdx = 0;
    init();
}

bool SearchAlgoBase::isFinished() const {
    return currentStepIdx >= (int)steps.size() - 1;
}

AlgoState SearchAlgoBase::getCurrentState() const {
    AlgoState st;
    if (steps.empty()) return st;
    const auto& snap = steps[currentStepIdx];
    st.currentStep = currentStepIdx;
    st.totalSteps = (int)steps.size() - 1;
    st.comparisons = snap.comparisons;
    st.pseudoCodeLine = snap.pseudocode_line;
    st.calcLines.push_back(snap.description);
    st.currentPixelInfo = "Step " + std::to_string(currentStepIdx) + " / " + std::to_string(st.totalSteps);
    return st;
}

std::vector<Cell> SearchAlgoBase::getCells() const {
    std::vector<Cell> cells;
    if (steps.empty()) return cells;
    const auto& snap = steps[currentStepIdx];
    for (int i = 0; i < (int)snap.array.size(); i++) {
        Cell c;
        c.index = i;
        c.value = snap.array[i];
        c.highlighted = false;

        if (snap.found_idx == i) {
            c.r = 0.0f; c.g = 1.0f; c.b = 0.6f;
            c.highlighted = true;
        } else if (snap.current_idx == i) {
            c.r = 1.0f; c.g = 0.85f; c.b = 0.0f;
            c.highlighted = true;
        } else if (snap.mid == i) {
            c.r = 1.0f; c.g = 0.85f; c.b = 0.0f;
            c.highlighted = true;
        } else if (i < (int)snap.eliminated.size() && snap.eliminated[i]) {
            c.r = 0.15f; c.g = 0.15f; c.b = 0.20f;
        } else if (i < (int)snap.checked.size() && snap.checked[i]) {
            c.r = 0.4f; c.g = 0.1f; c.b = 0.5f;
        } else {
            c.r = 0.10f; c.g = 0.13f; c.b = 0.18f;
        }
        cells.push_back(c);
    }
    return cells;
}

std::vector<Edge> SearchAlgoBase::getEdges() const {
    return {};
}

VisMode SearchAlgoBase::getVisMode() const {
    return VisMode::SEARCH_ARRAY;
}

int SearchAlgoBase::getTargetValue() const {
    return targetValue;
}

int SearchAlgoBase::getArraySize() const {
    return arraySize;
}

void SearchAlgoBase::setArraySize(int n) {
    arraySize = std::max(5, std::min(40, n));
    shuffle();
}

void SearchAlgoBase::setCustomArray(const std::vector<int>& arr) {
    if (arr.empty()) return;
    data = arr;
    std::sort(data.begin(), data.end());
    arraySize = (int)data.size();
    bool found = false;
    for (int v : data) { if (v == targetValue) { found = true; break; } }
    if (!found && !data.empty()) targetValue = data[data.size() / 2];
    steps.clear();
    currentStepIdx = 0;
    init();
}

void SearchAlgoBase::setTargetValue(int t) {
    targetValue = t;
    steps.clear();
    currentStepIdx = 0;
    init();
}

void LinearSearch::init() {
    if (data.empty()) generateSortedArray();
    steps.clear();
    currentStepIdx = 0;

    int n = (int)data.size();
    int cmps = 0;
    std::vector<bool> checked(n, false);

    SearchStep initial;
    initial.array = data;
    initial.eliminated.resize(n, false);
    initial.checked.resize(n, false);
    initial.description = "Searching for value " + std::to_string(targetValue) + " in array of " + std::to_string(n) + " elements";
    initial.pseudocode_line = -1;
    steps.push_back(initial);

    for (int i = 0; i < n; i++) {
        cmps++;
        SearchStep s;
        s.array = data;
        s.current_idx = i;
        s.eliminated.resize(n, false);
        s.checked = checked;
        s.comparisons = cmps;
        s.description = "Checking arr[" + std::to_string(i) + "] = " + std::to_string(data[i]) +
            (data[i] == targetValue ? " == " : " != ") + std::to_string(targetValue);
        s.pseudocode_line = 1;

        if (data[i] == targetValue) {
            s.found_idx = i;
            s.description = "Found " + std::to_string(targetValue) + " at index " + std::to_string(i) + "!";
            s.pseudocode_line = 2;
            steps.push_back(s);
            break;
        }

        checked[i] = true;
        steps.push_back(s);
    }

    if (steps.back().found_idx == -1) {
        SearchStep nf;
        nf.array = data;
        nf.eliminated.resize(n, false);
        nf.checked = checked;
        nf.comparisons = cmps;
        nf.description = "Value " + std::to_string(targetValue) + " not found in array.";
        nf.pseudocode_line = 3;
        steps.push_back(nf);
    }
}

std::string LinearSearch::getName() const { return "Linear Search"; }

std::string LinearSearch::getTheory() const {
    return
        "LINEAR SEARCH\n"
        "=============\n\n"
        "WHAT IS IT?\n"
        "  Linear Search (Sequential Search) is the simplest search algorithm.\n"
        "  It checks each element of the array one by one until the target\n"
        "  is found or the end of the array is reached.\n\n"
        "HOW IT WORKS:\n"
        "  1. Start from the first element (index 0)\n"
        "  2. Compare current element with the target\n"
        "  3. If match found, return the index\n"
        "  4. If not, move to the next element\n"
        "  5. If end reached without finding, return -1\n\n"
        "COMPLEXITY:\n"
        "  Best Case:    O(1)   — target is the first element\n"
        "  Average Case: O(n)\n"
        "  Worst Case:   O(n)   — target is last or not present\n"
        "  Space:        O(1)\n\n"
        "WHEN TO USE:\n"
        "  - Unsorted or small arrays\n"
        "  - When simplicity matters more than speed\n"
        "  - Single search on unindexed data\n\n"
        "KEY INSIGHT:\n"
        "  Linear Search works on any collection (sorted or unsorted)\n"
        "  but is inefficient for large datasets. For sorted data,\n"
        "  Binary Search is always preferred.\n";
}

std::vector<std::string> LinearSearch::getPseudoCode() const {
    return {
        "for i = 0 to n-1:",
        "  if arr[i] == target:",
        "    return i",
        "return -1  (not found)"
    };
}

std::string LinearSearch::getComplexity() const {
    return "Best: O(1)  |  Avg: O(n)  |  Worst: O(n)  |  Space: O(1)";
}

void BinarySearch::init() {
    if (data.empty()) generateSortedArray();
    steps.clear();
    currentStepIdx = 0;

    int n = (int)data.size();
    int cmps = 0;
    int left = 0, right = n - 1;
    std::vector<bool> eliminated(n, false);

    SearchStep initial;
    initial.array = data;
    initial.left = left; initial.right = right;
    initial.eliminated.resize(n, false);
    initial.checked.resize(n, false);
    initial.description = "Binary Search for " + std::to_string(targetValue) +
        " in sorted array [0.." + std::to_string(n-1) + "]";
    initial.pseudocode_line = -1;
    steps.push_back(initial);

    while (left <= right) {
        int mid = left + (right - left) / 2;
        cmps++;

        SearchStep s;
        s.array = data;
        s.left = left; s.right = right; s.mid = mid;
        s.eliminated = eliminated;
        s.checked.resize(n, false);
        s.comparisons = cmps;
        s.description = "Left=" + std::to_string(left) + " Mid=" + std::to_string(mid) +
            " Right=" + std::to_string(right) + " | arr[" + std::to_string(mid) + "]=" +
            std::to_string(data[mid]);
        s.pseudocode_line = 2;

        if (data[mid] == targetValue) {
            s.found_idx = mid;
            s.description = "Found " + std::to_string(targetValue) + " at index " + std::to_string(mid) + "!";
            s.pseudocode_line = 3;
            steps.push_back(s);
            return;
        } else if (data[mid] < targetValue) {
            s.description += " < " + std::to_string(targetValue) + " -> search right half";
            s.pseudocode_line = 4;
            steps.push_back(s);
            for (int i = left; i <= mid; i++) eliminated[i] = true;
            left = mid + 1;
        } else {
            s.description += " > " + std::to_string(targetValue) + " -> search left half";
            s.pseudocode_line = 5;
            steps.push_back(s);
            for (int i = mid; i <= right; i++) eliminated[i] = true;
            right = mid - 1;
        }
    }

    SearchStep nf;
    nf.array = data;
    nf.eliminated = eliminated;
    nf.checked.resize(n, false);
    nf.comparisons = cmps;
    nf.description = "Value " + std::to_string(targetValue) + " not found.";
    nf.pseudocode_line = 6;
    steps.push_back(nf);
}

std::string BinarySearch::getName() const { return "Binary Search"; }

std::string BinarySearch::getTheory() const {
    return
        "BINARY SEARCH\n"
        "=============\n\n"
        "WHAT IS IT?\n"
        "  Binary Search is an efficient algorithm for finding a target value\n"
        "  in a SORTED array. It works by repeatedly dividing the search\n"
        "  interval in half.\n\n"
        "PREREQUISITE:\n"
        "  The array MUST be sorted in ascending order.\n\n"
        "HOW IT WORKS:\n"
        "  1. Set left = 0, right = n-1\n"
        "  2. Calculate mid = (left + right) / 2\n"
        "  3. If arr[mid] == target: found!\n"
        "  4. If arr[mid] < target: search right half (left = mid+1)\n"
        "  5. If arr[mid] > target: search left half (right = mid-1)\n"
        "  6. Repeat until found or left > right\n\n"
        "COMPLEXITY:\n"
        "  Best Case:    O(1)      — target is at the middle\n"
        "  Average Case: O(log n)\n"
        "  Worst Case:   O(log n)\n"
        "  Space:        O(1)     — iterative version\n\n"
        "WHEN TO USE:\n"
        "  - Sorted arrays (always preferred over linear search)\n"
        "  - Large datasets where O(n) is too slow\n"
        "  - Databases, file systems, and indexing\n\n"
        "KEY INSIGHT:\n"
        "  Each comparison eliminates half the remaining elements.\n"
        "  For an array of 1 million elements, Binary Search needs\n"
        "  at most 20 comparisons. Linear Search needs up to 1,000,000.\n";
}

std::vector<std::string> BinarySearch::getPseudoCode() const {
    return {
        "left = 0, right = n-1",
        "while left <= right:",
        "  mid = (left + right) / 2",
        "  if arr[mid] == target: return mid",
        "  if arr[mid] < target: left = mid+1",
        "  else: right = mid-1",
        "return -1  (not found)"
    };
}

std::string BinarySearch::getComplexity() const {
    return "Best: O(1)  |  Avg: O(log n)  |  Worst: O(log n)  |  Space: O(1)";
}

