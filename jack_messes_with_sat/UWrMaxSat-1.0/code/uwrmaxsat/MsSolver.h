/**************************************************************************************[MsSolver.h ]
  Copyright (c) 2018-2019, Marek Piotrów

  Based on PbSolver.h ( Copyright (c) 2005-2010, Niklas Een, Niklas Sorensson)

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
  associated documentation files (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge, publish, distribute,
  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or
  substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
  OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  **************************************************************************************************/

#ifndef MsSolver_h
#define MsSolver_h

#include "PbSolver.h"

static inline int left (int i)  { return i * 2; }
static inline int right(int i)  { return i * 2 + 1; }
static inline int parent(int i) { return i / 2; }

class IntLitQueue {
  private:
    vec<Pair<Int, Lit> > heap;

    bool cmp(int x, int y) { 
        return heap[x].fst > heap[y].fst /*|| heap[x].fst == heap[y].fst && heap[x].snd > heap[y].snd*/; }

  public:
    IntLitQueue() { heap.push(Pair_new(1, lit_Undef)); }

    bool empty() { return heap.size() <= 1; }

    const Pair<Int, Lit>& top() { return heap[1]; }

    void push(Pair<Int, Lit> p) { 
        heap.push();
        int i = heap.size() - 1;
        heap[0] = std::move(p);
        while (parent(i) != 0 && cmp(0, parent(i))) { // percolate up
            heap[i] = std::move(heap[parent(i)]);
            i       = parent(i);
        }
        heap[i] = std::move(heap[0]);
    }

    void pop(void) {
        heap[1] = std::move(heap.last());
        heap.pop();
        if (heap.size() > 1) { // percolate down
            int i = 1;
            heap[0] = std::move(heap[1]);
            while (left(i) < heap.size()){
                int child = right(i) < heap.size() && cmp(right(i), left(i)) ? right(i) : left(i);
                if (!cmp(child, 0)) break;
                heap[i] = std::move(heap[child]);
                i       = child;
            }
            heap[i] = std::move(heap[0]);
        }
    }

} ;

class MsSolver : public PbSolver {
  public:
    MsSolver(bool use_preprocessing = false) : 
          PbSolver(use_preprocessing)
        , harden_goalval(0)
        , fixed_goalval(0)  {}

    Int                 harden_goalval,  //  Harden goalval used in the MaxSAT preprocessing 
                        fixed_goalval;   // The sum of weights of soft clauses that must be false
    vec<Pair<weight_t, Minisat::vec<Lit>* > > soft_cls; // Relaxed non-unit soft clauses with weights; a relaxing var is the last one in a vector. 
    int                 top_for_strat, top_for_hard; // Top indices to soft_cls for stratification and hardening operations.
    Map<Lit, Int>       harden_lits;    // The weights of literals included into "At most 1" clauses (MaxSAT preprocessing of soft clauese).
    vec<Pair<Lit,Int> > am1_rels;       // The weights of relaxing vars in "At most 1" clauses

    void    storeSoftClause(const vec<Lit>& ps, weight_t weight) {
                Minisat::vec<Lit> *ps_copy = new Minisat::vec<Lit>; 
                for (int i = 0; i < ps.size(); i++) ps_copy->push(ps[i]); 
                soft_cls.push(Pair_new(weight, ps_copy)); }

    void    harden_soft_cls(Minisat::vec<Lit>& assump_ps, vec<Int>& assump_Cs);
    int     optimize_last_constraint(vec<Linear*>& constrs);
    void    maxsat_solve(solve_Command cmd = sc_Minimize); 
    void    preprocess_soft_cls(Minisat::vec<Lit>& assump_ps, vec<Int>& assump_Cs, const Lit max_assump, const Int& max_assump_Cs, 
                                           IntLitQueue& delayed_assump, Int& delayed_assump_sum);
} ;

#endif
