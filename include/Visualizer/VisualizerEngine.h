#pragma once
#include "IAlgorithm.h"
#include "../DSA/SortAlgos.h"
#include "../DSA/SearchAlgos.h"
#include "../DSA/GraphAlgos.h"
#include <memory>
#include <string>
#include <vector>

enum class Category { SORT, SEARCH, GRAPH };

class VisualizerEngine {
private:
    std::unique_ptr<IAlgorithm> currentAlgo;
    Category currentCategory = Category::SORT;
    int selectedAlgoInCategory = 0;

    bool isPlaying = false;
    float speedMultiplier = 1.0f;
    float stepTimer = 0.0f;
    float stepInterval = 0.4f;

    int arraySize = 30;
    int searchTarget = 42;
    int graphStartNode = 0;

    int activeTab = 1;

    char customArrayBuf[512] = "";
    int  customSearchTarget = 42;

    std::vector<float> displayedHeights;

    float elapsedTime = 0.0f;
    float lastFrameTime = 0.0f;

    std::unique_ptr<IAlgorithm> createAlgorithm(Category cat, int idx);
    void selectAlgorithm(Category cat, int idx);

    void renderLeftPanel();
    void renderTheoryTab();
    void renderSortCanvas(int x, int y, int w, int h);
    void renderSearchCanvas(int x, int y, int w, int h);
    void renderGraphCanvas(int x, int y, int w, int h);
    void renderPseudoCode(const std::vector<std::string>& lines, int activeLine);
    void updatePlayback(float deltaTime);

    void drawGlowRect(float x, float y, float w, float h, float r, float g, float b);
    void drawGlowCircle(float cx, float cy, float radius, float r, float g, float b, int segments = 32);

public:
    VisualizerEngine();
    void renderUI();
    void renderGrid(int windowWidth, int windowHeight);
};

