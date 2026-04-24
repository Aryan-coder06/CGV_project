#include "../../include/DSA/SortAlgos.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <chrono>
#include <numeric>

SortAlgoBase::SortAlgoBase()
    : rng(std::chrono::steady_clock::now().time_since_epoch().count()) {}

void SortAlgoBase::generateRandomArray() {
    data.resize(arraySize);
    for (int i = 0; i < arraySize; i++)
        data[i] = 5 + rng() % (arraySize * 3);
}

StepSnapshot SortAlgoBase::makeSnapshot(const std::vector<int>& arr, int cmps, int swps,
                                        const std::string& desc, int pline) {
    StepSnapshot s;
    s.array = arr;
    s.comparisons = cmps;
    s.swaps = swps;
    s.description = desc;
    s.pseudocode_line = pline;
    return s;
}

void SortAlgoBase::step() {
    if (currentStepIdx < (int)steps.size() - 1)
        currentStepIdx++;
}

void SortAlgoBase::stepK(int k) {
    for (int i = 0; i < k && currentStepIdx < (int)steps.size() - 1; i++)
        currentStepIdx++;
}

void SortAlgoBase::runToCompletion() {
    currentStepIdx = (int)steps.size() - 1;
}

void SortAlgoBase::reset() {
    currentStepIdx = 0;
}

void SortAlgoBase::shuffle() {
    generateRandomArray();
    steps.clear();
    currentStepIdx = 0;
    init();
}

bool SortAlgoBase::isFinished() const {
    return currentStepIdx >= (int)steps.size() - 1;
}

AlgoState SortAlgoBase::getCurrentState() const {
    AlgoState st;
    if (steps.empty()) return st;
    const auto& snap = steps[currentStepIdx];
    st.currentStep = currentStepIdx;
    st.totalSteps = (int)steps.size() - 1;
    st.comparisons = snap.comparisons;
    st.swaps = snap.swaps;
    st.pseudoCodeLine = snap.pseudocode_line;
    st.calcLines.push_back(snap.description);
    st.currentPixelInfo = "Step " + std::to_string(currentStepIdx) + " / " + std::to_string(st.totalSteps);
    return st;
}

std::vector<Cell> SortAlgoBase::getCells() const {
    std::vector<Cell> cells;
    if (steps.empty()) return cells;
    const auto& snap = steps[currentStepIdx];
    for (int i = 0; i < (int)snap.array.size(); i++) {
        Cell c;
        c.index = i;
        c.value = snap.array[i];
        c.highlighted = false;

        bool isSorted = false;
        for (int si : snap.sorted_indices) {
            if (si == i) { isSorted = true; break; }
        }

        if (i == snap.swap_a || i == snap.swap_b) {
            c.r = 1.0f; c.g = 0.17f; c.b = 0.47f;
            c.highlighted = true;
        } else if (i == snap.pivot_idx) {
            c.r = 1.0f; c.g = 0.55f; c.b = 0.0f;
            c.highlighted = true;
        } else if (i == snap.cmp_a || i == snap.cmp_b) {
            c.r = 1.0f; c.g = 0.85f; c.b = 0.0f;
            c.highlighted = true;
        } else if (isSorted) {
            c.r = 0.0f; c.g = 1.0f; c.b = 0.6f;
        } else {
            c.r = 0.0f; c.g = 0.9f; c.b = 1.0f;
        }
        cells.push_back(c);
    }
    return cells;
}

std::vector<Edge> SortAlgoBase::getEdges() const {
    return {};
}

VisMode SortAlgoBase::getVisMode() const {
    return VisMode::SORT_BARS;
}

int SortAlgoBase::getArraySize() const {
    return arraySize;
}

void SortAlgoBase::setArraySize(int n) {
    arraySize = std::max(10, std::min(80, n));
    shuffle();
}

void SortAlgoBase::setCustomArray(const std::vector<int>& arr) {
    if (arr.empty()) return;
    data = arr;
    arraySize = (int)arr.size();
    steps.clear();
    currentStepIdx = 0;
    init();
}

