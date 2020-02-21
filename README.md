# Convex Partition - Team UBC
This repository contains the source code of Team UBC in [CG:SHOP 2020,
the SoCG 2020 Competition](https://cgshop.ibr.cs.tu-bs.de/competition/cg-shop-2020/).

There are two different completed solvers:
- A simulated annealing-based local search solver.
- A SAT formulation.

In addition, there is also an imcomplete symbolic edge constraint solving formulation.

# Required libraries:
- MonoSAT: https://www.cs.ubc.ca/labs/isd/Projects/monosat/
- cgshop2020-pyutils: `pip install cgshop2020-pyutils` (https://pypi.org/project/cgshop2020-pyutils/)

# Instance format

CG:SHOP 2020 used a [JSON-based instance format](https://cgshop.ibr.cs.tu-bs.de/competition/cg-shop-2020/instance-format).

To simplify the format, and allow easier parsing using standard IO,
all the instances, and their solutions, have been mapped to the following format
using the tool
TODO (formatout.py).

## Input format
Point sets will be stored in the following format (everything is 0 indexed):

**n** - The number of points

For the next **n** lines:
**i x<sub>i</sub> y<sub>i</sub>** - index, x-coordinate, y-coordinate

## Output format
Planar graphs will be stored in the following format (everything is 0 indexed):

**n** - The number of points

For the next **n** lines:
**i x<sub>i</sub> y<sub>i</sub>** - index, x-coordinate, y-coordinate

For the next **n** lines:
**k<sub>i</sub> a<sub>1</sub>, a<sub>2</sub>, ... a<sub>k<sub>i</sub></sub>** - number of neighbors for point i, then the index of its neighbours


# Directory Organization

TODO update

- in/ contains raw point sets of all the instances. 
- triangulations/ contains the triangulated point sets of all instances.
- *.txt files contain base names of instances.
- src/ contains local search, triangulization code, graph and point struct.
- sat/ contains Jack's SAT stuff as well as python scripts for printing polyganizations and packaging solutions. (should be moved to src)
