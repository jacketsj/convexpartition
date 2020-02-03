# Convex Partition
CG:SHOP 2020 (SoCG Competition)
https://cgshop.ibr.cs.tu-bs.de/competition/cg-shop-2020/

# Required libraries:
- CGAL: https://doc.cgal.org/latest/Manual/usage.html (libcgal-dev)
- MonoSAT: https://www.cs.ubc.ca/labs/isd/Projects/monosat/
- cmake
- zlib (zlib1g-dev)
- gmp (libgmp3-dev)
- SWIG (swig)
- cgshop2020-pyutils: `pip install cgshop2020-pyutils` (https://pypi.org/project/cgshop2020-pyutils/)

# Instance format

Planar graphs will be stored in the following format (everything is 0 indexed):

**n** - The number of points

For the next **n** lines:
**i x<sub>i</sub> y<sub>i</sub>** - index, x-coordinate, y-coordinate

For the next **n** lines:
**k<sub>i</sub> a<sub>1</sub>, a<sub>2</sub>, ... a<sub>k<sub>i</sub></sub>** - number of neighbors for point i, then the index of its neighbours 

# Directory Organization

- in/ contains raw point sets of all the instances. 
- triangulations/ contains the triangulated point sets of all instances.
- *.txt files contain base names of instances.
- src/ contains local search, triangulization code, graph and point struct.
- sat/ contains Jack's SAT stuff as well as python scripts for printing polyganizations and packaging solutions. (should be moved to src)


