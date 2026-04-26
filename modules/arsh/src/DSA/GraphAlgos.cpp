#include "../../include/DSA/GraphAlgos.h"
#include <queue>
#include <stack>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <functional>

GraphAlgoBase::GraphAlgoBase() {
    buildGraph();
    computeNodePositions();
}

void GraphAlgoBase::buildGraph() {
    adj.clear();
    adj.resize(numNodes);
    edgeList.clear();

    auto addEdge = [&](int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
        edgeList.push_back({u, v});
    };

    addEdge(0, 1);
    addEdge(0, 2);
    addEdge(0, 3);
    addEdge(1, 4);
    addEdge(1, 5);
    addEdge(2, 5);
    addEdge(2, 6);
    addEdge(3, 6);
    addEdge(3, 7);
    addEdge(4, 8);
    addEdge(5, 8);
    addEdge(6, 9);
    addEdge(7, 9);
    addEdge(8, 9);

    for (auto& nbrs : adj) {
        std::sort(nbrs.begin(), nbrs.end());
    }
}

void GraphAlgoBase::computeNodePositions() {
    nodePositions.resize(numNodes);
    const float PI = 3.14159265f;

    nodePositions[0] = {0.5f, 0.5f};

    for (int i = 0; i < 6; i++) {
        float angle = -PI / 2.0f + i * (2.0f * PI / 6.0f);
        nodePositions[1 + i] = {0.5f + 0.32f * cosf(angle), 0.5f + 0.32f * sinf(angle)};
    }

    for (int i = 0; i < 3; i++) {
        float angle = -PI / 2.0f + i * (2.0f * PI / 3.0f);
        nodePositions[7 + i] = {0.5f + 0.44f * cosf(angle), 0.5f + 0.44f * sinf(angle)};
    }
}

void GraphAlgoBase::step() {
    if (currentStepIdx < (int)steps.size() - 1)
        currentStepIdx++;
}

void GraphAlgoBase::stepK(int k) {
    for (int i = 0; i < k && currentStepIdx < (int)steps.size() - 1; i++)
        currentStepIdx++;
}

void GraphAlgoBase::runToCompletion() {
    currentStepIdx = (int)steps.size() - 1;
}

void GraphAlgoBase::reset() {
    currentStepIdx = 0;
}

void GraphAlgoBase::shuffle() {
    steps.clear();
    currentStepIdx = 0;
    init();
}

bool GraphAlgoBase::isFinished() const {
    return currentStepIdx >= (int)steps.size() - 1;
}

AlgoState GraphAlgoBase::getCurrentState() const {
    AlgoState st;
    if (steps.empty()) return st;
    const auto& snap = steps[currentStepIdx];
    st.currentStep = currentStepIdx;
    st.totalSteps = (int)steps.size() - 1;
    st.pseudoCodeLine = snap.pseudocode_line;
    st.calcLines.push_back(snap.description);

    if (!snap.queue_or_stack.empty()) {
        std::string contents = "Contents: [";
        for (int i = 0; i < (int)snap.queue_or_stack.size(); i++) {
            if (i > 0) contents += ", ";
            contents += std::to_string(snap.queue_or_stack[i]);
        }
        contents += "]";
        st.calcLines.push_back(contents);
    }

    std::string vis = "Visited: {";
    bool first = true;
    for (int i = 0; i < (int)snap.visited.size(); i++) {
        if (snap.visited[i]) {
            if (!first) vis += ", ";
            vis += std::to_string(i);
            first = false;
        }
    }
    vis += "}";
    st.calcLines.push_back(vis);

    st.currentPixelInfo = "Step " + std::to_string(currentStepIdx) + " / " + std::to_string(st.totalSteps);
    return st;
}

