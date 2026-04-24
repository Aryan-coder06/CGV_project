#pragma once
#include <vector>
#include <string>

struct Cell {
    int   index;
    int   value;
    float r, g, b;
    bool  highlighted;
};

struct Edge {
    int from, to;
    bool traversed;
};

struct AlgoState {
    int  currentStep  = 0;
    int  totalSteps   = 0;
    std::vector<std::string> calcLines;
    std::string currentPixelInfo;
    int  comparisons  = 0;
    int  swaps        = 0;
    int  pseudoCodeLine = -1;
};

enum class VisMode { SORT_BARS, SEARCH_ARRAY, GRAPH };

class IAlgorithm {
public:
    virtual ~IAlgorithm() = default;

    virtual void init()            = 0;
    virtual void step()            = 0;
    virtual void stepK(int k)      = 0;
    virtual void runToCompletion() = 0;
    virtual void reset()           = 0;
    virtual void shuffle()         = 0;

    virtual bool isFinished()                    const = 0;
    virtual AlgoState getCurrentState()          const = 0;
    virtual std::vector<Cell> getCells()         const = 0;
    virtual std::vector<Edge> getEdges()         const = 0;
    virtual std::string getTheory()              const = 0;
    virtual std::string getName()                const = 0;
    virtual VisMode getVisMode()                 const = 0;
    virtual std::vector<std::string> getPseudoCode() const = 0;
    virtual std::string getComplexity()          const = 0;
    virtual int getTargetValue()                 const { return -1; }
    virtual int getArraySize()                   const { return 0; }
    virtual void setArraySize(int n)                   {}
    virtual void setSpeedMultiplier(float s)           {}
    virtual void setCustomArray(const std::vector<int>& arr) {}
    virtual void setTargetValue(int t)                 {}
    virtual void setStartNode(int n)                   {}
};