void BubbleSort::init() {
    if (data.empty()) generateRandomArray();
    steps.clear();
    currentStepIdx = 0;

    std::vector<int> arr = data;
    int n = (int)arr.size();
    int cmps = 0, swps = 0;
    std::vector<int> sorted;

    StepSnapshot initial;
    initial.array = arr;
    initial.description = "Initial array — ready to sort";
    initial.pseudocode_line = -1;
    steps.push_back(initial);

    for (int i = 0; i < n - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1; j++) {
            cmps++;
            StepSnapshot cmp;
            cmp.array = arr;
            cmp.cmp_a = j; cmp.cmp_b = j + 1;
            cmp.sorted_indices = sorted;
            cmp.comparisons = cmps; cmp.swaps = swps;
            cmp.description = "Comparing arr[" + std::to_string(j) + "]=" +
                std::to_string(arr[j]) + " and arr[" + std::to_string(j+1) + "]=" +
                std::to_string(arr[j+1]);
            cmp.pseudocode_line = 3;
            steps.push_back(cmp);

            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                swps++;
                swapped = true;
                StepSnapshot sw;
                sw.array = arr;
                sw.swap_a = j; sw.swap_b = j + 1;
                sw.sorted_indices = sorted;
                sw.comparisons = cmps; sw.swaps = swps;
                sw.description = "Swapped arr[" + std::to_string(j) + "] and arr[" +
                    std::to_string(j+1) + "] (" + std::to_string(arr[j+1]) + " > " +
                    std::to_string(arr[j]) + ")";
                sw.pseudocode_line = 4;
                steps.push_back(sw);
            }
        }
        sorted.push_back(n - i - 1);

        if (!swapped) {
            for (int k = 0; k < n; k++) {
                bool found = false;
                for (int si : sorted) if (si == k) { found = true; break; }
                if (!found) sorted.push_back(k);
            }
            StepSnapshot brk;
            brk.array = arr;
            brk.sorted_indices = sorted;
            brk.comparisons = cmps; brk.swaps = swps;
            brk.description = "No swaps in this pass — array is sorted! Early exit.";
            brk.pseudocode_line = 6;
            steps.push_back(brk);
            break;
        }
    }

    if (sorted.size() < (size_t)n) {
        sorted.clear();
        for (int i = 0; i < n; i++) sorted.push_back(i);
    }
    StepSnapshot fin;
    fin.array = arr;
    fin.sorted_indices = sorted;
    fin.comparisons = cmps; fin.swaps = swps;
    fin.description = "Sorting complete!";
    fin.pseudocode_line = -1;
    steps.push_back(fin);
}

std::string BubbleSort::getName() const { return "Bubble Sort"; }

std::string BubbleSort::getTheory() const {
    return
        "BUBBLE SORT\n"
        "===========\n\n"
        "WHAT IS IT?\n"
        "  Bubble Sort is the simplest sorting algorithm. It repeatedly steps\n"
        "  through the list, compares adjacent elements, and swaps them if they\n"
        "  are in the wrong order. This process repeats until no swaps are needed.\n\n"
        "HOW IT WORKS:\n"
        "  1. Start from index 0\n"
        "  2. Compare arr[i] and arr[i+1]\n"
        "  3. If arr[i] > arr[i+1], swap them\n"
        "  4. Move to next pair\n"
        "  5. After each full pass, the largest unsorted element\n"
        "     \"bubbles up\" to its correct position\n"
        "  6. Repeat for n-1 passes (or until no swaps occur)\n\n"
        "COMPLEXITY:\n"
        "  Best Case:    O(n)     — already sorted (with early exit)\n"
        "  Average Case: O(n^2)\n"
        "  Worst Case:   O(n^2)  — reverse sorted\n"
        "  Space:        O(1)    — in-place sorting\n\n"
        "WHEN TO USE:\n"
        "  - Educational purposes / understanding sorting\n"
        "  - Very small datasets (< 20 elements)\n"
        "  - Nearly sorted data (with early exit)\n\n"
        "KEY INSIGHT:\n"
        "  After k passes, the last k elements are guaranteed\n"
        "  to be in their final sorted positions.\n";
}

std::vector<std::string> BubbleSort::getPseudoCode() const {
    return {
        "for i = 0 to n-1:",
        "  swapped = false",
        "  for j = 0 to n-i-2:",
        "    if arr[j] > arr[j+1]:",
        "      swap(arr[j], arr[j+1])",
        "      swapped = true",
        "  if not swapped: break"
    };
}

std::string BubbleSort::getComplexity() const {
    return "Best: O(n)  |  Avg: O(n^2)  |  Worst: O(n^2)  |  Space: O(1)";
}

