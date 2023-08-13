# coding = utf - 8
import os
import random
import sys
import time

import numpy as np
from deap import base
from deap import creator
from deap import tools
from igraph import *  # pip3 install python-igraph

from statistics import geometric_mean

SANITY_CHECK = True
RANDOM_SEEDS = False

SEEDS = [342, 24, 242332, 218, 31, 3364, 121, 7451]
seed = SEEDS[0]
random.seed(seed)

######## GLOBAL VARS START ########
# global variables
# the following values might be overriden after and graph was read from a file
n_nodes = 5372  # number of nodes in the graph
e = np.zeros((n_nodes, n_nodes))  # shape (n_nodes, n_nodes)
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


######## GLOBAL VARS END ########


# la funcion objetivo  ('fitness')  que se busca maximizar
# the objective function ('fitness') that is sought to be maximized
def eval2packing(x):
    """ Determine the fitness of an individual
    :param x: is a solution indicator for the current adj matrix e
    :return: fitness
    """
    fitness = 0
    n = len(x)
    # assert (n == n_nodes and n == len(e) and (n == 0 or n == len(e[0])))
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
    # ind = offspring[0]
    size_ind = len(ind)

    # identifica el conjunto solucion S
    S = []
    for i in range(0, size_ind):
        if (ind[i] == 1):
            S.append(i)

    if (len(S) == 0):
        ind[random.randint(0, size_ind - 1)] = 1

    # identifica los conjuntos 0-tight y 1 tight y no-tight
    # Un nodo i pertenece al conjunto no-tight si i tiene 2 o más vecinos en S (incluyendose él)
    zero_tight = []
    one_tight = []
    no_tight = []
    for i in range(0, size_ind):
        neighbors = g.neighbors(i)
        if (ind[i] == 0):
            # neighbors = g.neighbors(i)
            if (len(set(neighbors).intersection(set(S))) == 0):
                zero_tight.append(i)
            elif (len(set(neighbors).intersection(set(S))) == 1):
                one_tight.append(i)
            else:
                no_tight.append(i)
        elif (len(set(neighbors).intersection(set(S))) > 0):
            no_tight.append(i)

    # Si existe un nodo u no-tight:
    # => Se elimina a un vecino de u en S
    for i in no_tight:
        if (i in S):
            ind[i] = 0
            S.remove(i)
        neighbors = g.neighbors(i)
        for j in neighbors:
            if (j in S):
                ind[j] = 0
                S.remove(j)
                break
        zero_tight.append(i)
    return ind


# ----------
# Registro de Operadores
# ----------

# registra el objetivo / funcion para determinar fitness
toolbox.register("evaluate", eval2packing)

# registro del operador de cruzamiento
# toolbox.register("mate", tools.cxOnePoint)
toolbox.register("mate", tools.cxTwoPoint)

# registra un operador de mutacion con una probabilidad  de
# cambiar cada atributo/gen del  0.05
# toolbox.register("mutate", tools.mutFlipBit, indpb=0.15)
toolbox.register("mutate", tools.mutFlipBit, indpb=0.17)

# registra un operador de mejora (improvement)
toolbox.register("improve", improvement)

# seleccion de padres
# toolbox.register("select", tools.selTournament, tournsize=3)
toolbox.register("select", tools.selRoulette)

# seleccion de sobrevivientes
toolbox.register("survival_selection", tools.selBest)


# ----------

# Se declara el metodo principal que contendra el algoritmo genetico
def algoritmoGenetico(improvement=True):
    # crea una poblacion inicial de 300 individuos (Donde
    # cada individuo es una lista de enteros)
    pop = toolbox.population(n=200)
    # N numero maximo de generaciones
    # CXPB  es la probabilidad con la que cada dos individuos
    #       son cruzados
    # MUTPB es la probabilidad para mutar un  individuo
    N, CXPB, MUTPB = 200, 0.9, 0.3
    print("Comienza la evolucion")

    # Evalua la problacion entera
    fitnesses = list(map(toolbox.evaluate, pop))
    for ind, fit in zip(pop, fitnesses):
        ind.fitness.values = fit

    print("  Evaluando %i individuos" % len(pop))
    fits = [ind.fitness.values[0] for ind in pop]

    g = 0
    while g < N:
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
        # pop[:] = offspring
        pop[:] = toolbox.survival_selection(pop + offspring, len(pop))

        # Reune todos los valores de aptitud en una lista e imprime las estadisticas
        fits = [ind.fitness.values[0] for ind in pop]
        length = len(pop)
        mean = sum(fits) / length
        sum2 = sum(x * x for x in fits)
        std = abs(sum2 / length - mean ** 2) ** 0.5
        print("  Min fitness %s" % min(fits))
        print("  Max fitness %s" % max(fits))
        print("  Media %s" % mean)
        print("  Std %s" % std)

    print("-- Fin de la evolucion --")
    best_ind = tools.selBest(pop, 1)[0]
    return best_ind


###### SETUP EXPERIMENT ######
# read all GML files from the instances folder
# PathGML = "./instances_wake"
# lstFilesGML = []  # create an empty list
# for dirName, subdirList, fileList in os.walk(PathGML):
#    for filename in fileList:
#        print(filename)
#        if ".gml" in filename.lower():  # check whether the file's GML
#            lstFilesGML.append(os.path.join(dirName, filename))

filename = sys.argv[1]  # .gml file
outdir = sys.argv[2]

graphName = os.path.basename(filename)  # [len(PathGML) + 0:]
print(graphName)