std::vector<Cell> GraphAlgoBase::getCells() const {
    std::vector<Cell> cells;
    if (steps.empty()) return cells;
    const auto& snap = steps[currentStepIdx];
    for (int i = 0; i < numNodes; i++) {
        Cell c;
        c.index = i;
        c.value = i;
        c.highlighted = false;

        bool inQueue = false;
        for (int q : snap.queue_or_stack) {
            if (q == i) { inQueue = true; break; }
        }

        if (snap.current_node == i) {
            c.r = 0.0f; c.g = 0.85f; c.b = 1.0f;
            c.highlighted = true;
        } else if (i < (int)snap.visited.size() && snap.visited[i]) {
            c.r = 0.45f; c.g = 0.05f; c.b = 0.75f;
        } else if (inQueue) {
            c.r = 1.0f; c.g = 0.55f; c.b = 0.0f;
        } else {
            c.r = 0.10f; c.g = 0.13f; c.b = 0.20f;
        }
        cells.push_back(c);
    }
    return cells;
}

std::vector<Edge> GraphAlgoBase::getEdges() const {
    std::vector<Edge> edges;
    if (steps.empty()) return edges;
    const auto& snap = steps[currentStepIdx];

    for (const auto& ep : edgeList) {
        Edge e;
        e.from = ep.first;
        e.to = ep.second;
        e.traversed = false;

        for (const auto& te : snap.traversed_edges) {
            if ((te.first == ep.first && te.second == ep.second) ||
                (te.first == ep.second && te.second == ep.first)) {
                e.traversed = true;
                break;
            }
        }

        if ((snap.edge_from == ep.first && snap.edge_to == ep.second) ||
            (snap.edge_from == ep.second && snap.edge_to == ep.first)) {
            e.traversed = true;
        }

        edges.push_back(e);
    }
    return edges;
}

VisMode GraphAlgoBase::getVisMode() const {
    return VisMode::GRAPH;
}

int GraphAlgoBase::getArraySize() const {
    return numNodes;
}

void GraphAlgoBase::setStartNode(int n) {
    if (n >= 0 && n < numNodes) {
        startNode = n;
        steps.clear();
        currentStepIdx = 0;
        init();
    }
}

void BFS::init() {
    steps.clear();
    currentStepIdx = 0;
    buildGraph();
    computeNodePositions();

    std::vector<bool> visited(numNodes, false);
    std::queue<int> q;
    std::vector<std::pair<int,int>> traversedEdges;

    GraphStep initial;
    initial.visited.resize(numNodes, false);
    initial.description = "BFS starting from node " + std::to_string(startNode);
    initial.pseudocode_line = 0;
    steps.push_back(initial);

    visited[startNode] = true;
    q.push(startNode);

    GraphStep startStep;
    startStep.visited = visited;
    startStep.current_node = startNode;
    {
        std::queue<int> tmp = q;
        while (!tmp.empty()) { startStep.queue_or_stack.push_back(tmp.front()); tmp.pop(); }
    }
    startStep.traversed_edges = traversedEdges;
    startStep.description = "Enqueue node " + std::to_string(startNode) + " (start node)";
    startStep.pseudocode_line = 1;
    steps.push_back(startStep);

    while (!q.empty()) {
        int node = q.front();
        q.pop();

        GraphStep proc;
        proc.visited = visited;
        proc.current_node = node;
        {
            std::queue<int> tmp = q;
            while (!tmp.empty()) { proc.queue_or_stack.push_back(tmp.front()); tmp.pop(); }
        }
        proc.traversed_edges = traversedEdges;
        proc.description = "Dequeue and process node " + std::to_string(node);
        proc.pseudocode_line = 2;
        steps.push_back(proc);

        for (int neighbor : adj[node]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
                traversedEdges.push_back({node, neighbor});

                GraphStep nb;
                nb.visited = visited;
                nb.current_node = node;
                nb.edge_from = node;
                nb.edge_to = neighbor;
                {
                    std::queue<int> tmp = q;
                    while (!tmp.empty()) { nb.queue_or_stack.push_back(tmp.front()); tmp.pop(); }
                }
                nb.traversed_edges = traversedEdges;
                nb.description = "Visit neighbor " + std::to_string(neighbor) +
                    " from node " + std::to_string(node) + " — enqueue";
                nb.pseudocode_line = 4;
                steps.push_back(nb);
            }
        }
    }

    GraphStep fin;
    fin.visited = visited;
    fin.traversed_edges = traversedEdges;
    fin.description = "BFS traversal complete! All reachable nodes visited.";
    fin.pseudocode_line = -1;
    steps.push_back(fin);
}