void SelectionSort::init() {
    if (data.empty()) generateRandomArray();
    steps.clear();
    currentStepIdx = 0;

    std::vector<int> arr = data;
    int n = (int)arr.size();
    int cmps = 0, swps = 0;
    std::vector<int> sorted;

    StepSnapshot initial;
    initial.array = arr;
    initial.description = "Initial array — ready to sort";
    initial.pseudocode_line = -1;
    steps.push_back(initial);

    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;

        StepSnapshot sel;
        sel.array = arr;
        sel.cmp_a = i;
        sel.sorted_indices = sorted;
        sel.comparisons = cmps; sel.swaps = swps;
        sel.description = "Pass " + std::to_string(i+1) + ": Finding minimum from index " + std::to_string(i);
        sel.pseudocode_line = 1;
        steps.push_back(sel);

        for (int j = i + 1; j < n; j++) {
            cmps++;
            StepSnapshot cmp;
            cmp.array = arr;
            cmp.cmp_a = minIdx; cmp.cmp_b = j;
            cmp.sorted_indices = sorted;
            cmp.comparisons = cmps; cmp.swaps = swps;
            cmp.description = "Comparing arr[" + std::to_string(minIdx) + "]=" +
                std::to_string(arr[minIdx]) + " with arr[" + std::to_string(j) + "]=" +
                std::to_string(arr[j]);
            cmp.pseudocode_line = 3;
            steps.push_back(cmp);

            if (arr[j] < arr[minIdx]) {
                minIdx = j;
            }
        }

        if (minIdx != i) {
            std::swap(arr[i], arr[minIdx]);
            swps++;
            StepSnapshot sw;
            sw.array = arr;
            sw.swap_a = i; sw.swap_b = minIdx;
            sw.sorted_indices = sorted;
            sw.comparisons = cmps; sw.swaps = swps;
            sw.description = "Swapping arr[" + std::to_string(i) + "] and arr[" +
                std::to_string(minIdx) + "] — placing " + std::to_string(arr[i]) + " at index " + std::to_string(i);
            sw.pseudocode_line = 4;
            steps.push_back(sw);
        }
        sorted.push_back(i);
    }

    sorted.push_back(n - 1);
    StepSnapshot fin;
    fin.array = arr;
    fin.sorted_indices = sorted;
    fin.comparisons = cmps; fin.swaps = swps;
    fin.description = "Sorting complete!";
    fin.pseudocode_line = -1;
    steps.push_back(fin);
}

std::string SelectionSort::getName() const { return "Selection Sort"; }

std::string SelectionSort::getTheory() const {
    return
        "SELECTION SORT\n"
        "==============\n\n"
        "WHAT IS IT?\n"
        "  Selection Sort divides the array into a sorted and unsorted region.\n"
        "  It repeatedly finds the minimum element from the unsorted region\n"
        "  and places it at the beginning of the unsorted portion.\n\n"
        "HOW IT WORKS:\n"
        "  1. Set the first unsorted element as the minimum\n"
        "  2. Scan the rest of the unsorted array for a smaller element\n"
        "  3. If found, update the minimum\n"
        "  4. Swap the minimum with the first unsorted position\n"
        "  5. Move the boundary one element to the right\n\n"
        "COMPLEXITY:\n"
        "  Best Case:    O(n^2)\n"
        "  Average Case: O(n^2)\n"
        "  Worst Case:   O(n^2)\n"
        "  Space:        O(1)   — in-place\n\n"
        "WHEN TO USE:\n"
        "  - When memory writes are expensive (minimizes swaps)\n"
        "  - Small datasets\n"
        "  - Understanding fundamental sorting principles\n\n"
        "KEY INSIGHT:\n"
        "  Selection Sort always makes O(n) swaps regardless of input,\n"
        "  making it optimal when swap cost is high.\n";
}

std::vector<std::string> SelectionSort::getPseudoCode() const {
    return {
        "for i = 0 to n-2:",
        "  min_idx = i",
        "  for j = i+1 to n-1:",
        "    if arr[j] < arr[min_idx]:",
        "      min_idx = j",
        "  swap(arr[i], arr[min_idx])"
    };
}

std::string SelectionSort::getComplexity() const {
    return "Best: O(n^2)  |  Avg: O(n^2)  |  Worst: O(n^2)  |  Space: O(1)";
}

