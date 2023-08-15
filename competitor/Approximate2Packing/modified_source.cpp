#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>
//#include "conio.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>


using namespace std;

class Edge
{
       public:
        Edge()
        {
        }
        Edge(unsigned int v1, unsigned int v2)
        {
                vertex1 = v1; vertex2 = v2;
        }
        unsigned int vertex1, vertex2;

};

class Vertex
{
       public:
        Vertex()
        {
        }
        Vertex(string iden, int etiq, int nivel)
        {
                name = iden;
                id = etiq;
                level = nivel;
        }
        vector<int> neighborhood;
        string name;
        int id, level;
        float centro[2];
        float color[3];
        vector<int> liga;
};

//ILOSTLBEGIN
//using namespace std;
const int num_of_threads = 1;
int ColisionNodo, NumNodes = 780000, NumEdges = 785000; //casi un millon
int NumEdgesSG[5000], NumNodosSG[5000], NumSG = -1; //ahora son 5,000 niveles
unsigned int restaSG[5000]; //a lo mas serian 5000 SGs

Vertex v[780000], vSG[500][20000]; //500 SubGrafos de a 20,000 nodos c/u *
Edge edg[785000], edgSG[500][20000]; //500 SubGrafos de a 20,000 aristas c/u *

//- - - - SubGrafos
bool NivelCreado[5000], SGcreado[500]; //5,000 niveles, 500 SGs *
unsigned int NodoLeader = 0, NumLayers = 12, NivelLeader = 0;
short int capa = -1, MaxNumSG = 1;
unsigned int MaxNivel = 0, NodosPorNivel[5000]; //5000 niveles

unsigned int NivelBegin[500], NivelEnd[500]; //500 SubGrafos maximo *
bool valores[780000], valoresSG[500][20000]; //500 SubGrafos de a 20,000 nodos c/u *
float tiempo[500]; //500 SGs *
int costo[500], costo_old[500]; //* 500 SG
vector<int> PorNivel[5000]; //5000 niveles en G maximo
int CostoGlobal = 0, CostoGlobal_old = 0;