std::string BFS::getName() const { return "BFS (Breadth-First Search)"; }

std::string BFS::getTheory() const {
    return
        "BREADTH-FIRST SEARCH (BFS)\n"
        "==========================\n\n"
        "WHAT IS IT?\n"
        "  BFS explores a graph level by level, visiting all neighbors\n"
        "  of a node before moving to the next level. It uses a QUEUE\n"
        "  (FIFO) to track which node to visit next.\n\n"
        "HOW IT WORKS:\n"
        "  1. Start from the source node, mark as visited\n"
        "  2. Add it to the queue\n"
        "  3. While queue is not empty:\n"
        "     a. Dequeue a node\n"
        "     b. For each unvisited neighbor:\n"
        "        - Mark as visited\n"
        "        - Enqueue it\n\n"
        "PROPERTIES:\n"
        "  - Explores nodes in order of distance from source\n"
        "  - Finds shortest path (unweighted graphs)\n"
        "  - Complete: will find solution if one exists\n\n"
        "COMPLEXITY:\n"
        "  Time:  O(V + E)  — visits every vertex and edge\n"
        "  Space: O(V)      — queue can hold all vertices\n\n"
        "WHEN TO USE:\n"
        "  - Shortest path in unweighted graphs\n"
        "  - Level-order traversal\n"
        "  - Web crawling, social network analysis\n"
        "  - Maze solving (shortest path)\n\n"
        "KEY INSIGHT:\n"
        "  BFS guarantees the shortest path in unweighted graphs.\n"
        "  The order of exploration is like ripples spreading\n"
        "  outward from the source node.\n";
}

std::vector<std::string> BFS::getPseudoCode() const {
    return {
        "BFS(start):",
        "  enqueue(start), visited[start]=T",
        "  while queue not empty:",
        "    node = dequeue()",
        "    for neighbor in adj[node]:",
        "      if not visited[neighbor]:",
        "        visited[neighbor] = true",
        "        enqueue(neighbor)"
    };
}

std::string BFS::getComplexity() const {
    return "Time: O(V+E)  |  Space: O(V)";
}

void DFS::init() {
    steps.clear();
    currentStepIdx = 0;
    buildGraph();
    computeNodePositions();

    std::vector<bool> visited(numNodes, false);
    std::stack<int> stk;
    std::vector<std::pair<int,int>> traversedEdges;

    GraphStep initial;
    initial.visited.resize(numNodes, false);
    initial.description = "DFS starting from node " + std::to_string(startNode);
    initial.pseudocode_line = 0;
    steps.push_back(initial);

    stk.push(startNode);

    GraphStep startStep;
    startStep.visited = visited;
    {
        std::stack<int> tmp = stk;
        while (!tmp.empty()) { startStep.queue_or_stack.push_back(tmp.top()); tmp.pop(); }
    }
    startStep.traversed_edges = traversedEdges;
    startStep.description = "Push node " + std::to_string(startNode) + " onto stack";
    startStep.pseudocode_line = 1;
    steps.push_back(startStep);

    while (!stk.empty()) {
        int node = stk.top();
        stk.pop();

        if (visited[node]) continue;
        visited[node] = true;

        GraphStep proc;
        proc.visited = visited;
        proc.current_node = node;
        {
            std::stack<int> tmp = stk;
            while (!tmp.empty()) { proc.queue_or_stack.push_back(tmp.top()); tmp.pop(); }
        }
        proc.traversed_edges = traversedEdges;
        proc.description = "Pop and visit node " + std::to_string(node);
        proc.pseudocode_line = 3;
        steps.push_back(proc);

        for (int i = (int)adj[node].size() - 1; i >= 0; i--) {
            int neighbor = adj[node][i];
            if (!visited[neighbor]) {
                stk.push(neighbor);
                traversedEdges.push_back({node, neighbor});

                GraphStep nb;
                nb.visited = visited;
                nb.current_node = node;
                nb.edge_from = node;
                nb.edge_to = neighbor;
                {
                    std::stack<int> tmp = stk;
                    while (!tmp.empty()) { nb.queue_or_stack.push_back(tmp.top()); tmp.pop(); }
                }
                nb.traversed_edges = traversedEdges;
                nb.description = "Push neighbor " + std::to_string(neighbor) +
                    " from node " + std::to_string(node);
                nb.pseudocode_line = 5;
                steps.push_back(nb);
            }
        }
    }

    GraphStep fin;
    fin.visited = visited;
    fin.traversed_edges = traversedEdges;
    fin.description = "DFS traversal complete! All reachable nodes visited.";
    fin.pseudocode_line = -1;
    steps.push_back(fin);
}

