# coding=utf-8
import random
import numpy as np
import time
from igraph import * # pip3 install python-igraph
from utils import *
from deap import base
from deap import creator
from deap import tools
from scipy.stats import ttest_ind


n_nodes = 34 # number of nodes in the graph
e = np.zeros((n_nodes, n_nodes))
g = Graph()


# The function that assigns the fitness. (max)
creator.create("FitnessMax", base.Fitness, weights=(1.0,))

# the type of data that is the gene of the algorithm is defined
creator.create("Individual", list, fitness=creator.FitnessMax)

# an instance of the Toolbox class is created with the methods for genetic algorithms
toolbox = base.Toolbox()

# Generador de atributos
#                      define  a 'attr_bool' como un atributo ('gene')
#                      que corresponde a enteros aleatorios de forma equitativa
#                      en el rango de  [0,1] (i.e. 0 o 1 con igual
#                      probabilidad)
toolbox.register("attr_bool", random.randint, 0, 1)

# Inicializacion de estructuras
#                         define  'individual' como un individuo
#                         consistiendo de  100 'attr_bool' elementos  ('genes')
toolbox.register("individual", tools.initRepeat, creator.Individual, 
    toolbox.attr_bool, n_nodes)


# define la poblacion como una lista de individuos
toolbox.register("population", tools.initRepeat, list, toolbox.individual)


# la funcion objetivo  ('fitness')  que se busca maximizar
def eval2packing(x):
    fitness=0
    n = len(x)
    for i in range(n):
        fitness += x[i]
        penaliza = 0
        for j in range(i, n):
            penaliza += x[j] * e[i][j]            
        penaliza *= n * x[i]
        fitness -= penaliza
    return float(fitness),


######
# 2-improvements
def improvement(ind):
    #ind = offspring[0]
    size_ind = len(ind)

    # identifica el conjunto solucion S
    S = []
    for i in range(0, size_ind):
        if(ind[i] == 1):
            S.append(i)
    
    if(len(S) == 0):
        ind[random.randint(0,size_ind-1)] = 1
    
    # identifica los conjuntos 0-tight y 1 tight y no-tight 
    # Un nodo i pertenece al conjunto no-tight si i tiene 2 o más vecinos en S (incluyendose él)
    zero_tight = []
    one_tight = []
    no_tight = []
    for i in range(0, size_ind):
        neighbors = g.neighbors(i)
        if(ind[i] == 0):
            #neighbors = g.neighbors(i) 
            if(len(set(neighbors).intersection(set(S))) == 0): 
                zero_tight.append(i)
            elif(len(set(neighbors).intersection(set(S))) == 1): 
                one_tight.append(i)
            else:
                no_tight.append(i)
        elif(len(set(neighbors).intersection(set(S))) > 0):
            no_tight.append(i)
    
    # Si existe un nodo u no-tight:
    # => Se elimina a un vecino de u en S
    for i in no_tight:
        if(i in S):
            ind[i] = 0
            S.remove(i)
        neighbors = g.neighbors(i)
        for j in neighbors:
            if(j in S):
                ind[j] = 0
                S.remove(j)
                break
        zero_tight.append(i)
    return ind


#----------
# Registro de Operadores
#----------

# registra el objetivo / funcion para determinar fitness 
toolbox.register("evaluate", eval2packing)

# registro del operador de cruzamiento
#toolbox.register("mate", tools.cxOnePoint)
toolbox.register("mate", tools.cxTwoPoint)

# registra un operador de mutacion con una probabilidad  de 
# cambiar cada atributo/gen del  0.05
#toolbox.register("mutate", tools.mutFlipBit, indpb=0.15)
toolbox.register("mutate", tools.mutFlipBit, indpb=0.17)

# registra un operador de mejora (improvement)
toolbox.register("improve", improvement)

# seleccion de padres
#toolbox.register("select", tools.selTournament, tournsize=3)
toolbox.register("select", tools.selRoulette)

# seleccion de sobrevivientes
toolbox.register("survival_selection", tools.selBest)

#----------

#Se declara el metodo principal que contendra el algoritmo genetico
def algoritmoGenetico(improvement = True):
    # crea una poblacion inicial de 300 individuos (Donde
    # cada individuo es una lista de enteros)
    pop = toolbox.population(n=200)
    # N numero maximo de generaciones
    # CXPB  es la probabilidad con la que cada dos individuos
    #       son cruzados 
    # MUTPB es la probabilidad para mutar un  individuo
    N, CXPB, MUTPB = 400, 0.9, 0.3
    print("Comienza la evolucion")
    
    # Evalua la problacion entera
    fitnesses = list(map(toolbox.evaluate, pop))
    for ind, fit in zip(pop, fitnesses):
        ind.fitness.values = fit
    
    print("  Evaluando %i individuos" % len(pop))
    fits = [ind.fitness.values[0] for ind in pop]
    
    g = 0
    while  g < N:
        g = g + 1
        print("-- Generacion  %i --" % g)
        # selecciona la siguiente generacion de individuos
        offspring = toolbox.select(pop, len(pop))
        # clona descendencia 
        offspring = list(map(toolbox.clone, offspring))
        
        # aplica el cruzamiento y mutacion
        for child1, child2 in zip(offspring[::2], offspring[1::2]):
            if random.random() < CXPB:
                toolbox.mate(child1, child2)
                del child1.fitness.values
                del child2.fitness.values
        
        for mutant in offspring:
            if random.random() < MUTPB:
                toolbox.mutate(mutant)
                del mutant.fitness.values
            if improvement:
                toolbox.improve(mutant)
        
        # Evalua los individuos con un fitness invalido
        invalid_ind = [ind for ind in offspring if not ind.fitness.valid]
        fitnesses = map(toolbox.evaluate, invalid_ind)
        for ind, fit in zip(invalid_ind, fitnesses):
            ind.fitness.values = fit  
        
        print("  Evaluando %i individuos" % len(invalid_ind))
        
        # seleccion de sobrevivientes
        #pop[:] = offspring
        pop[:] = toolbox.survival_selection(pop + offspring, len(pop))
        
        # Reune todos los valores de aptitud en una lista e imprime las estadisticas
        fits = [ind.fitness.values[0] for ind in pop]
        length = len(pop)
        mean = sum(fits) / length
        sum2 = sum(x*x for x in fits)
        std = abs(sum2 / length - mean**2)**0.5
        print("  Min fitness %s" % min(fits))
        print("  Max fitness %s" % max(fits))
        print("  Media %s" % mean)
        print("  Std %s" % std)
    
    print("-- Fin de la evolucion --")
    best_ind = tools.selBest(pop, 1)[0]
    return best_ind
    