//***********************************************
void GMLread(const char* p_filename)
{
        using namespace std;
        fstream f;
        char c;
        char str[20], labelValue[20], label[20];
        int count = 0;
        int value, idValue, idValue2, idValue3, nivel = 0, colorValue = 0;
        unsigned int bien = 0, i, j;

        NumNodes = 0; NumEdges = 0;
        f.open(p_filename, ios::in);
        printf("Openning %s\n", p_filename);
        if (f.is_open()) { printf("File opened \n"); }
        else { printf("Unable to open the file \n"); return; }

        while (!f.eof())
        {
                f >> str;
                if (bien == 0) //comenzando a leer el archivo
                {
                        if (strcmp(str, "graph") == 0)
                        {
                                printf("Al primer intento tengo a %s =\n", str); bien++;
                        }
                        else
                        {
                                printf("%s + ", str);
                                while (strcmp(str, "graph") != 0)
                                {
                                        f >> str;
                                        printf("%s, ", str);
                                }
                                printf("\n despues del while se tiene a %s \n", str); bien++;
                        }
                }
                else
                {
                        if (bien == 1)
                                if (strcmp(str, "node") == 0)
                                {
                                        //-printf("%s", str);
                                        f >> str;
                                        if (strcmp(str, "[") == 0)
                                        {
                                                //-printf("%s", str);
                                                while (strcmp(str, "]") != 0)
                                                {
                                                        f >> str;
                                                        if (strcmp(str, "id") == 0)
                                                        {
                                                                f >> idValue;
                                                                //-printf("%d", idValue);
                                                        }//if
                                                        if (strcmp(str, "label") == 0)
                                                        {
                                                                f >> labelValue;
                                                        }//if
                                                        if (strcmp(str, "level") == 0)
                                                        {
                                                                f >> nivel;
                                                                if (nivel > MaxNivel) MaxNivel = nivel;
                                                                NodosPorNivel[nivel - 1]++;
                                                        }//if
                                                        if (strcmp(str, "color") == 0)
                                                        {
                                                                f >> colorValue;
                                                        }//if
                                                }//while
                                                //-printf("%s = ", str); //]
                                                j = 0;
                                                std::memset(label, '\0', sizeof(label));
                                                //-printf("%s, %d", labelValue, nivel);

                                                v[NumNodes] = Vertex(labelValue, NumNodes, nivel);
                                                valores[NumNodes] = 0;
                                                if (colorValue == 1) { v[NumNodes].color[0] = 0; v[NumNodes].color[1] = 0; v[NumNodes].color[2] = 0; }
                                                else { v[NumNodes].color[0] = 0; v[NumNodes].color[1] = 0; v[NumNodes].color[2] = 0; }
                                                PorNivel[nivel - 1].push_back(NumNodes); //*
                                                NumNodes++;
                                                //-printf("\n");
                                        } //if de if(strcmp(str,"[")==0)
                                }//if de if(strcmp(str,"node")==0)
                                else if (strcmp(str, "edge") == 0)
                                {
                                        f >> str;
                                        if (strcmp(str, "[") == 0)
                                        {
                                                while (strcmp(str, "]") != 0)
                                                {
                                                        f >> str;
                                                        if (strcmp(str, "source") == 0)
                                                        {
                                                                f >> idValue;
                                                        }//if
                                                        if (strcmp(str, "target") == 0)
                                                        {
                                                                f >> idValue2;
                                                        }//if
                                                        if (strcmp(str, "value") == 0)
                                                        {
                                                                f >> idValue3;
                                                        }//if
                                                }//while
                                                edg[NumEdges] = Edge(idValue, idValue2);
                                                //printf("edg[%d]=(%d, %d)...", NumEdges, idValue, idValue2);
                                                //relacion con los dos nodos de esta arista
                                                v[idValue].liga.push_back(NumEdges);
                                                v[idValue2].liga.push_back(NumEdges);
                                                NumEdges++;
                                        }//if(strcmp(str,"[")==0)
                                }//else if(strcmp(str,"edge")==0)
                }//else de if(bien==0)

        }//while
        f.close();
        printf("\n - - NumNodes=%d, NumEdges=%d, MaxNivel=%d\n", NumNodes, NumEdges, MaxNivel);
}
//***********************************************
void Subdivide()
{
        unsigned int k, j1;
        unsigned int temp = 0, cuentaNodos = 0;
        bool entra = 0;
        NumSG = 0;
        printf("Para cada nodo v[] en G\n");
        for (int i = 0; i < NumNodes; i++) //para cada nodo v[] en G
        {
                entra = 0;
                j1 = 0;  //si es el primer SG, entonces
                if (v[i].level >= NivelBegin[j1] && v[i].level < NivelEnd[j1]) {
                        vSG[j1][NumNodosSG[j1]] = Vertex(v[i].name, i, v[i].level);
                        vSG[j1][NumNodosSG[j1]].id = v[i].id;
                        vSG[j1][NumNodosSG[j1]].centro[0] = 0; vSG[j1][NumNodosSG[j1]].centro[1] = 0;
                        vSG[j1][NumNodosSG[j1]].color[0] = v[i].color[0]; vSG[j1][NumNodosSG[j1]].color[1] = v[i].color[1]; vSG[j1][NumNodosSG[j1]].color[2] = v[i].color[2];
                        NumNodosSG[j1]++;
                        entra = 1;
                }
                if (!entra) //si son SGs intermedios
                        for (unsigned int j = 0; j < MaxNumSG - 1; j++)
                        {
                                if (!entra) {
                                        if (v[i].level == NivelEnd[j] || v[i].level == NivelBegin[j + 1]) { restaSG[j]++; entra = 1; }
                                        else if (v[i].level > NivelBegin[j] && v[i].level < NivelEnd[j])
                                        {
                                                vSG[j][NumNodosSG[j]] = Vertex(v[i].name, i, v[i].level);
                                                vSG[j][NumNodosSG[j]].id = v[i].id;
                                                vSG[j][NumNodosSG[j]].centro[0] = 0; vSG[j][NumNodosSG[j]].centro[1] = 0;
                                                vSG[j][NumNodosSG[j]].color[0] = v[i].color[0]; vSG[j][NumNodosSG[j]].color[1] = v[i].color[1]; vSG[j][NumNodosSG[j]].color[2] = v[i].color[2];
                                                NumNodosSG[j]++; entra = 1;
                                        }
                                }
                        }

                j1 = MaxNumSG - 1;
                if (!entra) //si es el ultimo SG
                        if (v[i].level > NivelBegin[j1] && v[i].level <= NivelEnd[j1]) {
                                vSG[j1][NumNodosSG[j1]] = Vertex(v[i].name, i, v[i].level);
                                vSG[j1][NumNodosSG[j1]].id = v[i].id;
                                vSG[j1][NumNodosSG[j1]].centro[0] = 0; vSG[j1][NumNodosSG[j1]].centro[1] = 0;
                                vSG[j1][NumNodosSG[j1]].color[0] = v[i].color[0]; vSG[j1][NumNodosSG[j1]].color[1] = v[i].color[1]; vSG[j1][NumNodosSG[j1]].color[2] = v[i].color[2];
                                NumNodosSG[j1]++;
                        }

        }//i
        //printf("\n Hay %d nodos en la capa 50 \n", cuentaNodos);
        //restaSG[] = cuentaNodos;
        //edges

        for (unsigned int i = 0; i < NumEdges; i++) //para cada arista de G
        {
                k = 0; entra = 0;
                //para el SG=0, el primero
                j1 = 0;
                if (v[edg[i].vertex1].level >= NivelBegin[j1] && v[edg[i].vertex1].level < NivelEnd[j1] &&
                    v[edg[i].vertex2].level >= NivelBegin[j1] && v[edg[i].vertex2].level < NivelEnd[j1])
                {
                        edgSG[j1][NumEdgesSG[j1]] = Edge(edg[i].vertex1 - k, edg[i].vertex2 - k);
                        vSG[j1][edg[i].vertex1 - k].liga.push_back(NumEdgesSG[j1]);
                        vSG[j1][edg[i].vertex2 - k].liga.push_back(NumEdgesSG[j1]);
                        NumEdgesSG[j1]++; entra = 1;
                }
                if (!entra)
                        for (unsigned int j = 1; j < MaxNumSG - 1; j++)
                        {
                                if (!entra) {
                                        k += NumNodosSG[j - 1] + restaSG[j - 1];
                                        if (v[edg[i].vertex1].level > NivelBegin[j] && v[edg[i].vertex1].level < NivelEnd[j] &&
                                            v[edg[i].vertex2].level > NivelBegin[j] && v[edg[i].vertex2].level < NivelEnd[j])
                                        {
                                                edgSG[j][NumEdgesSG[j]] = Edge(edg[i].vertex1 - k, edg[i].vertex2 - k);
                                                vSG[j][edg[i].vertex1 - k].liga.push_back(NumEdgesSG[j]);
                                                vSG[j][edg[i].vertex2 - k].liga.push_back(NumEdgesSG[j]);
                                                NumEdgesSG[j]++;
                                                temp++; entra = 1;
                                        }
                                }//!entra
                        }//for j, SGs
                j1 = MaxNumSG - 1;
                if (!entra) {
                        k += NumNodosSG[j1 - 1] + restaSG[j1 - 1];
                        if (v[edg[i].vertex1].level > NivelBegin[j1] && v[edg[i].vertex1].level <= NivelEnd[j1] &&
                            v[edg[i].vertex2].level > NivelBegin[j1] && v[edg[i].vertex2].level <= NivelEnd[j1])
                        {
                                edgSG[j1][NumEdgesSG[j1]] = Edge(edg[i].vertex1 - k, edg[i].vertex2 - k);
                                vSG[j1][edg[i].vertex1 - k].liga.push_back(NumEdgesSG[j1]);
                                vSG[j1][edg[i].vertex2 - k].liga.push_back(NumEdgesSG[j1]);
                                NumEdgesSG[j1]++; entra = 1;
                        }
                }//!entra

        }//for i, aristas en G

        for (unsigned int j = 0; j < MaxNumSG; j++) SGcreado[j] = 1;

        NumSG = 0;
        printf("\n");
}
//***********************************************
void Subdivide_SG_Capas_Contiguas() {
        unsigned int k, j1, arista, neighbor;
        unsigned int h = 0; //el id del vertice
        unsigned int temp = 0, cuentaNodos = 0;
        bool entra = 0;
        NumSG = 0;

        k = 0; //for (NumSG = 0; NumSG < MaxNumSG-1; NumSG++)
        for (NumSG = 0; NumSG < MaxNumSG - 1; NumSG++) {
                NumNodosSG[NumSG] = 0;
                //----------------------------------
                while (v[k].level < NivelEnd[NumSG] - 1) { //avanza indice nodos
                        k++;
                }
                //indice nodos llega a NivelEnd[NumSG] - 1
                temp = 0;
                while (v[k].level >= NivelEnd[NumSG] - 1 && v[k].level <= NivelEnd[NumSG] + 2) {
                        //llena arreglos
                        vSG[NumSG][temp] = Vertex(v[k].name, k, v[k].level);
                        vSG[NumSG][temp].id = v[k].id;
                        vSG[NumSG][temp].centro[0] = 0; vSG[NumSG][temp].centro[1] = 0;
                        vSG[NumSG][temp].color[0] = 0; vSG[NumSG][temp].color[1] = 0; vSG[NumSG][temp].color[2] = 0;
                        //

                        /*if (temp < 10) {
                                printf("vSG[%d][%d] =%d k=%d (%d). p=", NumSG, temp, vSG[NumSG][temp].id, k, vSG[NumSG][temp].level);

                                for (int p : v[k].liga) {
                                        printf("%d:", p);
                                        printf("<%d(%d), %d(%d)>, ", edg[p].vertex1, v[edg[p].vertex1].level, edg[p].vertex2, v[edg[p].vertex2].level);
                                }
                                printf("\n");
                        }*/

                        k++;
                        temp++;
                }//while
                NumNodosSG[NumSG] = temp;
                printf("NumNodosSG[%d] =%d \n", NumSG, NumNodosSG[NumSG]);
                //----------------------------------

        }

        printf("\n aristas \n");
        //NumSG = 0; //probamos para el primer SG
        //aristas - - - -- - - - - - - - - - - - - - - - - - - - - - - - -  --  - - - - - -
        for (NumSG = 0; NumSG < MaxNumSG - 1; NumSG++) {
                temp = 0;
                NumEdgesSG[NumSG] = 0;
                for (unsigned int i = 0; i < NumEdges; i++) {//para cada arista de G
                        if (v[edg[i].vertex1].level >= NivelEnd[NumSG] - 1 && v[edg[i].vertex2].level <= NivelEnd[NumSG] + 2) {
                                edgSG[NumSG][temp] = Edge(edg[i].vertex1 - vSG[NumSG][0].id, edg[i].vertex2 - vSG[NumSG][0].id);
                                vSG[NumSG][edg[i].vertex1 - vSG[NumSG][0].id].liga.push_back(temp);
                                vSG[NumSG][edg[i].vertex2 - vSG[NumSG][0].id].liga.push_back(temp);
                                //printf("edgSG[%d][%d] =<%d,%d>,  ", NumSG, temp, edgSG[NumSG][temp].vertex1, edgSG[NumSG][temp].vertex2);
                                //printf("<vSG[%d][%d].liga =%d, ", NumSG, edg[i].vertex1 - vSG[NumSG][0].id, temp);
                                //printf("vSG[%d][%d].liga =%d>, ", NumSG, edg[i].vertex2 - vSG[NumSG][0].id, temp);
                                temp++;
                                //printf("\n");
                        }//if
                }//for i
                NumEdgesSG[NumSG]=temp;
        }//for NumSGs

}
//***********************************************
void Alg()
{
        stringstream logfile;
        IloEnv env;
        IloModel model(env);
        IloNumVarArray x(env);
        IloExpr Objetivo(env);
        IloExpr cond(env);
        IloRange condicion;
        int k = 0;
        std::chrono::high_resolution_clock::time_point start, end;

        start = std::chrono::high_resolution_clock::now();
        printf("\n creando variables: ");
        for (int i = 0; i < NumNodosSG[NumSG]; i++)
        {
                /*if (i == 50) x.add(IloNumVar(env,1, 1, IloNumVar::Int));
                else*/ x.add(IloNumVar(env, 0, 1, IloNumVar::Int));
        }
        Objetivo = x[0];
        for (int i = 1; i < NumNodosSG[NumSG]; i++) Objetivo += x[i];

        printf("\n ============= NumSG=%d\n", NumSG);
        //Segundo tipo de condicion
        for (int j = 0; j < NumNodosSG[NumSG]; j++)
        {
                //probando
                //if (j == NumNodosSG[NumSG] - 5) x[j]. = 1;
                cond = x[j];
                for (int i : vSG[NumSG][j].liga)
                {
                        //la liga consta de al menos 2 vertices, uno de ellos es j
                        if (edgSG[NumSG][i].vertex1 != j) {
                                cond += x[edgSG[NumSG][i].vertex1];
                        }
                        else {
                                cond += x[edgSG[NumSG][i].vertex2];
                        }
                }
                model.add(cond <= 1);
        }

        printf("Creando condiciones\n");
        printf("Especificando Funcion Objetivo\n");
        model.add(IloMaximize(env, Objetivo));

        IloCplex cplex(model);
        cplex.setParam(IloCplex::Param::Threads, 1);
        cplex.setOut(env.getNullStream());
        cplex.setWarning(env.getNullStream());
        cplex.solve();

        if (cplex.getStatus() == IloAlgorithm::Infeasible)
                env.out() << "No Solution\n";

        env.out() << "Solution status:\n ";
        costo[NumSG] = cplex.getObjValue();
        env.out() << "Cost:" << costo[NumSG];

        //tiempo[NumSG] = cplex.getTime();
        //env.out() << "\n Time:" << tiempo[NumSG];
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(end -start);
        double total_time = elapsed_time.count();
        //printf("\nElapsed time : %0.3f \n", total_time);
        tiempo[NumSG] = total_time;

        if (cplex.getStatus() == IloAlgorithm::Optimal) {
                IloNumArray vals(env, ILOINT);
                cplex.getValues(vals, x);
                //
                //		if (NumSG == 9) {printf("%d, %d, %d, %d, %d \n", vals[10], vals[49], vals[50], vals[51], vals[100]);
                //		env.out() << "\n Values = " << vals[49] << vals[50] << endl;}

                for (unsigned int i = 0; i < NumNodosSG[NumSG]; i++)
                {
                        valoresSG[NumSG][i] = 0;
                        if (vals[i] > 0.9999) valoresSG[NumSG][i] = 1;
                }
                x.end();
        }
        //printf(". . . . . . k=%d \n", k);
        const string str = logfile.str();
        cout << str << endl;

        env.end();
}
//***********************************************
void Alg_SG_Capas_Contiguas() {
        stringstream logfile;
        IloEnv env;
        IloModel model(env);
        IloNumVarArray x(env);
        IloExpr Objetivo(env);
        IloExpr cond(env);
        IloRange condicion;
        int k = 0;
        time_t start_t, end_t;
        double total_time;

        start_t = clock();
        printf("\n creando variables: ");
        for (int i = 0; i < NumNodosSG[NumSG]; i++)
        {
                /*if (i == 50) x.add(IloNumVar(env,1, 1, IloNumVar::Int));
                else*/
                x.add(IloNumVar(env, 0, 1, IloNumVar::Int));
                //x.add(IloNumVar(env, valoresSG[NumSG][i], 1, IloNumVar::Int));
                /*if(valoresSG[NumSG][i]) x.add(IloNumVar(env, 1, 1, IloNumVar::Int));
                else x.add(IloNumVar(env, 0, 1, IloNumVar::Int));*/
        }
        Objetivo = x[0];
        for (int i = 1; i < NumNodosSG[NumSG]; i++) Objetivo += x[i];

        printf("\n ============= NumSG=%d\n", NumSG);
        //Segundo tipo de condicion
        /*for (int j = 0; j < NumNodosSG[NumSG]; j++)
        {
                //probando
                //if (j == NumNodosSG[NumSG] - 5) x[j]. = 1;
                cond = x[j];
                for (int i : vSG[NumSG][j].liga)
                {
                        //la liga consta de al menos 2 vertices, uno de ellos es j
                        if (edgSG[NumSG][i].vertex1 != vSG[NumSG][i].id) { //if (edgSG[NumSG][i].vertex1 != j)
                                cond += x[edgSG[NumSG][i].vertex1];
                        }
                        else {
                                cond += x[edgSG[NumSG][i].vertex2];
                        }
                }
                model.add(cond <= 1);
        }*/
        for (int j = 0; j < NumNodosSG[NumSG]; j++)
        {
                //printf("vSG[%d][%d].id=%d, liga:", NumSG, j, vSG[NumSG][j].id);
                cond = x[j];
                for (int i : vSG[NumSG][j].liga)
                {
                        //printf("%d<%d,%d>,", i, edgSG[NumSG][i].vertex1, edgSG[NumSG][i].vertex2);
                        if (edgSG[NumSG][i].vertex1 != j) {
                                //printf("x[%d],", edgSG[NumSG][i].vertex1);
                                cond += x[edgSG[NumSG][i].vertex1];
                        }
                        else {
                                //printf("x[%d],", edgSG[NumSG][i].vertex2);
                                cond += x[edgSG[NumSG][i].vertex2];
                        }
                }//for i
                //printf("\n");
                model.add(cond <= 1);
        }//for j

        printf("Creando condiciones\n");
        printf("Especificando Funcion Objetivo\n");
        model.add(IloMaximize(env, Objetivo));

        IloCplex cplex(model);
        cplex.setParam(IloCplex::Param::Threads, 1);
        cplex.setOut(env.getNullStream());
        cplex.setWarning(env.getNullStream());
        cplex.solve();

        if (cplex.getStatus() == IloAlgorithm::Infeasible) {
                env.out() << "No Solution......................................................\n";
                const string str = logfile.str();
                cout << str << endl;
                x.end();
                env.end();
                return;
        }

        env.out() << "Solution status:\n ";
        costo[NumSG] = cplex.getObjValue();
        env.out() << "Costo:" << costo[NumSG];

        //tiempo[NumSG] = cplex.getTime();
        //env.out() << "\n Time:" << tiempo[NumSG];
        end_t = clock();
        //total_time = (double)(end_t - start_t) / (double)CLK_TCK;
        //printf("\nElapsed time : %0.3f \n", total_time);
        tiempo[NumSG] = total_time;
        //env.out() << "\n Time:" << cplex.getTime();

        if (cplex.getStatus() == IloAlgorithm::Optimal) {
                IloNumArray vals(env, ILOINT);
                cplex.getValues(vals, x);
                //

                for (unsigned int i = 0; i < NumNodosSG[NumSG]; i++)
                {
                        valoresSG[NumSG][i] = 0;
                        if (vals[i] > 0.9999) valoresSG[NumSG][i] = 1;
                }
                x.end();
        }
        const string str = logfile.str();
        cout << str << endl;

        env.end();
}
//***********************************************
void TestConstraintsSG(int N) {
        unsigned int RigthConstraint = 0;
        for (unsigned int i = 0; i < NumEdgesSG[N]; i++) {
                if (valoresSG[N][edgSG[N][i].vertex1] + valoresSG[N][edgSG[N][i].vertex2] <= 1) RigthConstraint++;
        }
        printf("SG=%d, NumEdges=%d, RigthConstraint=%d\n", N, NumEdgesSG[N], RigthConstraint);
}
//***********************************************
void TestConstraintsSGv1(int i, int SumaNodosSG) {
        //SG=i
        unsigned int h = 0; //el id del vertice
        unsigned int TotalValores; //para verificar la condicion por cada nodo
        unsigned int costoSG = 0; //Los nuevos costos
        printf("Verificando condiciones de SG=%d \n", i);
        for (unsigned int j = 0; j < NumNodosSG[i]; j++) {
                h = vSG[i][j].id; //para cada nodo en el SG
                costoSG += valoresSG[i][j];
                TotalValores = valoresSG[i][j]; //el valor 0, 1 del nodo analizado
                for (int p : vSG[i][h - SumaNodosSG].liga) {
                        if (edgSG[i][p].vertex1 != h - SumaNodosSG) {
                                TotalValores += valoresSG[i][edgSG[i][p].vertex1];
                        }
                        else {
                                TotalValores += valoresSG[i][edgSG[i][p].vertex2];
                        }
                }//for p
                if (TotalValores > 1) printf("Error ! Condicion no  cumplida . . . . . \n");
                TotalValores = 0;

        }// for NumNodosSG[i]
        costo[i] = costoSG;
        printf("   costo[%d]=%d \n", i, costo[i]);
}
//***********************************************
void TestConstraintsSGv1_SG_Capas_Contiguas(int NumeSG) {
        //SG=NumeSG
        unsigned int h = 0; //el id del vertice
        unsigned int TotalValores; //para verificar la condicion por cada nodo
        unsigned int costoSG = 0; //Los nuevos costoseSG
        for (unsigned int j = 0; j < NumNodosSG[NumeSG]; j++) {
                TotalValores = valoresSG[NumeSG][j]; //el valor 0, 1 del nodo analizado
                for (int i : vSG[NumeSG][j].liga)
                {
                        if (edgSG[NumeSG][i].vertex1 != j) {
                                TotalValores += valoresSG[NumeSG][edgSG[NumeSG][i].vertex1];
                        }
                        else {
                                TotalValores += valoresSG[NumeSG][edgSG[NumeSG][i].vertex2];
                        }
                }//for i
                if (TotalValores > 1) printf("Error ! Condicion no  cumplida . . . . . \n");
                TotalValores = 0;
        }//for j
}

