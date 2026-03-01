#pragma once
#include <vector>
#include <cstring>
#include <string>
#include <algorithm>
#include <time.h>
#include <map>
#include <unordered_map>
#include <deque>
#include <stdio.h>
#include <queue>

namespace htwis {

#define INF 2147483647
using namespace std;

struct Vertex {
        Vertex() {}
        bool del = 0;
        int weight = 0;
        int neiSum = 0;
        int neiSize = 0;
        vector<int> adjacent;
};

typedef vector<int> VI;
typedef unsigned int ui;

class Graph {
       public:
        int total_weight = 0;
        Graph();
        Graph(int vertexNum);
        ~Graph();
        int n, m;

        // authors of red2pack: head is now a member of the Graph class instead of a global pointer
        Vertex* head = nullptr;

        void readGraph(string dir);
        void outputGraph();
        void htThree();
        void htThreeAll();
        void dtThreeAll();
        void wtThreeAll();

        void htPvertex();
        void htDegreeOne();
        void htDegreeTwo();
        void htLowDeg();
        void htNeighborhood();
        void htCommon();
        void bfs();

        void deleteVertex(int id, int par, VI& pone, VI& dominate, VI& degree_one, VI& degree_two);
        void deleteVertex(int id, int par, VI& pone, VI& degree_one, VI& degree_two);
        void deleteVertex(int v, VI& pone, VI& dominate, VI& degree_one, VI& degree_two);
        void deleteVertex(int v, VI& pone, VI& degree_one, VI& degree_two);

        void deleteVertex(int u, int* tri, int* edges, bool* adj, bool* dominate, VI& pone, VI& degree_one,
                          VI& degree_two, VI& dominated);
        bool isNei(int v1, int v2);
        int commonWeight(VI& a, VI& b);
        int commonWeight(VI& a, VI& b, VI& common);
        void findTwoNeibor(int v, int& n1, int& n2);

        int oneEdge(vector<int>& a);
        int twoEdge(vector<int>& a);
        int threeEdge(vector<int>& a);
        int edgeHandler(vector<int>& list, int len);

        void bfsReduction(VI& pone, VI& degree_one, VI& degree_two, bool* adj);
        void common_neighbor_reduction(VI& pone, VI& degree_one, VI& degree_two, vector<pair<int, int>>& backTrack,
                                       vector<vector<int>>& circles, bool* adj);
        void neighborhood_reduction(VI& pone, VI& degree_one, VI& degree_two, vector<pair<int, int>>& backTrack,
                                    vector<vector<int>>& circles, bool* adj);
        void symmetric_folding(VI& pone, VI& degree_one, VI& degree_two, vector<pair<int, int>>& backTrack,
                               vector<vector<int>>& circles, bool* adj, int* AdjShare);
        void initial_reduction(VI& pone, VI& degree_one, VI& degree_two, vector<pair<int, int>>& backTrack,
                               vector<vector<int>>& circles, bool* adj, int* AdjShare);
        void compute_triangles(int* tri, int* edges, int* AdjShare, bool* adj, bool* dominate, VI& pone, VI& degree_one,
                               VI& degree_two, VI& dominated, vector<pair<int, int>>& backTrack,
                               vector<vector<int>>& circles);
        void clearPone(VI& pone, VI& degree_one, VI& degree_two, vector<pair<int, int>>& backTrack,
                       vector<vector<int>>& circles, VI& dominated, int* edges, int* tri, bool* dominate, bool* adj);
        void clearPone(bool* adj, VI& pone, VI& degree_one, VI& degree_two, vector<pair<int, int>>& backTrack,
                       vector<vector<int>>& circles, bool key);
        void clearsingle(bool* adj, VI& pone, VI& degree_one, VI& degree_two);
        int path_reduction(int* dp, int start, int end);
        void path_recover(int* V, int num);
};

}