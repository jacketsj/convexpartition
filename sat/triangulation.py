"""
This code computes the Delaunay triangulation for every instance (the most trivial
feasible solution) and creates a zip ready for upload.
"""

# Load an instance
import os
from cgshop2020_pyutils import InstanceDatabase, BestSolutionSet, SolutionZipWriter
from cgshop2020_pyutils import TrivialTriangulationSolver

# load challenge instances
idb = InstanceDatabase(os.path.join(os.path.dirname(__file__), ".././challenge_instances"))

# compute the triangulation for all instances
triangulation_solver = TrivialTriangulationSolver()
solutions = BestSolutionSet()
for instance in idb:
    solutions.add(triangulation_solver(instance))
    print(f"Computed triangulation for {instance.name}")

# write solutions into zip
print("Creating zip. This can take some time...")
with SolutionZipWriter("my_first_upload.zip") as zipper:
    zipper.add_solutions(solutions)

print("You can now upload 'my_first_upload.zip' on",
      "https://cgshop.ibr.cs.tu-bs.de/competition/cg-shop-2020/")
