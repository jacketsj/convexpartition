(python3 formatout.py) > uniform.0000100.2.pts
g++ -std=c++11 -o solve solve.cpp && (cat uniform.0000100.2.pts | ./solve) > uniform.0000100.2.cnf
monosat uniform.0000100.2.cnf -witness-file=uniform.0000100.2.out
g++ -g -std=c++11 -o read_sln read_sln.cpp && ./read_sln | python3 formatin.py
