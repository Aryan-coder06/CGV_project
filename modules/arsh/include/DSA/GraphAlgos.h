#pragma once
#include "../Visualizer/IAlgorithm.h"
#include <vector>
#include <string>
#include <utility>

struct GraphStep {
    std::vector<bool> visited;
    std::vector<int>  queue_or_stack;
    int current_node = -1;
    int edge_from = -1, edge_to = -1;
    std::vector<std::pair<int,int>> traversed_edges;
    std::string description;
    int pseudocode_line = -1;
};

class GraphAlgoBase : public IAlgorithm {
protected:
    int numNodes = 10;
    int startNode = 0;
    std::vector<std::vector<int>> adj;
    std::vector<std::pair<int,int>> edgeList;
    std::vector<std::pair<float,float>> nodePositions;
    std::vector<GraphStep> steps;
    int currentStepIdx = 0;

    void buildGraph();
    void computeNodePositions();

public:
    GraphAlgoBase();
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
    void setStartNode(int n) override;

    const std::vector<std::pair<float,float>>& getNodePositions() const { return nodePositions; }
};

class BFS : public GraphAlgoBase {
public:
    void init() override;
    std::string getTheory() const override;
    std::string getName() const override;
    std::vector<std::string> getPseudoCode() const override;
    std::string getComplexity() const override;
};

class DFS : public GraphAlgoBase {
public:
    void init() override;
    std::string getTheory() const override;
    std::string getName() const override;
    std::vector<std::string> getPseudoCode() const override;
    std::string getComplexity() const override;
};