void InsertionSort::init() {
    if (data.empty()) generateRandomArray();
    steps.clear();
    currentStepIdx = 0;

    std::vector<int> arr = data;
    int n = (int)arr.size();
    int cmps = 0, swps = 0;
    std::vector<int> sorted;
    sorted.push_back(0);

    StepSnapshot initial;
    initial.array = arr;
    initial.sorted_indices = sorted;
    initial.description = "Initial array — first element is trivially sorted";
    initial.pseudocode_line = -1;
    steps.push_back(initial);

    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;

        StepSnapshot pick;
        pick.array = arr;
        pick.cmp_a = i;
        pick.sorted_indices = sorted;
        pick.comparisons = cmps; pick.swaps = swps;
        pick.description = "Picking key = arr[" + std::to_string(i) + "] = " + std::to_string(key) + " to insert";
        pick.pseudocode_line = 1;
        steps.push_back(pick);

        while (j >= 0 && arr[j] > key) {
            cmps++;
            arr[j + 1] = arr[j];
            swps++;

            StepSnapshot shift;
            shift.array = arr;
            shift.swap_a = j; shift.swap_b = j + 1;
            shift.sorted_indices = sorted;
            shift.comparisons = cmps; shift.swaps = swps;
            shift.description = "Shifting arr[" + std::to_string(j) + "]=" + std::to_string(arr[j]) +
                " right to index " + std::to_string(j+1);
            shift.pseudocode_line = 3;
            steps.push_back(shift);

            j--;
        }
        if (j >= 0) cmps++;

        arr[j + 1] = key;
        sorted.push_back(i);

        StepSnapshot place;
        place.array = arr;
        place.cmp_a = j + 1;
        place.sorted_indices = sorted;
        place.comparisons = cmps; place.swaps = swps;
        place.description = "Inserted " + std::to_string(key) + " at index " + std::to_string(j+1);
        place.pseudocode_line = 4;
        steps.push_back(place);
    }

    sorted.clear();
    for (int i = 0; i < n; i++) sorted.push_back(i);
    StepSnapshot fin;
    fin.array = arr;
    fin.sorted_indices = sorted;
    fin.comparisons = cmps; fin.swaps = swps;
    fin.description = "Sorting complete!";
    fin.pseudocode_line = -1;
    steps.push_back(fin);
}

std::string InsertionSort::getName() const { return "Insertion Sort"; }

std::string InsertionSort::getTheory() const {
    return
        "INSERTION SORT\n"
        "==============\n\n"
        "WHAT IS IT?\n"
        "  Insertion Sort builds the sorted array one element at a time.\n"
        "  It picks each element and inserts it into its correct position\n"
        "  among the already-sorted elements to its left.\n\n"
        "HOW IT WORKS:\n"
        "  1. Start from the second element (index 1)\n"
        "  2. Store it as 'key'\n"
        "  3. Compare key with elements to its left\n"
        "  4. Shift larger elements one position right\n"
        "  5. Insert key at the correct position\n"
        "  6. Repeat for all elements\n\n"
        "COMPLEXITY:\n"
        "  Best Case:    O(n)     — already sorted\n"
        "  Average Case: O(n^2)\n"
        "  Worst Case:   O(n^2)  — reverse sorted\n"
        "  Space:        O(1)    — in-place\n\n"
        "WHEN TO USE:\n"
        "  - Small datasets (fastest for n < 15)\n"
        "  - Nearly sorted data (adaptive — runs in O(n))\n"
        "  - Online sorting (can sort as data arrives)\n\n"
        "KEY INSIGHT:\n"
        "  Insertion Sort is adaptive: if the input is nearly sorted,\n"
        "  it runs in nearly linear time. It's often used as the base\n"
        "  case for hybrid algorithms like TimSort.\n";
}

std::vector<std::string> InsertionSort::getPseudoCode() const {
    return {
        "for i = 1 to n-1:",
        "  key = arr[i]",
        "  j = i - 1",
        "  while j >= 0 and arr[j] > key:",
        "    arr[j+1] = arr[j]",
        "    j = j - 1",
        "  arr[j+1] = key"
    };
}

std::string InsertionSort::getComplexity() const {
    return "Best: O(n)  |  Avg: O(n^2)  |  Worst: O(n^2)  |  Space: O(1)";
}

