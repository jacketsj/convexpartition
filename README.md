# Convex Partition
CG:SHOP 2020 (SoCG Competition)
https://cgshop.ibr.cs.tu-bs.de/competition/cg-shop-2020/

# Required libraries:
- CGAL: https://doc.cgal.org/latest/Manual/usage.html
- MonoSAT: https://www.cs.ubc.ca/labs/isd/Projects/monosat/
- cmake
- cgshop2020-pyutils: `pip install cgshop2020-pyutils` (https://pypi.org/project/cgshop2020-pyutils/)

# Instance format

Planar graphs will be stored in the following format (everything is 0 indexed):

**n** - The number of points

For the next **n** lines:
**i x\_i y\_i** - index, x-coordinate, y-coordinate

For the next **n** lines:
**k\_i a\_1, a\_2, ... a\_** - number of neighbors for point i, then the index of its neighbours 
