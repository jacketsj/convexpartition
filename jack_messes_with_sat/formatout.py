# Load an instance
import os
from cgshop2020_pyutils import InstanceDatabase, BestSolutionSet, SolutionZipWriter
from cgshop2020_pyutils import SolutionChecker, Visualizer
from cgshop2020_pyutils import Solution, Instance, Edge

# load challenge instances
idb = InstanceDatabase(os.path.join(os.path.dirname(__file__), ".././challenge_instances"))

def printPoints(instance):
    node_list = []
    for point in instance:
        print(f"{point.get_x()} {point.get_y()}")

def readSolution(instance):
    m = input()
    solution = Solution(instance=instance.name)
    for i in range(0,m):
        a, b = input()
        solution.add_edge(Edge(a,b))
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