std::string DFS::getName() const { return "DFS (Depth-First Search)"; }

std::string DFS::getTheory() const {
    return
        "DEPTH-FIRST SEARCH (DFS)\n"
        "========================\n\n"
        "WHAT IS IT?\n"
        "  DFS explores a graph by going as deep as possible along\n"
        "  each branch before backtracking. It uses a STACK (LIFO)\n"
        "  or recursion to track the path.\n\n"
        "HOW IT WORKS:\n"
        "  1. Start from the source node, push onto stack\n"
        "  2. While stack is not empty:\n"
        "     a. Pop a node\n"
        "     b. If not visited, mark as visited\n"
        "     c. Push all unvisited neighbors\n\n"
        "PROPERTIES:\n"
        "  - Explores deep paths before wide ones\n"
        "  - Uses less memory than BFS for wide graphs\n"
        "  - Does NOT find shortest path\n"
        "  - Can be implemented recursively\n\n"
        "COMPLEXITY:\n"
        "  Time:  O(V + E)  — visits every vertex and edge\n"
        "  Space: O(V)      — stack depth\n\n"
        "WHEN TO USE:\n"
        "  - Cycle detection\n"
        "  - Topological sorting\n"
        "  - Finding connected components\n"
        "  - Maze generation\n"
        "  - Puzzle solving\n\n"
        "KEY INSIGHT:\n"
        "  DFS is like exploring a maze: go as far as you can,\n"
        "  then backtrack when you hit a dead end. This makes\n"
        "  it ideal for problems requiring exhaustive search.\n";
}

std::vector<std::string> DFS::getPseudoCode() const {
    return {
        "DFS(start):",
        "  push(start)",
        "  while stack not empty:",
        "    node = pop()",
        "    if not visited[node]:",
        "      visited[node] = true",
        "      for neighbor in adj[node]:",
        "        push(neighbor)"
    };
}

std::string DFS::getComplexity() const {
    return "Time: O(V+E)  |  Space: O(V)";
}

