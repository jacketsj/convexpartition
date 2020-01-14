/**************************************************************************************[MsSolver.cc]
  Copyright (c) 2018-2019, Marek Piotrów

  Based on PbSolver.cc ( Copyright (c) 2005-2010, Niklas Een, Niklas Sorensson)

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

#include <unistd.h>
#include <signal.h>
#include "minisat/utils/System.h"
#include "Sort.h"
#include "MsSolver.h"
#include "Debug.h"
#include <limits>

template<typename int_type>
static int_type gcd(int_type small, int_type big) {
    if (small < 0) small = -small;
    if (big < 0) big = -big;
    return (small == 0) ? big: gcd(big % small, small); }

template<typename T>
static int bin_search(const Minisat::vec<T>& seq, const T& elem)
{
    int fst = 0, cnt = seq.size();
    while (cnt > 0) {
        int step = cnt / 2, mid = fst + step;
        if (seq[mid] < elem) fst = mid + 1, cnt -= step + 1; 
        else cnt = step;
    }
    return (fst < seq.size() && seq[fst] == elem ? fst : -1);
}
        
template<typename T>
static int bin_search(const vec<T>& seq, const T& elem)
{
    int fst = 0, cnt = seq.size();
    while (cnt > 0) {
        int step = cnt / 2, mid = fst + step;
        if (seq[mid] < elem) fst = mid + 1, cnt -= step + 1; 
        else cnt = step;
    }
    return (fst < seq.size() && seq[fst] == elem ? fst : -1);
}
        
static MsSolver *pb_solver;
static
void SIGINT_interrupt(int /*signum*/) { pb_solver->sat_solver.interrupt(); pb_solver->asynch_interrupt=true; }

extern int verbosity;

static void clear_assumptions(Minisat::vec<Lit>& assump_ps, vec<Int>& assump_Cs)
{
    int removed, j = 0;
    for (int i = 0; i < assump_ps.size(); i++) {
        if (assump_Cs[i] < 0) continue;
        if (j < i) assump_ps[j] = assump_ps[i], assump_Cs[j] = assump_Cs[i];
        j++;
    }
    if ((removed = assump_ps.size() - j) > 0)
        assump_ps.shrink(removed), assump_Cs.shrink(removed);
}

static
bool satisfied_soft_cls(Minisat::vec<Lit> *cls, vec<bool>& model)
{
    assert(cls != NULL);
    for (int i = cls->size() - 2; i >= 0; i--)
        if ((( sign((*cls)[i]) && !model[var((*cls)[i])]) 
          || (!sign((*cls)[i]) &&  model[var((*cls)[i])])))
            return true;
    return false;
}


static
Int evalGoal(const vec<Pair<weight_t, Minisat::vec<Lit>* > >& soft_cls, vec<bool>& model)
{
    Int sum = 0;
    bool sat = false;
    for (int i = 0; i < soft_cls.size(); i++) {
        Lit p = soft_cls[i].snd->last(); if (soft_cls[i].snd->size() == 1) p = ~p;
        assert(var(p) < model.size());
        if ((( sign(p) && !model[var(p)]) || (!sign(p) &&  model[var(p)])) 
            && !(sat = satisfied_soft_cls(soft_cls[i].snd, model)))
            sum += soft_cls[i].fst;
        if (sat) { sat = false; model[var(p)] = !model[var(p)]; }
    }
    return sum;
}

static
void core_minimization(SimpSolver &sat_solver, Minisat::vec<Lit> &mus)
{
    int last_size = sat_solver.conflict.size();

    sat_solver.setConfBudget(1000);
    int verb = sat_solver.verbosity; sat_solver.verbosity = 0;
    for (int i = 0; last_size > 1 && i < last_size; ) {
        Lit p = mus[i];
        for (int j = i+1; j < last_size; j++) mus[j-1] = mus[j];
        mus.pop();
        if (sat_solver.solveLimited(mus) != l_False) {
            mus.push();
            for (int j = last_size - 1; j > i; j--) mus[j] = mus[j-1];
            mus[i] = p; i++;
        } else last_size--;
    }
    sat_solver.budgetOff(); sat_solver.verbosity = verb;

    for (int i = mus.size() - 1; i >= 0; i--) mus[i] = ~mus[i];
}

/*static void core_trimming(SimpSolver &sat_solver, int max_size, int n)
{
    int last_size = sat_solver.conflict.size();
    Minisat::vec<Lit> assump(last_size);
    for (int i = n; i > 0 && last_size > max_size; i--) {
        assump.clear();
        for (int j = 0; j < last_size; j++) assump.push(~sat_solver.conflict[j]);
        sat_solver.solve(assump);
        if (sat_solver.conflict.size() >= last_size) return;
        last_size = sat_solver.conflict.size();
    }
}*/

static Int next_sum(Int bound, const vec<Int>& cs)
{ // find the smallest sum of a subset of cs that is greater that bound
    vec<Int> sum[2];
    Int x, next_min = Int_MAX;
    int oldv =0, newv = 1, lst = 0;

    sum[oldv].push(0); ++bound;
    for (int sz = 1, j = 0; j < cs.size(); j++, oldv = newv, newv = 1-oldv, lst = 0) {
        for (int i = 0; i < sz; i++)
            if ((x = sum[oldv][i] + cs[j]) < bound) {
                while (lst < sz && sum[oldv][lst] > x) sum[newv].push(sum[oldv][lst++]);
                if (lst == sz || sum[oldv][lst] < x) sum[newv].push(x);
            } else if (x < next_min) {
                if (x == bound) return x;
                next_min = x;
            }
        while (lst < sz) sum[newv].push(sum[oldv][lst++]);
        sz = sum[newv].size(); sum[oldv].clear();
    }
    return (next_min == Int_MAX ? bound - 1 : next_min);

}

static
Int evalPsCs(vec<Lit>& ps, vec<Int>&Cs, vec<bool>& model)
{
    Int sum = 0;
    assert(ps.size() == Cs.size());
    for (int i = 0; i < ps.size(); i++){
        if (( var(ps[i]) >= model.size())
        ||  ( sign(ps[i]) && model[var(ps[i])] == false)
        ||  (!sign(ps[i]) && model[var(ps[i])] == true )
        )
            sum += Cs[i];
    }
    return sum;
}

