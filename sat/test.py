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

# to run subprocess.run
import subprocess

# load challenge instances
idb = InstanceDatabase(os.path.join(os.path.dirname(__file__), ".././challenge_instances"))

def printPoints(instance):
    node_list = []
    i = 0
    for point in instance:
        print(f"{i} {point.get_x()} {point.get_y()}")
        i = i + 1

def readSolution(instance):
    n = int(input())
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

og = sys.stdin
vis = Visualizer()
checker = SolutionChecker()
solutions = BestSolutionSet()

def runOn(instance):
    print("Trying to gen for " + instance.name)
    subprocess.run("echo \""+instance.name+"\" | ./solve",shell=True)
    print("Done " + instance.name)
    print("Solving " + instance.name)
    subprocess.run("UWrMaxSat-1.0/bin/uwrmaxsat -m cnf/" + instance.name + ".cnf -v0 -cpu-lim=500 > sat/" + instance.name + ".sat",shell=True)
    print("Solved " + instance.name)
    print("Reading .sat for " + instance.name)
    subprocess.run("echo \""+instance.name+"\" | ./read_sln",shell=True)
    print("Done .out for " + instance.name)
    print(f"Reading solution to {instance.name}")
    f = open('out/'+instance.name+'.out','r')
    sys.stdin = f
    solution = readSolution(instance)
    solutions.add(solution)
    sys.stdin = og
    status = checker(instance=instance, solution=solution)
    print(status.is_feasible())
    print(status.get_message())
    print(status.get_objective_value())
    vis.visualize_solution(solution=solution, instance=instance) # opens plot if possible

subprocess.run("g++ -O2 -o solve solve.cpp",shell=True)
#instance_loc = "stars-0000050"
#instance_loc = "euro-night-0000020"
#instance_loc = "uniform-0000015-2"
instance_loc = "rop0000101"
#instance_loc = "us-night-0000060"
instance = idb[instance_loc]
runOn(instance)