//***********************************************
void SolConflictos(int i) {
        unsigned int h = 0; //el id del vertice
        unsigned int ValoresCambiados; //Suma los valores que se hacen cero
        ValoresCambiados = 0;
        for (unsigned int j = 0; j < NumNodosSG[i]; j++)
        {
                h = vSG[i][j].id; //para cada nodo en el SG

                if (vSG[i][j].level == NivelBegin[i]) { //para la primera cpa de SG
                        if (valoresSG[i][j]) { //esta pregunta la hago para el contador
                                valoresSG[i][j] = 0;
                                ValoresCambiados++;
                                printf("%d,%d",i,j);
                        }
                }
        }// for NumNodosSG[i]
        CostoGlobal = CostoGlobal - ValoresCambiados;
        printf("ValoresCambiados=%d \n", ValoresCambiados);
}


void CapasContiguas()
{
        int k = 0;
        printf("SG=0: \n");
        //Al primer SG se le cambia NivelEnd[ ]
        for (unsigned int i = 0; i < NumNodosSG[0]; i++) {
                //hay que preguntar por los nodos de la ultima capa de este SG
                if (vSG[0][i].level == NivelEnd[0]) {
                        if (valoresSG[0][i]) {
                                //printf("vSG[0][%d].level = %d, valoresSG[0][%d]=%d cambiara a 0\n", i, vSG[0][i].level, i, valoresSG[0][i]);
                                valoresSG[0][i] = 0; //k++;
                        }
                        //if (valoresSG[0][i]) printf("cambia a valoresSG[0][%d]=%d ",i, valoresSG[0][i]);
                }
        }
        for (unsigned int i = 1; i < MaxNumSG - 1; i++)
                for (unsigned int j = 0; j < NumNodosSG[i]; j++) {
                        if (vSG[i][j].level == NivelBegin[i]) {
                                if (valoresSG[i][j]) {
                                        //printf("vSG[%d][%d].level = %d, valoresSG[%d][%d]=%d cambiara a 0\n", i, j, vSG[i][j].level, i, j, valoresSG[i][j]);
                                        valoresSG[i][j] = 0;
                                }
                                //if (valoresSG[i][j]) printf("cambia  valoresSG[%d][%d]=%d ", i, j, valoresSG[i][j]);
                        }
                        else if (vSG[i][j].level == NivelEnd[i]) {
                                if (valoresSG[i][j]) {
                                        //printf("vSG[%d][%d].level = %d, valoresSG[%d][%d]=%d cambiara a 0\n", i, j, vSG[i][j].level, i, j, valoresSG[i][j]);
                                        valoresSG[i][j] = 0;
                                }
                                //if (valoresSG[i][j]) printf("cambia  valoresSG[%d][%d]=%d ", i, j, valoresSG[i][j]);
                        }
                }
        printf("SG = MaxNumSG \n");
        //Al ultimo SG se le cambia NivelBegin[ ]
        for (unsigned int i = 0; i < NumNodosSG[MaxNumSG - 1]; i++) {
                //hay que preguntar por los nodos de la primera capa de este SG
                //printf("vSG[MaxNumSG-1][%d].level = %d \n", i, vSG[MaxNumSG-1][i].level);
                if (vSG[MaxNumSG - 1][i].level == NivelBegin[MaxNumSG - 1]) {
                        if (valoresSG[MaxNumSG - 1][i]) {
                                //printf("vSG[%d][%d].level = %d, valoresSG[%d][%d]=%d cambiara a 0\n", MaxNumSG - 1, i, vSG[MaxNumSG - 1][i].level, MaxNumSG - 1, i, valoresSG[MaxNumSG - 1][i]);
                                valoresSG[MaxNumSG - 1][i] = 0;
                        }
                        //if (valoresSG[MaxNumSG - 1][i]) printf("cambia a valoresSG[%d][%d]=%d ",  MaxNumSG - 1, i, valoresSG[MaxNumSG - 1][i]);
                }
        }
}