/*static
Int evalPsCs(vec<Lit>& ps, vec<Int>&Cs, Minisat::vec<lbool>& model)
{
    Int sum = 0;
    assert(ps.size() == Cs.size());
    for (int i = 0; i < ps.size(); i++){
        if (( sign(ps[i]) && model[var(ps[i])] == l_False)
        ||  (!sign(ps[i]) && model[var(ps[i])] == l_True )
        )
            sum += Cs[i];
    }
    return sum;
}*/

static void opt_stratification(vec<weight_t>& sorted_assump_Cs, vec<Pair<Int, bool> >& sum_sorted_soft_cls)
{
    assert(sorted_assump_Cs.size() == sum_sorted_soft_cls.size());

    int m = max(1, sum_sorted_soft_cls.size() - 10);
    if (m < 10) m = 1;
    for (int i = sum_sorted_soft_cls.size() - 1; i >= m; i--)
        if (sorted_assump_Cs[i] > sorted_assump_Cs[i-1] + 1 || 
                i < sum_sorted_soft_cls.size() - 1 && !sum_sorted_soft_cls[i + 1].snd) 
            sum_sorted_soft_cls[i].snd = true;
    if (m == 1) return;
    vec<Pair<weight_t, int> > gaps;
    for (int i = 0; i < m; i++) gaps.push(Pair_new(sorted_assump_Cs[i+1] - sorted_assump_Cs[i], i + 1));
    sort(gaps);
    for (int i = gaps.size() - 1, j = 0; j < 10; j++, i--) sum_sorted_soft_cls[gaps[i].snd].snd = true;
}

template <class T> struct LT {bool operator()(T x, T y) { return x.snd->last() < y.snd->last(); }};

static weight_t do_stratification(SimpSolver& S, vec<weight_t>& sorted_assump_Cs, vec<Pair<weight_t, Minisat::vec<Lit>* > >& soft_cls, int& top_for_strat,
                                            Minisat::vec<Lit>& assump_ps, vec<Int>& assump_Cs)
{
    weight_t  max_assump_Cs;
    //do { max_assump_Cs = sorted_assump_Cs.last(); sorted_assump_Cs.pop(); 
    //} while (sorted_assump_Cs.size() > 0 && !sum_sorted_soft_cls[sorted_assump_Cs.size()].snd);
    max_assump_Cs = sorted_assump_Cs.last(); sorted_assump_Cs.pop();
    if (sorted_assump_Cs.size() > 0 && sorted_assump_Cs.last() == max_assump_Cs - 1) 
        max_assump_Cs = sorted_assump_Cs.last(), sorted_assump_Cs.pop(); 
    int start = top_for_strat - 1;
    while (start >= 0 && soft_cls[start].fst >= max_assump_Cs) start--;
    start++;
    if (start < top_for_strat) {
        int sz = top_for_strat - start, to = 0, fr = sz;
        sort(&soft_cls[start], sz, LT<Pair<weight_t, Minisat::vec<Lit>*> >());
        assump_ps.growTo(assump_ps.size() + sz); assump_Cs.growTo(assump_Cs.size() + sz);
        for (int i = assump_ps.size() - 1; i >= sz; i--)
            assump_ps[i] = assump_ps[i-sz], assump_Cs[i] = assump_Cs[i-sz];
        for (int i = start; i < top_for_strat; i++) {
            Lit p = ~soft_cls[i].snd->last();
            if (soft_cls[i].snd->size() > 1) S.addClause(*soft_cls[i].snd); else p = ~p;
            while (fr < assump_ps.size() && assump_ps[fr] <= p)
                assump_ps[to] = assump_ps[fr], assump_Cs[to++] = assump_Cs[fr++];
            assump_ps[to] = p; assump_Cs[to++] = soft_cls[i].fst;
        }
        sort(&soft_cls[start], sz);
        top_for_strat = start;
    }
    return max_assump_Cs;
}

void MsSolver::harden_soft_cls(Minisat::vec<Lit>& assump_ps, vec<Int>& assump_Cs)
{
    int cnt_unit = 0, cnt_assump = 0, sz = 0;
    Int Ibound = UB_goalvalue - LB_goalvalue, WMAX = Int(WEIGHT_MAX);
    weight_t       wbound = (Ibound >= WMAX ? WEIGHT_MAX : tolong(Ibound));
    weight_t ub_goalvalue = (UB_goalvalue >= WMAX ? WEIGHT_MAX : tolong(UB_goalvalue - fixed_goalval));
    for (int i = top_for_hard - 1; i >= 0 && soft_cls[i].fst > wbound; i--) { // hardening soft clauses with weights > the current goal interval length 
        if (soft_cls[i].fst > ub_goalvalue) sz++;
        Lit p = soft_cls[i].snd->last(); if (soft_cls[i].snd->size() > 1) p = ~p;
        int j = bin_search(assump_ps, p);
        if (j >= 0 && assump_Cs[j] > Ibound) {
            if (opt_minimization == 1) harden_lits.set(p, Int(soft_cls[i].fst));
            assump_Cs[j] = -assump_Cs[j]; // mark a corresponding assumption to be deleted
            cnt_assump++; cnt_unit++; sat_solver.addClause(p);
        } else if (soft_cls[i].fst > ub_goalvalue) { 
            if (opt_minimization == 1) harden_lits.set(p, Int(soft_cls[i].fst));
            cnt_unit++, sat_solver.addClause(p);
        }
    }
    if (opt_verbosity >= 2 && cnt_unit > 0) reportf("Hardened %d soft clauses\n", cnt_unit);
    if (sz > 0 ) top_for_hard -= sz;
    if (cnt_assump > 0) clear_assumptions(assump_ps, assump_Cs);
}

