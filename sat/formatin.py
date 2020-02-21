'''
This file will create a ZIP of the best solutions.
'''

# Load an instance
import os
import io
import sys
# Don't output the CGAL info
sys.stdout = io.StringIO()
from cgshop2020_pyutils import InstanceDatabase, BestSolutionSet, SolutionZipWriter
from cgshop2020_pyutils import SolutionChecker, Visualizer
from cgshop2020_pyutils import Solution, Instance, Edge
sys.stdout = sys.__stdout__

# load challenge instances
idb = InstanceDatabase(os.path.join(os.path.dirname(__file__), ".././challenge_instances/"))

def printPoints(instance):
    node_list = []
    i = 0
    for point in instance:
        print(f"{i} {point.get_x()} {point.get_y()}")
        i = i + 1

def readSolution(instance):
    n = int(input())
    solution = Solution(instance=name)
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

og = sys.stdin
vis = Visualizer()
checker = SolutionChecker()
solutions = BestSolutionSet()
names = open("../base_names.txt")
for name in names:
    name = name[:-1]
    # Here we can filter our instances. For example:
    #if len(instance) < 150 and instance.name.find("stars") == -1:
    # Reviewing all instances gives us the following:
    if True:
        print(f"Reading solution to {name}")
        # Solutions should be stored in ../best (this can be changed):
        f = open('../best/'+name+'.out','r')
        sys.stdin = f
        solution = readSolution(name)
        solutions.add(solution)
        sys.stdin = og
        # The following can be used to check our solutions, and print to PDF.
        # However, it is too slow to run on all instances, and will not run on mona-lisa.
        #status = checker(instance=instance, solution=solution)
        #print(status.is_feasible())
        #print(status.get_message())
        #print(status.get_objective_value())
        #vis.visualize_solution(solution=solution,instance=instance,path="pdf/"+instance.name+".pdf")

# write solutions into zip
print("Creating zip. This can take some time...")
with SolutionZipWriter("continuous-restart.zip") as zipper:
    zipper.add_solutions(solutions)