//***********************************************
void compute_overall_solution()
{
        int k = 0;
        printf("SG=0: \n");
        int ValoresCambiados = 0;

        // Delete solution vertices of the first two and last two layers per subgraph
        for (unsigned int i = 0; i < NumNodosSG[0]; i++) {
                if (vSG[0][i].level == NivelEnd[0] || vSG[0][i].level == NivelEnd[0]-1) {
                        if (valoresSG[0][i]) {
                                valoresSG[0][i] = 0;
                                ValoresCambiados++;
                        }
                }
        }
        printf("SGs intermedios \n");
        for (unsigned int i = 1; i < MaxNumSG - 1; i++)
                for (unsigned int j = 0; j < NumNodosSG[i]; j++) {
                        if (vSG[i][j].level == NivelBegin[i] || vSG[i][j].level == NivelBegin[i]+1) {
                                if (valoresSG[i][j]) {
                                        valoresSG[i][j] = 0;
                                        ValoresCambiados++;
                                }
                        }
                        else if (vSG[i][j].level == NivelEnd[i]-1 || vSG[i][j].level == NivelEnd[i]) {
                                if (valoresSG[i][j]) {
                                        valoresSG[i][j] = 0;
                                        ValoresCambiados++;
                                }
                        }
                }
        printf("SG = MaxNumSG \n");
        for (unsigned int i = 0; i < NumNodosSG[MaxNumSG - 1]; i++) {
                if (vSG[MaxNumSG - 1][i].level == NivelBegin[MaxNumSG - 1] +1|| vSG[MaxNumSG - 1][i].level == NivelBegin[MaxNumSG - 1]) {
                        if (valoresSG[MaxNumSG - 1][i]) {
                                valoresSG[MaxNumSG - 1][i] = 0;
                                ValoresCambiados++;
                        }
                }
        }
        CostoGlobal_old = CostoGlobal_old - ValoresCambiados;
        printf("ValoresCambiados=%d \n", ValoresCambiados);
}