int MsSolver::optimize_last_constraint(vec<Linear*>& constrs)
{
    if (constrs.size() == 0) return 0;
    Minisat::vec<Lit> assump;
    assump.push(constrs.last()->lit);
    sat_solver.setConfBudget(1000);

    int verb = sat_solver.verbosity; sat_solver.verbosity = 0;
    int diff = 0;
    if (sat_solver.solveLimited(assump) == l_False) { 
        if (constrs.size() > 1) {
            constrs[0]->~Linear(); constrs[0] = constrs.last();
            for (int i = 1; i < constrs.size(); i++) constrs[i]->~Linear();
            constrs.shrink(constrs.size() - 1);
        }
        diff++;
        while (constrs[0]->lo > 1 || constrs[0]->hi < constrs[0]->size - 1) {
            Lit newp = mkLit(sat_solver.newVar(VAR_UPOL, !opt_branch_pbvars), true);
            sat_solver.setFrozen(var(newp),true);
            if (constrs[0]->lo > 1) --constrs[0]->lo; else ++constrs[0]->hi;
            constrs[0]->lit = newp;
            convertPbs(false);
            sat_solver.addClause(~assump[0], newp);
            assump[0] = newp;
            if (sat_solver.solveLimited(assump) != l_False) break;
            diff++;
        }
    }
    sat_solver.budgetOff(); sat_solver.verbosity = verb;
    if (diff > 0) reportf("AM1 constraint optimized to <= %d\n", diff+1);
    return diff;
}

static inline int log2(int n) { int i=0; while (n>>=1) i++; return i; }