void MergeSort::merge(std::vector<int>& arr, int left, int mid, int right, int& cmps, int& swps) {
    std::vector<int> temp(arr.begin() + left, arr.begin() + right + 1);
    int i = 0, j = mid - left + 1, k = left;
    int leftSize = mid - left + 1;
    int rightSize = right - mid;

    while (i < leftSize && j < leftSize + rightSize) {
        cmps++;

        std::vector<int> sl, sr;
        for (int x = left; x <= mid; x++) sl.push_back(x);
        for (int x = mid + 1; x <= right; x++) sr.push_back(x);

        StepSnapshot cmp;
        cmp.array = arr;
        cmp.cmp_a = left + i;
        cmp.cmp_b = left + j;
        cmp.subarray_left = sl;
        cmp.subarray_right = sr;
        cmp.comparisons = cmps; cmp.swaps = swps;
        cmp.description = "Merging: comparing " + std::to_string(temp[i]) + " and " + std::to_string(temp[j]);
        cmp.pseudocode_line = 4;
        steps.push_back(cmp);

        if (temp[i] <= temp[j]) {
            arr[k] = temp[i];
            i++;
        } else {
            arr[k] = temp[j];
            j++;
        }
        swps++;
        k++;
    }

    while (i < leftSize) {
        arr[k] = temp[i];
        swps++;
        i++; k++;
    }
    while (j < leftSize + rightSize) {
        arr[k] = temp[j];
        swps++;
        j++; k++;
    }

    StepSnapshot merged;
    merged.array = arr;
    std::vector<int> sl, sr;
    for (int x = left; x <= mid; x++) sl.push_back(x);
    for (int x = mid + 1; x <= right; x++) sr.push_back(x);
    merged.subarray_left = sl;
    merged.subarray_right = sr;
    merged.comparisons = cmps; merged.swaps = swps;
    merged.description = "Merged subarray [" + std::to_string(left) + ".." + std::to_string(right) + "]";
    merged.pseudocode_line = 5;
    steps.push_back(merged);
}

void MergeSort::mergeSortHelper(std::vector<int>& arr, int left, int right, int& cmps, int& swps) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;

    StepSnapshot div;
    div.array = arr;
    std::vector<int> sl, sr;
    for (int x = left; x <= mid; x++) sl.push_back(x);
    for (int x = mid + 1; x <= right; x++) sr.push_back(x);
    div.subarray_left = sl;
    div.subarray_right = sr;
    div.comparisons = cmps; div.swaps = swps;
    div.description = "Dividing [" + std::to_string(left) + ".." + std::to_string(right) +
        "] at mid=" + std::to_string(mid);
    div.pseudocode_line = 1;
    steps.push_back(div);

    mergeSortHelper(arr, left, mid, cmps, swps);
    mergeSortHelper(arr, mid + 1, right, cmps, swps);
    merge(arr, left, mid, right, cmps, swps);
}

void MergeSort::init() {
    if (data.empty()) generateRandomArray();
    steps.clear();
    currentStepIdx = 0;

    std::vector<int> arr = data;
    int cmps = 0, swps = 0;

    StepSnapshot initial;
    initial.array = arr;
    initial.description = "Initial array — ready for Merge Sort";
    initial.pseudocode_line = -1;
    steps.push_back(initial);

    mergeSortHelper(arr, 0, (int)arr.size() - 1, cmps, swps);

    std::vector<int> sorted;
    for (int i = 0; i < (int)arr.size(); i++) sorted.push_back(i);
    StepSnapshot fin;
    fin.array = arr;
    fin.sorted_indices = sorted;
    fin.comparisons = cmps; fin.swaps = swps;
    fin.description = "Sorting complete!";
    fin.pseudocode_line = -1;
    steps.push_back(fin);
}

std::string MergeSort::getName() const { return "Merge Sort"; }