# read all GML files from the instances folder
import os
PathGML = "./instances"
lstFilesGML = []  # create an empty list
for dirName, subdirList, fileList in os.walk(PathGML):
    for filename in fileList:
        if ".gml" in filename.lower():  # check whether the file's GML
            lstFilesGML.append(os.path.join(dirName,filename))

# read the optimal values from summary.csv
optimalValue = {}
import csv

#with open("summary.csv") as fd:
#    df_reader = csv.reader(fd) # con comas
#    for row in df_reader:
#        optimalValue[row[0]] = int(row[1])

# file to store solution numbers
fd = open('results_iGraphCIF.csv','w')
fd.write('Graph,Exp,GA_withoutImp,GA_withImp\n')

# file to store statistics
fd_stat = open('statistics_iGraphCIF.csv','w')
fd_stat.write('Graph,Mean_withoutImp,SD_withoutImp,Max_withoutImp,Min_withoutImp,Mean_withImp,SD_withImp,Max_withImp,Min_withImp,T-Value,P-Value,Optimum_value\n')


numExp = 3
for filename in lstFilesGML:
    g = Graph.Read_GML(filename)
    start_time1 = time.time()
    n_nodes = g.vcount() # number of nodes in the graph
    

    # Inicializacion de estructuras
    #                         define  'individual' como un individuo
    #                         consistiendo de  n_nodes 'attr_bool' elementos  ('genes')
    toolbox.register("individual", tools.initRepeat, creator.Individual, toolbox.attr_bool, n_nodes)

    g_array = np.zeros((n_nodes, n_nodes))
    #print(g_array)
    g_array = np.array(g.get_adjacency().data)
    #print(g_array)
    for i in range(n_nodes):
        g_array[i][i]=1
    print(g_array.shape)
    e=np.matmul(g_array,g_array)
    #_MA=matrixWithDiagonal(g)
    #e=np.matmul(_MA,_MA)
    
    # diagonal en ceros
    for i in range(n_nodes):
         e[i][i]=0
    
    end_time1 = time.time()
    elapsed_time1 = (end_time1 - start_time1)*1000
    print(f"Elapsed time: {elapsed_time1:.2f} milliseconds")
    bestSolution_withoutImp = []
    bestSolution_withImp = []
    graphName = filename[len(PathGML)+0:]
    print(graphName)
    for i in range(1,numExp+1):
        fd.write(graphName + ',')
        fd.write(str(i)+',')
        #start_time2 = time.time()
        best_ind = algoritmoGenetico(improvement = False)
        bestSolution_withoutImp.append(best_ind.fitness.values[0])
        #allWhite(g)
        outputVector=best_ind
        #ColorOutput(g,outputVector)
        #plot(g, vertex_label = id_nodes)
        #print("The best individual is %s, %s" % (best_ind, best_ind.fitness.values))
        fd.write(str(best_ind.fitness.values[0])+',')
        
        start_time2 = time.time()
        best_ind = algoritmoGenetico(improvement = True)
        bestSolution_withImp.append(best_ind.fitness.values[0])
        outputVector=best_ind
        end_time2 = time.time()
        elapsed_time2 = end_time2 - start_time2
        print("The best individual (with improvement) is  %s, %s" % (best_ind, best_ind.fitness.values))
        cardinality = sum(1 for x in best_ind if x != 0)
        print(cardinality)
        print(f"Elapsed time: {elapsed_time2:.2f} seconds")
        fd.write(str(best_ind.fitness.values[0])+'\n')
    
    
    data1 = np.asarray(bestSolution_withoutImp)
    data2 = np.asarray(bestSolution_withImp)
    print(data2)
    
    # reemplaza los fitness negativos por cero para no sesgar las estadisticas
    data1[data1<0] = 0
    data2[data2<0] = 0
    
    # escribe estadisticas a archivo
    fd_stat.write( graphName +',')
    fd_stat.write(str( np.mean(data1) ) + ',')
    fd_stat.write(str( np.std(data1) ) + ',')
    fd_stat.write(str( np.max(data1) ) + ',')
    fd_stat.write(str( np.min(data1) ) + ',')
    fd_stat.write(str( np.mean(data2) ) + ',')
    fd_stat.write(str( np.std(data2) ) + ',')
    fd_stat.write(str( np.max(data2) ) + ',')
    fd_stat.write(str( np.min(data2) ) + ',')
    twosample_results = ttest_ind(data1, data2)
    fd_stat.write(str( twosample_results[0] ) + ',')
    fd_stat.write(str( twosample_results[1] ) + ',')
    #fd_stat.write(str( optimalValue[graphName] ) + '\n')

fd.close()
fd_stat.close()