//***********************************************
bool Checa_Vecinos(unsigned int subGrafo, unsigned int id, unsigned int index)
{
        bool flag = 1;
        for (int p : vSG[subGrafo][index].liga) {
                if (edgSG[subGrafo][p].vertex1 != index) {
                        printf("      valoresSG[%d][%d]=%d\n", subGrafo, edgSG[subGrafo][p].vertex1, valoresSG[subGrafo][edgSG[subGrafo][p].vertex1]);
                        if (valoresSG[subGrafo][edgSG[subGrafo][p].vertex1]) flag = 0;
                }
                else {
                        printf("      valoresSG[%d][%d]=%d\n", subGrafo, edgSG[subGrafo][p].vertex2, valoresSG[subGrafo][edgSG[subGrafo][p].vertex2]);
                        if (valoresSG[subGrafo][edgSG[subGrafo][p].vertex2]) flag = 0;
                }
        }//for p
        if (flag) printf("=        valido \n");
        else printf("=        No valido \n");
        return flag;
}
//***********************************************
void CapasContiguas_Improved()
{
        printf("CapasContiguas_Improved() \n");
        int cond = 0;
        unsigned indexSG_1 = 0; //para cambiar valores[][] de SG=i-1
        unsigned int g = 0; //para el printf
        unsigned int h = 0; //el id del vertice
        unsigned int NumNeighb = 0; //numero de vecinos
        unsigned int indice = 0; //le restas el numero de nodos del SG anterior para que coincida con v[ ]
        bool entra = 0; //si tiene vecinos en la otra capa ent checar vecinos de la misma capa
        bool ChecaEnSG = 0; //checa vecinos en el mismo SG
        bool flag = 0; //para marcar los valores de sus vecinos
        int i = 1; //SG
        unsigned int SumaNodosSG = NumNodosSG[0]; //para sumar el numero de nodos de cada capa y obtener el indice del vertice en v[ ]
        for (unsigned int t = 1; t < MaxNumSG; t++) //del SG 1 al 19
        {
                printf("SG=%d, SumaNodosSG=%d \n", t, SumaNodosSG);
                i = t;
                printf("Comenzamos con SG=%d \n", i);
                printf(". . . . . . . . . . . . . nodos vecinos de la capa %d son \n", NivelBegin[i]);//---------Begin Layer
                for (unsigned int j = 0; j < NumNodosSG[i]; j++) {
                        if (vSG[i][j].level == NivelBegin[i]) {
                                h = vSG[i][j].id;
                                NumNeighb = 0;
                                for (int k : v[h].liga) NumNeighb++;
                                if (NumNeighb > 2)
                                {
                                        for (int k : v[h].liga) {//para los nodos vecinos en edg[ ]
                                                ChecaEnSG = 0;
                                                if (v[edg[k].vertex1].level < NivelBegin[i] || v[edg[k].vertex2].level < NivelBegin[i]) {
                                                        printf("v[%d].id=%d = vSG[%d][%d]=%d,     valoresSG[%d][%d]=%d\n", h, v[h].id, i, h - SumaNodosSG, vSG[i][h - SumaNodosSG].id, i, h - SumaNodosSG, valoresSG[i][h - SumaNodosSG]);
                                                        printf("vecino en SG-1: ");
                                                        entra = 1;
                                                        if (i > 1) indice = SumaNodosSG - NumNodosSG[i - 1];
                                                        else indice = 0;
                                                        if (edg[k].vertex1 != h) {
                                                                printf(" valoresSG[%d][%d]=%d ", i - 1, edg[k].vertex1 - indice, valoresSG[i - 1][edg[k].vertex1 - indice]);
                                                                if (!valoresSG[i][h - SumaNodosSG])
                                                                        if (!valoresSG[i - 1][edg[k].vertex1 - indice]) { printf("(valido)\n"); } //0-0
                                                                        else {
                                                                                printf("(check en SG) \n"); ChecaEnSG = 1; indexSG_1 = edg[k].vertex1 - indice;
                                                                        } //0-1, check en SG
                                                                else
                                                                    if (!valoresSG[i - 1][edg[k].vertex1 - indice]) {
                                                                        printf("(check)\n"); //hago cero el valor del nodo SG
                                                                        if (!Checa_Vecinos(i - 1, vSG[i - 1][edg[k].vertex1 - indice].id, edg[k].vertex1 - indice)) valoresSG[i][h - SumaNodosSG] = 0;
                                                                }//1-0, check aqui
                                                                else { printf("(Reject) \n"); valoresSG[i - 1][edg[k].vertex1 - indice] = 0; } //1-1, hago 0 el valor del nodo de SG-1
                                                        }
                                                        else {
                                                                printf(" valoresSG[%d][%d]=%d \n", i - 1, edg[k].vertex2 - indice, valoresSG[i - 1][edg[k].vertex2 - indice]);
                                                                if (!valoresSG[i][h - SumaNodosSG])
                                                                        if (!valoresSG[i - 1][edg[k].vertex2 - indice]) { printf("(valido)\n"); } //0-0
                                                                        else { printf("(check  en SG) \n"); ChecaEnSG = 1; indexSG_1 = edg[k].vertex2 - indice; } //0-1, check en SG
                                                                else
                                                                    if (!valoresSG[i - 1][edg[k].vertex2 - indice]) {
                                                                        printf("(check)\n"); //hago cero el valor del nodo SG
                                                                        if (!Checa_Vecinos(i - 1, vSG[i - 1][edg[k].vertex2 - indice].id, edg[k].vertex2 - indice)) valoresSG[i][h - SumaNodosSG] = 0;
                                                                }//1-0, check aqui
                                                                else { printf("(Reject) \n"); valoresSG[i - 1][edg[k].vertex2 - indice] = 0; } //1-1, hago 0 el valor del nodo de SG-1
                                                        }
                                                } // if, vecinos de la capa anterior
                                                else {
                                                        //checar aqui o mejor desde edgSG[][]
                                                }//else, vecinos de su misma capa
                                                if (entra) {
                                                        printf("vecino en SG: "); //checa edgSG[][]
                                                        flag = 1;
                                                        for (int p : vSG[i][h - SumaNodosSG].liga) {
                                                                if (edgSG[i][p].vertex1 != h - SumaNodosSG) {
                                                                        printf(" valoresSG[%d][%d]=%d\n", i, edgSG[i][p].vertex1, valoresSG[i][edgSG[i][p].vertex1]);
                                                                        if (ChecaEnSG)
                                                                                if (valoresSG[i][edgSG[i][p].vertex1]) flag = 0;
                                                                }
                                                                else {
                                                                        printf("  valoresSG[%d][%d]=%d\n", i, edgSG[i][p].vertex2, valoresSG[i][edgSG[i][p].vertex2]);
                                                                        if (ChecaEnSG)
                                                                                if (valoresSG[i][edgSG[i][p].vertex2]) flag = 0;
                                                                }
                                                        }//for p
                                                        //printf("\n");
                                                        if (ChecaEnSG)
                                                                if (flag) { printf("*        valido \n"); }
                                                                else { printf("*        No valido \n"); valoresSG[i - 1][indexSG_1] = 0; }//hay que hacer cero el nodo en SG-1
                                                }//checar vecinos de su misma capa, entra=1
                                                entra = 0;
                                        }//for k

                                }//NumNeighb>2
                                //g++;
                        }
                }//for j, nodos de la lista general de nodos v[ ]
                printf("\n");

                SumaNodosSG += NumNodosSG[t];
        } //for t , global, para cada SG
        printf("\n");
}
//***********************************************
/* void GMLwriteToExcel(std::string p_filename) */