void Dijkstra::init() {
    steps.clear();
    currentStepIdx = 0;
    buildGraph();
    computeNodePositions();

    std::vector<bool> visited(numNodes, false);
    std::vector<int> dist(numNodes, 1e9); // 1e9 represents Infinity
    
    // Priority queue: pair<distance, node>
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> pq;
    std::vector<std::pair<int,int>> traversedEdges;

    GraphStep initial;
    initial.visited.resize(numNodes, false);
    initial.description = "Dijkstra starting from node " + std::to_string(startNode);
    initial.pseudocode_line = 0;
    steps.push_back(initial);

    dist[startNode] = 0;
    pq.push({0, startNode});

    GraphStep startStep;
    startStep.visited = visited;
    startStep.current_node = startNode;
    {
        auto tmp = pq;
        while (!tmp.empty()) { startStep.queue_or_stack.push_back(tmp.top().second); tmp.pop(); }
    }
    startStep.traversed_edges = traversedEdges;
    startStep.description = "Set dist[" + std::to_string(startNode) + "] = 0, push to PQ";
    startStep.pseudocode_line = 1;
    steps.push_back(startStep);

    while (!pq.empty()) {
        int d = pq.top().first;
        int node = pq.top().second;
        pq.pop();

        if (visited[node]) continue;
        visited[node] = true;

        GraphStep proc;
        proc.visited = visited;
        proc.current_node = node;
        {
            auto tmp = pq;
            while (!tmp.empty()) { proc.queue_or_stack.push_back(tmp.top().second); tmp.pop(); }
        }
        proc.traversed_edges = traversedEdges;
        proc.description = "Pop node " + std::to_string(node) + " (dist: " + std::to_string(d) + ")";
        proc.pseudocode_line = 3;
        steps.push_back(proc);

        for (int neighbor : adj[node]) {
            int weight = 1; // Unweighted graph defaults to weight 1
            if (!visited[neighbor] && dist[node] + weight < dist[neighbor]) {
                dist[neighbor] = dist[node] + weight;
                pq.push({dist[neighbor], neighbor});
                traversedEdges.push_back({node, neighbor});

                GraphStep nb;
                nb.visited = visited;
                nb.current_node = node;
                nb.edge_from = node;
                nb.edge_to = neighbor;
                {
                    auto tmp = pq;
                    while (!tmp.empty()) { nb.queue_or_stack.push_back(tmp.top().second); tmp.pop(); }
                }
                nb.traversed_edges = traversedEdges;
                nb.description = "Relax edge to " + std::to_string(neighbor) + ", new dist=" + std::to_string(dist[neighbor]);
                nb.pseudocode_line = 7;
                steps.push_back(nb);
            }
        }
    }

    GraphStep fin;
    fin.visited = visited;
    fin.traversed_edges = traversedEdges;
    fin.description = "Dijkstra's algorithm complete! Shortest paths found.";
    fin.pseudocode_line = -1;
    steps.push_back(fin);
}

std::string Dijkstra::getName() const { return "Dijkstra's Algorithm"; }

std::string Dijkstra::getTheory() const {
    return
        "DIJKSTRA'S ALGORITHM\n"
        "====================\n\n"
        "WHAT IS IT?\n"
        "  An algorithm for finding the shortest paths between nodes\n"
        "  in a graph. It uses a PRIORITY QUEUE to greedily select\n"
        "  the closest unvisited node.\n\n"
        "HOW IT WORKS:\n"
        "  1. Assign tentative distances to all nodes (infinity).\n"
        "  2. Set the start node's distance to 0.\n"
        "  3. While unvisited nodes exist:\n"
        "     a. Select the unvisited node with the smallest distance.\n"
        "     b. For each neighbor, calculate the tentative distance.\n"
        "     c. If the new distance is smaller, update it.\n"
        "     d. Mark the current node as visited.\n\n"
        "PROPERTIES:\n"
        "  - Guarantees shortest path in graphs with non-negative weights.\n"
        "  - Fails if there are negative edge weights.\n\n"
        "COMPLEXITY:\n"
        "  Time:  O((V + E) log V) with a min-priority queue.\n"
        "  Space: O(V) for distances and queue.\n";
}

std::vector<std::string> Dijkstra::getPseudoCode() const {
    return {
        "Dijkstra(start):",
        "  dist[start] = 0, pq.push(start)",
        "  while pq is not empty:",
        "    node = pq.pop()",
        "    if visited[node] continue",
        "    visited[node] = true",
        "    for neighbor in adj[node]:",
        "      if dist[node] + weight < dist[neighbor]:",
        "        dist[neighbor] = dist[node] + weight",
        "        pq.push(neighbor)"
    };
}

std::string Dijkstra::getComplexity() const {
    return "Time: O((V+E)logV)  |  Space: O(V)";
}
void BellmanFord::init() {
    steps.clear(); currentStepIdx = 0; buildGraph();

    std::vector<int> dist(numNodes, 1e9);
    std::vector<bool> visited(numNodes, false);
    std::vector<std::pair<int,int>> traversedEdges; // 🔥 IMPORTANT

    dist[startNode] = 0;
    visited[startNode] = true;

    for (int i = 0; i < numNodes - 1; i++) {
        for (auto &e : edgeList) {
            int u = e.first, v = e.second;

            if (dist[u] + 1 < dist[v]) {
                dist[v] = dist[u] + 1;
                visited[v] = true;

                traversedEdges.push_back({u, v}); // 🔥 TRACK EDGE

                GraphStep s;
                s.visited = visited;
                s.traversed_edges = traversedEdges; // 🔥 IMPORTANT
                s.edge_from = u;
                s.edge_to = v;
                s.current_node = v;
                s.description = "Relax " + std::to_string(u) + " → " + std::to_string(v);
                steps.push_back(s);
            }
        }
    }

    GraphStep f;
    f.visited = visited;
    f.traversed_edges = traversedEdges; // 🔥 FINAL STATE FIX
    f.description = "Bellman-Ford complete";
    steps.push_back(f);
}

