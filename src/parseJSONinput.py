'''
This file generates the .in files from the JSON format.
'''

# Load an instance
import os
import io
import sys
# Don't output the CGAL info
sys.stdout = io.StringIO()
from cgshop2020_pyutils import InstanceDatabase, BestSolutionSet, SolutionZipWriter
sys.stdout = sys.__stdout__

# load challenge instances
idb = InstanceDatabase(os.path.join(os.path.dirname(__file__), ".././challenge_instances/data/second_instance_batch"))

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

# compute the triangulation for all instances
og = sys.stdout
for instance in idb:
    f = open(instance.name+'.in','w')
    sys.stdout = f
    printPoints(instance)
    sys.stdout = og
    print("Completed " + instance.name)