void GMLwriteToconsole(int value_of_new_subgraphs)
{
        using namespace std;
        //std::stringstream ss;
        //ss << std::tab;
        cout << "SG" << "\t" << "Capas" << "\t" << "Nodos" << "\t" << "Aristas" << "\t" << "Costo" << "\t" << "Tiempo" << "\t\t" << "CostoS" << endl;
        for (int i = 0; i < MaxNumSG; i++)
        {
                cout << i << "\t" << NivelBegin[i] << "," << NivelEnd[i] << "  " << NumNodosSG[i] << "\t" << NumEdgesSG[i] << "\t" << costo_old[i] << "\t" << tiempo[i] << "\t\t" << costo[i] << endl;
        }
        cout << " " << "\t" << " " << "\t" << " " << "\t" << " " << "\t" << "Perlim. Costs" << "\t" << " " << "\t" << "Costo APX_2P" << endl;
        cout << " " << "\t" << " " << "\t" << " " << "\t" << " " << "\t" << CostoGlobal + value_of_new_subgraphs << "\t" << " " << "\t\t" << CostoGlobal << endl;

}

//***********************************************
bool checa_rojo_End(unsigned int NumeroSG, int i) {
        int arista, parent, aristaNextLevel, parentNextLevel;
        bool rojo = 1; //asumo que puede ser rojo
        for (int j = 0; j < v[PorNivel[NivelEnd[NumeroSG] - 1][i]].liga.size(); j++) {
                arista = v[PorNivel[NivelEnd[NumeroSG] - 1][i]].liga[j];
                parent = v[PorNivel[NivelEnd[NumeroSG] - 1][i]].id;
                //printf("%d<%d,%d>,", arista, edg[arista].vertex1, edg[arista].vertex2);
                parentNextLevel = -1;
                if (edg[arista].vertex1 != parent) parentNextLevel = edg[arista].vertex1; // printf("<%d>,", edg[arista].vertex1);
                else parentNextLevel = edg[arista].vertex2; //printf("<%d>,", edg[arista].vertex2);
                //printf("<%d>,", parentNextLevel);
                if (!valores[parentNextLevel]) {
                        //printf("\n <%d> con vecinos: ", parentNextLevel); //aristaNextLevel;
                        for (int k = 0; k < v[parentNextLevel].liga.size(); k++) {
                                aristaNextLevel = v[parentNextLevel].liga[k];
                                if (edg[aristaNextLevel].vertex1 != parentNextLevel) {
                                        if (edg[aristaNextLevel].vertex1 != parent)
                                                if (!valores[edg[aristaNextLevel].vertex1]) printf("<%d>,", edg[aristaNextLevel].vertex1);
                                                else { printf(" rojo \n"); rojo = 0;  break; }
                                }
                                else {
                                        if (edg[aristaNextLevel].vertex2 != parent)
                                                if (!valores[edg[aristaNextLevel].vertex2]) printf("<%d>,", edg[aristaNextLevel].vertex2);
                                                else { printf(" rojo \n"); rojo = 0;  break; }
                                }
                        }//for k
                }//azul
                else {//rojo
                        printf(" rojo \n"); rojo = 0;  break;
                }
        }
        return rojo;
}
//***********************************************
bool checa_rojo_Begin(unsigned int NumeroSG, int i) {
        int arista, parent, aristaNextLevel, parentNextLevel;
        bool rojo = 1; //asumo que puede ser rojo
        for (int j = 0; j < v[PorNivel[NivelBegin[NumeroSG] - 1][i]].liga.size(); j++) {
                arista = v[PorNivel[NivelBegin[NumeroSG] - 1][i]].liga[j];
                parent = v[PorNivel[NivelBegin[NumeroSG] - 1][i]].id;
                //printf("%d<%d,%d>,", arista, edg[arista].vertex1, edg[arista].vertex2);
                parentNextLevel = -1;
                if (edg[arista].vertex1 != parent) parentNextLevel = edg[arista].vertex1; // printf("<%d>,", edg[arista].vertex1);
                else parentNextLevel = edg[arista].vertex2; //printf("<%d>,", edg[arista].vertex2);
                //printf("<%d>,", parentNextLevel);
                if (!valores[parentNextLevel]) {
                        //printf("\n <%d> con vecinos: ", parentNextLevel); //aristaNextLevel;
                        for (int k = 0; k < v[parentNextLevel].liga.size(); k++) {
                                aristaNextLevel = v[parentNextLevel].liga[k];
                                if (edg[aristaNextLevel].vertex1 != parentNextLevel) {
                                        if (edg[aristaNextLevel].vertex1 != parent)
                                                if (!valores[edg[aristaNextLevel].vertex1]) printf("<%d>,", edg[aristaNextLevel].vertex1);
                                                else { printf(" rojo \n"); rojo = 0;  break; }
                                }
                                else {
                                        if (edg[aristaNextLevel].vertex2 != parent)
                                                if (!valores[edg[aristaNextLevel].vertex2]) printf("<%d>,", edg[aristaNextLevel].vertex2);
                                                else { printf(" rojo \n"); rojo = 0;  break; }
                                }
                        }//for k
                }//azul
                else {//rojo
                        printf(" rojo \n"); rojo = 0;  break;
                }
        }
        return rojo;
}
//***********************************************
int main(int argc, char *argv[])
{

        char tecla = 'a';
        unsigned int k = 0;
        int SumaSGparaIndex = 0; //va sumando el numero de nodos de los SGs
        float SumaSGTiempo = 0.0f; //va sumando el tiempo en segundos de los SGs
        int summing = 0, summingV1 = 0; //checando el numero de nodos
        //int A; // , arista, parent, aristaNextLevel, parentNextLevel;
        bool B;
        unsigned int h, index;
        unsigned int valueNodeEnd, valueNodeBegin;
        printf("Cargando archivo GML\n");
        //GMLread("D:/semestres/2021/sem1-2021/GraphsJoel/02 Febrero/gmlFeb18/Outerplanar100_5.gml");
        const char* filename = argv[1];
        GMLread(filename);


        /****************************** NEW DATA TO CHECK THE SOLUTION *****************************/
        for(int j = 0; j < NumEdges; j++) {
                int x = edg[j].vertex1;
                int w = edg[j].vertex2;
                v[x].neighborhood.push_back(w);
                v[w].neighborhood.push_back(x);
        }

        for(int i = 0; i < v[0].neighborhood.size(); i++) {
                std::cout << v[0].neighborhood[i] << " ";
        }
        std::cout << std::endl;


        /******************************************* START TIME MEASUREMENT *************************************************/

        std::chrono::high_resolution_clock::time_point start, end;
        start = std::chrono::high_resolution_clock::now();

        printf("Termina de cargar archivo G, presiona una tecla\n");
        h = 15; //numero de niveles o capas por SG
        MaxNumSG = MaxNivel / h; //Num SGs
        for (unsigned int i = 0; i < 5000; i++) { NivelCreado[i] = 0;  /*NodosPorNivel[i] = 0;*/ } //para G
        for (unsigned int i = 0; i < MaxNumSG; i++) { SGcreado[i] = 0; NumNodosSG[i] = 0; restaSG[i] = 0; } //MaxNumSG

        for (unsigned int i = 0; i < MaxNumSG; i++)
        {
                printf("SG %d con niveles %d a %d   ->   ", i, h * i + 1, h * (i + 1));
                NivelBegin[i] = h * i + 1; NivelEnd[i] = h * (i + 1);
        }

        int sobran;//Son los del último nivel
        sobran = MaxNivel;
        sobran -= NivelEnd[MaxNumSG - 1];
        printf("\n MaxNivel=%d h=%d, MaxNumSG=%d, sobran=%d\n\n", MaxNivel, h, MaxNumSG, sobran);

        if (sobran > 0) {
                unsigned int incr = 0;
                k = 1;
                printf("sobran>0, k=%d\n", k);
                unsigned int i = 0;
                while (incr < sobran)
                {
                        NivelBegin[i] += k - 1; NivelEnd[i] += k;
                        printf("k=%d, NivelBegin[%d]=%d NivelEnd[%d]=%d \n", k, i, NivelBegin[i], i, NivelEnd[i]);
                        k++;
                        if (i < MaxNumSG - 1) i++;
                        else { i = 0; k = 1; }
                        incr++;
                }

                if (i > 0) {
                        k = i; incr = NivelEnd[i - 1] - NivelBegin[i] + 1;
                        printf("Hay que ajustar los arreglos del i=%d a MaxNumSG-1=%d, incr=%d \n", i, MaxNumSG - 1, incr);
                        for (unsigned int i = k; i < MaxNumSG; i++) {
                                NivelBegin[i] += incr; NivelEnd[i] += incr;
                                printf("NivelBegin[%d]=%d NivelEnd[%d]=%d \n", i, NivelBegin[i], i, NivelEnd[i]);
                        }
                }//i>0
                else {
                        printf("NO ajustar los arreglos del i=%d a MaxNumSG-1=%d \n", i, MaxNumSG - 1);
                }
        }
        //----------------------------------------------------------------------------------------------------Primeros SGs
        printf("Llamando a Subdivide\n");
        Subdivide();

        for (unsigned int j = 0; j < MaxNumSG; j++) {
                printf("NumNodosSG[%d]=%d, restaSG[%d]=%d, NumEdgesSG[%d]=%d\n", j, NumNodosSG[j], j, restaSG[j], j, NumEdgesSG[j]);
                summing += NumNodosSG[j] + restaSG[j];
        }


        printf("summing=%d \n", summing);
        printf("Termina Subdivide y empieza Alg()\n");
        printf("Pulsa una tecla...\n"); //_getch();
        //----------------------------------------------------------------------------------------------------Llama a Cplex
        for (unsigned int j = 0; j < MaxNumSG; j++)
        {
                printf("SG=%d--------\n", j);
                NumSG = j; Alg();
        }

        /* 	for (unsigned int j = 0; j < MaxNumSG; j++) SolConflictos(j);  */
        printf("CostoGlobal=%d, SumaSGTiempo=%f\n", CostoGlobal_old, SumaSGTiempo);

        printf("\n Verificando condiciones . . . . . . . . . . . . . . . . \n"); //---------------------------costo, tiempo
        SumaSGparaIndex = 0;
        SumaSGTiempo = 0.0f;
        for (unsigned int j = 0; j < MaxNumSG; j++)
        {
                costo_old[j] = costo[j];
                TestConstraintsSGv1(j, SumaSGparaIndex);
                CostoGlobal = CostoGlobal + costo[j];
                SumaSGparaIndex += NumNodosSG[j] + restaSG[j];
                SumaSGTiempo += tiempo[j];
        }
        printf("CostoGlobal=%d, SumaSGTiempo=%f\n", CostoGlobal, SumaSGTiempo);
        CostoGlobal_old = CostoGlobal;
        for(int SG = 0; SG < MaxNumSG; SG++) {
                for(int vertex = 0; vertex < NumNodosSG[SG]; vertex++) {
                        if(valoresSG[SG][vertex] == 1) {
                                valores[vSG[SG][vertex].id] = 1;
                        }
                }
        }

        /*
                std::vector<bool> checked_3(NumNodes, 0);
                for(int i = 0; i < NumNodes; i++) {
                        if(valores[v[i].id] == 1) {
                                int is_valid = 0;
                                checked_3[v[i].id] = 1;
                                for(int j = 0; j < v[i].neighborhood.size(); j++) {
                                        int neighh = v[v[i].neighborhood[j]].id;
                                        if(valores[neighh] == 1 && checked_3[neighh] == 0) {
                                                is_valid++;
                                                //valores[neighh] = 0;
                                        }
                                        checked_3[neighh] = 1;
                                        for(int k = 0; k < v[neighh].neighborhood.size(); k++) {
                                                if(valores[v[v[neighh].neighborhood[k]].id] == 1 && checked_3[v[v[neighh].neighborhood[k]].id] == 0) {
                                                        is_valid++;
                                                        checked_3[v[v[neighh].neighborhood[k]].id] = 1;
                                                        //valores[v[v[neighh].neighborhood[k]].id] = 0;
                                                        std::cout << v[v[neighh].neighborhood[k]].id << std::endl;
                                                }
                                        }
                                }
                                if(is_valid != 0) {
                                        std::cout << "error" << std::endl;
                                }
                        }
                }
        */








        /*
        std::vector<bool> checked(NumNodes, 0);
        for(int i = 0; i < NumNodes; i++) {
                if(valores[v[i].id] == 1) {
                        int is_valid = 0;
                        checked[v[i].id] = 1;
                        for(int j = 0; j < v[i].neighborhood.size(); j++) {
                                int neighh = v[v[i].neighborhood[j]].id;
                                if(valores[neighh] == 1) {
                                        is_valid++;
                                }
                                checked[neighh] = 1;
                                for(int k = 0; k < v[neighh].neighborhood.size(); k++) {
                                        if(valores[v[v[neighh].neighborhood[k]].id] == 1 && checked[v[v[neighh].neighborhood[k]].id] == 0) {
                                                is_valid++;
                                                checked[v[v[neighh].neighborhood[k]].id] = 1;
                                                std::cout << v[v[neighh].neighborhood[k]].id << std::endl;
                                        }
                                }
                        }
                        if(is_valid != 0) {
                                std::cout << "error" << std::endl;
                        }
                }
        } */
        printf("Pulsa una tecla...\n"); //_getch();
        //----------------------------------------------------------------------------------------------------Los que valen 1
        NumSG = 0;

        /*
        for (int i = 0; i < 266; i++) {
                printf("vSG[%d][%d].id=%d(%d),  ", NumSG, i, vSG[NumSG][i].id, vSG[NumSG][i].level);
                printf("valoresSG[%d][%d]=%d \n", NumSG, i, valoresSG[NumSG][i]);
        }
    printf("Pulsa una tecla...para salir del Programa. . . \n"); _getch();*/

        while (NumSG < MaxNumSG - 1) {
                index = NumNodosSG[NumSG] - 1;
                summing = PorNivel[NivelEnd[NumSG] - 1].size();
                while (vSG[NumSG][index].level == NivelEnd[NumSG] - 1) {
                        //printf("vSG[%d][%d].id=%d(%d),  ", NumSG, index, vSG[NumSG][index].id, vSG[NumSG][index].level);
                        //printf("valoresSG[%d][%d]=%d ", NumSG, index, valoresSG[NumSG][index]);
                        valoresSG[NumSG][summing - 1] = valoresSG[NumSG][index];
                        //printf("->valoresSG[%d][%d]=%d \n", NumSG, summing-1, valoresSG[NumSG][summing - 1]);
                        index--; summing--;
                }//while
                //printf("\n");
                NumSG++;
                index = 0;
                while (vSG[NumSG][index].level == NivelBegin[NumSG] + 1) {
                        //printf("vSG[%d][%d].id=%d(%d),  ", NumSG, index, vSG[NumSG][index].id, vSG[NumSG][index].level);
                        //printf("valoresSG[%d][%d]=%d ", NumSG, index, valoresSG[NumSG][index]);

                        summingV1 = index + PorNivel[NivelEnd[NumSG - 1] - 1].size() +
                                    PorNivel[NivelEnd[NumSG - 1]].size() + PorNivel[NivelEnd[NumSG - 1] + 1].size();
                        valoresSG[NumSG][summingV1] = valoresSG[NumSG][index];
                        //printf("->valoresSG[%d][%d]=%d \n", NumSG-1, summingV1, valoresSG[NumSG][summingV1]);
                        index++;
                }//while
                //printf("\n");
        }//while NumSG

           /*
                   std::vector<bool> checked(NumNodes, 0);
                   for(int i = 0; i < NumNodes; i++) {
                           if(valores[v[i].id] == 1) {
                                   int is_valid = 0;
                                   checked[v[i].id] = 1;
                                   for(int j = 0; j < v[i].neighborhood.size(); j++) {
                                           int neighh = v[v[i].neighborhood[j]].id;
                                           if(valores[neighh] == 1) {
                                                   is_valid++;
                                           }
                                           checked[neighh] = 1;
                                           for(int k = 0; k < v[neighh].neighborhood.size(); k++) {
                                                   if(valores[v[v[neighh].neighborhood[k]].id] == 1 && checked[v[v[neighh].neighborhood[k]].id] == 0) {
                                                           is_valid++;
                                                           checked[v[v[neighh].neighborhood[k]].id] = 1;
                                                           std::cout << v[v[neighh].neighborhood[k]].id << std::endl;
                                                   }
                                           }
                                   }
                                   if(is_valid != 0) {
                                           std::cout << "error" << std::endl;
                                   }
                           }
                   }
           */
        //-----------------------------------------------------------------------------------------------------Nuevos SGs
        printf("Llamando a Subdivide_SG_Capas_Contiguas()\n");
        Subdivide_SG_Capas_Contiguas();
        printf("Pulsa una tecla...\n"); //_getch();

        for(NumSG = 0; NumSG< MaxNumSG - 1; NumSG++)
                for (int i = 0; i < NumNodosSG[NumSG]; i++) {

                        if (vSG[NumSG][i].level == NivelEnd[NumSG] || vSG[NumSG][i].level == NivelEnd[NumSG]+1) {
                                valoresSG[NumSG][i] = 0;
                                valores[vSG[NumSG][i].id] = 0;
                                //printf("- - -  ");
                        }
                        //printf("vSG[%d][%d].id=%d(%d),  ", NumSG, i, vSG[NumSG][i].id, vSG[NumSG][i].level);
                        //printf("valoresSG[%d][%d]=%d \n", NumSG, i, valoresSG[NumSG][i]);
                }
        //----------------------------------------------------------------------------------------------------Llama a Cplex

        /*NumSG = 2;
        Alg_SG_Capas_Contiguas();
        NumSG = 3;
        Alg_SG_Capas_Contiguas();*/


        for (NumSG = 0; NumSG < MaxNumSG - 1; NumSG++)
        {
                printf("SG=%d--------\n", NumSG);
                Alg_SG_Capas_Contiguas();

        }//for NumSG

        /*************************** HERE IS NEW CODE *********************************************/

        // We compute the solution size in each new subgraph.
        int value_of_new_subgraphs = 0;
        // Compute solution size which is too big
        for (int SG = 0; SG < MaxNumSG-1; SG++) {
                for(int vertex = 0; vertex < NumNodosSG[SG]; vertex++) {
                        if(valoresSG[SG][vertex] == 1) {
                                value_of_new_subgraphs++;
                                valores[vSG[SG][vertex].id] = 1;
                                //std::cout << vSG[SG][vertex].id << std::endl;
                        }
                }
        }

        int min_level = 5000; // This is the maximum amount of levels we can have so this should be safe.
        int max_level = 0;
        /*	for(int i = 0; i < NumNodosSG[0]; i++) {
                        if (vSG[0][i].level < min_level) {
                                min_level = vSG[0][i].level;
                        }
                        if(vSG[0][i].level > max_level) {
                                max_level = vSG[0][i].level;
                        }
                }*/


        std::vector<int> min_levels(MaxNumSG-1, 5000);
        std::vector<int> max_levels(MaxNumSG-1, 0);

        for(int j = 0; j < MaxNumSG-1; j++) {
                for(int i = 0; i < NumNodosSG[j]; i++) {
                        if (vSG[j][i].level < min_levels[j]) {
                                min_levels[j] = vSG[j][i].level;
                        }
                        if(vSG[j][i].level > max_levels[j]) {
                                max_levels[j] = vSG[j][i].level;
                        }
                }
        }
        /*
                std::vector<int> middle_level_minus(MaxNumSG-1, 0);
                std::vector<int> middle_level_plus(MaxNumSG-1, 0);

                for(int i = 0; i < MaxNumSG-1; i++) {
                        middle_level_minus[i] = min_levels[i] + 1;
                        middle_level_plus[i] = max_levels[i] - 1;
                }
                for(int SG = 0; SG < MaxNumSG-1; SG++) {
                        for(int vertex = 0; vertex < NumNodosSG[SG]; vertex++) {
                                if(valoresSG[SG][vertex] == 1) {
                                        //valores[vSG[SG][vertex].id] = 1;
                                }
                        }
                }
        */
        /*
                // This is what is count twice so we have to substract it from the solution
                //int to_substract_from_solution = 0;
                for(int SG = 0; SG < MaxNumSG-1; SG++) {
                        for(int vertex = 0; vertex < NumNodosSG[SG]; vertex++) {
                                if(vSG[SG][vertex].level == min_levels[SG] || vSG[SG][vertex].level == max_levels[SG]) {
                                        if(valoresSG[SG][vertex] == 1) {
                                                //to_substract_from_solution++;
                                                //valoresSG[SG][vertex] == 0;
                                                //valores[vSG[SG][vertex].id] == 0;
                                        }
                                }
                        }
                }
        */


        std::vector<bool> checked(NumNodes, 0);
        for(int i = 0; i < NumNodes; i++) {
                if(valores[v[i].id] == 1) {
                        int is_valid = 0;
                        checked[v[i].id] = 1;
                        for(int j = 0; j < v[i].neighborhood.size(); j++) {
                                int neighh = v[v[i].neighborhood[j]].id;
                                if(valores[neighh] == 1 && checked[neighh] == 0) {
                                        is_valid++;
                                        valores[neighh] = 0;
                                }
                                checked[neighh] = 1;
                                for(int k = 0; k < v[neighh].neighborhood.size(); k++) {
                                        if(valores[v[v[neighh].neighborhood[k]].id] == 1 && checked[v[v[neighh].neighborhood[k]].id] == 0) {
                                                is_valid++;
                                                checked[v[v[neighh].neighborhood[k]].id] = 1;
                                                valores[v[v[neighh].neighborhood[k]].id] = 0;
                                                //						std::cout << v[v[neighh].neighborhood[k]].id << std::endl;
                                        }
                                }
                        }
                        if(is_valid != 0) {
                                //				std::cout << "error" << std::endl;
                        }
                }
        }

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(end-start);
        double total_time = elapsed_time.count();


        // Check again
        std::cout << "Check again: " << std::endl;
        bool correct = true;
        std::vector<bool> checked_2(NumNodes, 0);
        for(int i = 0; i < NumNodes; i++) {
                if(valores[v[i].id] == 1) {
                        int is_valid = 0;
                        checked_2[v[i].id] = 1;
                        for(int j = 0; j < v[i].neighborhood.size(); j++) {
                                int neighh = v[v[i].neighborhood[j]].id;
                                if(valores[neighh] == 1 && checked_2[neighh] == 0) {
                                        is_valid++;
                                }
                                checked_2[neighh] = 1;
                                for(int k = 0; k < v[neighh].neighborhood.size(); k++) {
                                        if(valores[v[v[neighh].neighborhood[k]].id] == 1 && checked_2[v[v[neighh].neighborhood[k]].id] == 0) {
                                                is_valid++;
                                                checked_2[v[v[neighh].neighborhood[k]].id] = 1;
                                                std::cout << v[v[neighh].neighborhood[k]].id << std::endl;
                                        }
                                }
                        }
                        if(is_valid != 0) {
                                std::cout << "error" << std::endl;
                                correct = false;
                        }
                }
        }

        if(correct) {
                std::cout << "Solution is valid now." << std::endl;
        }







        /************************************** CHECK SOLUTION **********************************************************/

        int checked_solution_size = 0;
        for(int i = 0; i < NumNodes; i++) {
                if(valores[i] == 1) {
                        checked_solution_size++;
                }
        }


        //************************************************************************************************************************
        std::cout << std::endl;
        //GMLwriteToconsole(value_of_new_subgraphs);
        //for(int i = 0; i < MaxNumSG-1; i++) {
        //	std::cout << "Levels to be considered for SG " << i << ": " << middle_level_minus[i] << " " << middle_level_plus[i] << std::endl;
        //}
        //std::cout << std::endl;
        std::cout << "\n" << "Time: " << total_time << std::endl;
        std::cout << "\n" << "Checked solution size: " << checked_solution_size << std::endl;


        return 1;

}