void MsSolver::maxsat_solve(solve_Command cmd)
{
    if (!okay()) {
        if (opt_verbosity >= 1) sat_solver.printVarsCls();
        return;
    }
#if defined(GLUCOSE3) || defined(GLUCOSE4)    
    if (opt_verbosity >= 1) sat_solver.verbEveryConflicts = 100000;
    sat_solver.setIncrementalMode();
#endif
    if (soft_cls.size() == 0) { opt_maxsat_msu = false; solve(cmd); return; }
    // Convert constraints:
    pb_n_vars = nVars();
    pb_n_constrs = nClauses();
    if (constrs.size() > 0) {
        if (opt_verbosity >= 1)
            reportf("Converting %d PB-constraints to clauses...\n", constrs.size());
        propagate();
        if (!convertPbs(true)){
            if (opt_verbosity >= 1) sat_solver.printVarsCls(constrs.size() > 0);
            assert(!okay()); return;
        }
    }

    // Freeze goal function variables (for SatELite):
    for (int i = 0; i < soft_cls.size(); i++)
        sat_solver.setFrozen(var(soft_cls[i].snd->last()), true);
    sat_solver.verbosity = opt_verbosity - 1;

    weight_t goal_gcd = soft_cls[0].fst;
    for (int i = 1; i < soft_cls.size() && goal_gcd != 1; ++i) goal_gcd = gcd(goal_gcd, soft_cls[i].fst);
    if (goal_gcd != 1) {
        if (LB_goalvalue != Int_MIN) LB_goalvalue /= Int(goal_gcd);
        if (UB_goalvalue != Int_MAX) UB_goalvalue /= Int(goal_gcd);
    }

    assert(best_goalvalue == Int_MAX);

    if (opt_convert_goal != ct_Undef)
        opt_convert = opt_convert_goal;
    opt_sort_thres *= opt_goal_bias;
    opt_shared_fmls = true;

    if (opt_cnf != NULL)
        reportf("Exporting CNF to: \b%s\b\n", opt_cnf),
        sat_solver.toDimacs(opt_cnf),
        exit(0);

    pb_solver = this;
    signal(SIGINT, SIGINT_interrupt);
    signal(SIGXCPU,SIGINT_interrupt);

    Map<int,int> assump_map(-1);
    vec<Linear*> saved_constrs;
    vec<Lit> goal_ps;
    Minisat::vec<Lit> assump_ps;
    vec<Int> assump_Cs, goal_Cs, saved_constrs_Cs;
    vec<weight_t> sorted_assump_Cs;
    vec<Pair<Int, bool> > sum_sorted_soft_cls;
    bool    sat = false, weighted_instance = true;
    Lit inequality = lit_Undef, max_assump = lit_Undef;
    Int     try_lessthan = opt_goal, max_assump_Cs = Int_MIN;
    int     n_solutions = 0;    // (only for AllSolutions mode)
    vec<Pair<Lit,int> > psCs;
    vec<bool> multi_level_opt;
    bool opt_delay_init_constraints = false, 
         opt_core_minimization = (nClauses() > 0 || soft_cls.size() < 100000);
    IntLitQueue delayed_assump;
    Int delayed_assump_sum = 0;

    Int LB_goalval = 0, UB_goalval = 0;    
    sort(&soft_cls[0], soft_cls.size(), LT<Pair<weight_t, Minisat::vec<Lit>*> >());
    int j = 0; Lit pj;
    for (int i = 0; i < soft_cls.size(); ++i) {
        soft_cls[i].fst /= goal_gcd;
        if (soft_cls[i].fst < 0) { 
            fixed_goalval += soft_cls[i].fst; soft_cls[i].fst = -soft_cls[i].fst; soft_cls[i].snd->last() = ~soft_cls[i].snd->last(); 
        }
        Lit p = soft_cls[i].snd->last();
        if (soft_cls[i].snd->size() == 1) p = ~p;
        if (value(p) != l_Undef) {
            if (value(p) == l_True) {
                fixed_goalval += soft_cls[i].fst;
                addUnit(p);
            } else {
                if (soft_cls[i].snd->size() > 1) sat_solver.addClause(*soft_cls[i].snd);
                addUnit(~p);
            }
        } else if (j > 0 && p == pj)  
            soft_cls[j-1].fst += soft_cls[i].fst;
        else if (j > 0 && p == ~pj) {
            fixed_goalval += (soft_cls[j-1].fst < soft_cls[i].fst ? soft_cls[j-1].fst : soft_cls[i].fst); 
            soft_cls[j-1].fst -= soft_cls[i].fst;
            if (soft_cls[j-1].fst < 0) soft_cls[j-1].fst = -soft_cls[j-1].fst, soft_cls[j-1].snd->last() = pj, pj = ~pj; 
        } else {
            if (j > 0 && soft_cls[j-1].fst == 0) j--;
            if (j < i) soft_cls[j] = soft_cls[i];
            pj = p; j++;
        }
    }
    if (j < soft_cls.size()) soft_cls.shrink(soft_cls.size() - j);
    top_for_strat = top_for_hard = soft_cls.size();
    sort(soft_cls);
    weighted_instance = (soft_cls[0].fst != soft_cls.last().fst);
    for (int i = 0; i < soft_cls.size(); i++) {
        Lit p = soft_cls[i].snd->last();
        psCs.push(Pair_new(soft_cls[i].snd->size() == 1 ? p : ~p, i));
        if (weighted_instance) sorted_assump_Cs.push(soft_cls[i].fst);
        UB_goalval += soft_cls[i].fst;
    }
    LB_goalval += fixed_goalval, UB_goalval += fixed_goalval;
    sort(psCs);
    if (weighted_instance) sortUnique(sorted_assump_Cs);
    if (LB_goalvalue < LB_goalval) LB_goalvalue = LB_goalval;
    if (UB_goalvalue == Int_MAX)   UB_goalvalue = UB_goalval;
    else {
        for (int i = 0; i < psCs.size(); i++)
            goal_ps.push(~psCs[i].fst), goal_Cs.push(soft_cls[psCs[i].snd].fst);
        try_lessthan = ++UB_goalvalue;
        if (goal_ps.size() > 0) {
            addConstr(goal_ps, goal_Cs, try_lessthan - fixed_goalval, -2, inequality);
            convertPbs(false);
        }
    }
    if (opt_minimization != 1 || sorted_assump_Cs.size() == 0) {
        for (int i = 0; i < psCs.size(); i++)
            assump_ps.push(psCs[i].fst), assump_Cs.push(Int(soft_cls[psCs[i].snd].fst));
        for (int i = 0; i < soft_cls.size(); i++) { 
            if (soft_cls[i].snd->size() > 1) sat_solver.addClause(*soft_cls[i].snd);
        }
        top_for_strat = top_for_hard = 0;
    } else {
        for (int i = soft_cls.size() - 1; i >= 0; i--) 
            for (int j = soft_cls[i].snd->size() - 1; j >= 0; j--) 
                sat_solver.setFrozen(var((*soft_cls[i].snd)[j]), true);
        Int sum = 0;
        int ml_opt = 0;
        multi_level_opt.push(false); sum_sorted_soft_cls.push(Pair_new(0, true));
        for (int i = 0, cnt = 0, sz = sorted_assump_Cs.size(), j = 1; j < sz; j++) {
            while (i < soft_cls.size() && soft_cls[i].fst < sorted_assump_Cs[j])
                sum += soft_cls[i++].fst, cnt++;
            sum_sorted_soft_cls.push(Pair_new(sum, sum < sorted_assump_Cs[j]));
            multi_level_opt.push(sum < sorted_assump_Cs[j]);
            if (multi_level_opt.last()) ml_opt++;
        }
        opt_stratification(sorted_assump_Cs, sum_sorted_soft_cls);
        //if (ml_opt >= 2 || !opt_to_bin_search && ml_opt >= 1) 
            opt_lexicographic = true;
        if (opt_verbosity >= 1 && ml_opt > 0) 
            reportf("Boolean lexicographic optimization can be done in %d point(s).%s\n", 
                    ml_opt, (opt_lexicographic ? "" : " Try -lex-opt option."));
        max_assump_Cs = do_stratification(sat_solver, sorted_assump_Cs, soft_cls, top_for_strat, assump_ps, assump_Cs);
    }
    if (psCs.size() > 0) max_assump = psCs.last().fst;
    if (opt_minimization == 1 && opt_maxsat_prepr) 
        preprocess_soft_cls(assump_ps, assump_Cs, max_assump, max_assump_Cs, delayed_assump, delayed_assump_sum);
    if (opt_verbosity >= 1)
        sat_solver.printVarsCls(goal_ps.size() > 0, &soft_cls, top_for_strat);

    if (opt_polarity_sug != 0)
        for (int i = 0; i < soft_cls.size(); i++){
            Lit p = soft_cls[i].snd->last(); if (soft_cls[i].snd->size() == 1) p = ~p;
            bool dir = opt_polarity_sug > 0 ? !sign(p) : sign(p);
            sat_solver.setPolarity(var(p), LBOOL(dir));
        }

    while (1) {
      if (asynch_interrupt) { reportf("Interrupted ***\n"); break; }
      if (sat_solver.solve(assump_ps)) { // SAT returned
        if (opt_minimization == 1 && opt_delay_init_constraints) {
            opt_delay_init_constraints = false;
            convertPbs(false);
            constrs.clear();
            continue;
        }
        Int lastCs = 1;
        if(opt_minimization != 1 && assump_ps.size() == 1 && assump_ps.last() == inequality) {
          addUnit(inequality);
          lastCs = assump_Cs.last();
          assump_ps.pop(); assump_Cs.pop(); inequality = lit_Undef;
        }
        sat = true;

        if (cmd == sc_AllSolutions){
            Minisat::vec<Lit>    ban;
            n_solutions++;
            reportf("MODEL# %d:", n_solutions);
            for (Var x = 0; x < pb_n_vars; x++){
                assert(sat_solver.model[x] != l_Undef);
                ban.push(mkLit(x, sat_solver.model[x] == l_True));
                if (index2name[x][0] != '#')
                    reportf(" %s%s", (sat_solver.model[x] == l_False)?"-":"", index2name[x]);
            }
            reportf("\n");
            sat_solver.addClause_(ban);
        }else{
            vec<bool> model;
            for (Var x = 0; x < pb_n_vars; x++)
                assert(sat_solver.model[x] != l_Undef),
                model.push(sat_solver.model[x] == l_True);
            for (int i = 0; i < top_for_strat; i++)
                if (soft_cls[i].snd->size() > 1)
                    model[var(soft_cls[i].snd->last())] = !sign(soft_cls[i].snd->last());
            Int goalvalue = evalGoal(soft_cls, model) + fixed_goalval;
            if (goalvalue < best_goalvalue) {
                best_goalvalue = goalvalue;
                model.moveTo(best_model);
                char* tmp = toString(best_goalvalue * goal_gcd);
                if (opt_satlive || opt_verbosity == 0)
                    printf("o %s\n", tmp), fflush(stdout);
                else reportf("\bFound solution: %s\b\n", tmp);
                xfree(tmp);
            } else model.clear(); 
            if (best_goalvalue < UB_goalvalue) UB_goalvalue = best_goalvalue;

            if (cmd == sc_FirstSolution || (opt_minimization == 1 || UB_goalvalue == LB_goalvalue) && sorted_assump_Cs.size() == 0 && delayed_assump.empty()) break;
            if (opt_minimization == 1) {
                assert(sorted_assump_Cs.size() > 0 || !delayed_assump.empty()); 
                int old_top = top_for_strat;
                if (delayed_assump.empty() || sorted_assump_Cs.size() > 0 && Int(sorted_assump_Cs.last()) > delayed_assump.top().fst) {
                    if (opt_lexicographic && multi_level_opt[sorted_assump_Cs.size()]) {
                        Int bound = sum_sorted_soft_cls[sorted_assump_Cs.size()].fst + delayed_assump_sum;
                        int cnt_assump = 0;
                        for (int i = 0; i < assump_ps.size() && assump_ps[i] <= max_assump; i++)
                            if (assump_Cs[i] > bound)
                                addUnit(assump_ps[i]), assump_Cs[i] = -assump_Cs[i], cnt_assump++;
                        if (cnt_assump > 0) {
                            clear_assumptions(assump_ps, assump_Cs);
                            if (opt_verbosity > 0) reportf("Boolean lexicographic optimization - done.\n");
                        }
                    }
                    max_assump_Cs = do_stratification(sat_solver, sorted_assump_Cs, soft_cls, top_for_strat, assump_ps, assump_Cs);
                } else {
                    max_assump_Cs = delayed_assump.top().fst;
                    vec<Pair<Lit, Int> > new_assump;
                    do { 
                        new_assump.push(Pair_new(delayed_assump.top().snd, max_assump_Cs));
                        delayed_assump_sum -= delayed_assump.top().fst;
                        delayed_assump.pop(); 
                    } while (!delayed_assump.empty() && delayed_assump.top().fst >= max_assump_Cs);
                    sort(new_assump); int sz = new_assump.size();
                    assump_ps.growTo(assump_ps.size() + sz); assump_Cs.growTo(assump_Cs.size() + sz);
                    for (int i = assump_ps.size() - 1; i >= sz; i--)
                        assump_ps[i] = assump_ps[i-sz], assump_Cs[i] = assump_Cs[i-sz];
                    for (int fr = sz, to = 0, i = 0; i < new_assump.size(); i++) {
                        Lit p = new_assump[i].fst;
                        while (fr < assump_ps.size() && assump_ps[fr] <= p)
                            assump_ps[to] = assump_ps[fr], assump_Cs[to++] = assump_Cs[fr++];
                        assump_ps[to] = p; assump_Cs[to++] = new_assump[i].snd;
                    }
                }
                harden_soft_cls(assump_ps, assump_Cs);
                if (top_for_strat < old_top) {
                    try_lessthan = best_goalvalue;
                    if (opt_maxsat_prepr) 
                        preprocess_soft_cls(assump_ps, assump_Cs, max_assump, max_assump_Cs, delayed_assump, delayed_assump_sum);
                }
                continue;
            } else harden_soft_cls(assump_ps, assump_Cs);
            if (opt_minimization == 0 || best_goalvalue - LB_goalvalue < opt_seq_thres) {
                inequality = (assump_ps.size() == 0 ? lit_Undef : mkLit(sat_solver.newVar(VAR_UPOL, !opt_branch_pbvars), true));
                try_lessthan = best_goalvalue;
            } else {
                inequality = mkLit(sat_solver.newVar(VAR_UPOL, !opt_branch_pbvars), true);
                try_lessthan = (LB_goalvalue*(100-opt_bin_percent) + best_goalvalue*(opt_bin_percent))/100;
            }
            Int goal_diff = harden_goalval+fixed_goalval;
            if (!addConstr(goal_ps, goal_Cs, try_lessthan - goal_diff, -2, inequality))
                break; // unsat
            if (inequality != lit_Undef) {
                sat_solver.setFrozen(var(inequality),true);
                assump_ps.push(inequality), assump_Cs.push(opt_minimization == 2 ? try_lessthan : lastCs);
            }
            convertPbs(false);
        }
      } else { // UNSAT returned
        if (assump_ps.size() == 0) break;

        Minisat::vec<Lit> core_mus;
        if (opt_core_minimization) {
            if (weighted_instance) {
                vec<Pair<Pair<Int, int>, Lit> > Cs_mus;
                for (int i = 0; i < sat_solver.conflict.size(); i++) {
                    Lit p = ~sat_solver.conflict[i];
                    int j = bin_search(assump_ps, p);
                    Cs_mus.push(Pair_new(Pair_new((j>=0 ? assump_Cs[j] : 0),i),p));
                }
                sort(Cs_mus);
                for (int i = 0; i < Cs_mus.size(); i++) core_mus.push(Cs_mus[i].snd);
            } else
                for (int i = 0; i < sat_solver.conflict.size(); i++) core_mus.push(~sat_solver.conflict[i]);
            core_minimization(sat_solver, core_mus);
        } else
            for (int i = 0; i < sat_solver.conflict.size(); i++) core_mus.push(sat_solver.conflict[i]);
        if (core_mus.size() > 0 && core_mus.size() < 6) sat_solver.addClause(core_mus);
        Int min_removed = Int_MAX, min_bound = Int_MAX;
        int removed = 0;
        bool other_conflict = false;

        if (opt_minimization == 1) { 
            goal_ps.clear(); goal_Cs.clear();
        }
        for (int j, i = 0; i < core_mus.size(); i++) {
            Lit p = ~core_mus[i];
            if ((j = bin_search(assump_ps, p)) >= 0) { 
                if (opt_minimization == 1 || p <= max_assump) {
                    goal_ps.push(~p), goal_Cs.push(opt_minimization == 1 ? 1 : assump_Cs[j]);
                    if (assump_Cs[j] < min_removed) min_removed = assump_Cs[j];
                } else { 
                    other_conflict = true;
                    if (assump_Cs[j] < min_bound) min_bound = assump_Cs[j];
                }
                assump_Cs[j] = -assump_Cs[j]; removed++;
            }
        }
        if (other_conflict && min_removed != Int_MAX && opt_minimization != 1) min_removed = 0;
        if (removed > 0) {
            int j = 0;
            for (int i = 0; i < assump_ps.size(); i++) {
                if (assump_Cs[i] < 0) {
                    Minisat::Lit p = assump_ps[i];
                    if (opt_minimization == 1 && p > max_assump) { // && assump_Cs[i] == -min_removed) {
                        int k = assump_map.at(toInt(p));
                        if (k >= 0 && k < saved_constrs.size() &&  saved_constrs[k] != NULL && saved_constrs[k]->lit == p) {
                            if (saved_constrs[k]->lo != Int_MIN && saved_constrs[k]->lo > 1 || 
                                    saved_constrs[k]->hi != Int_MAX && saved_constrs[k]->hi < saved_constrs[k]->size - 1) {
                                Lit newp = mkLit(sat_solver.newVar(VAR_UPOL, !opt_branch_pbvars), true);
                                sat_solver.setFrozen(var(newp),true);
                                if (saved_constrs[k]->lo != Int_MIN) --saved_constrs[k]->lo; else ++saved_constrs[k]->hi;
                                saved_constrs[k]->lit = newp;
                                assump_ps.push(newp); assump_Cs.push(saved_constrs_Cs[k]);
                                constrs.push(saved_constrs[k]);
                                sat_solver.addClause(~p, newp);
                                if (saved_constrs[k]->lo > 1 || saved_constrs[k]->hi < saved_constrs[k]->size - 1) assump_map.set(toInt(newp), k);
                            } else { saved_constrs[k]->~Linear(); saved_constrs[k] = NULL; }
                            assump_map.set(toInt(p), -1);
                        }
                    }
                    if (assump_Cs[i] == -min_removed || opt_minimization != 1) continue;
                    assump_Cs[i] = -min_removed - assump_Cs[i];
                    if (opt_minimization == 1 &&  assump_Cs[i] < max_assump_Cs ) {
                        delayed_assump.push(Pair_new(assump_Cs[i], assump_ps[i]));
                        delayed_assump_sum += assump_Cs[i];
                        continue;
                    }
                }
                if (j < i) assump_ps[j] = assump_ps[i], assump_Cs[j] = assump_Cs[i];
                j++;
            }
            if ((removed = assump_ps.size() - j) > 0)
                assump_ps.shrink(removed), assump_Cs.shrink(removed);
            if (min_bound == Int_MAX || min_bound < LB_goalvalue) min_bound = LB_goalvalue + 1;
            LB_goalvalue = (min_removed == 0 ? next_sum(LB_goalvalue - fixed_goalval - harden_goalval, goal_Cs) + fixed_goalval + harden_goalval: 
                            min_removed == Int_MAX ? min_bound : LB_goalvalue + min_removed);
        } else if (opt_minimization == 1) LB_goalvalue = next_sum(LB_goalvalue - fixed_goalval - harden_goalval, goal_Cs) + fixed_goalval + harden_goalval; 
        else LB_goalvalue = try_lessthan;

        if (LB_goalvalue == best_goalvalue && opt_minimization != 1) break;

        Int goal_diff = harden_goalval+fixed_goalval;
        if (opt_minimization == 1) {
            inequality = mkLit(sat_solver.newVar(VAR_UPOL, !opt_branch_pbvars), true);
            try_lessthan = goal_diff + 2;
	} else if (opt_minimization == 0 || best_goalvalue == Int_MAX || best_goalvalue - LB_goalvalue < opt_seq_thres) {
            inequality = (assump_ps.size() == 0 ? lit_Undef : mkLit(sat_solver.newVar(VAR_UPOL, !opt_branch_pbvars), true));
	    try_lessthan = (best_goalvalue != Int_MAX ? best_goalvalue : UB_goalvalue+1);
	} else {
	    inequality = mkLit(sat_solver.newVar(VAR_UPOL, !opt_branch_pbvars), true);
	    try_lessthan = (LB_goalvalue*(100-opt_bin_percent) + best_goalvalue*(opt_bin_percent))/100;
	}
 
        if (!addConstr(goal_ps, goal_Cs, try_lessthan - goal_diff, -2, inequality))
            break; // unsat
        if (inequality != lit_Undef) {
            sat_solver.setFrozen(var(inequality),true);
            assump_ps.push(inequality); assump_Cs.push(opt_minimization == 2 ? try_lessthan : 
                                                       min_removed != Int_MAX && min_removed != 0 ? min_removed : 1);
        }
        if (constrs.size() > 0 && (opt_minimization != 1 || !opt_delay_init_constraints)) convertPbs(false);
        if (opt_minimization == 1) {
            if (constrs.size() > 0 && constrs.last()->lit == inequality) {
                int diff = 0; //optimize_last_constraint(constrs);
                LB_goalvalue += Int(diff) * assump_Cs.last();
                if (constrs.last()->lit != inequality) inequality = assump_ps.last() = constrs.last()->lit;
                saved_constrs.push(constrs.last()), assump_map.set(toInt(inequality),saved_constrs.size() - 1);
                saved_constrs_Cs.push(assump_Cs.last());
            }
            if (!opt_delay_init_constraints) {
                int j = 0;
                for (int i = 0; i < saved_constrs.size(); i++)
                    if (saved_constrs[i] != NULL) {
                        if (saved_constrs[i]->lo == 1 && saved_constrs[i]->hi == Int_MAX || 
                                saved_constrs[i]->hi == saved_constrs[i]->size - 1 && saved_constrs[i]->lo == Int_MIN ) {
                            saved_constrs[i]->~Linear();
                            saved_constrs[i] = NULL;
                        } else {
                            if (j < i) {
                                saved_constrs[j] = saved_constrs[i],  saved_constrs[i] = NULL, saved_constrs_Cs[j] = saved_constrs_Cs[i];
                                if (saved_constrs[j]->lit != lit_Undef) assump_map.set(toInt(saved_constrs[j]->lit), j); 
                            }
                            j++;
                        }
                    }
                if (j < saved_constrs.size()) 
                    saved_constrs.shrink(saved_constrs.size() - j), saved_constrs_Cs.shrink(saved_constrs_Cs.size() - j);
                constrs.clear();
            }
        }
        if (weighted_instance && sat && sat_solver.conflicts > 10000)
            harden_soft_cls(assump_ps, assump_Cs);
        if (opt_minimization >= 1 && opt_verbosity >= 2) {
            char *t; reportf("Lower bound  = %s, assump. size = %d, stratif. level = %d (cls: %d, wght: %s)\n", t=toString(LB_goalvalue * goal_gcd), 
                    assump_ps.size(), sorted_assump_Cs.size(), top_for_strat, toString(sorted_assump_Cs.size() > 0 ? sorted_assump_Cs.last() : 0)); xfree(t); }
        if (opt_minimization == 1 && opt_to_bin_search && LB_goalvalue + 5 < UB_goalvalue &&
                Minisat::cpuTime() >= opt_unsat_cpu && sat_solver.conflicts > opt_unsat_cpu * 100) {
            int cnt = 0;
            for (int j = 0, i = 0; i < psCs.size(); i++) {
                const Int &w = soft_cls[psCs[i].snd].fst;
                if (j == assump_ps.size() || psCs[i].fst < assump_ps[j] || psCs[i].fst == assump_ps[j] && w > assump_Cs[j])
                    if (++cnt >= 50000) { opt_to_bin_search = false; break; }
                if (j < assump_ps.size() && psCs[i].fst == assump_ps[j]) j++;
            }
            if (opt_to_bin_search) {
                for (int j = am1_rels.size() - 1, i = assump_ps.size() - 1; i >= 0 && assump_ps[i] > max_assump; i--) {
                    while (j >= 0 && am1_rels[j].fst > assump_ps[i]) j--;
                    if (j < 0 || am1_rels[j].fst < assump_ps[i]) sat_solver.addClause(~assump_ps[i]); 
                    assump_ps.pop(), assump_Cs.pop();
                }
                goal_ps.clear(); goal_Cs.clear();
                bool clear_assump = (cnt * 3 >= assump_ps.size());
                int k = 0;
                for (int j = 0, i = 0; i < psCs.size(); i++) {
                    const Int &w = soft_cls[psCs[i].snd].fst;
                    bool in_harden = harden_lits.has(psCs[i].fst);
                    if ((j == assump_ps.size() || psCs[i].fst < assump_ps[j] || 
                            psCs[i].fst == assump_ps[j] && (clear_assump || w > assump_Cs[j] || in_harden)) &&
                        (!in_harden || harden_lits.at(psCs[i].fst) < w))
                            goal_ps.push(~psCs[i].fst), goal_Cs.push(in_harden ? w - harden_lits.at(psCs[i].fst) : w);
                    if (j < assump_ps.size() && psCs[i].fst == assump_ps[j]) {
                        if (!clear_assump && w == assump_Cs[j] && !in_harden) { 
                            if (k < j) assump_ps[k] = assump_ps[j], assump_Cs[k] = assump_Cs[j];
                            k++;
                        }
                        j++;
                    }
                }      
                if (k < assump_ps.size()) assump_ps.shrink(assump_ps.size() - k), assump_Cs.shrink(assump_Cs.size() - k);
                for (int i = 0; i < top_for_strat; i++) { 
                    if (soft_cls[i].snd->size() > 1) sat_solver.addClause(*soft_cls[i].snd);
                }
                for (int i = 0; i < am1_rels.size(); i++) goal_ps.push(~am1_rels[i].fst), goal_Cs.push(am1_rels[i].snd);
                top_for_strat = 0; sorted_assump_Cs.clear(); am1_rels.clear(); harden_lits.clear();
                if (opt_verbosity >= 1) {
                    reportf("Switching to binary search ... (after %g s and %d conflicts) with %d goal literals and %d assumptions\n", 
                            Minisat::cpuTime(), sat_solver.conflicts, goal_ps.size(), assump_ps.size());
                }
                opt_minimization = 2;
                if (sat) {
                    try_lessthan = evalPsCs(goal_ps, goal_Cs, best_model); 
                    inequality = (assump_ps.size() == 0 ? lit_Undef : mkLit(sat_solver.newVar(VAR_UPOL, !opt_branch_pbvars), true));
                    if (inequality != lit_Undef) assump_ps.push(inequality), assump_Cs.push(try_lessthan);
                    if (!addConstr(goal_ps, goal_Cs, try_lessthan, -2, inequality)) break; // unsat
                    try_lessthan += fixed_goalval + harden_goalval;
                    if (constrs.size() > 0) convertPbs(false);
                }
            }
        }
      }         
    } // END OF LOOP

    if (goal_gcd != 1) {
        if (best_goalvalue != Int_MAX) best_goalvalue *= goal_gcd;
        if (LB_goalvalue   != Int_MIN) LB_goalvalue *= goal_gcd;
        if (UB_goalvalue   != Int_MAX) UB_goalvalue *= goal_gcd;
    }
    if (opt_verbosity >= 1){
        if      (!sat)
            reportf(asynch_interrupt ? "\bUNKNOWN\b\n" : "\bUNSATISFIABLE\b\n");
        else if (soft_cls.size() == 0)
            reportf("\bSATISFIABLE: No goal function specified.\b\n");
        else if (cmd == sc_FirstSolution){
            char* tmp = toString(best_goalvalue);
            reportf("\bFirst solution found: %s\b\n", tmp);
            xfree(tmp);
        }else if (asynch_interrupt){
            char* tmp = toString(best_goalvalue);
            reportf("\bSATISFIABLE: Best solution found: %s\b\n", tmp);
            xfree(tmp);
       }else{
            char* tmp = toString(best_goalvalue);
            reportf("\bOptimal solution: %s\b\n", tmp);
            xfree(tmp);
        }
    }
}

