#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#include "cstdio"
#include "Graph.h"


namespace htwis {

#define PII pair<int,int>
//#define MAX_WEIGHT 201
//#define iter
#define edge_id(a,b) (a > 0 ? (edges[a-1]+b) : b)

int factorial[] = {0, 0, 2, 6, 12, 20, 30, 42, 56, 72, 90};

Graph::Graph() {}

Graph::~Graph() { delete[] head; }

FILE* open_file(const char* file_name, const char* mode) {
        FILE* f = fopen(file_name, mode);
        if (f == NULL) {
                printf("Can not open file: %s\n", file_name);
                exit(1);
        }
        return f;
}

void Graph::readGraph(string dir) {
        clock_t start = clock();
        printf("Graph: %s\n", dir.c_str());
        FILE* f = open_file(("../Data/" + dir + ".gra").c_str(), "rb");

        int tt;
        fread(&tt, sizeof(int), 1, f);
        if (tt != (int)sizeof(int)) {
                printf("sizeof int is different: edge.bin(%d), machine(%d)\n", tt, (int)sizeof(int));
                return;
        }
        fread(&n, sizeof(int), 1, f);
        fread(&m, sizeof(int), 1, f);

        if (head == nullptr) head = new Vertex[n];
        for (int i = 0; i < n; i++) {
                Vertex a;
                head[i] = a;
        }
        printf("\tn = %u; m = %u (undirected)\n", n, m >> 1);

        ui* degree = new ui[n];
        fread(degree, sizeof(int), n, f);

        int* weight = new int[n];
        fread(weight, sizeof(int), n, f);

        int debug_weight[] = {10, 5, 6, 7, 5};
        for (ui i = 0; i < n; i++) {
                head[i].weight = weight[i];
                // head[i].weight = debug_weight[i];
        }

        delete[] weight;

        ui* buf = new ui[n];

        for (ui i = 0; i < n; i++) {
                if (degree[i] > 0) fread(buf, sizeof(int), degree[i], f);
                // sort(buf, buf + degree[i]);
                head[i].neiSize = degree[i];
                for (ui j = 0; j < degree[i]; j++) {
                        head[i].adjacent.push_back(buf[j]);
                        if (buf[j] == i) printf("Self-loop!\n");
                }
        }

        // debug
        /*n = 5, m = 8;
        int degree_d[] = {4,1,1,1,1};
        int edge[] = {1,2,3,4,0,0,0,0};
        int w[] = {10,2,2,3,1};
        for (int i = 0, j = 0; i < n; i++) {
                int d = degree_d[i];
                head[i].weight = w[i];
                for (int k = 0; k < d; k++) {
                        head[i].adjacent.push_back(edge[j++]);
                }
        }*/

        delete[] buf;
        buf = NULL;
        fclose(f);
        delete[] degree;
        printf("graph reading done...\n");
        clock_t end = clock();
        printf("time to read graph: %fms\n", (float)(end - start) * 1000 / CLOCKS_PER_SEC);
}

/*

*/
void Graph::bfsReduction(VI& pone, VI& degree_one, VI& degree_two, bool* adj) {
        // count sorting;
        int MAX_WEIGHT = 0;
        for (int i = 0; i < n; i++)
                if (!head[i].del && head[i].neiSize > 0) {
                        if (head[i].weight > MAX_WEIGHT) {
                                MAX_WEIGHT = head[i].weight;
                        }
                }
        MAX_WEIGHT += 1;
        int* count = new int[MAX_WEIGHT];
        memset(count, 0, sizeof(int) * MAX_WEIGHT);

        for (int i = 0; i < n; i++)
                if (!head[i].del && head[i].neiSize > 0) {
                        count[head[i].weight]++;
                }
        for (int i = 1; i < MAX_WEIGHT; i++) {
                count[i] += count[i - 1];
        }

        int survival = count[MAX_WEIGHT - 1];
        int* order = new int[n];
        for (int i = 0; i < n; i++)
                if (!head[i].del && head[i].neiSize > 0) {
                        order[i] = count[head[i].weight];
                        count[head[i].weight]--;
                }

        delete[] count;

        int* idx = new int[survival + 1];
        /*memset(idx, -1, sizeof(int) * (survival + 1));*/
        for (int i = 0; i < n; i++)
                if (!head[i].del && head[i].neiSize > 0) {
                        idx[order[i]] = i;
                }
        delete[] order;

        bool* visited = new bool[n];
        memset(visited, 0, sizeof(bool) * n);

        bool* tag = new bool[n];
        memset(tag, 0, sizeof(bool) * n);

        for (int k = survival; k > 0; k--) {
                int i = idx[k];
                //
                if (tag[i] || head[i].del) continue;
                for (auto n : head[i].adjacent)
                        if (!head[n].del) {
                                adj[n] = 1;
                                tag[n] = 1;
                        }
                VI list;
                queue<int> Q;
                int wgt = 0;
                for (auto n : head[i].adjacent)
                        if (!head[n].del) {
                                Q.push(n);
                                // visited[n] = 1;
                                while (!Q.empty()) {
                                        int h = Q.front();
                                        Q.pop();
                                        for (auto nn : head[h].adjacent)
                                                if (adj[nn] && !visited[nn]) {
                                                        Q.push(nn);
                                                }
                                        if (!visited[h]) {
                                                bool ok = 0;
                                                visited[h] = 1;
                                                for (auto nn : head[h].adjacent)
                                                        if (adj[nn] && !visited[nn]) {
                                                                visited[nn] = 1;
                                                                ok = 1;
                                                                list.push_back(h);
                                                                list.push_back(nn);
                                                                if (list.size() == 6) break;
                                                        }
                                                if (ok) {
                                                        wgt += edgeHandler(list, list.size());
                                                        list.clear();
                                                } else {
                                                        wgt += head[h].weight;
                                                }
                                                if (wgt > head[i].weight) break;
                                        }
                                }
                                if (wgt > head[i].weight) break;
                        }
                for (auto n : head[i].adjacent)
                        if (!head[n].del) {
                                adj[n] = 0;
                                visited[n] = 0;
                        }

                if (wgt <= head[i].weight) {
                        // printf("%d %d %d\n", i, head[i].weight, wgt);
                        for (auto n : head[i].adjacent)
                                if (!head[n].del) {
                                        deleteVertex(n, pone, degree_one, degree_two);
                                }
                }
        }
        delete[] visited;
        delete[] idx;
        delete[] tag;
        // delete[] count;
        // delete[] order;
}

// void htTwo();
// void Graph::htSingle() {
//	clock_t start = clock();
//	// ����ÿ�����ھ�Ȩֵ�͡�
//
//	long long int max_diff = 0;
//	for (int i = 0; i < n; i++) {
//		int sum = 0;
//		for (auto v : head[i].adjacent) {
//			sum += head[v].weight;
//		}
//		head[i].neiSum = sum;
//
//		if (sum - head[i].weight > max_diff) max_diff = sum - head[i].weight;
//	}
//	printf("max diff:%d \n", max_diff);
//	int  nn = max_diff + 1;
//	int* bin_head = new int[nn];
//	memset(bin_head, -1, sizeof(int) * (nn));
//
//	int* bin_next = new int[n];
//	memset(bin_next, -1, sizeof(int) * n);
//	for (int i = 0; i < n; i++) {
//		int dif = head[i].neiSum - head[i].weight;
//		if (dif <= 0) continue;
//		bin_next[i] = bin_head[dif];
//		bin_head[dif] = i;
//	}
//
//	vector<int> pone, S, lb_set;;
//	for (int i = 0; i < n; i++) {
//		if (head[i].weight >= head[i].neiSum) {
//			pone.push_back(i);
//		}
//	}
//
//
//	// ����upperbound������pone����Ӧ�ûص�pone���¼��㡣
//	while (1) {
//		while (!pone.empty()) {
//			int v = pone.back();
//			pone.pop_back();
//			if (head[v].del == 0 && head[v].weight >= head[v].neiSum) {
//				for (ui nn : head[v].adjacent) {
//					if (head[nn].del == 0) {
//						deleteVertex(nn, v, pone,dominate);
//					}
//				}
//			}
//		}
//		// ̰��ɾ��  dt-oritented.
//		while (pone.empty() || max_diff >= 0) {
//			while (max_diff >= 0 && bin_head[max_diff] == -1) max_diff--;
//			if (max_diff < 0) break;
//			int v = -1;
//			for (v = bin_head[max_diff]; v != -1;) {
//				int tmp = bin_next[v];
//				//int deg = head[v].neiSize;
//				int deg = head[v].neiSum - head[v].weight;
//				if (head[v].del == 0 && deg >= 0 && head[v].neiSize > 0) {
//					if (deg < max_diff) {
//						bin_next[v] = bin_head[deg];
//						bin_head[deg] = v;
//					}
//					else {
//						S.push_back(v);
//						deleteVertex(v, pone);
//						bin_head[max_diff] = tmp;
//						break;
//					}
//				}
//				v = tmp;
//			}
//			if (v == -1) bin_head[max_diff] = -1;
//		}
//		if (max_diff < 0) break;
//	}
//	// ���� induced graph
//	clock_t end = clock();
//	delete[] bin_head;
//	delete[] bin_next;
//	int total_weight = 0;
//	int remainder = 0;
//	int s_size = S.size();
//	printf("S size: %d\n", s_size);
//
//	for (int i = 0; i < n; i++) {
//		if (head[i].del == 0 && head[i].neiSize > 0) {
//			printf("WA exists.\n");
//			printf("%d %d\n", head[i].weight, head[i].neiSum);
//		}
//	}
//	for (int i = 0; i < s_size; i++) {
//		int v = S[i];
//		int ok = 1;
//
//		for (auto nn : head[v].adjacent) {
//			if (head[nn].del == 0) {
//				ok = 0;
//				break;
//			}
//		}
//		if (ok) {
//			head[v].del = 0;
//			head[v].neiSize = 0;
//		}
//	}
//	int mwis = 0;
//	for (int i = 0; i < n; i++) {
//		if (head[i].del == 0) {
//			if (head[i].neiSize == 0) {
//				mwis++;
//				total_weight += head[i].weight;
//			}
//			else {
//				remainder++;
//			}
//		}
//	}
//	printf("total weight :%d , mwis: %d , remainder : %d , total time cost : %f\n", total_weight, mwis, remainder, (float)(end - start) * 1000 / CLOCKS_PER_SEC);
// }

void Graph::htNeighborhood() {
        for (int i = 0; i < n; i++) {
                // clear the useless vertex
                if (head[i].weight <= 0) {
                        printf("Invalid weight 0\n");
                        return;
                }
        }
        clock_t start = clock();
        // ����ÿ�����ھ�Ȩֵ�͡�
        int* weight_copy = new int[n];
        for (int i = 0; i < n; i++) {
                weight_copy[i] = head[i].weight;
        }
        int max_d = 0;
        for (int i = 0; i < n; i++) {
                int sum = 0;
                for (auto v : head[i].adjacent) {
                        sum += head[v].weight;
                }
                head[i].neiSum = sum;
                head[i].neiSize = head[i].adjacent.size();
        }

        vector<int> pone, dominated, degree_two, degree_one;
        vector<pair<int, int>> backTrack;
        vector<vector<int>> circles;

        for (int i = 0; i < n; i++) {
                if (head[i].weight >= head[i].neiSum) {
                        pone.push_back(i);
                } else if (head[i].neiSize == 1) {
                        degree_one.push_back(i);
                } else if (head[i].neiSize == 2) {
                        degree_two.push_back(i);
                }
        }

        bool* adj = new bool[n];
        memset(adj, 0, sizeof(bool) * n);
        int* AdjShare = new int[n];
        memset(AdjShare, 0, sizeof(int) * n);

        // clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 1);

        neighborhood_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        // symmetric_folding(pone, degree_one, degree_two, backTrack, circles, adj, AdjShare);

        // common_neighbor_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        // bfsReduction(pone, degree_one, degree_two, adj);
        //// reorganize the ajacent matrix.
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0 && head[i].neiSize > 0) {
                        int* neis = new int[head[i].neiSize];
                        int index = 0;
                        for (auto n : head[i].adjacent) {
                                if (head[n].del == 0) {
                                        neis[index++] = n;
                                }
                        }
                        if (index != head[i].neiSize) printf("error info: index != neisize\n");
                        head[i].adjacent.resize(index);
                        for (int j = 0; j < index; j++) {
                                head[i].adjacent[j] = neis[j];
                        }
                        delete[] neis;
                }
        }

        // neighborhood_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        delete[] AdjShare;
        delete[] adj;

        int count = 0, d_two = 0;
        for (int k = 0; k < n; k++) {
                if (head[k].del == 0 && head[k].neiSize > 0) {
                        if (head[k].neiSum - head[k].weight > max_d) {
                                max_d = head[k].neiSum - head[k].weight;
                        }
                        if (head[k].neiSize == 2) d_two++;
                        count++;
                }
        }
        printf("degree_two_size: %d \nKernel Size:%d\n", d_two, count);

        int* bin_head = new int[max_d + 1];
        memset(bin_head, -1, sizeof(int) * (max_d + 1));
        int* bin_next = new int[n];
        for (int i = 0; i < n; i++) {
                if (head[i].del || head[i].neiSize < 1) continue;
                int dif = head[i].neiSum - head[i].weight;
                if (dif <= 0) continue;
                bin_next[i] = bin_head[dif];
                bin_head[dif] = i;
        }

        while (max_d > 0) {
                while (pone.empty() && degree_two.empty() && degree_one.empty()) {
                        while (max_d >= 0 && bin_head[max_d] == -1) max_d--;
                        if (max_d < 0) break;
                        int v = -1;
                        for (v = bin_head[max_d]; v != -1;) {
                                int tmp = bin_next[v];
                                int deg = head[v].neiSum - head[v].weight;
                                if (head[v].del == 0 && deg > 0) {
                                        if (deg < max_d) {
                                                bin_next[v] = bin_head[deg];
                                                bin_head[deg] = v;
                                        } else {
                                                deleteVertex(v, pone, degree_one, degree_two);
                                                backTrack.push_back(make_pair(v, n));
                                                bin_head[max_d] = tmp;
                                                break;
                                        }
                                }
                                v = tmp;
                        }
                        if (v == -1) bin_head[max_d] = -1;
                }
                // cannot add edge
                clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 0);
        }

        int greedy_include = 0, remainder = 0;
        for (int k = backTrack.size() - 1; k >= 0; k--) {
                int a = backTrack[k].first;
                bool ok = 1;
                if (a == n) {
                        int b = backTrack[k].second;
                        VI& T = circles.back();
                        int* tmp = nullptr;
                        int index = 0;
                        int T_size = T.size();
                        // attahed circle
                        if (b == 1) {
                                // if v
                                if (head[T[0]].del) {
                                        tmp = new int[T_size - 1];
                                        for (int i = 1; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                }
                        }
                        // circle ends with two adjacent vertex.
                        else if (b == 2) {
                                if (!head[T[0]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (!head[T[1]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 3; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (head[T[1]].del && head[T[0]].del) {
                                        tmp = new int[T_size - 2];
                                        for (int i = 2; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        printf("error info: T[0] and T[0] cann't exist at the same time\n");
                                }
                        }
                        path_recover(tmp, index);
                        delete[] tmp;
                        circles.pop_back();
                } else {
                        for (auto n : head[a].adjacent) {
                                if (head[n].del == 0) {
                                        ok = 0;
                                        break;
                                }
                        }
                        if (ok) {
                                head[a].del = 0;
                        }
                }
        }

        if (!circles.empty()) printf("error info: circles is not empty!\n");

        for (int i = 0; i < n; i++) {
                if (!head[i].del) total_weight += weight_copy[i];
        }

        clock_t end = clock();
        delete[] bin_head;
        delete[] bin_next;

        /*printf("solution:\n");
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0) {
                        printf("%d ", i);
                }
        }*/

        printf("total weight :%d ,  remainder : %d , total time cost : %f\n", total_weight, remainder,
               (float)(end - start) * 1000 / CLOCKS_PER_SEC);
        delete[] weight_copy;
}

void Graph::htPvertex() {
        // ��ʱ
        clock_t start = clock();
        // ����ÿ�����ھ�Ȩֵ�͡�
        int* weight_copy = new int[n];
        for (int i = 0; i < n; i++) {
                weight_copy[i] = head[i].weight;
        }
        int max_d = 0;
        for (int i = 0; i < n; i++) {
                int sum = 0;
                for (auto v : head[i].adjacent) {
                        sum += head[v].weight;
                }
                head[i].neiSum = sum;
                head[i].neiSize = head[i].adjacent.size();
        }

        vector<int> pone, dominated, degree_two, degree_one;
        vector<pair<int, int>> backTrack;
        vector<vector<int>> circles;

        for (int i = 0; i < n; i++) {
                if (head[i].weight >= head[i].neiSum) {
                        pone.push_back(i);
                }
                /*else if (head[i].neiSize == 1) {
                        degree_one.push_back(i);
                }
                else if (head[i].neiSize == 2) {
                        degree_two.push_back(i);
                }*/
        }

        bool* adj = new bool[n];
        memset(adj, 0, sizeof(bool) * n);
        int* AdjShare = new int[n];
        memset(AdjShare, 0, sizeof(int) * n);

        // clear the invalid vertice��
        for (int i = 0; i < n; i++) {
                // clear the useless vertex
                if (head[i].weight <= 0) deleteVertex(i, pone, degree_one, degree_two);
        }

        while (!pone.empty()) {
                int v = pone.back();
                pone.pop_back();
                if (head[v].del == 0 && head[v].weight >= head[v].neiSum && head[v].neiSize > 0) {
                        for (auto nn : head[v].adjacent) {
                                if (head[nn].del == 0) {
                                        deleteVertex(nn, v, pone, degree_one, degree_two);
                                }
                        }
                }
        }

        for (int i = 0; i < n; i++) {
                if (!head[i].del && head[i].neiSize > 0 && head[i].neiSum <= head[i].weight) printf("error condition");
        }

        VI vs;
        for (int i = 0; i < n; i++)
                if (!head[i].del && head[i].neiSize > 0) {
                        vs.push_back(i);
                }
        ui* ids = new ui[vs.size()];
        memset(ids, 0, sizeof(ui) * vs.size());
        for (ui i = 0; i < vs.size(); i++) ++ids[head[vs[i]].neiSize];
        for (ui i = 1; i < vs.size(); i++) ids[i] += ids[i - 1];

        int* order = new int[n];
        memset(order, -1, sizeof(int) * n);
        for (ui i = 0; i < vs.size(); i++) order[vs[i]] = (--ids[head[vs[i]].neiSize]);
        for (ui i = 0; i < vs.size(); i++) ids[order[vs[i]]] = vs[i];

        int* times = new int[n];
        memset(times, 0, sizeof(int) * n);

        for (int i = vs.size() - 1; i >= 0; i--) {
                int u = ids[i];
                if (head[u].del || head[u].neiSize <= 0) continue;
                for (auto n : head[u].adjacent)
                        if (head[n].del == 0) {
                                adj[n] = 1;
                        }
                adj[u] = 1;

                int dominate_sum = 0, edge_sum = 0;
                bool dominate = 0, two_hop_key = 0;
                vector<int> two_hop;
                for (auto x : head[u].adjacent)
                        if (!head[x].del /*&& head[x].neiSize <= head[u].neiSize*/) {
                                int tri_count = 0, nei_sum_of_x = 0;
                                bool dominate_key = 1;
                                for (auto nn : head[x].adjacent)
                                        if (!head[nn].del) {
                                                if (!adj[nn]) {
                                                        two_hop.push_back(nn);

                                                        AdjShare[nn] += head[x].weight;
                                                        if (head[nn].weight + head[u].weight >=
                                                            head[nn].neiSum + head[u].neiSum - AdjShare[nn]) {
                                                                // printf("run");
                                                                for (auto nei : head[u].adjacent) {
                                                                        deleteVertex(nei, u, pone, degree_one,
                                                                                     degree_two);
                                                                }
                                                                for (auto nei : head[nn].adjacent) {
                                                                        deleteVertex(nei, nn, pone, degree_one,
                                                                                     degree_two);
                                                                }
                                                                two_hop_key = 1;
                                                                break;
                                                        }
                                                        // check if nn and u are symmetry.
                                                        // if (head[u].neiSum - AdjShare[nn] <= head[u].weight/*AdjShare[nn] == head[u].neiSum */ && head[nn].neiSum - AdjShare[nn] <= head[nn].weight) {
                                                        //	printf("symmetric foldig\n");
                                                        //	// u and nn are bound to be in the MWIS together.
                                                        //	deleteVertex(u, pone, degree_one, degree_two);
                                                        //	//head[u].del = 1;

                                                        //	head[nn].weight += head[u].weight;
                                                        //	backTrack.push_back(make_pair(u, -1));
                                                        //	for (auto k : head[nn].adjacent) if (!head[k].del) {
                                                        //		//head[k].neiSize--;
                                                        //		if (adj[k]) {
                                                        //			if (head[k].neiSize == 1) {
                                                        //				degree_one.push_back(k);
                                                        //			}
                                                        //			else if (head[k].neiSize == 2) {
                                                        //				degree_two.push_back(k);
                                                        //			}
                                                        //			adj[k] = 0;
                                                        //		}
                                                        //		else {
                                                        //			head[k].neiSum += head[u].weight;
                                                        //		}
                                                        //	}
                                                        //	for (auto k : head[u].adjacent) if (!head[k].del) {
                                                        //		if (adj[k]) {
                                                        //			head[k].neiSum += head[nn].weight;
                                                        //			head[k].neiSize++;
                                                        //			head[k].adjacent.push_back(nn);

                                                        //			head[nn].neiSum += head[k].weight;
                                                        //			head[nn].neiSize++;
                                                        //			head[nn].adjacent.push_back(k);
                                                        //		}
                                                        //		head[k].neiSum += head[u].weight;
                                                        //	}
                                                        //	if (head[nn].weight >= head[nn].neiSum) {
                                                        //		pone.push_back(nn);
                                                        //	}
                                                        //	two_hop_key = 1;
                                                        //	break;

                                                        //}
                                                } else {
                                                        /*if (times[nn]) {
                                                                dominate_key = false;
                                                        }*/
                                                        if (nn != u) {
                                                                nei_sum_of_x += head[nn].weight;
                                                        }
                                                        ++tri_count;
                                                }
                                        }
                                if (two_hop_key) break;
                                // if (tri_count == head[x].neiSize) {
                                //	/*if (dominate_key) {
                                //		dominate_sum += head[x].weight;
                                //	}*/
                                //	if (head[x].weight >= head[u].weight /*|| dominate_sum >= head[u].weight*/) {
                                //		dominate = 1;
                                //		break;
                                //	}
                                //	else if (head[x].neiSum - head[u].weight <= head[x].weight) {
                                //		for (auto& nn : head[x].adjacent) if (!head[nn].del && nn != u) {
                                //			deleteVertex(nn, pone, degree_one, degree_two);
                                //		}
                                //	}
                                //	//times[x]++;
                                // }
                                /*else if (head[u].weight >= head[x].weight + head[u].neiSum - nei_sum_of_x) {
                                        deleteVertex(x, pone, degree_one, degree_two);
                                        edge_sum -= tri_count;
                                }
                                edge_sum += tri_count;*/
                        }
                for (auto v : two_hop) {
                        AdjShare[v] = 0;
                }

                for (auto n : head[u].adjacent)
                        if (head[n].del == 0) {
                                adj[n] = 0;
                                times[n] = 0;
                        }
                adj[u] = 0;
                if (two_hop_key) {
                        while (!pone.empty()) {
                                int v = pone.back();
                                pone.pop_back();
                                if (head[v].del == 0 && head[v].weight >= head[v].neiSum && head[v].neiSize > 0) {
                                        for (auto nn : head[v].adjacent) {
                                                if (head[nn].del == 0) {
                                                        deleteVertex(nn, v, pone, degree_one, degree_two);
                                                }
                                        }
                                }
                        }
                        continue;
                }
                //// dominate a vertex.
                // if (dominate) {
                //	deleteVertex(u, pone, degree_one, degree_two);

                //}
                // complete graphs
                while (!pone.empty()) {
                        int v = pone.back();
                        pone.pop_back();
                        if (head[v].del == 0 && head[v].weight >= head[v].neiSum && head[v].neiSize > 0) {
                                for (auto nn : head[v].adjacent) {
                                        if (head[nn].del == 0) {
                                                deleteVertex(nn, v, pone, degree_one, degree_two);
                                        }
                                }
                        }
                }
        }
        delete[] ids;
        delete[] order;
        for (int i = 0; i < n; i++) {
                if (!head[i].del && head[i].neiSize > 0 && head[i].neiSum <= head[i].weight) printf("error condition");
        }

        delete[] AdjShare;
        delete[] adj;

        int d_two = 0;

        int kk = 0;
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0 && head[i].neiSize > 0) {
                        for (auto n : head[i].adjacent) {
                                if (head[n].del == 0) {
                                        if ((head[i].weight >= head[i].neiSum - head[n].weight) ||
                                            (head[n].weight >= head[n].neiSum - head[i].weight)) {
                                                kk++;
                                        }
                                }
                        }
                }
        }
        printf("kk:%d\n", kk);

        int count = 0;
        for (int k = 0; k < n; k++) {
                if (head[k].del == 0 && head[k].neiSize > 0) {
                        if (head[k].neiSum - head[k].weight > max_d) {
                                max_d = head[k].neiSum - head[k].weight;
                        }
                        if (head[k].neiSize == 2) d_two++;
                        count++;
                }
        }
        printf("degree_two_size: %d \nKernel Size:%d\n kr: %f", d_two, count, 1 - count * 1.0 / n);

        // outputGraph();

        int* bin_head = new int[max_d + 1];
        memset(bin_head, -1, sizeof(int) * (max_d + 1));
        int* bin_next = new int[n];
        for (int i = 0; i < n; i++) {
                if (head[i].del || head[i].neiSize < 1) continue;
                int dif = head[i].neiSum - head[i].weight;
                if (dif <= 0) continue;
                bin_next[i] = bin_head[dif];
                bin_head[dif] = i;
        }

        while (max_d > 0) {
                while (pone.empty()) {
                        while (max_d >= 0 && bin_head[max_d] == -1) max_d--;
                        if (max_d < 0) break;
                        int v = -1;
                        for (v = bin_head[max_d]; v != -1;) {
                                int tmp = bin_next[v];
                                int deg = head[v].neiSum - head[v].weight;
                                if (head[v].del == 0 && deg > 0) {
                                        if (deg < max_d) {
                                                bin_next[v] = bin_head[deg];
                                                bin_head[deg] = v;
                                        } else {
                                                deleteVertex(v, pone, degree_one, degree_two);
                                                backTrack.push_back(make_pair(v, n));
                                                bin_head[max_d] = tmp;
                                                break;
                                        }
                                }
                                v = tmp;
                        }
                        if (v == -1) bin_head[max_d] = -1;
                }
                // cannot add edge
                while (!pone.empty()) {
                        int v = pone.back();
                        pone.pop_back();
                        if (head[v].del == 0 && head[v].weight >= head[v].neiSum && head[v].neiSize > 0) {
                                for (auto nn : head[v].adjacent) {
                                        if (head[nn].del == 0) {
                                                deleteVertex(nn, v, pone, degree_one, degree_two);
                                        }
                                }
                        }
                }
        }

        int greedy_include = 0, remainder = 0;
        for (int k = backTrack.size() - 1; k >= 0; k--) {
                int a = backTrack[k].first;
                bool ok = 1;
                if (a == n) {
                        int b = backTrack[k].second;
                        VI& T = circles.back();
                        int* tmp = nullptr;
                        int index = 0;
                        int T_size = T.size();
                        // attahed circle
                        if (b == 1) {
                                // if v
                                if (head[T[0]].del) {
                                        tmp = new int[T_size - 1];
                                        for (int i = 1; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                }
                        }
                        // circle ends with two adjacent vertex.
                        else if (b == 2) {
                                if (!head[T[0]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (!head[T[1]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 3; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (head[T[1]].del && head[T[0]].del) {
                                        tmp = new int[T_size - 2];
                                        for (int i = 2; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        printf("error info: T[0] and T[0] cann't exist at the same time\n");
                                }
                        }
                        path_recover(tmp, index);
                        delete[] tmp;
                        circles.pop_back();
                } else {
                        for (auto n : head[a].adjacent) {
                                if (head[n].del == 0) {
                                        ok = 0;
                                        break;
                                }
                        }
                        if (ok) {
                                head[a].del = 0;
                        }
                }
        }

        if (!circles.empty()) printf("error info: circles is not empty!\n");
        for (int i = 0; i < n; i++) {
                if (!head[i].del) total_weight += weight_copy[i];
        }

        clock_t end = clock();
        delete[] bin_head;
        delete[] bin_next;

        /*printf("solution:\n");
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0) {
                        printf("%d ", i);
                }
        }*/

        printf("total weight :%d ,  remainder : %d , total time cost : %f\n", total_weight, remainder,
               (float)(end - start) * 1000 / CLOCKS_PER_SEC);
        delete[] weight_copy;
}

void Graph::clearsingle(bool* adj, VI& pone, VI& degree_one, VI& degree_two) {
        while (!pone.empty()) {
                int v = pone.back();
                pone.pop_back();
                if (head[v].del == 0 && head[v].weight >= head[v].neiSum && head[v].neiSize > 0) {
                        for (auto nn : head[v].adjacent) {
                                if (head[nn].del == 0) {
                                        deleteVertex(nn, v, pone, degree_one, degree_two);
                                }
                        }
                }
        }
}

void Graph::clearPone(bool* adj, VI& pone, VI& degree_one, VI& degree_two, vector<pair<int, int>>& backTrack,
                      vector<vector<int>>& circles, bool key) {
        while (!pone.empty() || !degree_one.empty() || !degree_two.empty()) {
                while (!pone.empty() || !degree_one.empty()) {
                        while (!pone.empty()) {
                                int v = pone.back();
                                pone.pop_back();
                                if (head[v].del == 0 && head[v].weight >= head[v].neiSum && head[v].neiSize > 0) {
                                        for (auto nn : head[v].adjacent) {
                                                if (head[nn].del == 0) {
                                                        deleteVertex(nn, v, pone, degree_one, degree_two);
                                                }
                                        }
                                }
                        }

                        while (!degree_one.empty() && pone.empty()) {
                                int v = degree_one.back();
                                degree_one.pop_back();
                                if (head[v].neiSize != 1 || head[v].del) {
                                        continue;
                                }
                                int neibor = -1;
                                for (auto n : head[v].adjacent) {
                                        if (head[n].del == 0) {
                                                neibor = n;
                                                break;
                                        }
                                }
                                if (head[neibor].weight > head[v].weight) {
                                        head[neibor].weight -= head[v].weight;
                                        deleteVertex(v, pone, degree_one, degree_two);
                                        backTrack.push_back(make_pair(v, neibor));
                                        for (auto n : head[neibor].adjacent)
                                                if (head[n].del == 0) {
                                                        head[n].neiSum -= head[v].weight;
                                                        if (head[n].neiSum <= head[n].weight) {
                                                                pone.push_back(n);
                                                        }
                                                }
                                }
                        }
                }

                while (pone.empty() && degree_one.empty() && !degree_two.empty()) {
                        // path reduction.
                        int v = degree_two.back();
                        degree_two.pop_back();
                        if (head[v].neiSize != 2 || head[v].del) continue;
                        int n1, n2;
                        findTwoNeibor(v, n1, n2);
                        // if v,n1,n1 forms a triangle;
                        if (isNei(n1, n2)) {
                                if (head[n1].weight <= head[v].weight) {
                                        deleteVertex(n1, pone, degree_one, degree_two);
                                        continue;
                                }
                                if (head[n2].weight <= head[v].weight) {
                                        deleteVertex(n2, pone, degree_one, degree_two);
                                        continue;
                                }

                                head[n1].weight -= head[v].weight;
                                deleteVertex(v, pone, degree_one, degree_two);
                                for (auto n : head[n1].adjacent)
                                        if (head[n].del == 0) {
                                                head[n].neiSum -= head[v].weight;
                                                if (head[n].neiSum <= head[n].weight) {
                                                        pone.push_back(n);
                                                }
                                        }

                                head[n2].weight -= head[v].weight;
                                deleteVertex(v, pone, degree_one, degree_two);
                                for (auto n : head[n2].adjacent)
                                        if (head[n].del == 0) {
                                                head[n].neiSum -= head[v].weight;
                                                if (head[n].neiSum <= head[n].weight) {
                                                        pone.push_back(n);
                                                }
                                        }

                                backTrack.push_back(make_pair(v, n + 1));

                                continue;
                        }
                        // if n1,n2 is not adjacent;
                        else {
                                if (key) {  // edges are not allowed to be inserted.
                                        // if v > n1 and v > n2
                                        if (head[v].weight >= min(head[n1].weight, head[n2].weight)) {
                                                int bigger = n1, smaller = n2;
                                                if (head[n2].weight > head[n1].weight) {
                                                        bigger = n2, smaller = n1;
                                                }
                                                for (auto n : head[smaller].adjacent)
                                                        if (!head[n].del) {
                                                                adj[n] = 1;
                                                        }
                                                for (auto n : head[bigger].adjacent)
                                                        if (!head[n].del && !adj[n]) {
                                                                head[smaller].adjacent.push_back(n);
                                                                head[smaller].neiSize++;
                                                                head[smaller].neiSum += head[n].weight;
                                                                head[n].adjacent.push_back(smaller);
                                                                head[n].neiSize++;
                                                                head[n].neiSum += head[smaller].weight;
                                                        }
                                                for (auto n : head[smaller].adjacent)
                                                        if (!head[n].del) {
                                                                adj[n] = 0;
                                                        }
                                                if (head[v].weight >= max(head[n1].weight, head[n2].weight)) {
                                                        head[smaller].weight += head[bigger].weight;
                                                        for (auto nei : head[smaller].adjacent)
                                                                if (!head[nei].del) {
                                                                        head[nei].neiSum += head[bigger].weight;
                                                                }
                                                        deleteVertex(bigger, pone, degree_one, degree_two);
                                                        backTrack.push_back(make_pair(bigger, n));
                                                        continue;
                                                } else {
                                                        head[bigger].weight -= head[v].weight;
                                                        for (auto nei : head[bigger].adjacent)
                                                                if (!head[nei].del) {
                                                                        head[nei].neiSum -= head[v].weight;
                                                                        if (head[nei].weight >= head[nei].neiSum &&
                                                                            nei != v) {
                                                                                pone.push_back(nei);
                                                                        }
                                                                }
                                                        deleteVertex(v, pone, degree_one, degree_two);
                                                        backTrack.push_back(make_pair(v, n));
                                                }
                                                continue;
                                        }
                                }
                        }

                        // printf("")
                        deque<int> deq;
                        int next = n2, prev = v;
                        deq.push_back(v);
                        while (head[next].neiSize == 2) {
                                deq.push_back(next);
                                int v1, v2;
                                findTwoNeibor(next, v1, v2);
                                int t = next;
                                v1 == prev ? (next = v2) : (next = v1);
                                prev = t;
                                if (next == v) break;
                        }
                        int final2 = next;
                        int deq_size = deq.size();
                        // path reduction #1 the path forms a separate circle
                        if (next == v) {
                                if (deq_size < 4) printf("error code:001\n");
                                int* dp = new int[deq_size];
                                int index = 0;
                                for (int j = 1; j < deq_size; j++) {
                                        dp[index++] = head[deq[j]].weight;
                                }
                                int dp_sum1 = 0, dp_sum2 = 0;
                                dp_sum1 = path_reduction(dp, 1, index - 1);
                                index = 0;
                                for (int j = 1; j < deq_size; j++) {
                                        dp[index++] = head[deq[j]].weight;
                                }
                                dp_sum2 = path_reduction(dp, 0, index);

                                if (dp_sum2 > dp_sum1 + head[v].weight) {
                                        deleteVertex(v, pone, degree_one, degree_two);
                                } else {
                                        for (auto n : head[v].adjacent)
                                                if (!head[n].del) {
                                                        deleteVertex(n, pone, degree_one, degree_two);
                                                }
                                }
                                delete[] dp;

                                continue;
                        }
                        next = n1, prev = v;
                        while (head[next].neiSize == 2) {
                                deq.push_front(next);
                                int v1, v2;
                                findTwoNeibor(next, v1, v2);
                                int t = next;
                                v1 == prev ? (next = v2) : (next = v1);
                                prev = t;
                        }
                        deq_size = deq.size();

                        // path reduction #2 path formes an attached circle.
                        if (next == final2) {
                                // printf("attached circle\n");
                                int delta = 0;
                                if (deq_size < 3) {
                                        printf("error code:101");
                                }
                                int* dp = new int[deq_size + 1];
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                int dp_sum1 = 0, dp_sum2 = 0;
                                dp_sum1 = path_reduction(dp, 0, deq_size);
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                dp_sum2 = path_reduction(dp, 1, deq_size - 1);
                                delete[] dp;
                                delta = dp_sum1 - dp_sum2;
                                if (head[next].weight > delta) {
                                        backTrack.push_back(make_pair(n, 1));
                                        circles.push_back(vector<int>());
                                        VI& T = circles.back();
                                        T.push_back(next);

                                        head[next].weight -= delta;

                                        for (auto x : deq) {
                                                deleteVertex(x, pone, degree_one, degree_two);
                                                T.push_back(x);
                                        }

                                        // head[next].neiSum -= head[deq[0]].weight + head[deq[deq_size-1]].weight;
                                        // head[next].neiSize -= 2;

                                        /*if (head[next].weight >= head[next].neiSum) { pone.push_back(next); }
                                        else if (head[next].neiSize == 1) { degree_one.push_back(next); }
                                        else if (head[next].neiSize == 2) { degree_two.push_back(next); }*/

                                        for (auto n : head[next].adjacent) {
                                                if (head[n].del == 0) {
                                                        head[n].neiSum -= delta;
                                                        if (head[n].neiSum <= head[n].weight && n != deq[0] &&
                                                            n != deq[deq_size - 1]) {
                                                                pone.push_back(n);
                                                        }
                                                }
                                        }

                                } else {
                                        deleteVertex(next, pone, degree_one, degree_two);
                                }
                        }
                        //// path reduction #3 the non-degree-two vertices are adjacent.
                        else if (isNei(next, final2)) {
                                int dp_sum = 0, dp_b = 0, dp_f = 0;
                                int* dp = new int[deq_size + 1];
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                dp_sum = path_reduction(dp, 0, deq_size);
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                dp_b = path_reduction(dp, 1, deq_size);
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                dp_f = path_reduction(dp, 0, deq_size - 1);
                                if (dp_sum >= head[next].weight + dp_b) {
                                        deleteVertex(next, pone, degree_one, degree_two);
                                } else if (dp_sum >= head[final2].weight + dp_f) {
                                        deleteVertex(final2, pone, degree_one, degree_two);
                                } else {
                                        backTrack.push_back(make_pair(n, 2));
                                        circles.push_back(vector<int>());
                                        VI& T = circles.back();
                                        T.push_back(final2);
                                        T.push_back(next);
                                        int delta = dp_sum - dp_b;
                                        head[next].weight -= delta;
                                        for (auto n : head[next].adjacent)
                                                if (head[n].del == 0) {
                                                        head[n].neiSum -= delta;
                                                        if (head[n].neiSum <= head[n].weight && n != deq[0] &&
                                                            n != deq[deq_size - 1]) {
                                                                pone.push_back(n);
                                                        }
                                                }

                                        delta = dp_sum - dp_f;
                                        head[final2].weight -= delta;
                                        for (auto n : head[final2].adjacent)
                                                if (head[n].del == 0) {
                                                        head[n].neiSum -= delta;
                                                        if (head[n].neiSum <= head[n].weight && n != deq[0] &&
                                                            n != deq[deq_size - 1]) {
                                                                pone.push_back(n);
                                                        }
                                                }

                                        for (auto x : deq) {
                                                deleteVertex(x, pone, degree_one, degree_two);
                                                T.push_back(x);
                                        }
                                }
                        }

                        // path-reduction #4 the non-degree-two vertices are not adjacent.
                }
        }
}

void Graph::htLowDeg() {
        for (int i = 0; i < n; i++) {
                // clear the useless vertex
                if (head[i].weight <= 0) {
                        printf("Invalid weight 0\n");
                        return;
                }
        }
        clock_t start = clock();
        // ����ÿ�����ھ�Ȩֵ�͡�
        int* weight_copy = new int[n];
        for (int i = 0; i < n; i++) {
                weight_copy[i] = head[i].weight;
        }
        int max_d = 0;
        for (int i = 0; i < n; i++) {
                int sum = 0;
                for (auto v : head[i].adjacent) {
                        sum += head[v].weight;
                }
                head[i].neiSum = sum;
                head[i].neiSize = head[i].adjacent.size();
        }

        vector<int> pone, dominated, degree_two, degree_one;
        vector<pair<int, int>> backTrack;
        vector<vector<int>> circles;

        for (int i = 0; i < n; i++) {
                if (head[i].weight >= head[i].neiSum) {
                        pone.push_back(i);
                } else if (head[i].neiSize == 1) {
                        degree_one.push_back(i);
                } else if (head[i].neiSize == 2) {
                        degree_two.push_back(i);
                }
        }

        bool* adj = new bool[n];
        memset(adj, 0, sizeof(bool) * n);
        int* AdjShare = new int[n];
        memset(AdjShare, 0, sizeof(int) * n);

        printf("low degree:%d %d \n", degree_one.size(), degree_two.size());

        // clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 1);

        while (!degree_one.empty() || !degree_two.empty()) {
                while (!degree_one.empty()) {
                        int v = degree_one.back();
                        degree_one.pop_back();
                        if (head[v].neiSize != 1 || head[v].del) {
                                continue;
                        }
                        int neibor = -1;
                        for (auto n : head[v].adjacent) {
                                if (head[n].del == 0) {
                                        neibor = n;
                                        break;
                                }
                        }
                        if (head[neibor].weight > head[v].weight) {
                                head[neibor].weight -= head[v].weight;
                                deleteVertex(v, pone, degree_one, degree_two);
                                backTrack.push_back(make_pair(v, neibor));
                                for (auto n : head[neibor].adjacent)
                                        if (head[n].del == 0) {
                                                head[n].neiSum -= head[v].weight;
                                                if (head[n].neiSum <= head[n].weight) {
                                                        pone.push_back(n);
                                                }
                                        }
                        } else {
                                deleteVertex(neibor, pone, degree_one, degree_two);
                        }
                }

                while (degree_one.empty() && !degree_two.empty()) {
                        // path reduction.
                        int v = degree_two.back();
                        degree_two.pop_back();
                        if (head[v].neiSize != 2 || head[v].del) continue;
                        int n1, n2;
                        findTwoNeibor(v, n1, n2);
                        if (head[v].weight >= head[v].neiSum) {
                                deleteVertex(n1, pone, degree_one, degree_two);
                                deleteVertex(n2, pone, degree_one, degree_two);
                                continue;
                        }
                        // if v,n1,n1 forms a triangle;
                        if (isNei(n1, n2)) {
                                if (head[n1].weight <= head[v].weight) {
                                        deleteVertex(n1, pone, degree_one, degree_two);
                                        continue;
                                }
                                if (head[n2].weight <= head[v].weight) {
                                        deleteVertex(n2, pone, degree_one, degree_two);
                                        continue;
                                }

                                head[n1].weight -= head[v].weight;
                                deleteVertex(v, pone, degree_one, degree_two);
                                for (auto n : head[n1].adjacent)
                                        if (head[n].del == 0) {
                                                head[n].neiSum -= head[v].weight;
                                                if (head[n].neiSum <= head[n].weight) {
                                                        pone.push_back(n);
                                                }
                                        }

                                head[n2].weight -= head[v].weight;
                                deleteVertex(v, pone, degree_one, degree_two);
                                for (auto n : head[n2].adjacent)
                                        if (head[n].del == 0) {
                                                head[n].neiSum -= head[v].weight;
                                                if (head[n].neiSum <= head[n].weight) {
                                                        pone.push_back(n);
                                                }
                                        }

                                backTrack.push_back(make_pair(v, n + 1));

                                continue;
                        }
                        // if n1,n2 is not adjacent;
                        else {
                                if (1) {  // edges are not allowed to be inserted.
                                        // if v > n1 and v > n2
                                        if (head[v].weight >= min(head[n1].weight, head[n2].weight)) {
                                                int bigger = n1, smaller = n2;
                                                if (head[n2].weight > head[n1].weight) {
                                                        bigger = n2, smaller = n1;
                                                }
                                                for (auto n : head[smaller].adjacent)
                                                        if (!head[n].del) {
                                                                adj[n] = 1;
                                                        }
                                                for (auto n : head[bigger].adjacent)
                                                        if (!head[n].del && !adj[n]) {
                                                                head[smaller].adjacent.push_back(n);
                                                                head[smaller].neiSize++;
                                                                head[smaller].neiSum += head[n].weight;
                                                                head[n].adjacent.push_back(smaller);
                                                                head[n].neiSize++;
                                                                head[n].neiSum += head[smaller].weight;
                                                        }
                                                for (auto n : head[smaller].adjacent)
                                                        if (!head[n].del) {
                                                                adj[n] = 0;
                                                        }
                                                if (head[v].weight >= max(head[n1].weight, head[n2].weight)) {
                                                        head[smaller].weight += head[bigger].weight;
                                                        for (auto nei : head[smaller].adjacent)
                                                                if (!head[nei].del) {
                                                                        head[nei].neiSum += head[bigger].weight;
                                                                }
                                                        deleteVertex(bigger, pone, degree_one, degree_two);
                                                        backTrack.push_back(make_pair(bigger, n));
                                                        continue;
                                                } else {
                                                        head[bigger].weight -= head[v].weight;
                                                        for (auto nei : head[bigger].adjacent)
                                                                if (!head[nei].del) {
                                                                        head[nei].neiSum -= head[v].weight;
                                                                        if (head[nei].weight >= head[nei].neiSum &&
                                                                            nei != v) {
                                                                                pone.push_back(nei);
                                                                        }
                                                                }
                                                        deleteVertex(v, pone, degree_one, degree_two);
                                                        backTrack.push_back(make_pair(v, n));
                                                }
                                                continue;
                                        }
                                }
                        }

                        // printf("")
                        deque<int> deq;
                        int next = n2, prev = v;
                        deq.push_back(v);
                        while (head[next].neiSize == 2) {
                                deq.push_back(next);
                                int v1, v2;
                                findTwoNeibor(next, v1, v2);
                                int t = next;
                                v1 == prev ? (next = v2) : (next = v1);
                                prev = t;
                                if (next == v) break;
                        }
                        int final2 = next;
                        int deq_size = deq.size();
                        // path reduction #1 the path forms a separate circle
                        if (next == v) {
                                if (deq_size < 4) printf("error code:001\n");
                                int* dp = new int[deq_size];
                                int index = 0;
                                for (int j = 1; j < deq_size; j++) {
                                        dp[index++] = head[deq[j]].weight;
                                }
                                int dp_sum1 = 0, dp_sum2 = 0;
                                dp_sum1 = path_reduction(dp, 1, index - 1);
                                index = 0;
                                for (int j = 1; j < deq_size; j++) {
                                        dp[index++] = head[deq[j]].weight;
                                }
                                dp_sum2 = path_reduction(dp, 0, index);

                                if (dp_sum2 > dp_sum1 + head[v].weight) {
                                        deleteVertex(v, pone, degree_one, degree_two);
                                } else {
                                        for (auto n : head[v].adjacent)
                                                if (!head[n].del) {
                                                        deleteVertex(n, pone, degree_one, degree_two);
                                                }
                                }
                                delete[] dp;

                                continue;
                        }
                        next = n1, prev = v;
                        while (head[next].neiSize == 2) {
                                deq.push_front(next);
                                int v1, v2;
                                findTwoNeibor(next, v1, v2);
                                int t = next;
                                v1 == prev ? (next = v2) : (next = v1);
                                prev = t;
                        }
                        deq_size = deq.size();

                        // path reduction #2 path formes an attached circle.
                        if (next == final2) {
                                // printf("attached circle\n");
                                int delta = 0;
                                if (deq_size < 3) {
                                        printf("error code:101");
                                }
                                int* dp = new int[deq_size + 1];
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                int dp_sum1 = 0, dp_sum2 = 0;
                                dp_sum1 = path_reduction(dp, 0, deq_size);
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                dp_sum2 = path_reduction(dp, 1, deq_size - 1);
                                delete[] dp;
                                delta = dp_sum1 - dp_sum2;
                                if (head[next].weight > delta) {
                                        backTrack.push_back(make_pair(n, 1));
                                        circles.push_back(vector<int>());
                                        VI& T = circles.back();
                                        T.push_back(next);

                                        head[next].weight -= delta;

                                        for (auto x : deq) {
                                                deleteVertex(x, pone, degree_one, degree_two);
                                                T.push_back(x);
                                        }

                                        // head[next].neiSum -= head[deq[0]].weight + head[deq[deq_size-1]].weight;
                                        // head[next].neiSize -= 2;

                                        /*if (head[next].weight >= head[next].neiSum) { pone.push_back(next); }
                                        else if (head[next].neiSize == 1) { degree_one.push_back(next); }
                                        else if (head[next].neiSize == 2) { degree_two.push_back(next); }*/

                                        for (auto n : head[next].adjacent) {
                                                if (head[n].del == 0) {
                                                        head[n].neiSum -= delta;
                                                        if (head[n].neiSum <= head[n].weight && n != deq[0] &&
                                                            n != deq[deq_size - 1]) {
                                                                pone.push_back(n);
                                                        }
                                                }
                                        }

                                } else {
                                        deleteVertex(next, pone, degree_one, degree_two);
                                }
                        }
                        //// path reduction #3 the non-degree-two vertices are adjacent.
                        else if (isNei(next, final2)) {
                                int dp_sum = 0, dp_b = 0, dp_f = 0;
                                int* dp = new int[deq_size + 1];
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                dp_sum = path_reduction(dp, 0, deq_size);
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                dp_b = path_reduction(dp, 1, deq_size);
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                dp_f = path_reduction(dp, 0, deq_size - 1);
                                if (dp_sum >= head[next].weight + dp_b) {
                                        deleteVertex(next, pone, degree_one, degree_two);
                                } else if (dp_sum >= head[final2].weight + dp_f) {
                                        deleteVertex(final2, pone, degree_one, degree_two);
                                } else {
                                        backTrack.push_back(make_pair(n, 2));
                                        circles.push_back(vector<int>());
                                        VI& T = circles.back();
                                        T.push_back(final2);
                                        T.push_back(next);
                                        int delta = dp_sum - dp_b;
                                        head[next].weight -= delta;
                                        for (auto n : head[next].adjacent)
                                                if (head[n].del == 0) {
                                                        head[n].neiSum -= delta;
                                                        if (head[n].neiSum <= head[n].weight && n != deq[0] &&
                                                            n != deq[deq_size - 1]) {
                                                                pone.push_back(n);
                                                        }
                                                }

                                        delta = dp_sum - dp_f;
                                        head[final2].weight -= delta;
                                        for (auto n : head[final2].adjacent)
                                                if (head[n].del == 0) {
                                                        head[n].neiSum -= delta;
                                                        if (head[n].neiSum <= head[n].weight && n != deq[0] &&
                                                            n != deq[deq_size - 1]) {
                                                                pone.push_back(n);
                                                        }
                                                }

                                        for (auto x : deq) {
                                                deleteVertex(x, pone, degree_one, degree_two);
                                                T.push_back(x);
                                        }
                                }
                        }
                        // path-reduction #4 the non-degree-two vertices are not adjacent.
                }
        }

        delete[] AdjShare;
        delete[] adj;

        int count = 0, d_two = 0;
        for (int k = 0; k < n; k++) {
                if (head[k].del == 0 && head[k].neiSize > 0) {
                        if (head[k].neiSum - head[k].weight > max_d) {
                                max_d = head[k].neiSum - head[k].weight;
                        }
                        if (head[k].neiSize == 2) d_two++;
                        count++;
                }
        }
        printf("degree_two_size: %d \nKernel Size:%d\n", d_two, count);

        int* bin_head = new int[max_d + 1];
        memset(bin_head, -1, sizeof(int) * (max_d + 1));
        int* bin_next = new int[n];
        for (int i = 0; i < n; i++) {
                if (head[i].del || head[i].neiSize < 1) continue;
                int dif = head[i].neiSum - head[i].weight;
                if (dif <= 0) continue;
                bin_next[i] = bin_head[dif];
                bin_head[dif] = i;
        }

        while (max_d > 0) {
                while (pone.empty() && degree_two.empty() && degree_one.empty()) {
                        while (max_d >= 0 && bin_head[max_d] == -1) max_d--;
                        if (max_d < 0) break;
                        int v = -1;
                        for (v = bin_head[max_d]; v != -1;) {
                                int tmp = bin_next[v];
                                int deg = head[v].neiSum - head[v].weight;
                                if (head[v].del == 0 && deg > 0) {
                                        if (deg < max_d) {
                                                bin_next[v] = bin_head[deg];
                                                bin_head[deg] = v;
                                        } else {
                                                deleteVertex(v, pone, degree_one, degree_two);
                                                backTrack.push_back(make_pair(v, n));
                                                bin_head[max_d] = tmp;
                                                break;
                                        }
                                }
                                v = tmp;
                        }
                        if (v == -1) bin_head[max_d] = -1;
                }
                // cannot add edge
                clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 0);
        }

        int greedy_include = 0, remainder = 0;
        for (int k = backTrack.size() - 1; k >= 0; k--) {
                int a = backTrack[k].first;
                bool ok = 1;
                if (a == n) {
                        int b = backTrack[k].second;
                        VI& T = circles.back();
                        int* tmp = nullptr;
                        int index = 0;
                        int T_size = T.size();
                        // attahed circle
                        if (b == 1) {
                                // if v
                                if (head[T[0]].del) {
                                        tmp = new int[T_size - 1];
                                        for (int i = 1; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                }
                        }
                        // circle ends with two adjacent vertex.
                        else if (b == 2) {
                                if (!head[T[0]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (!head[T[1]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 3; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (head[T[1]].del && head[T[0]].del) {
                                        tmp = new int[T_size - 2];
                                        for (int i = 2; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        printf("error info: T[0] and T[0] cann't exist at the same time\n");
                                }
                        }
                        path_recover(tmp, index);
                        delete[] tmp;
                        circles.pop_back();
                } else {
                        for (auto n : head[a].adjacent) {
                                if (head[n].del == 0) {
                                        ok = 0;
                                        break;
                                }
                        }
                        if (ok) {
                                head[a].del = 0;
                        }
                }
        }

        if (!circles.empty()) printf("error info: circles is not empty!\n");

        for (int i = 0; i < n; i++) {
                if (!head[i].del) total_weight += weight_copy[i];
        }

        clock_t end = clock();
        delete[] bin_head;
        delete[] bin_next;

        /*printf("solution:\n");
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0) {
                        printf("%d ", i);
                }
        }*/

        printf("total weight :%d ,  remainder : %d , total time cost : %f\n", total_weight, remainder,
               (float)(end - start) * 1000 / CLOCKS_PER_SEC);
        delete[] weight_copy;
}
// designed for triangle function;
void Graph::clearPone(VI& pone, VI& degree_one, VI& degree_two, vector<pair<int, int>>& backTrack,
                      vector<vector<int>>& circles, VI& dominated, int* edges, int* tri, bool* dominate, bool* adj) {
        while (!dominated.empty() || !pone.empty() || !degree_one.empty() || !degree_two.empty()) {
                while (!dominated.empty() || !pone.empty() || !degree_one.empty()) {
                        while (!dominated.empty() || !pone.empty()) {
                                while (!dominated.empty()) {
                                        int u = dominated.back();
                                        dominated.pop_back();
                                        if (head[u].del || head[u].neiSize < 1) continue;
                                        deleteVertex(u, tri, edges, adj, dominate, pone, degree_one, degree_two,
                                                     dominated);
                                }
                                while (dominated.empty() && !pone.empty()) {
                                        int u = pone.back();
                                        pone.pop_back();
                                        if (head[u].del || head[u].neiSize < 1 || head[u].weight < head[u].neiSum)
                                                continue;
                                        for (auto n : head[u].adjacent) {
                                                deleteVertex(n, tri, edges, adj, dominate, pone, degree_one, degree_two,
                                                             dominated);
                                        }
                                }
                        }
                        while (dominated.empty() && pone.empty() && !degree_one.empty()) {
                                int u = degree_one.back();
                                degree_one.pop_back();
                                if (head[u].del || head[u].neiSize != 1) continue;
                                int neibor = -1;
                                for (auto n : head[u].adjacent) {
                                        if (head[n].del == 0) {
                                                neibor = n;
                                                break;
                                        }
                                }
                                if (head[neibor].weight > head[u].weight) {
                                        head[neibor].weight -= head[u].weight;
                                        deleteVertex(
                                            u, pone, degree_one,
                                            degree_two);  // no triangle destroyed. but neibor might be dominated.
                                        backTrack.push_back(make_pair(u, neibor));
                                        for (auto n : head[neibor].adjacent)
                                                if (head[n].del == 0) {
                                                        head[n].neiSum -= head[u].weight;
                                                        if (head[n].neiSum <= head[n].weight) {
                                                                pone.push_back(n);
                                                        }
                                                }
                                        if (!dominate[neibor]) {
                                                for (auto n : head[neibor].adjacent)
                                                        if (!head[n].del) {
                                                                adj[n] = 1;
                                                        }
                                                VI& Nei_adj = head[neibor].adjacent;
                                                for (int i = 0, Nei_size = Nei_adj.size(); i < Nei_size; i++) {
                                                        int v = Nei_adj[i];
                                                        if (head[v].del || head[v].weight < head[neibor].weight ||
                                                            head[v].neiSize > head[neibor].neiSize)
                                                                continue;
                                                        if (tri[edge_id(neibor, i)] + 1 == head[v].neiSize) {
                                                                dominate[neibor] = 1;
                                                                dominated.push_back(neibor);
                                                        }
                                                }
                                                for (auto n : head[neibor].adjacent)
                                                        if (!head[n].del) {
                                                                adj[n] = 0;
                                                        }
                                        }
                                }
                        }
                }

                while (dominated.empty() && pone.empty() && degree_one.empty() && !degree_two.empty()) {
                        int v = degree_two.back();
                        degree_two.pop_back();
                        if (head[v].del || head[v].neiSize != 2) continue;
                        int n1, n2;
                        findTwoNeibor(v, n1, n2);
                        // if v,n1,n1 forms a triangle;
                        if (isNei(n1, n2)) {
                                bool ok = 0;
                                if (head[n1].weight <= head[v].weight) {
                                        deleteVertex(n1, tri, edges, adj, dominate, pone, degree_one, degree_two,
                                                     dominated);
                                        continue;
                                }
                                if (head[n2].weight <= head[v].weight) {
                                        deleteVertex(n2, tri, edges, adj, dominate, pone, degree_one, degree_two,
                                                     dominated);
                                        continue;
                                }
                                head[n1].weight -= head[v].weight;
                                deleteVertex(v, tri, edges, adj, dominate, pone, degree_one, degree_two, dominated);
                                for (auto n : head[n1].adjacent)
                                        if (head[n].del == 0) {
                                                head[n].neiSum -= head[v].weight;
                                                if (head[n].neiSum <= head[n].weight) {
                                                        pone.push_back(n);
                                                }
                                        }
                                if (!dominate[n1]) {
                                        for (auto n : head[n1].adjacent)
                                                if (!head[n].del) {
                                                        adj[n] = 1;
                                                }
                                        VI& Nei_adj = head[n1].adjacent;
                                        for (int i = 0, Nei_size = Nei_adj.size(); i < Nei_size; i++) {
                                                int v = Nei_adj[i];
                                                if (head[v].del || head[v].weight < head[n1].weight ||
                                                    head[v].neiSize > head[n1].neiSize)
                                                        continue;
                                                if (tri[edge_id(n1, i)] + 1 == head[v].neiSize) {
                                                        dominate[n1] = 1;
                                                        dominated.push_back(n1);
                                                }
                                        }
                                        for (auto n : head[n1].adjacent)
                                                if (!head[n].del) {
                                                        adj[n] = 0;
                                                }
                                }

                                head[n2].weight -= head[v].weight;
                                deleteVertex(v, tri, edges, adj, dominate, pone, degree_one, degree_two, dominated);
                                for (auto n : head[n2].adjacent)
                                        if (head[n].del == 0) {
                                                head[n].neiSum -= head[v].weight;
                                                if (head[n].neiSum <= head[n].weight) {
                                                        pone.push_back(n);
                                                }
                                        }

                                if (!dominate[n2]) {
                                        for (auto n : head[n2].adjacent)
                                                if (!head[n].del) {
                                                        adj[n] = 1;
                                                }
                                        VI& Nei_adj = head[n2].adjacent;
                                        for (int i = 0, Nei_size = Nei_adj.size(); i < Nei_size; i++) {
                                                int v = Nei_adj[i];
                                                if (head[v].del || head[v].weight < head[n2].weight ||
                                                    head[v].neiSize > head[n2].neiSize)
                                                        continue;
                                                if (tri[edge_id(n2, i)] + 1 == head[v].neiSize) {
                                                        dominate[n2] = 1;
                                                        dominated.push_back(n2);
                                                }
                                        }
                                        for (auto n : head[n2].adjacent)
                                                if (!head[n].del) {
                                                        adj[n] = 0;
                                                }
                                }
                                backTrack.push_back(make_pair(v, n + 1));
                                continue;
                        }

                        deque<int> deq;
                        int next = n2, prev = v;
                        deq.push_back(v);
                        while (head[next].neiSize == 2) {
                                deq.push_back(next);
                                int v1, v2;
                                findTwoNeibor(next, v1, v2);
                                int t = next;
                                v1 == prev ? (next = v2) : (next = v1);
                                prev = t;
                                if (next == v) break;
                        }
                        int final2 = next;
                        int deq_size = deq.size();
                        // path reduction #1 the path forms a separate circle
                        if (next == v) {
                                if (deq_size < 4) printf("error code:001\n");
                                int* dp = new int[deq_size];
                                int index = 0;
                                for (int j = 1; j < deq_size; j++) {
                                        dp[index++] = head[deq[j]].weight;
                                }
                                int dp_sum1 = 0, dp_sum2 = 0;
                                dp_sum1 = path_reduction(dp, 1, index - 1);
                                index = 0;
                                for (int j = 1; j < deq_size; j++) {
                                        dp[index++] = head[deq[j]].weight;
                                }
                                dp_sum2 = path_reduction(dp, 0, index);

                                if (dp_sum2 > dp_sum1 + head[v].weight) {
                                        deleteVertex(v, tri, edges, adj, dominate, pone, degree_one, degree_two,
                                                     dominated);
                                } else {
                                        for (auto n : head[v].adjacent)
                                                if (!head[n].del) {
                                                        deleteVertex(n, tri, edges, adj, dominate, pone, degree_one,
                                                                     degree_two, dominated);
                                                }
                                }
                                delete[] dp;

                                continue;
                        }
                        next = n1, prev = v;
                        while (head[next].neiSize == 2) {
                                deq.push_front(next);
                                int v1, v2;
                                findTwoNeibor(next, v1, v2);
                                int t = next;
                                v1 == prev ? (next = v2) : (next = v1);
                                prev = t;
                        }
                        deq_size = deq.size();

                        //// path reduction #2 path formes an attached circle.
                        if (next == final2) {
                                // printf("attached circle\n");
                                int delta = 0;
                                if (deq_size < 3) {
                                        printf("error code:101");
                                }
                                int* dp = new int[deq_size + 1];
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                int dp_sum1 = 0, dp_sum2 = 0;
                                dp_sum1 = path_reduction(dp, 0, deq_size);
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                dp_sum2 = path_reduction(dp, 1, deq_size - 1);
                                delete[] dp;
                                delta = dp_sum1 - dp_sum2;
                                if (head[next].weight > delta) {
                                        backTrack.push_back(make_pair(n, 1));
                                        circles.push_back(vector<int>());
                                        VI& T = circles.back();
                                        T.push_back(next);

                                        head[next].weight -= delta;

                                        for (auto x : deq) {
                                                deleteVertex(x, pone, degree_one, degree_two);
                                                T.push_back(x);
                                        }
                                        for (auto n : head[next].adjacent) {
                                                if (head[n].del == 0) {
                                                        head[n].neiSum -= delta;
                                                        if (head[n].neiSum <= head[n].weight && n != deq[0] &&
                                                            n != deq[deq_size - 1]) {
                                                                pone.push_back(n);
                                                        }
                                                }
                                        }
                                } else {
                                        deleteVertex(next, tri, edges, adj, dominate, pone, degree_one, degree_two,
                                                     dominated);
                                }
                        }
                        //// path reduction #3 the non-degree-two vertices are adjacent.
                        else if (isNei(next, final2)) {
                                int dp_sum = 0, dp_b = 0, dp_f = 0;
                                int* dp = new int[deq_size + 1];
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                dp_sum = path_reduction(dp, 0, deq_size);
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                dp_b = path_reduction(dp, 1, deq_size);
                                for (int j = 0; j < deq_size; j++) {
                                        dp[j] = head[deq[j]].weight;
                                }
                                dp_f = path_reduction(dp, 0, deq_size - 1);
                                if (dp_sum >= head[next].weight + dp_b) {
                                        deleteVertex(next, tri, edges, adj, dominate, pone, degree_one, degree_two,
                                                     dominated);
                                } else if (dp_sum >= head[final2].weight + dp_f) {
                                        deleteVertex(final2, tri, edges, adj, dominate, pone, degree_one, degree_two,
                                                     dominated);
                                } else {
                                        backTrack.push_back(make_pair(n, 2));
                                        circles.push_back(vector<int>());
                                        VI& T = circles.back();
                                        T.push_back(final2);
                                        T.push_back(next);
                                        int delta = dp_sum - dp_b;
                                        head[next].weight -= delta;
                                        for (auto n : head[next].adjacent)
                                                if (head[n].del == 0) {
                                                        head[n].neiSum -= delta;
                                                        if (head[n].neiSum <= head[n].weight && n != deq[0] &&
                                                            n != deq[deq_size - 1]) {
                                                                pone.push_back(n);
                                                        }
                                                }

                                        delta = dp_sum - dp_f;
                                        head[final2].weight -= delta;
                                        for (auto n : head[final2].adjacent)
                                                if (head[n].del == 0) {
                                                        head[n].neiSum -= delta;
                                                        if (head[n].neiSum <= head[n].weight && n != deq[0] &&
                                                            n != deq[deq_size - 1]) {
                                                                pone.push_back(n);
                                                        }
                                                }

                                        for (auto x : deq) {
                                                deleteVertex(x, pone, degree_one, degree_two);
                                                T.push_back(x);
                                        }
                                }
                        }
                }
        }
}

int Graph::path_reduction(int* dp, int start, int end) {
        int dp_sum1 = 0;
        for (int j = start; j < end; j++) {
                if (j == end - 1 && dp[j] <= 0) break;
                dp_sum1 += dp[j];
                dp[j + 1] -= dp[j];
        }
        return dp_sum1;
}

void Graph::findTwoNeibor(int v, int& n1, int& n2) {
        VI& adj = head[v].adjacent;
        int len = adj.size();
        char first = 1;
        int error = 0;
        for (int i = 0; i < len; i++) {
                if (head[adj[i]].del == 0) {
                        error++;
                        if (first) {
                                n1 = adj[i];
                                first = !first;
                        } else
                                n2 = adj[i];
                }
        }
        if (error != 2) {
                printf("vertex %d error neibor: %d != 2\n", v, error);
        }
}

bool Graph::isNei(int v1, int v2) {
        VI* target = nullptr;
        int id = -1;
        int size1 = head[v1].adjacent.size(), size2 = head[v2].adjacent.size();
        size1 < size2 ? (target = &head[v1].adjacent, id = v2) : (target = &head[v2].adjacent, id = v1);

        for (auto n : *target)
                if (!head[n].del) {
                        if (n == id) return 1;
                }
        return 0;
}

void Graph::neighborhood_reduction(VI& pone, VI& degree_one, VI& degree_two, vector<pair<int, int>>& backTrack,
                                   vector<vector<int>>& circles, bool* adj) {
        for (int i = 0; i < n; i++)
                if (!head[i].del) {
                        for (auto nei : head[i].adjacent)
                                if (!head[nei].del) {
                                        adj[nei] = 1;
                                }
                        for (auto nei : head[i].adjacent)
                                if (!head[nei].del) {
                                        int w = 0;
                                        for (auto nn : head[nei].adjacent)
                                                if (!head[nn].del && adj[nn]) {
                                                        w += head[nn].weight;
                                                        if (head[i].weight >= head[i].neiSum - w) {
                                                                deleteVertex(nei, pone, degree_one, degree_two);
                                                                break;
                                                        }
                                                }
                                }
                        for (auto nei : head[i].adjacent)
                                if (!head[nei].del) {
                                        adj[nei] = 0;
                                }
                        clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 1);
                }
}

void Graph::symmetric_folding(VI& pone, VI& degree_one, VI& degree_two, vector<pair<int, int>>& backTrack,
                              vector<vector<int>>& circles, bool* adj, int* AdjShare) {
        for (int i = 0; i < n; i++)
                if (!head[i].del) {
                        for (auto nei : head[i].adjacent)
                                if (!head[nei].del) {
                                        adj[nei] = 1;
                                }
                        adj[i] = 1;
                        vector<int> two_hop;
                        bool two_hop_key = 0;
                        for (auto nei : head[i].adjacent)
                                if (!head[nei].del) {
                                        for (auto nn : head[nei].adjacent)
                                                if (!head[nn].del && !adj[nn]) {
                                                        AdjShare[nn] += head[nei].weight;
                                                        two_hop.push_back(nn);
                                                        if (head[i].weight >= head[i].neiSum - AdjShare[nn] &&
                                                            head[nn].weight >= head[nn].neiSum - AdjShare[nn]) {
                                                                // fold nn and i

                                                                deleteVertex(i, pone, degree_one, degree_two);
                                                                head[nn].weight += head[i].weight;
                                                                backTrack.push_back(make_pair(i, -1));
                                                                for (auto k : head[nn].adjacent)
                                                                        if (!head[k].del) {
                                                                                if (adj[k]) {
                                                                                        if (head[k].neiSize == 1) {
                                                                                                degree_one.push_back(k);
                                                                                        } else if (head[k].neiSize ==
                                                                                                   2) {
                                                                                                degree_two.push_back(k);
                                                                                        }
                                                                                        adj[k] = 0;
                                                                                } else {
                                                                                        head[k].neiSum +=
                                                                                            head[i].weight;
                                                                                }
                                                                        }
                                                                for (auto k : head[i].adjacent)
                                                                        if (!head[k].del) {
                                                                                if (adj[k]) {
                                                                                        head[k].neiSum +=
                                                                                            head[nn].weight -
                                                                                            head[i].weight;
                                                                                        head[k].neiSize++;
                                                                                        head[k].adjacent.push_back(nn);

                                                                                        head[nn].neiSum +=
                                                                                            head[k].weight;
                                                                                        head[nn].neiSize++;
                                                                                        head[nn].adjacent.push_back(k);
                                                                                }
                                                                                head[k].neiSum += head[i].weight;
                                                                        }
                                                                if (head[nn].weight >= head[nn].neiSum) {
                                                                        pone.push_back(nn);
                                                                }
                                                                two_hop_key = 1;
                                                                break;
                                                        }
                                                }
                                        if (two_hop_key) break;
                                }
                        for (auto x : two_hop) {
                                AdjShare[x] = 0;
                        }
                        for (auto nei : head[i].adjacent)
                                if (!head[nei].del) {
                                        adj[nei] = 0;
                                }
                        adj[i] = 0;
                        clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 1);
                }
}

void Graph::common_neighbor_reduction(VI& pone, VI& degree_one, VI& degree_two, vector<pair<int, int>>& backTrack,
                                      vector<vector<int>>& circles, bool* adj) {
        //
        int times = 0;
        for (int i = 0; i < n; i++)
                if (!head[i].del) {
                        for (auto nei : head[i].adjacent)
                                if (!head[nei].del) {
                                        adj[nei] = 1;
                                }
                        for (auto nei : head[i].adjacent)
                                if (!head[nei].del) {
                                        if ((head[i].weight >= head[i].neiSum - head[nei].weight) ||
                                            (head[nei].weight >= head[nei].neiSum - head[i].weight)) {
                                                times++;
                                                for (auto nn : head[nei].adjacent)
                                                        if (!head[nn].del && adj[nn]) {
                                                                deleteVertex(nn, pone, degree_one, degree_two);
                                                        }
                                        }
                                }
                        for (auto nei : head[i].adjacent)
                                if (!head[nei].del) {
                                        adj[nei] = 0;
                                }
                        clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 1);
                }
}

void Graph::initial_reduction(VI& pone, VI& degree_one, VI& degree_two, vector<pair<int, int>>& backTrack,
                              vector<vector<int>>& circles, bool* adj, int* AdjShare) {
        VI vs;
        for (int i = 0; i < n; i++)
                if (!head[i].del && head[i].neiSize > 0) {
                        vs.push_back(i);
                }
        ui* ids = new ui[vs.size()];
        memset(ids, 0, sizeof(ui) * vs.size());
        for (ui i = 0; i < vs.size(); i++) ++ids[head[vs[i]].neiSize];
        for (ui i = 1; i < vs.size(); i++) ids[i] += ids[i - 1];

        int* order = new int[n];
        memset(order, -1, sizeof(int) * n);
        for (ui i = 0; i < vs.size(); i++) order[vs[i]] = (--ids[head[vs[i]].neiSize]);
        for (ui i = 0; i < vs.size(); i++) ids[order[vs[i]]] = vs[i];

        for (int i = vs.size() - 1; i >= 0; i--) {
                int u = ids[i];
                if (head[u].del || head[u].neiSize <= 0) continue;
                for (auto n : head[u].adjacent)
                        if (head[n].del == 0) {
                                adj[n] = 1;
                        }
                adj[u] = 1;
                int dominate_sum = 0;
                bool dominate = 0, two_hop_key = 0;
                vector<int> two_hop;
                for (auto x : head[u].adjacent)
                        if (!head[x].del) {
                                int tri_count = 0, nei_sum_of_x = 0, u_neiSum = head[u].neiSum;
                                for (auto nn : head[x].adjacent)
                                        if (!head[nn].del) {
                                                if (!adj[nn]) {
                                                        two_hop.push_back(nn);

                                                        AdjShare[nn] += head[x].weight;
                                                        /*if (head[nn].weight + head[u].weight >= head[nn].neiSum +
                                                        head[u].neiSum - AdjShare[nn]) { for (auto nei :
                                                        head[u].adjacent) { deleteVertex(nei, u, pone, degree_one,
                                                        degree_two);
                                                                }
                                                                for (auto nei : head[nn].adjacent) {
                                                                        deleteVertex(nei, nn, pone, degree_one,
                                                        degree_two);
                                                                }
                                                                two_hop_key = 1;
                                                                break;
                                                        }*/
                                                        // check if nn and u are symmetry.
                                                        if (head[u].neiSum - AdjShare[nn] <= head[u].weight &&
                                                            head[nn].neiSum - AdjShare[nn] <= head[nn].weight) {
                                                                // printf("symmetric foldig\n");
                                                                //  u and nn are bound to be in the MWIS together.
                                                                deleteVertex(u, pone, degree_one, degree_two);
                                                                head[nn].weight += head[u].weight;
                                                                backTrack.push_back(make_pair(u, -1));
                                                                for (auto k : head[nn].adjacent)
                                                                        if (!head[k].del) {
                                                                                // head[k].neiSize--;
                                                                                if (adj[k]) {
                                                                                        if (head[k].neiSize == 1) {
                                                                                                degree_one.push_back(k);
                                                                                        } else if (head[k].neiSize ==
                                                                                                   2) {
                                                                                                degree_two.push_back(k);
                                                                                        }
                                                                                        adj[k] = 0;
                                                                                } else {
                                                                                        head[k].neiSum +=
                                                                                            head[u].weight;
                                                                                }
                                                                        }
                                                                for (auto k : head[u].adjacent)
                                                                        if (!head[k].del) {
                                                                                if (adj[k]) {
                                                                                        head[k].neiSum +=
                                                                                            head[nn].weight -
                                                                                            head[u].weight;
                                                                                        head[k].neiSize++;
                                                                                        head[k].adjacent.push_back(nn);

                                                                                        head[nn].neiSum +=
                                                                                            head[k].weight;
                                                                                        head[nn].neiSize++;
                                                                                        head[nn].adjacent.push_back(k);
                                                                                }
                                                                                head[k].neiSum += head[u].weight;
                                                                        }
                                                                if (head[nn].weight >= head[nn].neiSum) {
                                                                        pone.push_back(nn);
                                                                }
                                                                two_hop_key = 1;
                                                                break;
                                                        }
                                                } else {
                                                        if (nn != u) {
                                                                nei_sum_of_x += head[nn].weight;
                                                        }
                                                        ++tri_count;
                                                }
                                        }
                                if (two_hop_key) break;
                                if (tri_count == head[x].neiSize) {
                                        if (head[x].weight >= head[u].weight) {
                                                dominate = 1;
                                                break;
                                        } else if (head[x].neiSum - head[u].weight <= head[x].weight) {
                                                for (auto& nn : head[x].adjacent)
                                                        if (!head[nn].del && nn != u) {
                                                                deleteVertex(nn, pone, degree_one, degree_two);
                                                        }
                                        }
                                }
                                /*int t = 0;
                                for (auto nn : head[x].adjacent) if (!head[nn].del) {
                                        if (adj[nn] && nn != u) {
                                                t += head[nn].weight;
                                        }
                                }
                                printf("%d %d\n", u_neiSum - nei_sum_of_x, head[u].neiSum - t);*/

                                // else if (head[u].weight >= u_neiSum - nei_sum_of_x) {
                                //	//printf("%d %d\n", head[u].neiSum, u_neiSum);
                                //	deleteVertex(x,pone,degree_one,degree_two);
                                //	clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 0);
                                // }
                        }
                for (auto v : two_hop) {
                        AdjShare[v] = 0;
                }

                for (auto n : head[u].adjacent)
                        if (head[n].del == 0) {
                                adj[n] = 0;
                        }
                adj[u] = 0;
                if (two_hop_key) {
                        clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 1);
                        continue;
                }
                // dominate a vertex.
                if (dominate) {
                        deleteVertex(u, pone, degree_one, degree_two);
                }
                clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 1);
        }
        delete[] ids;
        delete[] order;
}

void Graph::compute_triangles(int* tri, int* edges, int* AdjShare, bool* adj, bool* dominate, VI& pone, VI& degree_one,
                              VI& degree_two, VI& dominated, vector<pair<int, int>>& backTrack,
                              vector<vector<int>>& circles) {
        VI vs;
        for (int i = 0; i < n; i++)
                if (!head[i].del && head[i].neiSize > 0) {
                        vs.push_back(i);
                }

        ui* ids = new ui[vs.size()];
        memset(ids, 0, sizeof(ui) * vs.size());
        for (ui i = 0; i < vs.size(); i++) ++ids[head[vs[i]].neiSize];
        for (ui i = 1; i < vs.size(); i++) ids[i] += ids[i - 1];

        int* order = new int[n];
        memset(order, -1, sizeof(int) * n);
        for (ui i = 0; i < vs.size(); i++) order[vs[i]] = (--ids[head[vs[i]].neiSize]);
        for (ui i = 0; i < vs.size(); i++) ids[order[vs[i]]] = vs[i];

        for (int i = vs.size() - 1; i >= 0; i--) {
                int u = ids[i];

                if (head[u].del || head[u].neiSize < 1) continue;
                int two_hop_nei = -1;
                VI two_hop;
                bool two_hop_key = 0;
                for (auto n : head[u].adjacent)
                        if (head[n].del == 0) {
                                adj[n] = 1;
                        }
                if (!dominate[u]) {
                        // adj[u] = 1;
                        for (auto x : head[u].adjacent)
                                if (!head[x].del /*&& head[x].neiSize <= head[u].neiSize*/) {
                                        int tri_count = 0;
                                        for (auto nn : head[x].adjacent)
                                                if (!head[nn].del) {
                                                        if (!adj[nn]) {  // ע���������Ҫʹ�ã���Ӧ��֮ǰӦ�ò�Ҫ����u���weight��
                                                                break;
                                                                two_hop.push_back(nn);
                                                                AdjShare[nn] += head[x].weight;

                                                                if (head[nn].weight + head[u].weight >=
                                                                    head[nn].neiSum + head[u].neiSum - AdjShare[nn]) {
                                                                        // printf("run\n");
                                                                        two_hop_nei = nn;
                                                                        two_hop_key = 1;
                                                                        break;
                                                                }
                                                        } else {
                                                                ++tri_count;
                                                        }
                                                }
                                        if (two_hop_key) break;
                                        if (tri_count + 1 == head[x].neiSize && head[x].weight >= head[u].weight) {
                                                dominate[u] = 1;
                                                break;
                                        }
                                }
                        // adj[u] = 0;
                }
                /*if (two_hop_key) {
                        for (auto n : head[u].adjacent) if (head[n].del == 0) { adj[n] = 0; }
                        for (auto v : two_hop) { AdjShare[v] = 0; }
                        if (head[two_hop_nei].neiSum <= head[two_hop_nei].weight || head[u].neiSum <= head[u].weight) {
                                printf("error code : 102\n");
                        }
                        for (auto nei : head[u].adjacent) {
                                deleteVertex(nei, tri, edges, adj, dominate, pone, degree_one, degree_two, dominated);
                        }
                        for (auto nei : head[two_hop_nei].adjacent) {
                                deleteVertex(nei, tri, edges, adj, dominate, pone, degree_one, degree_two, dominated);
                        }
                        clearPone(pone, degree_one, degree_two, backTrack, circles,dominated, edges, tri, dominate,adj);
                        continue;
                }*/
                VI& U_adj_list = head[u].adjacent;
                if (dominate[u]) {
                        // deleteVertex(u, tri, edges, adj, dominate, pone, degree_one, degree_two, dominated);
                        deleteVertex(u, pone, degree_one, degree_two);
                        for (int k = 0, U_size = U_adj_list.size(); k < U_size; k++) {
                                int v = U_adj_list[k];
                                if (!head[v].del && order[v] > i) {
                                        VI& N_adj_list = head[v].adjacent;
                                        for (int j = 0, N_size = N_adj_list.size(); j < N_size; j++)
                                                if (!head[N_adj_list[j]].del) {
                                                        int w = N_adj_list[j];
                                                        int eid = edge_id(v, j);
                                                        if (adj[w]) {
                                                                tri[eid]--;
                                                        }
                                                        if (!dominate[v] && tri[eid] + 1 == head[w].neiSize &&
                                                            head[v].weight <= head[w].weight) {
                                                                dominate[v] = 1;
                                                                dominated.push_back(v);
                                                        }
                                                        if (!dominate[w] && tri[eid] + 1 == head[v].neiSize &&
                                                            head[w].weight <= head[v].weight) {
                                                                dominate[w] = 1;
                                                                if (order[w] > i) dominated.push_back(w);
                                                        }
                                                }
                                }
                        }
                } else {
                        for (int j = 0, U_size = U_adj_list.size(); j < U_size; j++) {
                                int v = U_adj_list[j], eid = edge_id(u, j);
                                if (head[v].del) continue;
                                tri[eid] = 0;
                                VI& N_adj_list = head[v].adjacent;
                                for (int k = 0, N_size = N_adj_list.size(); k < N_size; k++) {
                                        int w = N_adj_list[k];
                                        if (head[w].del) continue;
                                        if (adj[w]) ++tri[eid];
                                }
                                if (!dominate[v] && tri[eid] + 1 == head[u].neiSize &&
                                    head[v].weight <= head[u].weight) {
                                        dominate[v] = 1;
                                        if (order[v] > i) dominated.push_back(v);
                                }
                        }
                }
                for (auto n : head[u].adjacent)
                        if (head[n].del == 0) {
                                adj[n] = 0;
                        }
                for (auto v : two_hop) {
                        AdjShare[v] = 0;
                }
                clearPone(pone, degree_one, degree_two, backTrack, circles, dominated, edges, tri, dominate, adj);
        }
        delete[] order;
        delete[] ids;
}

void Graph::deleteVertex(int u, int* tri, int* edges, bool* adj, bool* dominate, VI& pone, VI& degree_one,
                         VI& degree_two, VI& dominated) {
        if (head[u].del) return;
        VI& U_adj = head[u].adjacent;
        for (int i = 0, U_size = U_adj.size(); i < U_size; i++) {
                int v = U_adj[i];
                if (head[v].del) continue;
                adj[v] = 1;
                head[v].neiSize--;
                head[v].neiSum -= head[u].weight;
                if (head[v].neiSum <= head[v].weight) {
                        pone.push_back(v);
                } else if (head[v].neiSize == 1) {
                        degree_one.push_back(v);
                } else if (head[v].neiSize == 2) {
                        degree_two.push_back(v);
                }
        }
        for (int i = 0, U_size = U_adj.size(); i < U_size; i++) {
                int v = U_adj[i];
                if (head[v].del) continue;
                VI& N_adj = head[v].adjacent;
                for (int j = 0, N_size = N_adj.size(); j < N_size; j++) {
                        int w = N_adj[j];
                        if (head[w].del) continue;
                        int eid = edge_id(v, j);
                        if (adj[w]) {
                                tri[eid]--;
                        }
                        if (tri[eid] + 1 == head[v].neiSize && !dominate[w] && head[w].weight <= head[v].weight) {
                                dominated.push_back(w);
                                dominate[w] = 1;
                        }
                }
        }
        for (auto n : U_adj) {
                adj[n] = 0;
        }
        head[u].del = 1;
}

void Graph::htThreeAll() {
        // clear the invalid vertice��
        for (int i = 0; i < n; i++) {
                // clear the useless vertex
                if (head[i].weight <= 0) {
                        printf("Invalid weight:%d %d \n", head[i].weight, i);
                        return;
                }
        }
        clock_t start = clock();
        // ����ÿ�����ھ�Ȩֵ�͡�
        int* weight_copy = new int[n];
        for (int i = 0; i < n; i++) {
                weight_copy[i] = head[i].weight;
        }
        int max_d = 0;
        for (int i = 0; i < n; i++) {
                int sum = 0;
                for (auto v : head[i].adjacent) {
                        sum += head[v].weight;
                }
                head[i].neiSum = sum;
                head[i].neiSize = head[i].adjacent.size();
        }

        vector<int> pone, dominated, degree_two, degree_one;
        vector<pair<int, int>> backTrack;
        vector<vector<int>> circles;

        for (int i = 0; i < n; i++) {
                if (head[i].weight >= head[i].neiSum) {
                        pone.push_back(i);
                } else if (head[i].neiSize == 1) {
                        degree_one.push_back(i);
                } else if (head[i].neiSize == 2) {
                        degree_two.push_back(i);
                }
        }

        bool* adj = new bool[n];
        memset(adj, 0, sizeof(bool) * n);
        int* AdjShare = new int[n];
        memset(AdjShare, 0, sizeof(int) * n);

        clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 1);

        neighborhood_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        // symmetric_folding(pone, degree_one, degree_two, backTrack, circles, adj, AdjShare);

        common_neighbor_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        bfsReduction(pone, degree_one, degree_two, adj);
        //// reorganize the ajacent matrix.
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0 && head[i].neiSize > 0) {
                        int* neis = new int[head[i].neiSize];
                        int index = 0;
                        for (auto n : head[i].adjacent) {
                                if (head[n].del == 0) {
                                        neis[index++] = n;
                                }
                        }
                        if (index != head[i].neiSize) printf("error info: index != neisize\n");
                        head[i].adjacent.resize(index);
                        for (int j = 0; j < index; j++) {
                                head[i].adjacent[j] = neis[j];
                        }
                        delete[] neis;
                }
        }

        neighborhood_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        delete[] AdjShare;
        delete[] adj;

        int count = 0, d_two = 0;
        for (int k = 0; k < n; k++) {
                if (head[k].del == 0 && head[k].neiSize > 0) {
                        if (head[k].neiSum - head[k].weight > max_d) {
                                max_d = head[k].neiSum - head[k].weight;
                        }
                        if (head[k].neiSize == 2) d_two++;
                        count++;
                }
        }
        printf("degree_two_size: %d \nKernel Size:%d\n", d_two, count);

        int* bin_head = new int[max_d + 1];
        memset(bin_head, -1, sizeof(int) * (max_d + 1));
        int* bin_next = new int[n];
        for (int i = 0; i < n; i++) {
                if (head[i].del || head[i].neiSize < 1) continue;
                int dif = head[i].neiSum - head[i].weight;
                if (dif <= 0) continue;
                bin_next[i] = bin_head[dif];
                bin_head[dif] = i;
        }

        while (max_d > 0) {
                while (pone.empty() && degree_two.empty() && degree_one.empty()) {
                        while (max_d >= 0 && bin_head[max_d] == -1) max_d--;
                        if (max_d < 0) break;
                        int v = -1;
                        for (v = bin_head[max_d]; v != -1;) {
                                int tmp = bin_next[v];
                                int deg = head[v].neiSum - head[v].weight;
                                if (head[v].del == 0 && deg > 0) {
                                        if (deg < max_d) {
                                                bin_next[v] = bin_head[deg];
                                                bin_head[deg] = v;
                                        } else {
                                                deleteVertex(v, pone, degree_one, degree_two);
                                                backTrack.push_back(make_pair(v, n));
                                                bin_head[max_d] = tmp;
                                                break;
                                        }
                                }
                                v = tmp;
                        }
                        if (v == -1) bin_head[max_d] = -1;
                }
                // cannot add edge
                clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 0);
        }

        int greedy_include = 0, remainder = 0;
        for (int k = backTrack.size() - 1; k >= 0; k--) {
                int a = backTrack[k].first;
                bool ok = 1;
                if (a == n) {
                        int b = backTrack[k].second;
                        VI& T = circles.back();
                        int* tmp = nullptr;
                        int index = 0;
                        int T_size = T.size();
                        // attahed circle
                        if (b == 1) {
                                // if v
                                if (head[T[0]].del) {
                                        tmp = new int[T_size - 1];
                                        for (int i = 1; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                }
                        }
                        // circle ends with two adjacent vertex.
                        else if (b == 2) {
                                if (!head[T[0]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (!head[T[1]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 3; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (head[T[1]].del && head[T[0]].del) {
                                        tmp = new int[T_size - 2];
                                        for (int i = 2; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        printf("error info: T[0] and T[0] cann't exist at the same time\n");
                                }
                        }
                        path_recover(tmp, index);
                        delete[] tmp;
                        circles.pop_back();
                } else {
                        for (auto n : head[a].adjacent) {
                                if (head[n].del == 0) {
                                        ok = 0;
                                        break;
                                }
                        }
                        if (ok) {
                                head[a].del = 0;
                        }
                }
        }

        if (!circles.empty()) printf("error info: circles is not empty!\n");

        for (int i = 0; i < n; i++) {
                if (!head[i].del) total_weight += weight_copy[i];
        }

        clock_t end = clock();
        delete[] bin_head;
        delete[] bin_next;

        /*printf("solution:\n");
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0) {
                        printf("%d ", i);
                }
        }*/

        printf("total weight :%d ,  remainder : %d , total time cost : %f\n", total_weight, remainder,
               (float)(end - start) * 1000 / CLOCKS_PER_SEC);
        delete[] weight_copy;
}

void Graph::dtThreeAll() {
        // clear the invalid vertice��
        for (int i = 0; i < n; i++) {
                // clear the useless vertex
                if (head[i].weight <= 0) {
                        printf("Invalid weight 0\n");
                        return;
                }
        }
        clock_t start = clock();
        // ����ÿ�����ھ�Ȩֵ�͡�
        int* weight_copy = new int[n];
        for (int i = 0; i < n; i++) {
                weight_copy[i] = head[i].weight;
        }
        int max_d = 0;
        for (int i = 0; i < n; i++) {
                int sum = 0;
                for (auto v : head[i].adjacent) {
                        sum += head[v].weight;
                }
                head[i].neiSum = sum;
                head[i].neiSize = head[i].adjacent.size();
        }

        vector<int> pone, dominated, degree_two, degree_one;
        vector<pair<int, int>> backTrack;
        vector<vector<int>> circles;

        for (int i = 0; i < n; i++) {
                if (head[i].weight >= head[i].neiSum) {
                        pone.push_back(i);
                } else if (head[i].neiSize == 1) {
                        degree_one.push_back(i);
                } else if (head[i].neiSize == 2) {
                        degree_two.push_back(i);
                }
        }

        bool* adj = new bool[n];
        memset(adj, 0, sizeof(bool) * n);
        int* AdjShare = new int[n];
        memset(AdjShare, 0, sizeof(int) * n);

        clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 1);

        neighborhood_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        // symmetric_folding(pone, degree_one, degree_two, backTrack, circles, adj, AdjShare);

        common_neighbor_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        bfsReduction(pone, degree_one, degree_two, adj);
        //// reorganize the ajacent matrix.
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0 && head[i].neiSize > 0) {
                        int* neis = new int[head[i].neiSize];
                        int index = 0;
                        for (auto n : head[i].adjacent) {
                                if (head[n].del == 0) {
                                        neis[index++] = n;
                                }
                        }
                        if (index != head[i].neiSize) printf("error info: index != neisize\n");
                        head[i].adjacent.resize(index);
                        for (int j = 0; j < index; j++) {
                                head[i].adjacent[j] = neis[j];
                        }
                        delete[] neis;
                }
        }

        neighborhood_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        delete[] AdjShare;
        delete[] adj;

        int count = 0, d_two = 0;
        for (int k = 0; k < n; k++) {
                if (head[k].del == 0 && head[k].neiSize > 0) {
                        if (head[k].neiSize > max_d) {
                                max_d = head[k].neiSize;
                        }
                        if (head[k].neiSize == 2) d_two++;
                        count++;
                }
        }
        printf("degree_two_size: %d \nKernel Size:%d\n", d_two, count);

        int* bin_head = new int[max_d + 1];
        memset(bin_head, -1, sizeof(int) * (max_d + 1));
        int* bin_next = new int[n];
        for (int i = 0; i < n; i++) {
                if (head[i].del || head[i].neiSize < 1) continue;
                int dif = head[i].neiSize;
                if (dif <= 0) continue;
                bin_next[i] = bin_head[dif];
                bin_head[dif] = i;
        }

        while (max_d > 0) {
                while (pone.empty() && degree_two.empty() && degree_one.empty()) {
                        while (max_d >= 0 && bin_head[max_d] == -1) max_d--;
                        if (max_d < 0) break;
                        int v = -1;
                        for (v = bin_head[max_d]; v != -1;) {
                                int tmp = bin_next[v];
                                int deg = head[v].neiSize;
                                if (head[v].del == 0 && deg > 0) {
                                        if (deg < max_d) {
                                                bin_next[v] = bin_head[deg];
                                                bin_head[deg] = v;
                                        } else {
                                                deleteVertex(v, pone, degree_one, degree_two);
                                                backTrack.push_back(make_pair(v, n));
                                                bin_head[max_d] = tmp;
                                                break;
                                        }
                                }
                                v = tmp;
                        }
                        if (v == -1) bin_head[max_d] = -1;
                }
                // cannot add edge
                clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 0);
        }

        int greedy_include = 0, remainder = 0;
        for (int k = backTrack.size() - 1; k >= 0; k--) {
                int a = backTrack[k].first;
                bool ok = 1;
                if (a == n) {
                        int b = backTrack[k].second;
                        VI& T = circles.back();
                        int* tmp = nullptr;
                        int index = 0;
                        int T_size = T.size();
                        // attahed circle
                        if (b == 1) {
                                // if v
                                if (head[T[0]].del) {
                                        tmp = new int[T_size - 1];
                                        for (int i = 1; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                }
                        }
                        // circle ends with two adjacent vertex.
                        else if (b == 2) {
                                if (!head[T[0]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (!head[T[1]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 3; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (head[T[1]].del && head[T[0]].del) {
                                        tmp = new int[T_size - 2];
                                        for (int i = 2; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        printf("error info: T[0] and T[0] cann't exist at the same time\n");
                                }
                        }
                        path_recover(tmp, index);
                        delete[] tmp;
                        circles.pop_back();
                } else {
                        for (auto n : head[a].adjacent) {
                                if (head[n].del == 0) {
                                        ok = 0;
                                        break;
                                }
                        }
                        if (ok) {
                                head[a].del = 0;
                        }
                }
        }

        if (!circles.empty()) printf("error info: circles is not empty!\n");

        for (int i = 0; i < n; i++) {
                if (!head[i].del) total_weight += weight_copy[i];
        }

        clock_t end = clock();
        delete[] bin_head;
        delete[] bin_next;

        /*printf("solution:\n");
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0) {
                        printf("%d ", i);
                }
        }*/

        printf("total weight :%d ,  remainder : %d , total time cost : %f\n", total_weight, remainder,
               (float)(end - start) * 1000 / CLOCKS_PER_SEC);
        delete[] weight_copy;
}

void Graph::wtThreeAll() {
        // clear the invalid vertice��
        for (int i = 0; i < n; i++) {
                // clear the useless vertex
                if (head[i].weight <= 0) {
                        printf("Invalid weight 0\n");
                        return;
                }
        }
        clock_t start = clock();
        // ����ÿ�����ھ�Ȩֵ�͡�
        int* weight_copy = new int[n];
        for (int i = 0; i < n; i++) {
                weight_copy[i] = head[i].weight;
        }
        int max_d = 0;
        for (int i = 0; i < n; i++) {
                int sum = 0;
                for (auto v : head[i].adjacent) {
                        sum += head[v].weight;
                }
                head[i].neiSum = sum;
                head[i].neiSize = head[i].adjacent.size();
        }

        vector<int> pone, dominated, degree_two, degree_one;
        vector<pair<int, int>> backTrack;
        vector<vector<int>> circles;

        for (int i = 0; i < n; i++) {
                if (head[i].weight >= head[i].neiSum) {
                        pone.push_back(i);
                } else if (head[i].neiSize == 1) {
                        degree_one.push_back(i);
                } else if (head[i].neiSize == 2) {
                        degree_two.push_back(i);
                }
        }

        bool* adj = new bool[n];
        memset(adj, 0, sizeof(bool) * n);
        int* AdjShare = new int[n];
        memset(AdjShare, 0, sizeof(int) * n);

        clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 1);

        neighborhood_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        // symmetric_folding(pone, degree_one, degree_two, backTrack, circles, adj, AdjShare);

        common_neighbor_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        bfsReduction(pone, degree_one, degree_two, adj);
        //// reorganize the ajacent matrix.
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0 && head[i].neiSize > 0) {
                        int* neis = new int[head[i].neiSize];
                        int index = 0;
                        for (auto n : head[i].adjacent) {
                                if (head[n].del == 0) {
                                        neis[index++] = n;
                                }
                        }
                        if (index != head[i].neiSize) printf("error info: index != neisize\n");
                        head[i].adjacent.resize(index);
                        for (int j = 0; j < index; j++) {
                                head[i].adjacent[j] = neis[j];
                        }
                        delete[] neis;
                }
        }

        neighborhood_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        delete[] AdjShare;
        delete[] adj;

        int count = 0, d_two = 0;
        for (int k = 0; k < n; k++) {
                if (head[k].del == 0 && head[k].neiSize > 0) {
                        if (head[k].weight > max_d) {
                                max_d = head[k].weight;
                        }
                        if (head[k].neiSize == 2) d_two++;
                        count++;
                }
        }
        printf("degree_two_size: %d \nKernel Size:%d\n", d_two, count);

        int* bin_head = new int[max_d + 1];
        memset(bin_head, -1, sizeof(int) * (max_d + 1));
        int* bin_next = new int[n];
        for (int i = 0; i < n; i++) {
                if (head[i].del || head[i].neiSize < 1) continue;
                int dif = head[i].weight;
                if (dif <= 0) continue;
                bin_next[i] = bin_head[dif];
                bin_head[dif] = i;
        }

        int idx = 1;
        while (idx <= max_d) {
                while (pone.empty()) {
                        while (idx <= max_d && bin_head[idx] == -1) idx++;
                        if (idx > max_d) break;
                        int v = -1;
                        for (v = bin_head[idx]; v != -1;) {
                                int tmp = bin_next[v];
                                int deg = head[v].weight;
                                if (head[v].del == 0 && deg > 0) {
                                        deleteVertex(v, pone, degree_one, degree_two);
                                        backTrack.push_back(make_pair(v, n));
                                        bin_head[idx] = tmp;
                                        break;
                                }
                                v = tmp;
                        }
                        if (v == -1) bin_head[idx] = -1;
                }
                // cannot add edge
                clearsingle(adj, pone, degree_one, degree_two);
                // clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 0);
        }

        int greedy_include = 0, remainder = 0;
        for (int k = backTrack.size() - 1; k >= 0; k--) {
                int a = backTrack[k].first;
                bool ok = 1;
                if (a == n) {
                        int b = backTrack[k].second;
                        VI& T = circles.back();
                        int* tmp = nullptr;
                        int index = 0;
                        int T_size = T.size();
                        // attahed circle
                        if (b == 1) {
                                // if v
                                if (head[T[0]].del) {
                                        tmp = new int[T_size - 1];
                                        for (int i = 1; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                }
                        }
                        // circle ends with two adjacent vertex.
                        else if (b == 2) {
                                if (!head[T[0]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (!head[T[1]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 3; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (head[T[1]].del && head[T[0]].del) {
                                        tmp = new int[T_size - 2];
                                        for (int i = 2; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        printf("error info: T[0] and T[0] cann't exist at the same time\n");
                                }
                        }
                        path_recover(tmp, index);
                        delete[] tmp;
                        circles.pop_back();
                } else {
                        for (auto n : head[a].adjacent) {
                                if (head[n].del == 0) {
                                        ok = 0;
                                        break;
                                }
                        }
                        if (ok) {
                                head[a].del = 0;
                        }
                }
        }

        if (!circles.empty()) printf("error info: circles is not empty!\n");

        for (int i = 0; i < n; i++) {
                if (!head[i].del) total_weight += weight_copy[i];
        }

        clock_t end = clock();
        delete[] bin_head;
        delete[] bin_next;

        /*printf("solution:\n");
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0) {
                        printf("%d ", i);
                }
        }*/

        printf("total weight :%d ,  remainder : %d , total time cost : %f\n", total_weight, remainder,
               (float)(end - start) * 1000 / CLOCKS_PER_SEC);
        delete[] weight_copy;
}

void Graph::path_recover(int* V, int num) {
        int* dp = new int[num];
        int* track = new int[num];

        for (int i = 0; i < num; i++) {
                dp[i] = head[V[i]].weight;
        }
        track[0] = 1;
        dp[1] >= dp[0] ? track[1] = 1 : track[1] = 0;
        for (int i = 2; i < num; i++) {
                head[V[i]].weight + dp[i - 2] >= dp[i - 1] ? (track[i] = 1, dp[i] = head[V[i]].weight + dp[i - 2])
                                                           : (track[i] = 0, dp[i] = dp[i - 1]);
        }
        for (int i = num - 1; i >= 0;) {
                if (track[i]) {
                        head[V[i]].del = 0;
                        i -= 2;
                } else {
                        i--;
                }
        }
        delete[] dp;
        delete[] track;
}

void Graph::outputGraph() {
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0 && head[i].neiSize > 0) {
                        printf("%d\t%d\t%d\t%d\n ", i, head[i].weight, head[i].neiSize, head[i].neiSum);
                        /*for (auto n : head[i].adjacent) if (!head[n].del) {
                                printf("%d ", n);
                        }*/
                        // printf("\n");
                }
        }
}

int Graph::edgeHandler(vector<int>& list, int len) {
        switch (len) {
                case 2:
                        return oneEdge(list);
                case 4:
                        return twoEdge(list);
                case 6:
                        return threeEdge(list);
        }
}

int Graph::oneEdge(vector<int>& list) { return max(head[list[0]].weight, head[list[1]].weight); }

int Graph::twoEdge(vector<int>& list) {
        int a = list[0], b = list[1], c = list[2], d = list[3];
        int a_w = head[a].weight, b_w = head[b].weight, c_w = head[c].weight, d_w = head[d].weight;
        if (c == a) {
                return max(a_w, b_w + d_w);
        } else if (c == b) {
                return max(b_w, a_w + d_w);
        } else if (d == a) {
                return max(a_w, b_w + c_w);
        } else if (d == b) {
                return max(b_w, a_w + c_w);
        } else {
                return max(a_w, b_w) + max(c_w, d_w);
        }
}

int Graph::threeEdge(vector<int>& list) {
        map<int, int> m1, m2;
        int index = 0, res = 0;
        VI deg(6);
        for (int i = 0; i < 6; i++) {
                int ver = list[i];
                if (m1.find(ver) == m1.end()) {
                        m1.insert(make_pair(ver, index));
                        m2.insert(make_pair(index, ver));
                        index++;
                }
                deg[m1[ver]]++;
        }

        if (index == 6) {
                return max(head[list[0]].weight, head[list[1]].weight) +
                       max(head[list[2]].weight, head[list[3]].weight) +
                       max(head[list[4]].weight, head[list[5]].weight);
        }
        if (index == 3) {
                return max(max(head[list[0]].weight, head[list[1]].weight),
                           max(head[list[2]].weight, head[list[3]].weight));
        }
        VI one, two, three;
        for (int i = 0; i < index; i++) {
                switch (deg[i]) {
                        case 1:
                                one.push_back(m2[i]);
                                break;
                        case 2:
                                two.push_back(m2[i]);
                                break;
                        case 3:
                                three.push_back(m2[i]);
                                break;
                }
        }
        if (index == 5) {
                int max1 = 0, max2 = 0;
                int deg2v = two[0];
                for (int i = 0; i < 6; i += 2) {
                        if (deg[m1[list[i]]] == 2 || deg[m1[list[i + 1]]] == 2) {
                                max1 += head[deg2v ^ list[i] ^ list[i + 1]].weight;
                        } else {
                                max2 = max(head[list[i]].weight, head[list[i + 1]].weight);
                        }
                }
                max1 = max(max1, head[deg2v].weight);
                return max1 + max2;
        }
        if (three.size() == 1) {
                return max(head[three[0]].weight, head[one[0]].weight + head[one[1]].weight + head[one[2]].weight);
        }
        int ver1_1 = one[0], ver1_2 = one[1];
        int wei1 = head[ver1_1].weight + head[ver1_2].weight;
        int two1 = two[0], one1 = one[0];
        bool leap = true;
        for (int i = 0; i < 6; i += 2) {
                if ((list[i] == two1 && list[i + 1] == one1) || (list[i + 1] == two1 && list[i] == one1)) {
                        leap = false;
                        break;
                }
        }
        int ver2_1, ver2_2, ver3_1, ver3_2;
        int wei2 = 0, wei3 = 0;
        if (leap) {
                ver2_1 = one1, ver2_2 = two1;
                wei2 = head[one1].weight + head[two1].weight;
                ver3_1 = one[1], ver3_2 = two[1];
                wei3 = head[ver3_1].weight + head[ver3_2].weight;
        } else {
                ver2_1 = one1, ver2_2 = two[1];
                wei2 = head[one1].weight + head[ver2_2].weight;
                ver3_1 = two1, ver3_2 = one[1];
                wei3 = head[two1].weight + head[ver3_2].weight;
        }
        return max(wei1, max(wei2, wei3));
}

// delete a vset (for neighbors)
void Graph::deleteVertex(int id, int par, VI& pone, VI& dominate, VI& degree_one, VI& degree_two) {
        if (head[id].del == 1) {
                return;
        } else {
                VI& s = head[id].adjacent;
                for (auto nei : s) {
                        if (head[nei].del) continue;
                        head[nei].neiSum -= head[id].weight;
                        head[nei].neiSize--;
                        if (nei != par) {
                                if (head[nei].weight >= head[nei].neiSum) {
                                        pone.push_back(nei);
                                } else if (head[nei].neiSize == 1) {
                                        degree_one.push_back(nei);
                                } else if (head[nei].neiSize == 2) {
                                        degree_two.push_back(nei);
                                }
                        }
                }
                head[id].neiSize = 0;
                head[id].del = 1;
        }
}

// delete a vset (for neighbors)
void Graph::deleteVertex(int id, int par, VI& pone, VI& degree_one, VI& degree_two) {
        if (head[id].del == 1) {
                return;
        } else {
                VI& s = head[id].adjacent;
                for (auto nei : s) {
                        if (head[nei].del == 1) continue;
                        head[nei].neiSum -= head[id].weight;
                        head[nei].neiSize--;
                        if (nei != par) {
                                if (head[nei].weight >= head[nei].neiSum) {
                                        pone.push_back(nei);
                                } else if (head[nei].neiSize == 1) {
                                        degree_one.push_back(nei);
                                } else if (head[nei].neiSize == 2) {
                                        degree_two.push_back(nei);
                                }
                        }
                }
                head[id].neiSize = 0;
                head[id].del = 1;
        }
}

// designed for greedy_pickout;
void Graph::deleteVertex(int id, VI& pone, VI& dominate, VI& degree_one, VI& degree_two) {
        if (head[id].del == 1) {
                return;
        }
        VI& s = head[id].adjacent;
        for (auto nei : s) {
                if (head[nei].del == 1) continue;
                head[nei].neiSum -= head[id].weight;
                head[nei].neiSize--;
                if (head[nei].weight >= head[nei].neiSum) {
                        pone.push_back(nei);
                } else if (head[nei].neiSize == 1) {
                        degree_one.push_back(nei);
                } else if (head[nei].neiSize == 2) {
                        degree_two.push_back(nei);
                }
        }
        head[id].del = 1;
}

// designed for greedy_pickout;
void Graph::deleteVertex(int id, VI& pone, VI& degree_one, VI& degree_two) {
        if (head[id].del == 1) {
                return;
        }
        VI& s = head[id].adjacent;
        for (auto nei : s) {
                if (head[nei].del == 1) continue;
                head[nei].neiSum -= head[id].weight;
                head[nei].neiSize--;
                if (head[nei].weight >= head[nei].neiSum) {
                        pone.push_back(nei);
                } else if (head[nei].neiSize == 1) {
                        degree_one.push_back(nei);
                } else if (head[nei].neiSize == 2) {
                        degree_two.push_back(nei);
                }
        }
        head[id].del = 1;
}

void Graph::htDegreeOne() {
        for (int i = 0; i < n; i++) {
                // clear the useless vertex
                if (head[i].weight <= 0) {
                        printf("Invalid weight 0\n");
                        return;
                }
        }
        clock_t start = clock();
        // ����ÿ�����ھ�Ȩֵ�͡�
        int* weight_copy = new int[n];
        for (int i = 0; i < n; i++) {
                weight_copy[i] = head[i].weight;
        }
        int max_d = 0;
        for (int i = 0; i < n; i++) {
                int sum = 0;
                for (auto v : head[i].adjacent) {
                        sum += head[v].weight;
                }
                head[i].neiSum = sum;
                head[i].neiSize = head[i].adjacent.size();
        }

        vector<int> pone, dominated, degree_two, degree_one;
        vector<pair<int, int>> backTrack;
        vector<vector<int>> circles;

        for (int i = 0; i < n; i++) {
                if (head[i].weight >= head[i].neiSum) {
                        pone.push_back(i);
                } else if (head[i].neiSize == 1) {
                        degree_one.push_back(i);
                } else if (head[i].neiSize == 2) {
                        degree_two.push_back(i);
                }
        }

        bool* adj = new bool[n];
        memset(adj, 0, sizeof(bool) * n);
        int* AdjShare = new int[n];
        memset(AdjShare, 0, sizeof(int) * n);

        printf("low degree:%d %d \n", degree_one.size(), degree_two.size());

        // clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 1);

        while (!degree_one.empty()) {
                int v = degree_one.back();
                degree_one.pop_back();
                if (head[v].neiSize != 1 || head[v].del) {
                        continue;
                }
                int neibor = -1;
                for (auto n : head[v].adjacent) {
                        if (head[n].del == 0) {
                                neibor = n;
                                break;
                        }
                }

                if (head[neibor].weight > head[v].weight) {
                        head[neibor].weight -= head[v].weight;
                        deleteVertex(v, pone, degree_one, degree_two);
                        backTrack.push_back(make_pair(v, neibor));
                        for (auto n : head[neibor].adjacent)
                                if (head[n].del == 0) {
                                        head[n].neiSum -= head[v].weight;
                                        if (head[n].neiSum <= head[n].weight) {
                                                pone.push_back(n);
                                        }
                                }
                } else {
                        deleteVertex(neibor, pone, degree_one, degree_two);
                }
        }

        delete[] AdjShare;
        delete[] adj;

        int count = 0, d_two = 0;
        for (int k = 0; k < n; k++) {
                if (head[k].del == 0 && head[k].neiSize > 0) {
                        if (head[k].neiSum - head[k].weight > max_d) {
                                max_d = head[k].neiSum - head[k].weight;
                        }
                        if (head[k].neiSize == 2) d_two++;
                        count++;
                }
        }
        printf("degree_two_size: %d \nKernel Size:%d\n", d_two, count);

        int* bin_head = new int[max_d + 1];
        memset(bin_head, -1, sizeof(int) * (max_d + 1));
        int* bin_next = new int[n];
        for (int i = 0; i < n; i++) {
                if (head[i].del || head[i].neiSize < 1) continue;
                int dif = head[i].neiSum - head[i].weight;
                if (dif <= 0) continue;
                bin_next[i] = bin_head[dif];
                bin_head[dif] = i;
        }

        while (max_d > 0) {
                while (pone.empty() && degree_two.empty() && degree_one.empty()) {
                        while (max_d >= 0 && bin_head[max_d] == -1) max_d--;
                        if (max_d < 0) break;
                        int v = -1;
                        for (v = bin_head[max_d]; v != -1;) {
                                int tmp = bin_next[v];
                                int deg = head[v].neiSum - head[v].weight;
                                if (head[v].del == 0 && deg > 0) {
                                        if (deg < max_d) {
                                                bin_next[v] = bin_head[deg];
                                                bin_head[deg] = v;
                                        } else {
                                                deleteVertex(v, pone, degree_one, degree_two);
                                                backTrack.push_back(make_pair(v, n));
                                                bin_head[max_d] = tmp;
                                                break;
                                        }
                                }
                                v = tmp;
                        }
                        if (v == -1) bin_head[max_d] = -1;
                }
                // cannot add edge
                clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 0);
        }

        int greedy_include = 0, remainder = 0;
        for (int k = backTrack.size() - 1; k >= 0; k--) {
                int a = backTrack[k].first;
                bool ok = 1;
                if (a == n) {
                        int b = backTrack[k].second;
                        VI& T = circles.back();
                        int* tmp = nullptr;
                        int index = 0;
                        int T_size = T.size();
                        // attahed circle
                        if (b == 1) {
                                // if v
                                if (head[T[0]].del) {
                                        tmp = new int[T_size - 1];
                                        for (int i = 1; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                }
                        }
                        // circle ends with two adjacent vertex.
                        else if (b == 2) {
                                if (!head[T[0]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (!head[T[1]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 3; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (head[T[1]].del && head[T[0]].del) {
                                        tmp = new int[T_size - 2];
                                        for (int i = 2; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        printf("error info: T[0] and T[0] cann't exist at the same time\n");
                                }
                        }
                        path_recover(tmp, index);
                        delete[] tmp;
                        circles.pop_back();
                } else {
                        for (auto n : head[a].adjacent) {
                                if (head[n].del == 0) {
                                        ok = 0;
                                        break;
                                }
                        }
                        if (ok) {
                                head[a].del = 0;
                        }
                }
        }

        if (!circles.empty()) printf("error info: circles is not empty!\n");

        for (int i = 0; i < n; i++) {
                if (!head[i].del) total_weight += weight_copy[i];
        }

        clock_t end = clock();
        delete[] bin_head;
        delete[] bin_next;

        /*printf("solution:\n");
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0) {
                        printf("%d ", i);
                }
        }*/

        printf("total weight :%d ,  remainder : %d , total time cost : %f\n", total_weight, remainder,
               (float)(end - start) * 1000 / CLOCKS_PER_SEC);
        delete[] weight_copy;
}

void Graph::htDegreeTwo() {
        // clear the invalid vertice��
        for (int i = 0; i < n; i++) {
                // clear the useless vertex
                if (head[i].weight <= 0) {
                        printf("Invalid weight 0\n");
                        return;
                }
        }
        clock_t start = clock();
        // ����ÿ�����ھ�Ȩֵ�͡�
        int* weight_copy = new int[n];
        for (int i = 0; i < n; i++) {
                weight_copy[i] = head[i].weight;
        }
        int max_d = 0;
        for (int i = 0; i < n; i++) {
                int sum = 0;
                for (auto v : head[i].adjacent) {
                        sum += head[v].weight;
                }
                head[i].neiSum = sum;
                head[i].neiSize = head[i].adjacent.size();
        }

        vector<int> pone, dominated, degree_two, degree_one;
        vector<pair<int, int>> backTrack;
        vector<vector<int>> circles;

        for (int i = 0; i < n; i++) {
                if (head[i].weight >= head[i].neiSum) {
                        pone.push_back(i);
                } else if (head[i].neiSize == 1) {
                        degree_one.push_back(i);
                } else if (head[i].neiSize == 2) {
                        degree_two.push_back(i);
                }
        }

        bool* adj = new bool[n];
        memset(adj, 0, sizeof(bool) * n);
        int* AdjShare = new int[n];
        memset(AdjShare, 0, sizeof(int) * n);

        while (!degree_two.empty()) {
                // path reduction.
                int v = degree_two.back();
                degree_two.pop_back();
                if (head[v].neiSize != 2 || head[v].del) continue;
                int n1, n2;
                findTwoNeibor(v, n1, n2);
                if (head[v].weight >= head[v].neiSum) {
                        deleteVertex(n1, pone, degree_one, degree_two);
                        deleteVertex(n2, pone, degree_one, degree_two);
                        continue;
                }
                // if v,n1,n1 forms a triangle;
                if (isNei(n1, n2)) {
                        if (head[n1].weight <= head[v].weight) {
                                deleteVertex(n1, pone, degree_one, degree_two);
                                continue;
                        }
                        if (head[n2].weight <= head[v].weight) {
                                deleteVertex(n2, pone, degree_one, degree_two);
                                continue;
                        }

                        head[n1].weight -= head[v].weight;
                        deleteVertex(v, pone, degree_one, degree_two);
                        for (auto n : head[n1].adjacent)
                                if (head[n].del == 0) {
                                        head[n].neiSum -= head[v].weight;
                                        if (head[n].neiSum <= head[n].weight) {
                                                pone.push_back(n);
                                        }
                                }

                        head[n2].weight -= head[v].weight;
                        deleteVertex(v, pone, degree_one, degree_two);
                        for (auto n : head[n2].adjacent)
                                if (head[n].del == 0) {
                                        head[n].neiSum -= head[v].weight;
                                        if (head[n].neiSum <= head[n].weight) {
                                                pone.push_back(n);
                                        }
                                }

                        backTrack.push_back(make_pair(v, n + 1));

                        continue;
                }
                // if n1,n2 is not adjacent;
                else {
                        if (1) {  // edges are not allowed to be inserted.
                                // if v > n1 and v > n2
                                if (head[v].weight >= min(head[n1].weight, head[n2].weight)) {
                                        int bigger = n1, smaller = n2;
                                        if (head[n2].weight > head[n1].weight) {
                                                bigger = n2, smaller = n1;
                                        }
                                        for (auto n : head[smaller].adjacent)
                                                if (!head[n].del) {
                                                        adj[n] = 1;
                                                }
                                        for (auto n : head[bigger].adjacent)
                                                if (!head[n].del && !adj[n]) {
                                                        head[smaller].adjacent.push_back(n);
                                                        head[smaller].neiSize++;
                                                        head[smaller].neiSum += head[n].weight;
                                                        head[n].adjacent.push_back(smaller);
                                                        head[n].neiSize++;
                                                        head[n].neiSum += head[smaller].weight;
                                                }
                                        for (auto n : head[smaller].adjacent)
                                                if (!head[n].del) {
                                                        adj[n] = 0;
                                                }
                                        if (head[v].weight >= max(head[n1].weight, head[n2].weight)) {
                                                head[smaller].weight += head[bigger].weight;
                                                for (auto nei : head[smaller].adjacent)
                                                        if (!head[nei].del) {
                                                                head[nei].neiSum += head[bigger].weight;
                                                        }
                                                deleteVertex(bigger, pone, degree_one, degree_two);
                                                backTrack.push_back(make_pair(bigger, n));
                                                continue;
                                        } else {
                                                head[bigger].weight -= head[v].weight;
                                                for (auto nei : head[bigger].adjacent)
                                                        if (!head[nei].del) {
                                                                head[nei].neiSum -= head[v].weight;
                                                                if (head[nei].weight >= head[nei].neiSum && nei != v) {
                                                                        pone.push_back(nei);
                                                                }
                                                        }
                                                deleteVertex(v, pone, degree_one, degree_two);
                                                backTrack.push_back(make_pair(v, n));
                                        }
                                        continue;
                                }
                        }
                }

                // printf("")
                deque<int> deq;
                int next = n2, prev = v;
                deq.push_back(v);
                while (head[next].neiSize == 2) {
                        deq.push_back(next);
                        int v1, v2;
                        findTwoNeibor(next, v1, v2);
                        int t = next;
                        v1 == prev ? (next = v2) : (next = v1);
                        prev = t;
                        if (next == v) break;
                }
                int final2 = next;
                int deq_size = deq.size();
                // path reduction #1 the path forms a separate circle
                if (next == v) {
                        if (deq_size < 4) printf("error code:001\n");
                        int* dp = new int[deq_size];
                        int index = 0;
                        for (int j = 1; j < deq_size; j++) {
                                dp[index++] = head[deq[j]].weight;
                        }
                        int dp_sum1 = 0, dp_sum2 = 0;
                        dp_sum1 = path_reduction(dp, 1, index - 1);
                        index = 0;
                        for (int j = 1; j < deq_size; j++) {
                                dp[index++] = head[deq[j]].weight;
                        }
                        dp_sum2 = path_reduction(dp, 0, index);

                        if (dp_sum2 > dp_sum1 + head[v].weight) {
                                deleteVertex(v, pone, degree_one, degree_two);
                        } else {
                                for (auto n : head[v].adjacent)
                                        if (!head[n].del) {
                                                deleteVertex(n, pone, degree_one, degree_two);
                                        }
                        }
                        delete[] dp;

                        continue;
                }
                next = n1, prev = v;
                while (head[next].neiSize == 2) {
                        deq.push_front(next);
                        int v1, v2;
                        findTwoNeibor(next, v1, v2);
                        int t = next;
                        v1 == prev ? (next = v2) : (next = v1);
                        prev = t;
                }
                deq_size = deq.size();

                // path reduction #2 path formes an attached circle.
                if (next == final2) {
                        // printf("attached circle\n");
                        int delta = 0;
                        if (deq_size < 3) {
                                printf("error code:101");
                        }
                        int* dp = new int[deq_size + 1];
                        for (int j = 0; j < deq_size; j++) {
                                dp[j] = head[deq[j]].weight;
                        }
                        int dp_sum1 = 0, dp_sum2 = 0;
                        dp_sum1 = path_reduction(dp, 0, deq_size);
                        for (int j = 0; j < deq_size; j++) {
                                dp[j] = head[deq[j]].weight;
                        }
                        dp_sum2 = path_reduction(dp, 1, deq_size - 1);
                        delete[] dp;
                        delta = dp_sum1 - dp_sum2;
                        if (head[next].weight > delta) {
                                backTrack.push_back(make_pair(n, 1));
                                circles.push_back(vector<int>());
                                VI& T = circles.back();
                                T.push_back(next);

                                head[next].weight -= delta;

                                for (auto x : deq) {
                                        deleteVertex(x, pone, degree_one, degree_two);
                                        T.push_back(x);
                                }

                                // head[next].neiSum -= head[deq[0]].weight + head[deq[deq_size-1]].weight;
                                // head[next].neiSize -= 2;

                                /*if (head[next].weight >= head[next].neiSum) { pone.push_back(next); }
                                else if (head[next].neiSize == 1) { degree_one.push_back(next); }
                                else if (head[next].neiSize == 2) { degree_two.push_back(next); }*/

                                for (auto n : head[next].adjacent) {
                                        if (head[n].del == 0) {
                                                head[n].neiSum -= delta;
                                                if (head[n].neiSum <= head[n].weight && n != deq[0] &&
                                                    n != deq[deq_size - 1]) {
                                                        pone.push_back(n);
                                                }
                                        }
                                }

                        } else {
                                deleteVertex(next, pone, degree_one, degree_two);
                        }
                }
                //// path reduction #3 the non-degree-two vertices are adjacent.
                else if (isNei(next, final2)) {
                        int dp_sum = 0, dp_b = 0, dp_f = 0;
                        int* dp = new int[deq_size + 1];
                        for (int j = 0; j < deq_size; j++) {
                                dp[j] = head[deq[j]].weight;
                        }
                        dp_sum = path_reduction(dp, 0, deq_size);
                        for (int j = 0; j < deq_size; j++) {
                                dp[j] = head[deq[j]].weight;
                        }
                        dp_b = path_reduction(dp, 1, deq_size);
                        for (int j = 0; j < deq_size; j++) {
                                dp[j] = head[deq[j]].weight;
                        }
                        dp_f = path_reduction(dp, 0, deq_size - 1);
                        if (dp_sum >= head[next].weight + dp_b) {
                                deleteVertex(next, pone, degree_one, degree_two);
                        } else if (dp_sum >= head[final2].weight + dp_f) {
                                deleteVertex(final2, pone, degree_one, degree_two);
                        } else {
                                backTrack.push_back(make_pair(n, 2));
                                circles.push_back(vector<int>());
                                VI& T = circles.back();
                                T.push_back(final2);
                                T.push_back(next);
                                int delta = dp_sum - dp_b;
                                head[next].weight -= delta;
                                for (auto n : head[next].adjacent)
                                        if (head[n].del == 0) {
                                                head[n].neiSum -= delta;
                                                if (head[n].neiSum <= head[n].weight && n != deq[0] &&
                                                    n != deq[deq_size - 1]) {
                                                        pone.push_back(n);
                                                }
                                        }

                                delta = dp_sum - dp_f;
                                head[final2].weight -= delta;
                                for (auto n : head[final2].adjacent)
                                        if (head[n].del == 0) {
                                                head[n].neiSum -= delta;
                                                if (head[n].neiSum <= head[n].weight && n != deq[0] &&
                                                    n != deq[deq_size - 1]) {
                                                        pone.push_back(n);
                                                }
                                        }

                                for (auto x : deq) {
                                        deleteVertex(x, pone, degree_one, degree_two);
                                        T.push_back(x);
                                }
                        }
                }

                // path-reduction #4 the non-degree-two vertices are not adjacent.
        }

        // neighborhood_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        delete[] AdjShare;
        delete[] adj;

        int count = 0, d_two = 0;
        for (int k = 0; k < n; k++) {
                if (head[k].del == 0 && head[k].neiSize > 0) {
                        if (head[k].neiSum - head[k].weight > max_d) {
                                max_d = head[k].neiSum - head[k].weight;
                        }
                        if (head[k].neiSize == 2) d_two++;
                        count++;
                }
        }
        printf("degree_two_size: %d \nKernel Size:%d\n", d_two, count);

        int* bin_head = new int[max_d + 1];
        memset(bin_head, -1, sizeof(int) * (max_d + 1));
        int* bin_next = new int[n];
        for (int i = 0; i < n; i++) {
                if (head[i].del || head[i].neiSize < 1) continue;
                int dif = head[i].neiSum - head[i].weight;
                if (dif <= 0) continue;
                bin_next[i] = bin_head[dif];
                bin_head[dif] = i;
        }

        while (max_d > 0) {
                while (pone.empty() && degree_two.empty() && degree_one.empty()) {
                        while (max_d >= 0 && bin_head[max_d] == -1) max_d--;
                        if (max_d < 0) break;
                        int v = -1;
                        for (v = bin_head[max_d]; v != -1;) {
                                int tmp = bin_next[v];
                                int deg = head[v].neiSum - head[v].weight;
                                if (head[v].del == 0 && deg > 0) {
                                        if (deg < max_d) {
                                                bin_next[v] = bin_head[deg];
                                                bin_head[deg] = v;
                                        } else {
                                                deleteVertex(v, pone, degree_one, degree_two);
                                                backTrack.push_back(make_pair(v, n));
                                                bin_head[max_d] = tmp;
                                                break;
                                        }
                                }
                                v = tmp;
                        }
                        if (v == -1) bin_head[max_d] = -1;
                }
                // cannot add edge
                clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 0);
        }

        int greedy_include = 0, remainder = 0;
        for (int k = backTrack.size() - 1; k >= 0; k--) {
                int a = backTrack[k].first;
                bool ok = 1;
                if (a == n) {
                        int b = backTrack[k].second;
                        VI& T = circles.back();
                        int* tmp = nullptr;
                        int index = 0;
                        int T_size = T.size();
                        // attahed circle
                        if (b == 1) {
                                // if v
                                if (head[T[0]].del) {
                                        tmp = new int[T_size - 1];
                                        for (int i = 1; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                }
                        }
                        // circle ends with two adjacent vertex.
                        else if (b == 2) {
                                if (!head[T[0]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (!head[T[1]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 3; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (head[T[1]].del && head[T[0]].del) {
                                        tmp = new int[T_size - 2];
                                        for (int i = 2; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        printf("error info: T[0] and T[0] cann't exist at the same time\n");
                                }
                        }
                        path_recover(tmp, index);
                        delete[] tmp;
                        circles.pop_back();
                } else {
                        for (auto n : head[a].adjacent) {
                                if (head[n].del == 0) {
                                        ok = 0;
                                        break;
                                }
                        }
                        if (ok) {
                                head[a].del = 0;
                        }
                }
        }

        if (!circles.empty()) printf("error info: circles is not empty!\n");

        for (int i = 0; i < n; i++) {
                if (!head[i].del) total_weight += weight_copy[i];
        }

        clock_t end = clock();
        delete[] bin_head;
        delete[] bin_next;

        /*printf("solution:\n");
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0) {
                        printf("%d ", i);
                }
        }*/

        printf("total weight :%d ,  remainder : %d , total time cost : %f\n", total_weight, remainder,
               (float)(end - start) * 1000 / CLOCKS_PER_SEC);
        delete[] weight_copy;
}

void Graph::htCommon() {
        // clear the invalid vertice��
        for (int i = 0; i < n; i++) {
                // clear the useless vertex
                if (head[i].weight <= 0) {
                        printf("Invalid weight 0\n");
                        return;
                }
        }
        clock_t start = clock();
        // ����ÿ�����ھ�Ȩֵ�͡�
        int* weight_copy = new int[n];
        for (int i = 0; i < n; i++) {
                weight_copy[i] = head[i].weight;
        }
        int max_d = 0;
        for (int i = 0; i < n; i++) {
                int sum = 0;
                for (auto v : head[i].adjacent) {
                        sum += head[v].weight;
                }
                head[i].neiSum = sum;
                head[i].neiSize = head[i].adjacent.size();
        }

        vector<int> pone, dominated, degree_two, degree_one;
        vector<pair<int, int>> backTrack;
        vector<vector<int>> circles;

        for (int i = 0; i < n; i++) {
                if (head[i].weight >= head[i].neiSum) {
                        pone.push_back(i);
                } else if (head[i].neiSize == 1) {
                        degree_one.push_back(i);
                } else if (head[i].neiSize == 2) {
                        degree_two.push_back(i);
                }
        }

        bool* adj = new bool[n];
        memset(adj, 0, sizeof(bool) * n);
        int* AdjShare = new int[n];
        memset(AdjShare, 0, sizeof(int) * n);

        // clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 1);

        //(pone, degree_one, degree_two, backTrack, circles, adj);

        // symmetric_folding(pone, degree_one, degree_two, backTrack, circles, adj, AdjShare);

        common_neighbor_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        // bfsReduction(pone, degree_one, degree_two, adj);
        //// reorganize the ajacent matrix.
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0 && head[i].neiSize > 0) {
                        int* neis = new int[head[i].neiSize];
                        int index = 0;
                        for (auto n : head[i].adjacent) {
                                if (head[n].del == 0) {
                                        neis[index++] = n;
                                }
                        }
                        if (index != head[i].neiSize) printf("error info: index != neisize\n");
                        head[i].adjacent.resize(index);
                        for (int j = 0; j < index; j++) {
                                head[i].adjacent[j] = neis[j];
                        }
                        delete[] neis;
                }
        }

        // neighborhood_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        delete[] AdjShare;
        delete[] adj;

        int count = 0, d_two = 0;
        for (int k = 0; k < n; k++) {
                if (head[k].del == 0 && head[k].neiSize > 0) {
                        if (head[k].neiSum - head[k].weight > max_d) {
                                max_d = head[k].neiSum - head[k].weight;
                        }
                        if (head[k].neiSize == 2) d_two++;
                        count++;
                }
        }
        printf("degree_two_size: %d \nKernel Size:%d\n", d_two, count);

        int* bin_head = new int[max_d + 1];
        memset(bin_head, -1, sizeof(int) * (max_d + 1));
        int* bin_next = new int[n];
        for (int i = 0; i < n; i++) {
                if (head[i].del || head[i].neiSize < 1) continue;
                int dif = head[i].neiSum - head[i].weight;
                if (dif <= 0) continue;
                bin_next[i] = bin_head[dif];
                bin_head[dif] = i;
        }

        while (max_d > 0) {
                while (pone.empty() && degree_two.empty() && degree_one.empty()) {
                        while (max_d >= 0 && bin_head[max_d] == -1) max_d--;
                        if (max_d < 0) break;
                        int v = -1;
                        for (v = bin_head[max_d]; v != -1;) {
                                int tmp = bin_next[v];
                                int deg = head[v].neiSum - head[v].weight;
                                if (head[v].del == 0 && deg > 0) {
                                        if (deg < max_d) {
                                                bin_next[v] = bin_head[deg];
                                                bin_head[deg] = v;
                                        } else {
                                                deleteVertex(v, pone, degree_one, degree_two);
                                                backTrack.push_back(make_pair(v, n));
                                                bin_head[max_d] = tmp;
                                                break;
                                        }
                                }
                                v = tmp;
                        }
                        if (v == -1) bin_head[max_d] = -1;
                }
                // cannot add edge
                clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 0);
        }

        int greedy_include = 0, remainder = 0;
        for (int k = backTrack.size() - 1; k >= 0; k--) {
                int a = backTrack[k].first;
                bool ok = 1;
                if (a == n) {
                        int b = backTrack[k].second;
                        VI& T = circles.back();
                        int* tmp = nullptr;
                        int index = 0;
                        int T_size = T.size();
                        // attahed circle
                        if (b == 1) {
                                // if v
                                if (head[T[0]].del) {
                                        tmp = new int[T_size - 1];
                                        for (int i = 1; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                }
                        }
                        // circle ends with two adjacent vertex.
                        else if (b == 2) {
                                if (!head[T[0]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (!head[T[1]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 3; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (head[T[1]].del && head[T[0]].del) {
                                        tmp = new int[T_size - 2];
                                        for (int i = 2; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        printf("error info: T[0] and T[0] cann't exist at the same time\n");
                                }
                        }
                        path_recover(tmp, index);
                        delete[] tmp;
                        circles.pop_back();
                } else {
                        for (auto n : head[a].adjacent) {
                                if (head[n].del == 0) {
                                        ok = 0;
                                        break;
                                }
                        }
                        if (ok) {
                                head[a].del = 0;
                        }
                }
        }

        if (!circles.empty()) printf("error info: circles is not empty!\n");

        for (int i = 0; i < n; i++) {
                if (!head[i].del) total_weight += weight_copy[i];
        }

        clock_t end = clock();
        delete[] bin_head;
        delete[] bin_next;

        /*printf("solution:\n");
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0) {
                        printf("%d ", i);
                }
        }*/

        printf("total weight :%d ,  remainder : %d , total time cost : %f\n", total_weight, remainder,
               (float)(end - start) * 1000 / CLOCKS_PER_SEC);
        delete[] weight_copy;
}

void Graph::bfs() {
        // clear the invalid vertice��
        for (int i = 0; i < n; i++) {
                // clear the useless vertex
                if (head[i].weight <= 0) {
                        printf("Invalid weight 0\n");
                        return;
                }
        }
        clock_t start = clock();
        // ����ÿ�����ھ�Ȩֵ�͡�
        int* weight_copy = new int[n];
        for (int i = 0; i < n; i++) {
                weight_copy[i] = head[i].weight;
        }
        int max_d = 0;
        for (int i = 0; i < n; i++) {
                int sum = 0;
                for (auto v : head[i].adjacent) {
                        sum += head[v].weight;
                }
                head[i].neiSum = sum;
                head[i].neiSize = head[i].adjacent.size();
        }

        vector<int> pone, dominated, degree_two, degree_one;
        vector<pair<int, int>> backTrack;
        vector<vector<int>> circles;

        for (int i = 0; i < n; i++) {
                if (head[i].weight >= head[i].neiSum) {
                        pone.push_back(i);
                } else if (head[i].neiSize == 1) {
                        degree_one.push_back(i);
                } else if (head[i].neiSize == 2) {
                        degree_two.push_back(i);
                }
        }

        bool* adj = new bool[n];
        memset(adj, 0, sizeof(bool) * n);
        int* AdjShare = new int[n];
        memset(AdjShare, 0, sizeof(int) * n);

        // clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 1);

        //(pone, degree_one, degree_two, backTrack, circles, adj);

        // symmetric_folding(pone, degree_one, degree_two, backTrack, circles, adj, AdjShare);

        // common_neighbor_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        bfsReduction(pone, degree_one, degree_two, adj);
        //// reorganize the ajacent matrix.
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0 && head[i].neiSize > 0) {
                        int* neis = new int[head[i].neiSize];
                        int index = 0;
                        for (auto n : head[i].adjacent) {
                                if (head[n].del == 0) {
                                        neis[index++] = n;
                                }
                        }
                        if (index != head[i].neiSize) printf("error info: index != neisize\n");
                        head[i].adjacent.resize(index);
                        for (int j = 0; j < index; j++) {
                                head[i].adjacent[j] = neis[j];
                        }
                        delete[] neis;
                }
        }

        // neighborhood_reduction(pone, degree_one, degree_two, backTrack, circles, adj);

        delete[] AdjShare;
        delete[] adj;

        int count = 0, d_two = 0;
        for (int k = 0; k < n; k++) {
                if (head[k].del == 0 && head[k].neiSize > 0) {
                        if (head[k].neiSum - head[k].weight > max_d) {
                                max_d = head[k].neiSum - head[k].weight;
                        }
                        if (head[k].neiSize == 2) d_two++;
                        count++;
                }
        }
        printf("degree_two_size: %d \nKernel Size:%d\n", d_two, count);

        int* bin_head = new int[max_d + 1];
        memset(bin_head, -1, sizeof(int) * (max_d + 1));
        int* bin_next = new int[n];
        for (int i = 0; i < n; i++) {
                if (head[i].del || head[i].neiSize < 1) continue;
                int dif = head[i].neiSum - head[i].weight;
                if (dif <= 0) continue;
                bin_next[i] = bin_head[dif];
                bin_head[dif] = i;
        }

        while (max_d > 0) {
                while (pone.empty() && degree_two.empty() && degree_one.empty()) {
                        while (max_d >= 0 && bin_head[max_d] == -1) max_d--;
                        if (max_d < 0) break;
                        int v = -1;
                        for (v = bin_head[max_d]; v != -1;) {
                                int tmp = bin_next[v];
                                int deg = head[v].neiSum - head[v].weight;
                                if (head[v].del == 0 && deg > 0) {
                                        if (deg < max_d) {
                                                bin_next[v] = bin_head[deg];
                                                bin_head[deg] = v;
                                        } else {
                                                deleteVertex(v, pone, degree_one, degree_two);
                                                backTrack.push_back(make_pair(v, n));
                                                bin_head[max_d] = tmp;
                                                break;
                                        }
                                }
                                v = tmp;
                        }
                        if (v == -1) bin_head[max_d] = -1;
                }
                // cannot add edge
                clearPone(adj, pone, degree_one, degree_two, backTrack, circles, 0);
        }

        int greedy_include = 0, remainder = 0;
        for (int k = backTrack.size() - 1; k >= 0; k--) {
                int a = backTrack[k].first;
                bool ok = 1;
                if (a == n) {
                        int b = backTrack[k].second;
                        VI& T = circles.back();
                        int* tmp = nullptr;
                        int index = 0;
                        int T_size = T.size();
                        // attahed circle
                        if (b == 1) {
                                // if v
                                if (head[T[0]].del) {
                                        tmp = new int[T_size - 1];
                                        for (int i = 1; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                }
                        }
                        // circle ends with two adjacent vertex.
                        else if (b == 2) {
                                if (!head[T[0]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 2; i < T_size - 1; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (!head[T[1]].del) {
                                        tmp = new int[T_size - 3];
                                        for (int i = 3; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else if (head[T[1]].del && head[T[0]].del) {
                                        tmp = new int[T_size - 2];
                                        for (int i = 2; i < T_size; i++) {
                                                tmp[index++] = T[i];
                                        }
                                } else {
                                        printf("error info: T[0] and T[0] cann't exist at the same time\n");
                                }
                        }
                        path_recover(tmp, index);
                        delete[] tmp;
                        circles.pop_back();
                } else {
                        for (auto n : head[a].adjacent) {
                                if (head[n].del == 0) {
                                        ok = 0;
                                        break;
                                }
                        }
                        if (ok) {
                                head[a].del = 0;
                        }
                }
        }

        if (!circles.empty()) printf("error info: circles is not empty!\n");

        for (int i = 0; i < n; i++) {
                if (!head[i].del) total_weight += weight_copy[i];
        }

        clock_t end = clock();
        delete[] bin_head;
        delete[] bin_next;

        /*printf("solution:\n");
        for (int i = 0; i < n; i++) {
                if (head[i].del == 0) {
                        printf("%d ", i);
                }
        }*/

        printf("total weight :%d ,  remainder : %d , total time cost : %f\n", total_weight, remainder,
               (float)(end - start) * 1000 / CLOCKS_PER_SEC);
        delete[] weight_copy;
}

}