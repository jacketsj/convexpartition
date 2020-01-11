"""
This code uses MonoSAT to solve this problem,
and creates a zip ready for upload (maybe?).
"""

# Load an instance
import os
from cgshop2020_pyutils import InstanceDatabase, BestSolutionSet, SolutionZipWriter
from cgshop2020_pyutils import SolutionChecker, Visualizer
from cgshop2020_pyutils import Solution, Instance, Edge

# Load MonoSAT
from monosat import *

# load challenge instances
idb = InstanceDatabase(os.path.join(os.path.dirname(__file__), ".././challenge_instances"))

# https://bryceboe.com/2006/10/23/line-segment-intersection-algorithm/
def ccw(Ax,Ay,Bx,By,Cx,Cy):
    return (Cy-Ay)*(Bx-Ax) > (By-Ay)*(Cx-Ax)
def intersect(Ax,Ay,Bx,By,Cx,Cy,Dx,Dy):
    return ccw(Ax,Ay,Cx,Cy,Dx,Dy) != ccw(Bx,By,Cx,Cy,Dx,Dy) and ccw(Ax,Ay,Bx,By,Cx,Cy) != ccw(Ax,Ay,Bx,By,Dx,Dy)

origin = []
# https://stackoverflow.com/questions/41855695/sorting-list-of-two-dimensional-coordinates-by-clockwise-angle-using-python
def clockwiseangle_and_distance(pointxy):
    node, x, y = pointxy
    point = [x, y]
    # Vector between point and the origin: v = p - o
    vector = [point[0]-origin[0], point[1]-origin[1]]
    # Length of vector: ||v||
    lenvector = math.hypot(vector[0], vector[1])
    # If length is zero there is no angle
    if lenvector == 0:
        return -math.pi, 0
    # Normalize vector: v/||v||
    normalized = [vector[0]/lenvector, vector[1]/lenvector]
    dotprod  = normalized[0]*refvec[0] + normalized[1]*refvec[1]     # x1*x2 + y1*y2
    diffprod = refvec[1]*normalized[0] - refvec[0]*normalized[1]     # x1*y2 - y1*x2
    angle = math.atan2(diffprod, dotprod)
    # Negative angles represent counter-clockwise angles so we need to subtract them 
    # from 2*pi (360 degrees)
    if angle < 0:
        return 2*math.pi+angle, lenvector
    # I return first the angle because that's the primary sorting criterium
    # but if two vectors have the same angle then the shorter distance should come first.
    return angle, lenvector

def useMonoSat(instance):
    print("Running MonoSAT")
    g = Graph()
    node_list = []
    print("Creating nodes")
    for point in instance:
        node_list.append((g.addNode(), point.get_x(), point.get_y(), []))
    print(f"Created ", len(node_list), " nodes")
    edge_list = []
    print("Creating edges")
    for (node1, x1, y1, adj1) in node_list:
        for (node2, x2, y2, adj2) in node_list:
            if node1 != node2:
                edge_list.append((g.addEdge(node1,node2),x1,y1,x2,y2))
                adj1.append((node2, x2, y2))
                adj2.append((node1, x1, y1))
    print(f"Created ", len(edge_list), " edges")

    print("Creating NANDs")
    # the following code could be drastically improved
    # with k+nlogn intersection iteration
    for (edgea,x1a,y1a,x2a,y2a) in edge_list:
        for (edgeb,x1b,y1b,x2b,y2b) in edge_list:
            if intersect(x1a,y1a,x2a,y2a,x1b,y1b,x2b,y2b):
                AssertNand(edgea,edgeb)
    print("Done creating NANDs")

    for (node, x, y, adj) in node_list:
        origin = [x,y]
        adj_angle = []
        for (node, x, y) in adj:
            adj_angle.append(clockwiseangle_and_distance(x,y),node)
        sorted(adj_angle)
        # If I come back to python, start here
        first = iter(adj_angle)
        for (angle, distance, node) in adj_angle
        #sorted(adj, key=clockwiseangle_and_distance)

    solution = Solution(instance=instance.name)
    solution.add_edge(Edge(0, 1))
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
solution = useMonoSat(instance)
checker = SolutionChecker()
status = checker(instance=instance, solution=solution)
print(status.is_feasible())
print(status.get_message())
print(status.get_objective_value())
vis = Visualizer()
vis.visualize_solution(solution=solution, instance=instance) # opens plot if possible
#vis.visualize_solution(solution=solution, instance=instance, path="fig_of_sat_on_" + instance_loc + ".pdf") # writes plot to file

# write solutions into zip
#print("Creating zip. This can take some time...")
#with SolutionZipWriter("my_first_upload.zip") as zipper:
    #zipper.add_solutions(solutions)

#print("You can now upload 'my_first_upload.zip' on",
      #"https://cgshop.ibr.cs.tu-bs.de/competition/cg-shop-2020/")