std::string MergeSort::getTheory() const {
    return
        "MERGE SORT\n"
        "==========\n\n"
        "WHAT IS IT?\n"
        "  Merge Sort is a divide-and-conquer algorithm invented by John von Neumann\n"
        "  in 1945. It divides the array in half, recursively sorts each half, then\n"
        "  merges the sorted halves back together.\n\n"
        "HOW IT WORKS:\n"
        "  1. Divide the array into two halves\n"
        "  2. Recursively sort the left half\n"
        "  3. Recursively sort the right half\n"
        "  4. Merge the two sorted halves into one\n\n"
        "THE MERGE STEP:\n"
        "  - Use two pointers (one for each half)\n"
        "  - Compare the elements at both pointers\n"
        "  - Copy the smaller element to the result\n"
        "  - Advance the pointer of the smaller element\n"
        "  - Copy remaining elements when one half is exhausted\n\n"
        "COMPLEXITY:\n"
        "  Best Case:    O(n log n)\n"
        "  Average Case: O(n log n)\n"
        "  Worst Case:   O(n log n)\n"
        "  Space:        O(n)  — requires auxiliary array\n\n"
        "WHEN TO USE:\n"
        "  - When guaranteed O(n log n) is needed\n"
        "  - Linked lists (can be done in O(1) space)\n"
        "  - External sorting (large datasets on disk)\n"
        "  - When stability matters\n\n"
        "KEY INSIGHT:\n"
        "  Merge Sort is stable and always runs in O(n log n),\n"
        "  but requires O(n) extra memory. This is the price\n"
        "  for guaranteed performance.\n";
}

std::vector<std::string> MergeSort::getPseudoCode() const {
    return {
        "mergeSort(arr, l, r):",
        "  if l >= r: return",
        "  mid = (l + r) / 2",
        "  mergeSort(arr, l, mid)",
        "  mergeSort(arr, mid+1, r)",
        "  merge(arr, l, mid, r)"
    };
}

std::string MergeSort::getComplexity() const {
    return "Best: O(n log n)  |  Avg: O(n log n)  |  Worst: O(n log n)  |  Space: O(n)";
}

int QuickSort::partition(std::vector<int>& arr, int low, int high, int& cmps, int& swps) {
    int pivot = arr[high];
    int i = low - 1;

    StepSnapshot ps;
    ps.array = arr;
    ps.pivot_idx = high;
    ps.comparisons = cmps; ps.swaps = swps;
    ps.description = "Pivot selected: arr[" + std::to_string(high) + "] = " + std::to_string(pivot);
    ps.pseudocode_line = 2;
    steps.push_back(ps);

    for (int j = low; j < high; j++) {
        cmps++;
        StepSnapshot cmp;
        cmp.array = arr;
        cmp.pivot_idx = high;
        cmp.cmp_a = j; cmp.cmp_b = high;
        cmp.comparisons = cmps; cmp.swaps = swps;
        cmp.description = "Comparing arr[" + std::to_string(j) + "]=" +
            std::to_string(arr[j]) + " with pivot=" + std::to_string(pivot);
        cmp.pseudocode_line = 3;
        steps.push_back(cmp);

        if (arr[j] < pivot) {
            i++;
            if (i != j) {
                std::swap(arr[i], arr[j]);
                swps++;
                StepSnapshot sw;
                sw.array = arr;
                sw.pivot_idx = high;
                sw.swap_a = i; sw.swap_b = j;
                sw.comparisons = cmps; sw.swaps = swps;
                sw.description = "arr[" + std::to_string(j) + "] < pivot, swapping arr[" +
                    std::to_string(i) + "] and arr[" + std::to_string(j) + "]";
                sw.pseudocode_line = 4;
                steps.push_back(sw);
            }
        }
    }

    std::swap(arr[i + 1], arr[high]);
    swps++;
    StepSnapshot pivotPlace;
    pivotPlace.array = arr;
    pivotPlace.swap_a = i + 1; pivotPlace.swap_b = high;
    pivotPlace.comparisons = cmps; pivotPlace.swaps = swps;
    pivotPlace.description = "Placing pivot at index " + std::to_string(i+1);
    pivotPlace.pseudocode_line = 5;
    steps.push_back(pivotPlace);

    return i + 1;
}

void QuickSort::quickSortHelper(std::vector<int>& arr, int low, int high, int& cmps, int& swps) {
    if (low < high) {
        int pi = partition(arr, low, high, cmps, swps);
        quickSortHelper(arr, low, pi - 1, cmps, swps);
        quickSortHelper(arr, pi + 1, high, cmps, swps);
    }
}