std::string BellmanFord::getName() const { return "Bellman-Ford"; }
std::string BellmanFord::getTheory() const {
    return
        "BELLMAN-FORD ALGORITHM\n"
        "======================\n\n"
        "WHAT IS IT?\n"
        "  Bellman-Ford computes the shortest paths from a source node\n"
        "  to all other nodes in a graph. Unlike Dijkstra, it works even\n"
        "  when edges have NEGATIVE weights.\n\n"
        "HOW IT WORKS:\n"
        "  1. Initialize distances: dist[source] = 0, others = infinity\n"
        "  2. Repeat V-1 times:\n"
        "     a. For every edge (u, v):\n"
        "        - If dist[u] + weight < dist[v]\n"
        "        - Update dist[v]\n"
        "  3. (Optional) Check for negative cycles\n\n"
        "PROPERTIES:\n"
        "  - Handles negative weights\n"
        "  - Can detect negative cycles\n"
        "  - Slower than Dijkstra\n\n"
        "COMPLEXITY:\n"
        "  Time:  O(V * E)\n"
        "  Space: O(V)\n\n"
        "WHEN TO USE:\n"
        "  - Graphs with negative weights\n"
        "  - Currency arbitrage detection\n"
        "  - Network routing problems\n\n"
        "KEY INSIGHT:\n"
        "  Repeated relaxation ensures shortest paths propagate\n"
        "  through the graph step by step.\n";
}
std::string BellmanFord::getComplexity() const { return "O(VE)"; }

