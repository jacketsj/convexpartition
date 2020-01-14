/*****************************************************************************[PbSolver_convert.cc]
Copyright (c) 2005-2010, Niklas Een, Niklas Sorensson

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

#include "PbSolver.h"
#include "Hardware.h"

//-------------------------------------------------------------------------------------------------
void    linearAddition (const Linear& c, vec<Formula>& out);        // From: PbSolver_convertAdd.C
Formula buildConstraint(const Linear& c, int max_cost = INT_MAX);   // From: PbSolver_convertSort.C
Formula convertToBdd   (const Linear& c, int max_cost = INT_MAX);   // From: PbSolver_convertBdd.C
//-------------------------------------------------------------------------------------------------


bool PbSolver::convertPbs(bool first_call)
{
    vec<Formula>    converted_constrs;
    ConvertT saved_opt_convert = opt_convert;

    if (first_call){
        if (!opt_maxsat) findIntervals();
        if (!rewriteAlmostClauses()) {
            sat_solver.addEmptyClause();
            return false;
        }
    }

    for (int i = 0; i < constrs.size(); i++) {
        if (constrs[i] == NULL) continue;
        Linear& c   = *constrs[i]; assert(c.lo != Int_MIN || c.hi != Int_MAX);
//printf("constrs[%d]: size = %d, lo = %s, hi = %s, var = %s%d\n", i, constrs[i]->size, toString(constrs[i]->lo), toString(constrs[i]->hi), sign(constrs[i]->lit) ? "-" : "", var(constrs[i]->lit));
//for (int j = 0; j < constrs[i]->size; j++) printf(" %s %s%d",toString((*constrs[i])(j)), sign((*constrs[i])[j]) ? "-" : "", var((*constrs[i])[j]));
//printf("\n");

        if (opt_verbosity >= 1) 
            if (first_call && !opt_maxsat) /**/ reportf("---[%4d]---> ", constrs.size() - 1 - i); 
            else if (i == 0 && constrs.size() == 1 && opt_minimization != 1 && !opt_maxsat) /**/ reportf("---[goal]---> ");
    
        try { // M. Piotrow 11.10.2017
            if (opt_convert == ct_Sorters)
                converted_constrs.push(buildConstraint(c)), first_call ? ++srtEncodings : ++srtOptEncodings;
            else if (opt_convert == ct_Adders)
                linearAddition(c, converted_constrs), first_call ? ++addEncodings : ++addOptEncodings;
            else if (opt_convert == ct_BDDs)
                converted_constrs.push(convertToBdd(c)), first_call ? ++bddEncodings : ++bddOptEncodings;
            else if (opt_convert == ct_Mixed) {
                int adder_cost = estimatedAdderCost(c);
                //int different_cs = 1; for (int i = 1; i < c.size; i++) if (c(i) != c(i-1)) different_cs++;
                //**/printf("estimatedAdderCost: %d\n", estimatedAdderCost(c));
                /*if (first_call || opt_convert_goal == opt_convert) {
  	            Int max_coeff = c(c.size-1);
  	            int max_log = 0; while ((max_coeff >>= 1) > 0) max_log++;
                    double add_sort_factor = (max_log > 20 ? 1.0/(max_log-20) : max_log <= 10 ? 15.0 : 22.0-max_log);
                    Formula result = buildConstraint(c, (int)(adder_cost * opt_sort_thres * add_sort_factor));
                    if (result == _undef_) {
                        result = convertToBdd(c, (int)(adder_cost * opt_bdd_thres));
                        if (result != _undef_) 
                            if (!first_call) opt_convert = ct_BDDs, ++bddOptEncodings; else ++bddEncodings;
                    } else if (!first_call) opt_convert = ct_Sorters, ++srtOptEncodings; else ++srtEncodings;
                    if (result == _undef_) linearAddition(c, converted_constrs), first_call ? ++addEncodings : ++addOptEncodings;
                    else converted_constrs.push(result);
                } else*/ {
                    Formula result = buildConstraint(c, (int)(adder_cost * opt_sort_thres)); 
                    if (result == _undef_) {
                        result = convertToBdd(c, (int)(adder_cost * opt_bdd_thres));
                        if (result != _undef_)
                            if (!first_call) opt_convert = ct_BDDs, ++bddOptEncodings; else ++bddEncodings; 
                    } else if (!first_call) opt_convert = ct_Sorters, ++srtOptEncodings; else ++srtEncodings;
                    if (result == _undef_) linearAddition(c, converted_constrs), first_call ? ++addEncodings : ++addOptEncodings;
                    else converted_constrs.push(result);
                }
            } else assert(false);
            opt_convert = saved_opt_convert;
        } catch (std::bad_alloc& ba) { // M. Piotrow 11.10.2017
	    FEnv::pop(); i-=1;
	    if (opt_convert == ct_Sorters)  { opt_convert = ct_BDDs; continue; }
	    if (opt_convert != ct_Adders)  { opt_convert = ct_Adders; continue; }
            else {
	        reportf("Out of memery in converting constraints: %s\n",ba.what());
	        exit(1); //throw(std::bad_alloc);
	    }
        }
        if (!okay()) return false;
    }

    // NOTE: probably still leaks memory (if there are constraints that are NULL'ed elsewhere)
    if (!opt_maxsat_msu || !opt_shared_fmls || opt_minimization != 1) {
        for (int i = 0; i < constrs.size(); i++)
          if (constrs[i] != NULL)
            constrs[i]->~Linear();
        constrs.clear();
        mem.clear();
    }

    try { // M. Piotrow 15.06.2018
        int nvars = sat_solver.nVars(), ncls = sat_solver.nClauses();
        clausify(sat_solver, converted_constrs);
        if (opt_verbosity >=2 && ((nvars -= sat_solver.nVars()) < 0 | (ncls -= sat_solver.nClauses()) < 0)) 
            reportf("New vars/cls: %d/%d\n", -nvars, -ncls);
    } catch (std::bad_alloc& ba) { // M. Piotrow 15.06.2018
      reportf("Out of memery in converting constraints: %s\n",ba.what());
      exit(1);
    }
    return okay();
}