void QuickSort::init() {
    if (data.empty()) generateRandomArray();
    steps.clear();
    currentStepIdx = 0;

    std::vector<int> arr = data;
    int cmps = 0, swps = 0;

    StepSnapshot initial;
    initial.array = arr;
    initial.description = "Initial array — ready for Quick Sort";
    initial.pseudocode_line = -1;
    steps.push_back(initial);

    quickSortHelper(arr, 0, (int)arr.size() - 1, cmps, swps);

    std::vector<int> sorted;
    for (int i = 0; i < (int)arr.size(); i++) sorted.push_back(i);
    StepSnapshot fin;
    fin.array = arr;
    fin.sorted_indices = sorted;
    fin.comparisons = cmps; fin.swaps = swps;
    fin.description = "Sorting complete!";
    fin.pseudocode_line = -1;
    steps.push_back(fin);
}

std::string QuickSort::getName() const { return "Quick Sort"; }

std::string QuickSort::getTheory() const {
    return
        "QUICK SORT\n"
        "==========\n\n"
        "WHAT IS IT?\n"
        "  Quick Sort is a divide-and-conquer algorithm developed by Tony Hoare\n"
        "  in 1959. It picks a 'pivot' element and partitions the array around\n"
        "  it, placing smaller elements left and larger elements right.\n\n"
        "HOW IT WORKS:\n"
        "  1. Choose a pivot element (here: last element)\n"
        "  2. Partition: rearrange so elements < pivot are left, > are right\n"
        "  3. Recursively sort the left partition\n"
        "  4. Recursively sort the right partition\n\n"
        "PARTITION (Lomuto scheme):\n"
        "  - i = low - 1 (boundary of 'less than' region)\n"
        "  - For each element j from low to high-1:\n"
        "    - If arr[j] < pivot: increment i, swap arr[i] and arr[j]\n"
        "  - Finally swap pivot into position i+1\n\n"
        "COMPLEXITY:\n"
        "  Best Case:    O(n log n)\n"
        "  Average Case: O(n log n)\n"
        "  Worst Case:   O(n^2)  — already sorted input (without randomization)\n"
        "  Space:        O(log n) — recursion stack\n\n"
        "WHEN TO USE:\n"
        "  - General-purpose sorting (fastest in practice)\n"
        "  - In-place sorting with good cache performance\n"
        "  - When average case matters more than worst case\n\n"
        "KEY INSIGHT:\n"
        "  Quick Sort is typically 2-3x faster than Merge Sort in practice\n"
        "  due to better cache locality and smaller constant factors,\n"
        "  despite its worse worst-case complexity.\n";
}

std::vector<std::string> QuickSort::getPseudoCode() const {
    return {
        "quickSort(arr, low, high):",
        "  if low < high:",
        "    pivot = arr[high]",
        "    i = partition(arr, low, high)",
        "    quickSort(arr, low, i-1)",
        "    quickSort(arr, i+1, high)"
    };
}

std::string QuickSort::getComplexity() const {
    return "Best: O(n log n)  |  Avg: O(n log n)  |  Worst: O(n^2)  |  Space: O(log n)";
}

void HeapSort::heapify(std::vector<int>& arr, int n, int i, int& cmps, int& swps, std::vector<int>& sorted) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n) {
        cmps++;
        StepSnapshot cmp;
        cmp.array = arr;
        cmp.cmp_a = largest; cmp.cmp_b = left;
        cmp.sorted_indices = sorted;
        cmp.comparisons = cmps; cmp.swaps = swps;
        cmp.description = "Heapify: comparing node " + std::to_string(largest) + " (=" +
            std::to_string(arr[largest]) + ") with left child " + std::to_string(left) + " (=" +
            std::to_string(arr[left]) + ")";
        cmp.pseudocode_line = 2;
        steps.push_back(cmp);

        if (arr[left] > arr[largest])
            largest = left;
    }

    if (right < n) {
        cmps++;
        StepSnapshot cmp;
        cmp.array = arr;
        cmp.cmp_a = largest; cmp.cmp_b = right;
        cmp.sorted_indices = sorted;
        cmp.comparisons = cmps; cmp.swaps = swps;
        cmp.description = "Heapify: comparing node " + std::to_string(largest) + " (=" +
            std::to_string(arr[largest]) + ") with right child " + std::to_string(right) + " (=" +
            std::to_string(arr[right]) + ")";
        cmp.pseudocode_line = 3;
        steps.push_back(cmp);

        if (arr[right] > arr[largest])
            largest = right;
    }

    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        swps++;

        StepSnapshot sw;
        sw.array = arr;
        sw.swap_a = i; sw.swap_b = largest;
        sw.sorted_indices = sorted;
        sw.comparisons = cmps; sw.swaps = swps;
        sw.description = "Swapping node " + std::to_string(i) + " and " + std::to_string(largest) +
            " to maintain heap property";
        sw.pseudocode_line = 4;
        steps.push_back(sw);

        heapify(arr, n, largest, cmps, swps, sorted);
    }
}