void PrimMST::init() {
    steps.clear(); currentStepIdx = 0; buildGraph();

    std::vector<bool> inMST(numNodes,false);
    std::vector<std::pair<int,int>> traversedEdges; // 🔥 IMPORTANT

    using P = std::pair<int,std::pair<int,int>>;
    std::priority_queue<P,std::vector<P>,std::greater<P>> pq;

    pq.push({0,{startNode,-1}});

    while(!pq.empty()){
        auto top = pq.top(); pq.pop();
        int node = top.second.first;
        int parent = top.second.second;

        if(inMST[node]) continue;
        inMST[node]=true;

        GraphStep s;
        s.visited = inMST;
        s.current_node = node;

        if(parent!=-1){
            traversedEdges.push_back({parent,node}); // 🔥 TRACK EDGE

            s.edge_from=parent;
            s.edge_to=node;
            s.description="Add "+std::to_string(parent)+"-"+std::to_string(node);
        } else {
            s.description="Start at node "+std::to_string(node);
        }

        s.traversed_edges = traversedEdges; // 🔥 IMPORTANT
        steps.push_back(s);

        for(int nbr:adj[node])
            if(!inMST[nbr]) pq.push({1,{nbr,node}});
    }

    GraphStep f;
    f.visited = inMST;
    f.traversed_edges = traversedEdges; // 🔥 FINAL FIX
    f.description="Prim MST complete";
    steps.push_back(f);
}
std::string PrimMST::getName() const { return "Prim MST"; }
std::string PrimMST::getTheory() const {
    return
        "PRIM'S MINIMUM SPANNING TREE\n"
        "============================\n\n"
        "WHAT IS IT?\n"
        "  Prim’s algorithm finds a Minimum Spanning Tree (MST)\n"
        "  by growing a tree from a starting node, always picking\n"
        "  the smallest edge connecting to a new node.\n\n"
        "HOW IT WORKS:\n"
        "  1. Start from any node\n"
        "  2. Add it to the MST\n"
        "  3. Repeatedly:\n"
        "     a. Choose the smallest edge connecting MST to new node\n"
        "     b. Add that node to MST\n\n"
        "PROPERTIES:\n"
        "  - Greedy algorithm\n"
        "  - Builds MST incrementally\n"
        "  - Uses Priority Queue\n\n"
        "COMPLEXITY:\n"
        "  Time:  O(E log V)\n"
        "  Space: O(V)\n\n"
        "WHEN TO USE:\n"
        "  - Network design (roads, cables)\n"
        "  - Minimum wiring problems\n\n"
        "KEY INSIGHT:\n"
        "  Always expand the tree using the cheapest available edge.\n";
}
std::string PrimMST::getComplexity() const { return "O(E log V)"; }
void KruskalMST::init() {
    steps.clear(); currentStepIdx = 0; buildGraph();

    std::vector<int> parent(numNodes);
    std::vector<bool> visited(numNodes,false);
    std::vector<std::pair<int,int>> traversedEdges; // 🔥 IMPORTANT

    for(int i=0;i<numNodes;i++) parent[i]=i;

    std::function<int(int)> find = [&](int x){
        while(parent[x] != x) x = parent[x];
        return x;
    };

    auto unite = [&](int a,int b){
        parent[find(a)] = find(b);
    };

    for(auto &e:edgeList){
        int u=e.first,v=e.second;

        if(find(u)!=find(v)){
            unite(u,v);
            visited[u] = visited[v] = true;

            traversedEdges.push_back({u,v}); // 🔥 TRACK EDGE

            GraphStep s;
            s.visited = visited;
            s.traversed_edges = traversedEdges; // 🔥 IMPORTANT
            s.edge_from=u;
            s.edge_to=v;
            s.current_node=v;
            s.description="Add "+std::to_string(u)+"-"+std::to_string(v);
            steps.push_back(s);
        }
    }

    GraphStep f;
    f.visited = visited;
    f.traversed_edges = traversedEdges; // 🔥 FINAL FIX
    f.description="Kruskal MST complete";
    steps.push_back(f);
}
std::string KruskalMST::getName() const { return "Kruskal MST"; }
std::string KruskalMST::getTheory() const {
    return
        "KRUSKAL'S MINIMUM SPANNING TREE\n"
        "===============================\n\n"
        "WHAT IS IT?\n"
        "  Kruskal’s algorithm finds a Minimum Spanning Tree by\n"
        "  selecting edges in increasing order of weight while\n"
        "  avoiding cycles.\n\n"
        "HOW IT WORKS:\n"
        "  1. Sort all edges by weight\n"
        "  2. Initialize disjoint sets (Union-Find)\n"
        "  3. For each edge:\n"
        "     a. If it doesn't form a cycle\n"
        "     b. Add it to MST\n\n"
        "PROPERTIES:\n"
        "  - Greedy algorithm\n"
        "  - Uses Disjoint Set (DSU)\n"
        "  - Works well for sparse graphs\n\n"
        "COMPLEXITY:\n"
        "  Time:  O(E log E)\n"
        "  Space: O(V)\n\n"
        "WHEN TO USE:\n"
        "  - Sparse graphs\n"
        "  - Clustering problems\n\n"
        "KEY INSIGHT:\n"
        "  Always pick the smallest edge that does not create a cycle.\n";
}
std::string KruskalMST::getComplexity() const { return "O(E log E)"; }

std::vector<std::string> BellmanFord::getPseudoCode() const {
    return {
        "BellmanFord(start):",
        "  dist[start] = 0",
        "  repeat V-1 times:",
        "    for each edge (u,v):",
        "      if dist[u] + w < dist[v]:",
        "        dist[v] = dist[u] + w"
    };
}

std::vector<std::string> PrimMST::getPseudoCode() const {
    return {
        "Prim(start):",
        "  push(start)",
        "  while PQ not empty:",
        "    pick minimum edge",
        "    if not visited:",
        "      add to MST"
    };
}

std::vector<std::string> KruskalMST::getPseudoCode() const {
    return {
        "Kruskal():",
        "  sort edges",
        "  for each edge:",
        "    if no cycle:",
        "      add to MST"
    };
}