# file to store solution numbers
fd = open(os.path.join(outdir, "results_%s_wake.csv" % (graphName.split(".")[0])), 'w')
fd.write('Graph,Exp,Seed,GA_withImp,Init_Time,Solve_Time\n')

# file to store statistics
fd_stat = open(os.path.join(outdir, "statistics_%s_wake.csv" % (graphName.split(".")[0])), 'w')
fd_stat.write(
    'Graph,Mean_withImp,SD_withImp,Max_withImp,Min_withImp,GMean_InitTime,GMean_SolveTime,GMean_Time,Best_Time\n')

fd_best_sol = open(os.path.join(outdir, "best_sol_%s_wake.csv" % (graphName.split(".")[0])), 'w')

numExp = 1

# for filename in lstFilesGML:
g = Graph.Read_GML(filename)

bestSolution_withImp = []
max_best_solution_with_imp = None

solve_time = []
init_time = []
best_time = sys.maxsize

# Runs of init and genetic algorithm
for i in range(1, numExp + 1):
    #reset
    e = None

    # set random seed
    if RANDOM_SEEDS:
        seed = SEEDS[i]
        random.seed(seed)

    # Measure initialization time for toolbox, adj. matrix A and computation of A^2
    start_time1 = time.time()

    ######### BENCHMACK START #########
    n_nodes = g.vcount()  # number of nodes in the graph

    # Inicializacion de estructuras
    #                         define  'individual' como un individuo
    #                         consistiendo de  n_nodes 'attr_bool' elementos  ('genes')
    toolbox.register("individual", tools.initRepeat, creator.Individual, toolbox.attr_bool, n_nodes)

    # Jannick: population must be updated since toolbox.individual changed
    # define la poblacion como una lista de individuos
    toolbox.register("population", tools.initRepeat, list, toolbox.individual)

    # This is the patch by Dominik to initialize the adj. matrix.
    # g_array = np.zeros((n_nodes, n_nodes))
    g_array = np.array(g.get_adjacency().data)
    for j in range(n_nodes):
        g_array[j][j] = 1
    # print(g_array.shape)
    e = np.matmul(g_array, g_array)

    # diagonal en ceros
    # replace diagonal entries with zeros
    for j in range(n_nodes):
        e[j][j] = 0
    ######### BENCHMACK END #########

    end_time1 = time.time()
    elapsed_time1 = (end_time1 - start_time1) * 1000
    init_time.append(elapsed_time1)
    print(f"Elapsed time: {elapsed_time1:.2f} milliseconds")

    # Measure run of the genetic algorithm
    start_time2 = time.time()

    ######### BENCHMACK START #########
    # Set improvement to False if you want to test a worse variant of the algorithm
    best_ind = algoritmoGenetico(improvement=True)
    bestSolution_withImp.append(best_ind.fitness.values[0])
    ######### BENCHMACK END #########

    end_time2 = time.time()
    elapsed_time2 = end_time2 - start_time2
    solve_time.append(elapsed_time2)

    if i == 1 or best_ind.fitness.values[0] > max_best_solution_with_imp.fitness.values[0] \
            or (best_ind.fitness.values[0] == max_best_solution_with_imp.fitness.values[
        0] and best_time > elapsed_time2 + elapsed_time1):
        max_best_solution_with_imp = best_ind
        best_time = elapsed_time2 + elapsed_time1

    # log results
    print("The best individual (with improvement) is  %s, %s" % (best_ind, best_ind.fitness.values))
    cardinality = sum(1 for x in best_ind if x != 0)
    print(cardinality)
    print(f"Elapsed time: {elapsed_time2:.2f} seconds")

    # write results
    fd.write(graphName + ',')
    fd.write(str(i) + ',')
    fd.write(str(seed) + ',')
    fd.write(str(best_ind.fitness.values[0]) + ',')
    fd.write(str(elapsed_time1) + ',')
    fd.write(str(elapsed_time2) + '\n')

# Jannick: adding sanity check: independent set
if SANITY_CHECK and max_best_solution_with_imp.fitness.values[0] > 0:
    objc = 0
    for k in range(n_nodes):
        if max_best_solution_with_imp[k] == 1:
            objc += 1
            for j in range(k + 1, n_nodes):
                if e[k][j] != 0 and max_best_solution_with_imp[j] == 1:
                    print(e)
                    print("Best solution is no independent set (%s, %s, %s, %s)" % (graphName, i, k, j))
                    exit(1)
    objc = float(objc)
    if objc != max_best_solution_with_imp.fitness.values[0]:
        print("fittness != solution size (%s, %s, %s, %s)" % (
        graphName, i, objc, max_best_solution_with_imp.fitness.values[0]))
        exit(1)

data2 = np.asarray(bestSolution_withImp)
print(data2)

# reemplaza los fitness negativos por cero para no sesgar las estadisticas
data2[data2 < 0] = 0

# escribe estadisticas a archivo
fd_stat.write(graphName + ',')
fd_stat.write(str(np.mean(data2)) + ',')
fd_stat.write(str(np.std(data2)) + ',')
fd_stat.write(str(np.max(data2)) + ',')
fd_stat.write(str(np.min(data2)) + ',')
fd_stat.write(str(geometric_mean(init_time)) + ',')
fd_stat.write(str(geometric_mean(solve_time)) + ',')
fd_stat.write(str(geometric_mean(np.asarray(init_time) + np.asarray(solve_time))) + ',')
fd_stat.write(str(best_time))
fd_stat.write("\n")

# write best solution
fd_best_sol.write(str(max_best_solution_with_imp[0:-1]))

fd.close()
fd_stat.close()
fd_best_sol.close()