void set_difference(vec<Lit>& set1, const vec<Lit>& set2)
{
    int j =0, k = 0;
    for (int i = 0; i < set1.size(); i++) {
        while (k < set2.size() && set2[k] < set1[i]) k++;
        if (k < set2.size() && set1[i] == set2[k]) { k++; continue; }
        if (j < i) set1[j] = set1[i];
        j++;
    }
    if (j < set1.size()) set1.shrink(set1.size() - j);
}

struct mapLT { Map<Lit, vec<Lit>* >&c; bool operator()(Lit p, Lit q) { return c.at(p)->size() < c.at(q)->size(); }};

void MsSolver::preprocess_soft_cls(Minisat::vec<Lit>& assump_ps, vec<Int>& assump_Cs, const Lit max_assump, const Int& max_assump_Cs, 
                                              IntLitQueue& delayed_assump, Int& delayed_assump_sum)
{
    Map<Lit, vec<Lit>* > conns;
    vec<Lit> conns_lit;
    vec<Lit> confl;
    vec<Lit> lits;
    for (int i = 0; i < assump_ps.size() && assump_ps[i] <= max_assump; i++) {
        Minisat::vec<Lit> assump, props; 
        assump.push(assump_ps[i]);
        if (sat_solver.prop_check(assump, props, 2))
            for (int l, j = 0; j < props.size(); j++) {
                if ((l = bin_search(assump_ps,  ~props[j])) >= 0 && assump_ps[l] <= max_assump) {
                    if (!conns.has(assump[0])) conns.set(assump[0],new vec<Lit>());
                    conns.ref(assump[0])->push(~props[j]);
                    if (!conns.has(~props[j])) conns.set(~props[j], new vec<Lit>());
                    conns.ref(~props[j])->push(assump[0]);
                }
            }  
        else confl.push(assump_ps[i]);
    }
    conns.domain(conns_lit);
    if (confl.size() > 0) {
        for (int i = 0; i < conns_lit.size(); i++) {
            if (bin_search(confl, conns_lit[i]) >= 0) {
                delete conns.ref(conns_lit[i]);
                conns.exclude(conns_lit[i]);
            } else {
                vec<Lit>& dep_lit = *conns.ref(conns_lit[i]);
                sortUnique(dep_lit);
                set_difference(dep_lit, confl);
                if (dep_lit.size() == 0) { delete conns.ref(conns_lit[i]); conns.exclude(conns_lit[i]); }
                else lits.push(conns_lit[i]);
            }
        }
        conns_lit.clear(); conns.domain(conns_lit);
        for (int l, i = 0; i < confl.size(); i++) {
            Lit p = confl[i];
            if ((l = bin_search(assump_ps, p)) >= 0 && assump_ps[l] <= max_assump) {
                if (!harden_lits.has(p)) harden_lits.set(p, assump_Cs[l]); else harden_lits.ref(p) += assump_Cs[l];
                harden_goalval += assump_Cs[l];
                addUnit(~p); LB_goalvalue += assump_Cs[l]; assump_Cs[l] = -assump_Cs[l];
            }
        }
        if (opt_verbosity >= 2) reportf("Found %d Unit cores\n", confl.size());
    } else
        for (int i = 0; i < conns_lit.size(); i++) { 
            lits.push(conns_lit[i]); 
            sortUnique(*conns.ref(conns_lit[i])); 
        }
    sort(lits);
    mapLT cmp {conns};
    int am1_cnt = 0, am1_len_sum = 0;
    while (lits.size() > 0) {
        vec<Lit> am1;
        Lit minl = lits[0];
        for (int new_sz,  sz = conns.at(minl)->size(), i = 1; i < lits.size(); i++)
            if ((new_sz = conns.at(lits[i])->size()) < sz) minl = lits[i], sz = new_sz;
        am1.push(minl);
        vec<Lit>& dep_minl = *conns.ref(minl);
        sort(dep_minl, cmp);
        for (int sz = dep_minl.size(), i = 0; i < sz; i++) {
            Lit l = dep_minl[i];
            if (bin_search(lits, l) >= 0) {
                int i;
                const vec<Lit>& dep_l = *conns.at(l);
                for (i = 1; i < am1.size() && bin_search(dep_l, am1[i]) >= 0; ++i);
                if (i == am1.size()) am1.push(l);
            }
        }
        sort(dep_minl);
        sort(am1);
        set_difference(lits, am1);
        for (int i = 0; i < conns_lit.size(); i++)  set_difference(*conns.ref(conns_lit[i]), am1);
        if (am1.size() > 1) {
            Minisat::vec<Lit> cls;
            vec<int> ind;
            Int min_Cs = Int_MAX;
            for (int l, i = 0; i < am1.size(); i++)
                if ((l = bin_search(assump_ps, am1[i])) >= 0 && assump_Cs[l] > 0) {
                    ind.push(l);
                    if (assump_Cs[l] < min_Cs) min_Cs = assump_Cs[l];
                }
                else reportf("am1: %d %d %d %s\n", i, am1.size(), toInt(am1[0]), toInt(am1[i]), (l>=0 && l <assump_Cs.size()?toString(assump_Cs[l]):"???"));
            if (ind.size() < 2) continue;
            for (int i = 0; i < ind.size(); i++) {
                if (assump_Cs[ind[i]] == min_Cs) cls.push(assump_ps[ind[i]]), assump_Cs[ind[i]] = -assump_Cs[ind[i]];
                else {
                    //Lit r = mkLit(sat_solver.newVar(VAR_UPOL, !opt_branch_pbvars), true);
                    //sat_solver.addClause(assump_ps[ind[i]], r);
                    cls.push(assump_ps[ind[i]]); //~r);
                    assump_Cs[ind[i]] -= min_Cs;
                    if (assump_Cs[i] < max_assump_Cs) {
                        delayed_assump.push(Pair_new(assump_Cs[ind[i]], assump_ps[ind[i]]));
                        delayed_assump_sum += assump_Cs[ind[i]];
                        assump_Cs[ind[i]] = - assump_Cs[ind[i]];
                    }
                }
                if (!harden_lits.has(assump_ps[ind[i]])) harden_lits.set(assump_ps[ind[i]], min_Cs);
                else harden_lits.ref(assump_ps[ind[i]]) += min_Cs;
            }
            Lit r = mkLit(sat_solver.newVar(VAR_UPOL, !opt_branch_pbvars), true);
            sat_solver.setFrozen(var(r), true);
            cls.push(~r); assump_ps.push(r); assump_Cs.push(min_Cs);
            am1_rels.push(Pair_new(r,min_Cs));
            sat_solver.addClause(cls);
            if (ind.size() > 2) min_Cs = Int(ind.size() - 1) * min_Cs;
            am1_cnt++; am1_len_sum += am1.size();  LB_goalvalue += min_Cs; harden_goalval += min_Cs;
        }
    }
    if (am1_cnt > 0 || confl.size() > 0) clear_assumptions(assump_ps, assump_Cs);
    if (opt_verbosity >= 2 && am1_cnt > 0) 
        reportf("Found %d AtMostOne cores of avg size: %.2f\n", am1_cnt, (double)am1_len_sum/am1_cnt);
}

