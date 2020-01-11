# Load an instance
import os
from cgshop2020_pyutils import InstanceDatabase, BestSolutionSet, SolutionZipWriter
from cgshop2020_pyutils import SolutionChecker, Visualizer
from cgshop2020_pyutils import Solution, Instance, Edge

# load challenge instances
idb = InstanceDatabase(os.path.join(os.path.dirname(__file__), ".././challenge_instances"))

def printPoints(instance):
    node_list = []
    i = 0
    for point in instance:
        printf("{i} {point.get_x()} {point.get_y()}")
        i = i + 1

# UNTESTED
def readSolution(instance):
    n = input()
    solution = Solution(instance=instance.name)
    for i in range(0,n):
        i, x, y = input().split()
        i = int(i)
        x = int(x)
        y = int(y)
    for x in range(0,n):
        line = input().split()
        k = int(line[0])
        for j in range(1,k+1):
            y = int(line[j])
            solution.add_edge(Edge(x,y))
    solution.delete_double_edges()
    return solution

# compute the triangulation for all instances
# triangulation_solver = TrivialTriangulationSolver()
#solutions = BestSolutionSet()
#for instance in idb:
    #solutions.add(useMonoSat(instance))
    #solutions.add(triangulation_solver(instance))
    #print(f"Computed triangulation for {instance.name}")

instance_loc = "uniform-0000100-2"
instance = idb[instance_loc]
printPoints(instance)
