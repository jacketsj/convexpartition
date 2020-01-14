/*****************************************************************************************[Main.cc]

UWrMaxSat:   Copyright (c) 2019, Marek Piotrów, based on:
kp-minisatp: Copyright (c) 2017-2018, Michał Karpiński and Marek Piotrów, based on:
Minisat+:    Copyright (c) 2005-2010, Niklas Een, Niklas Sorensson

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

/**************************************************************************************************

Read a DIMACS file and apply the SAT-solver to it.

**************************************************************************************************/


#include <unistd.h>
#include <signal.h>
#include "System.h"
#include "MsSolver.h"
#include "PbParser.h"
#include "FEnv.h"

//=================================================================================================
// Command line options:

bool     opt_maxsat    = false;
bool     opt_satlive   = true;
bool     opt_ansi      = true;
char*    opt_cnf       = NULL;
int      opt_verbosity = 1;
bool     opt_model_out = true;
bool     opt_try       = false;     // (hidden option -- if set, then "try" to parse, but don't output "s UNKNOWN" if you fail, instead exit with error code 5)

bool     opt_preprocess    = true;
ConvertT opt_convert       = ct_Undef;
ConvertT opt_convert_goal  = ct_Undef;
bool     opt_convert_weak  = true;
double   opt_bdd_thres     = 10;
double   opt_sort_thres    = 200;
double   opt_goal_bias     = 10;
Int      opt_goal          = Int_MAX;
Command  opt_command       = cmd_Minimize;
bool     opt_branch_pbvars = false;
int      opt_polarity_sug  = 1;
bool     opt_old_format    = false;
bool     opt_shared_fmls   = false;
int      opt_base_max      = 47;
int      opt_cpu_lim       = INT32_MAX;
int      opt_mem_lim       = INT32_MAX;

int      opt_minimization  = -1; // -1 = to be set, 0 = sequential. 1 = alternating, 2 - binary
int      opt_seq_thres     = 96;
int      opt_bin_percent   = 65;
bool     opt_maxsat_msu    = true;
double   opt_unsat_cpu     = 1200; // in seconds
bool     opt_lexicographic = false;
bool     opt_to_bin_search = true;
bool     opt_maxsat_prepr  = true;

char*    opt_input  = NULL;
char*    opt_result = NULL;


// -- statistics;
unsigned long long int srtEncodings = 0, addEncodings = 0, bddEncodings = 0;
unsigned long long int srtOptEncodings = 0, addOptEncodings = 0, bddOptEncodings = 0;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

