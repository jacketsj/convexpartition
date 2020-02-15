# Load an instance
import os
import io
import sys
# Don't output the CGAL info
sys.stdout = io.StringIO()
from cgshop2020_pyutils import InstanceDatabase, BestSolutionSet, SolutionZipWriter
#from cgshop2020_pyutils import SolutionChecker, Visualizer
#from cgshop2020_pyutils import Solution, Instance, Edge
sys.stdout = sys.__stdout__

# to run subprocess.run
import subprocess

# load challenge instances
idb = InstanceDatabase(os.path.join(os.path.dirname(__file__), ".././challenge_instances"))

def printPoints(instance):
    node_list = []
    for point in instance:
        node_list.append((point.get_x(),point.get_y()))
    print(len(node_list))
    i = 0
    for (x,y) in node_list:
        x = int(x)
        y = int(y)
        print(f"{i} {x} {y}")
        i = i + 1

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

og = sys.stdout

def runOn(instance):
    print("Solving " + instance.name)
    subprocess.run("UWrMaxSat-1.0/bin/uwrmaxsat -m cnf/" + instance.name + ".cnf -v0 -cpu-lim=500 > sat/" + instance.name + ".sat",shell=True)
    print("Solved " + instance.name)

for instance in idb:
    print("Considering " + instance.name)
    #if len(instance) >= 50 and len(instance) <= 300:
    if len(instance) <= 50:
        runOn(instance)

# compute the triangulation for all instances
# triangulation_solver = TrivialTriangulationSolver()
# solutions = BestSolutionSet()
# for instance in idb:
#    print("Considering " + instance.name)
#    if len(instance) < 150 and instance.name.find("euro-night-0000020") != -1:
#    if len(instance) < 150:
#        runOn(instance)
#    #if len(instance) < 150 and instance.name.find("stars") == -1:
#    #solutions.add(triangulation_solver(instance))
#    #print(f"Computed triangulation for {instance.name}")

#instance_loc = "uniform-0000100-2"
#instance_loc = "euro-night-0000020"
#instance = idb[instance_loc]
#runOn(instance)
#printPoints(instance)