void HeapSort::init() {
    if (data.empty()) generateRandomArray();
    steps.clear();
    currentStepIdx = 0;

    std::vector<int> arr = data;
    int n = (int)arr.size();
    int cmps = 0, swps = 0;
    std::vector<int> sorted;

    StepSnapshot initial;
    initial.array = arr;
    initial.description = "Initial array — building max heap";
    initial.pseudocode_line = -1;
    steps.push_back(initial);

    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i, cmps, swps, sorted);
    }

    StepSnapshot heapBuilt;
    heapBuilt.array = arr;
    heapBuilt.comparisons = cmps; heapBuilt.swaps = swps;
    heapBuilt.description = "Max heap built — now extracting elements";
    heapBuilt.pseudocode_line = 0;
    steps.push_back(heapBuilt);

    for (int i = n - 1; i > 0; i--) {
        std::swap(arr[0], arr[i]);
        swps++;
        sorted.push_back(i);

        StepSnapshot sw;
        sw.array = arr;
        sw.swap_a = 0; sw.swap_b = i;
        sw.sorted_indices = sorted;
        sw.comparisons = cmps; sw.swaps = swps;
        sw.description = "Extracting max: swapping root arr[0]=" + std::to_string(arr[i]) +
            " with arr[" + std::to_string(i) + "]=" + std::to_string(arr[0]);
        sw.pseudocode_line = 5;
        steps.push_back(sw);

        heapify(arr, i, 0, cmps, swps, sorted);
    }

    sorted.push_back(0);
    StepSnapshot fin;
    fin.array = arr;
    fin.sorted_indices = sorted;
    fin.comparisons = cmps; fin.swaps = swps;
    fin.description = "Sorting complete!";
    fin.pseudocode_line = -1;
    steps.push_back(fin);
}

std::string HeapSort::getName() const { return "Heap Sort"; }

std::string HeapSort::getTheory() const {
    return
        "HEAP SORT\n"
        "=========\n\n"
        "WHAT IS IT?\n"
        "  Heap Sort uses a binary heap data structure to sort elements.\n"
        "  It first builds a max-heap from the array, then repeatedly\n"
        "  extracts the maximum element and places it at the end.\n\n"
        "HOW IT WORKS:\n"
        "  1. Build a max-heap from the unsorted array\n"
        "     (parent >= children for all nodes)\n"
        "  2. The largest element is now at index 0 (root)\n"
        "  3. Swap root with the last unsorted element\n"
        "  4. Reduce heap size by 1 (last element is now sorted)\n"
        "  5. Heapify the root to restore heap property\n"
        "  6. Repeat until heap size is 1\n\n"
        "HEAPIFY:\n"
        "  - Compare parent with both children\n"
        "  - If a child is larger, swap parent with largest child\n"
        "  - Recurse down the swapped path\n\n"
        "COMPLEXITY:\n"
        "  Best Case:    O(n log n)\n"
        "  Average Case: O(n log n)\n"
        "  Worst Case:   O(n log n)\n"
        "  Space:        O(1)  — in-place\n\n"
        "WHEN TO USE:\n"
        "  - When guaranteed O(n log n) is needed without extra space\n"
        "  - Priority queue implementations\n"
        "  - Systems with limited memory\n\n"
        "KEY INSIGHT:\n"
        "  Heap Sort combines the best of Selection Sort (in-place)\n"
        "  and Merge Sort (O(n log n)), but has worse cache performance\n"
        "  than Quick Sort due to non-sequential memory access.\n";
}

std::vector<std::string> HeapSort::getPseudoCode() const {
    return {
        "buildMaxHeap(arr)",
        "for i = n-1 to 1:",
        "  swap(arr[0], arr[i])",
        "  heapify(arr, i, 0):",
        "    compare with left child",
        "    compare with right child",
        "    if child > parent: swap"
    };
}

std::string HeapSort::getComplexity() const {
    return "Best: O(n log n)  |  Avg: O(n log n)  |  Worst: O(n log n)  |  Space: O(1)";
}