cchar* doc =
    "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
    "UWrMaxSat 1.0 -- University of Wrocław MaxSAT solver by Marek Piotrów (2019)\n" 
    "and PB solver by Marek Piotrów and Michał Karpiński (2018) -- an extension of\n"
    "MiniSat+ 1.1, based on MiniSat 2.2.0  -- (C) Niklas Een, Niklas Sorensson (2012)\n"
    "with COMiniSatPS by Chanseok Oh (2016) as the SAT solver\n"
    "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
    "USAGE: uwrmaxsat <input-file> [<result-file>] [-<option> ...]\n"
    "\n"
    "Solver options:\n"
    "  -ca -adders   Convert PB-constrs to clauses through adders.\n"
    "  -cs -sorters  Convert PB-constrs to clauses through sorters. (default)\n"
    "  -cb -bdds     Convert PB-constrs to clauses through bdds.\n"
    "  -cm -mixed    Convert PB-constrs to clauses by a mix of the above.\n"
    "  -ga/gs/gb/gm  Override conversion for goal function (long name: -goal-xxx).\n"
    "  -w -weak-off  Clausify with equivalences instead of implications.\n"
    "  -no-pre       Don't use MiniSat's CNF-level preprocessing.\n"
    "\n"
    "  -cpu-lim=     CPU time limit in seconds. Zero - no limit. (default)\n"
    "  -mem-lim=     Memory limit in MB. Zero - no limit. (default)\n"
    "\n"
    "  -bdd-thres=   Threshold for prefering BDDs in mixed mode.        [def: %g]\n"
    "  -sort-thres=  Threshold for prefering sorters. Tried before BDDs.[def: %g]\n"
    "  -goal-bias=   Bias goal function convertion towards sorters.     [def: %g]\n"
    "  -base-max=    Maximal number (<= %d) to be used in sorter base.  [def: %d]\n"
    "\n"
    "  -1 -first     Don\'t minimize, just give first solution found\n"
    "  -A -all       Don\'t minimize, give all solutions\n"
    "  -goal=<num>   Set initial goal limit to '<= num'.\n"
    "\n"
    "  -p -pbvars    Restrict decision heuristic of SAT to original PB variables.\n"
    "  -ps{+,-,0}    Polarity suggestion in SAT towards/away from goal (or neutral).\n"
    "  -seq          Sequential search for the optimum value of goal.\n"
    "  -bin          Binary search for the optimum value of goal. (default)\n"
    "  -alt          Alternating search for the optimum value of goal. (a mix of the above)\n"

    "\n"
    "Input options:\n"
    "  -m -maxsat    Use the MaxSAT input file format (wcnf).\n"
    "  -of -old-fmt  Use old variant of OPB file format.\n"
    "\n"
    "Output options:\n"
    "  -s -satlive   Turn off SAT competition output.\n"
    "  -a -ansi      Turn off ANSI codes in output.\n"
    "  -v0,-v1,-v2   Set verbosity level (1 default)\n"
    "  -cnf=<file>   Write SAT problem to a file. Trivial UNSAT => no file written.\n"
    "  -nm -no-model Supress model output.\n"
    "\n"
    "MaxSAT specific options:\n"
    "  -no-msu       Use PB specific search algoritms for MaxSAT (see -alt, -bin, -seq).\n"
    "  -unsat-cpu=   Time to switch UNSAT search strategy to SAT/UNSAT. [def: %g s]\n"
    "  -lex-opt      Do Boolean lexicographic optimizations on soft clauses.\n"
    "  -no-bin       Do not switch from UNSAT to SAT/UNSAT search strategy.\n"
    "  -no-ms-pre    Do not preprocess soft clauses (detect unit/am1 cores).\n"
    "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool oneof(cchar* arg, cchar* alternatives)
{
    // Force one leading '-', allow for two:
    if (*arg != '-') return false;
    arg++;
    if (*arg == '-') arg++;

    // Scan alternatives:
    vec<char*>  alts;
    splitString(alternatives, ",", alts);
    for (int i = 0; i < alts.size(); i++){
        if (strcmp(arg, alts[i]) == 0){
            xfreeAll(alts);
            return true;
        }
    }
    xfreeAll(alts);
    return false;
}


void parseOptions(int argc, char** argv)
{
    vec<char*>  args;   // Non-options

    for (int i = 1; i < argc; i++){
        char*   arg = argv[i];
        if (arg[0] == '-'){
            if (oneof(arg,"h,help")) fprintf(stderr, doc, opt_bdd_thres, opt_sort_thres, opt_goal_bias, opt_base_max, opt_base_max, opt_unsat_cpu), exit(0);

            else if (oneof(arg, "ca,adders" )) opt_convert = ct_Adders;
            else if (oneof(arg, "cs,sorters")) opt_convert = ct_Sorters;
            else if (oneof(arg, "cb,bdds"   )) opt_convert = ct_BDDs;
            else if (oneof(arg, "cm,mixed"  )) opt_convert = ct_Mixed;

            else if (oneof(arg, "ga,goal-adders" )) opt_convert_goal = ct_Adders;
            else if (oneof(arg, "gs,goal-sorters")) opt_convert_goal = ct_Sorters;
            else if (oneof(arg, "gb,goal-bdds"   )) opt_convert_goal = ct_BDDs;
            else if (oneof(arg, "gm,goal-mixed"  )) opt_convert_goal = ct_Mixed;

            else if (oneof(arg, "w,weak-off"     )) opt_convert_weak = false;
            else if (oneof(arg, "no-pre"))          opt_preprocess   = false;
            else if (oneof(arg, "nm,no-model" ))    opt_model_out    = false;
            else if (oneof(arg, "no-msu" ))         opt_maxsat_msu   = false;

            //(make nicer later)
            else if (strncmp(arg, "-bdd-thres=" , 11) == 0) opt_bdd_thres  = atof(arg+11);
            else if (strncmp(arg, "-sort-thres=", 12) == 0) opt_sort_thres = atof(arg+12);
            else if (strncmp(arg, "-goal-bias=",  11) == 0) opt_goal_bias  = atof(arg+11);
            else if (strncmp(arg, "-goal="     ,   6) == 0) opt_goal       = atoi(arg+ 6);  // <<== real bignum parsing here
            else if (strncmp(arg, "-cnf="      ,   5) == 0) opt_cnf        = arg + 5;
            else if (strncmp(arg, "-base-max=",   10) == 0) opt_base_max   = atoi(arg+10); 
            else if (strncmp(arg, "-bin-split=",  11) == 0) opt_bin_percent= atoi(arg+11); 
            else if (strncmp(arg, "-seq-thres=",  11) == 0) opt_seq_thres  = atoi(arg+11);
            else if (strncmp(arg, "-unsat-cpu=",  11) == 0) opt_unsat_cpu  = atoi(arg+11);
            //(end)

            else if (oneof(arg, "1,first"   )) opt_command = cmd_FirstSolution;
            else if (oneof(arg, "A,all"     )) opt_command = cmd_AllSolutions;

            else if (oneof(arg, "p,pbvars"  )) opt_branch_pbvars = true;
            else if (oneof(arg, "ps+"       )) opt_polarity_sug = +1;
            else if (oneof(arg, "ps-"       )) opt_polarity_sug = -1;
            else if (oneof(arg, "ps0"       )) opt_polarity_sug =  0;
            else if (oneof(arg, "seq"       )) opt_minimization =  0;
            else if (oneof(arg, "alt"       )) opt_minimization =  1;
            else if (oneof(arg, "bin"       )) opt_minimization =  2;

            else if (oneof(arg, "of,old-fmt" )) opt_old_format = true;
            else if (oneof(arg, "m,maxsat"  )) opt_maxsat  = true, opt_seq_thres = 3;
            else if (oneof(arg, "lex-opt"   )) opt_lexicographic = true;
            else if (oneof(arg, "no-bin"    )) opt_to_bin_search = false;
            else if (oneof(arg, "no-ms-pre" )) opt_maxsat_prepr = false;

            else if (oneof(arg, "s,satlive" )) opt_satlive = false;
            else if (oneof(arg, "a,ansi"    )) opt_ansi    = false;
            else if (oneof(arg, "try"       )) opt_try     = true;
            else if (oneof(arg, "v0"        )) opt_verbosity = 0;
            else if (oneof(arg, "v1"        )) opt_verbosity = 1;
            else if (oneof(arg, "v2"        )) opt_verbosity = 2;
            else if (strncmp(arg, "-sa", 3 ) == 0) {
                if (arg[3] == '2') opt_shared_fmls = true;
            }
            else if (strncmp(arg, "-cpu-lim=",  9) == 0) opt_cpu_lim  = atoi(arg+9);
            else if (strncmp(arg, "-mem-lim=",  9) == 0) opt_mem_lim  = atoi(arg+9);
            else
                fprintf(stderr, "ERROR! Invalid command line option: %s\n", argv[i]), exit(1);

        }else
            args.push(arg);
    }

    if (args.size() == 0)
        fprintf(stderr, doc, opt_bdd_thres, opt_sort_thres, opt_goal_bias), exit(0);
    if (args.size() >= 1)
        opt_input = args[0];
    if (args.size() == 2)
        opt_result = args[1];
    else if (args.size() > 2)
        fprintf(stderr, "ERROR! Too many files specified on commandline.\n"),
        exit(1);
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


void reportf(const char* format, ...)
{
    static bool col0 = true;
    static bool bold = false;
    va_list args;
    va_start(args, format);
    char* text = vnsprintf(format, args);
    va_end(args);

    for(char* p = text; *p != 0; p++){
        if (col0 && opt_satlive)
            putchar('c'), putchar(' ');

        if (*p == '\b'){
            bold = !bold;
            if (opt_ansi)
                putchar(27), putchar('['), putchar(bold?'1':'0'), putchar('m');
            col0 = false;
        }else{
            putchar(*p);
            col0 = (*p == '\n' || *p == '\r');
        }
    }
    fflush(stdout);
}


//=================================================================================================
// Helpers:


MsSolver*   pb_solver = NULL;   // Made global so that the SIGTERM handler can output best solution found.

void outputResult(const PbSolver& S, bool optimum = true)
{
    if (!opt_satlive) return;

    if (opt_model_out && S.best_goalvalue != Int_MAX){
        printf("v");
        for (int i = 0; i < S.best_model.size(); i++)
            if (S.index2name[i][0] != '#')
                printf(" %s%s", S.best_model[i]?"":"-", S.index2name[i]);
        printf("\n");
    }
    if (optimum){
        if (S.best_goalvalue == Int_MAX) printf("s UNSATISFIABLE\n");
        else                             printf("s OPTIMUM FOUND\n");
    }else{
        if (S.best_goalvalue == Int_MAX) printf("s UNKNOWN\n");
        else                             printf("s SATISFIABLE\n");
    }
    fflush(stdout);
}

static void handlerOutputResult(const PbSolver& S, bool optimum = true)
{     // Signal handler save version of the function outputResult
    constexpr int BUF_SIZE = 50000;
    static char buf[BUF_SIZE];
    static int lst = 0;
    if (!opt_satlive) return;
    if (opt_model_out && S.best_goalvalue != Int_MAX){
        buf[0] = '\n'; buf[1] = 'v'; lst += 2;
        for (int i = 0; i < S.best_model.size(); i++)
            if (S.index2name[i][0] != '#') {
                int sz = strlen(S.index2name[i]);
                if (lst + sz + 2 >= BUF_SIZE) { buf[lst++] = '\n'; lst = write(1, buf, lst); buf[0] = 'v'; lst = 1; }
                buf[lst++] = ' ';
                if (!S.best_model[i]) buf[lst++] = '-';
                strcpy(buf+lst,S.index2name[i]); lst += sz;
            }
        buf[lst++] = '\n';
        if (lst + 20 >= BUF_SIZE) { lst = write(1, buf, lst); lst = 0; }
    }
    const char *out;
    if (optimum){
        if (S.best_goalvalue == Int_MAX) out = "s UNSATISFIABLE\n";
        else                             out = "s OPTIMUM FOUND\n";
    }else{
        if (S.best_goalvalue == Int_MAX) out = "s UNKNOWN\n";
        else                             out = "s SATISFIABLE\n";
    }
    strcpy(buf + lst, out); lst += strlen(out);
    lst = write(1, buf, lst); lst = 0;
}


static void SIGINT_handler(int /*signum*/) {
    reportf("\n");
    reportf("*** INTERRUPTED ***\n");
    //SatELite::deleteTmpFiles();
    fflush(stdout);
    _exit(0); }     // (using 'exit()' rather than '_exit()' sometimes causes the solver to hang (why?))


static void SIGTERM_handler(int signum) {
    if (opt_verbosity >= 1) {
        reportf("\n");
        reportf("*** TERMINATED by signal %d ***\n", signum);
        reportf("_______________________________________________________________________________\n\n");
        pb_solver->printStats();
        reportf("_______________________________________________________________________________\n");
    }
    handlerOutputResult(*pb_solver, false);
    //SatELite::deleteTmpFiles();
    //fflush(stdout);
    _exit(0);
}

static void increase_stack_size(int new_size) // M. Piotrow 16.10.2017
{
#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <sys/resource.h>
  
  struct rlimit rl;
  rlim_t new_mem_lim = new_size*1024*1024;
  getrlimit(RLIMIT_STACK,&rl);
  if (rl.rlim_max == RLIM_INFINITY || new_mem_lim < rl.rlim_max) {
    rl.rlim_cur = new_mem_lim;
    if (setrlimit(RLIMIT_STACK, &rl) == -1)
      reportf("WARNING! Could not set resource limit: Stack memory.\n");
    else if (opt_verbosity > 1)
      reportf("Setting stack limit to %dMB.\n",new_size);
  }
#else
  reportf("WARNING! Setting stack limit not supported on this architecture.\n");
#endif
}


PbSolver::solve_Command convert(Command cmd) {
    switch (cmd){
    case cmd_Minimize:      return PbSolver::sc_Minimize;
    case cmd_FirstSolution: return PbSolver::sc_FirstSolution;
    default: 
        assert(cmd == cmd_AllSolutions);
        return PbSolver::sc_AllSolutions;
    }
}


//=================================================================================================


int main(int argc, char** argv)
{
  try {
    parseOptions(argc, argv);
    pb_solver = new MsSolver(opt_preprocess);
    signal(SIGINT , SIGINT_handler);
    signal(SIGTERM, SIGTERM_handler);

    // Set command from 'PBSATISFIABILITYONLY':
    char* value = getenv("PBSATISFIABILITYONLY");
    if (value != NULL && atoi(value) == 1)
        reportf("Setting switch '-first' from environment variable 'PBSATISFIABILITYONLY'.\n"),
        opt_command = cmd_FirstSolution;

    if (opt_cpu_lim != INT32_MAX) {
        reportf("Setting cpu limit to %ds.\n",opt_cpu_lim);
        signal(SIGXCPU, SIGTERM_handler); 
        limitTime(opt_cpu_lim);
    }
    if (opt_mem_lim != INT32_MAX) {
        reportf("Setting memory limit to %dMB.\n",opt_mem_lim);
        signal(SIGSEGV, SIGTERM_handler); 
        signal(ENOMEM, SIGTERM_handler); 
        signal(SIGABRT, SIGTERM_handler);
        limitMemory(opt_mem_lim);
    }
    increase_stack_size(256); // to at least 256MB - M. Piotrow 16.10.2017
    if (opt_maxsat || opt_input != NULL && strcmp(opt_input+strlen(opt_input)-4, "wcnf") == 0) {
        opt_maxsat = true; opt_seq_thres = 3;
        if (opt_minimization < 0) opt_minimization = 1; // alt (unsat based) algorithm
        if (opt_verbosity >= 1) reportf("Parsing MaxSAT file...\n");
        parse_WCNF_file(opt_input, *pb_solver);
        if (opt_maxsat_msu) {
            if (opt_convert == ct_Undef) opt_convert = ct_Sorters;
            pb_solver->maxsat_solve(convert(opt_command));
        } else {
            for (int i = pb_solver->soft_cls.size() - 1; i >= 0; i--) {
                if (pb_solver->soft_cls[i].snd->size() > 1) pb_solver->sat_solver.addClause(*pb_solver->soft_cls[i].snd);
                delete pb_solver->soft_cls[i].snd;
            }
            pb_solver->soft_cls.clear();
            if (opt_minimization < 0) opt_minimization = 2; // bin (sat/unsat based) algorithm
            if (opt_convert == ct_Undef) opt_convert = ct_Mixed;
            pb_solver->solve(convert(opt_command));
        }
    } else {
        if (opt_verbosity >= 1) reportf("Parsing PB file...\n");
        parse_PB_file(opt_input, *pb_solver, opt_old_format);
        if (!opt_maxsat_msu) {
            if (opt_minimization < 0) opt_minimization = 2; // bin (sat/unsat based) algorithm
            if (opt_convert == ct_Undef) opt_convert = ct_Mixed;
            pb_solver->solve(convert(opt_command));
        } else {
            if (pb_solver->goal != NULL) {
                for (int i = 0; i < pb_solver->goal->size; i++) {
                    Minisat::vec<Lit> *ps_copy = new Minisat::vec<Lit>;
                    ps_copy->push(~(*pb_solver->goal)[i]);
                    pb_solver->soft_cls.push(Pair_new(tolong((*pb_solver->goal)(i)), ps_copy));
                }
                delete pb_solver->goal; pb_solver->goal = NULL;
            }
            opt_maxsat = opt_maxsat_msu = true;
            if (opt_minimization < 0) opt_minimization = 1; // bin (sat/unsat based) algorithm
            if (opt_convert == ct_Undef) opt_convert = ct_Sorters;
            pb_solver->maxsat_solve(convert(opt_command));
        }
    }

    if (pb_solver->goal == NULL && pb_solver->soft_cls.size() == 0 && pb_solver->best_goalvalue != Int_MAX)
        opt_command = cmd_FirstSolution;    // (otherwise output will be wrong)
    if (!pb_solver->okay())
        opt_command = cmd_Minimize;         // (HACK: Get "UNSATISFIABLE" as output)

    // <<== write result to file 'opt_result'

    if (opt_verbosity >= 1) {
        reportf("_______________________________________________________________________________\n\n");
        pb_solver->printStats();
        reportf("_______________________________________________________________________________\n");
    }

    if (opt_command == cmd_Minimize)
        outputResult(*pb_solver, !pb_solver->asynch_interrupt);
    else if (opt_command == cmd_FirstSolution)
        outputResult(*pb_solver, false);

    exit(0); // (faster than "return", which will invoke the destructor for 'PbSolver')
    
  } catch (Minisat::OutOfMemoryException&){
        if (opt_verbosity >= 1) {
          reportf("_______________________________________________________________________________\n\n");
          pb_solver->printStats();
          reportf("_______________________________________________________________________________\n");
          reportf("Out of memory exception caught\n");
        }
        outputResult(*pb_solver, false);
        exit(0);
  }
}
