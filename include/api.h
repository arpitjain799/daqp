#ifndef DAQP_API_H
# define DAQP_API_H
#include "daqp.h"
#include "daqp_prox.h"
#include "bnb.h"
#include "hierarchical.h"

typedef struct{
    c_float *x;
    c_float *lam;
    c_float fval;
    c_float soft_slack;

    int exitflag;
    int iter;
    int nodes;
    c_float solve_time;
    c_float setup_time;

}DAQPResult;

void daqp_solve(DAQPResult* res, DAQPWorkspace *work);
void daqp_quadprog(DAQPResult* res, DAQPProblem* qp,DAQPSettings* settings);

int setup_daqp(DAQPProblem *qp, DAQPWorkspace* work, c_float* setup_time);
int setup_daqp_ldp(DAQPWorkspace *work, DAQPProblem* qp);
int setup_daqp_hiqp(DAQPWorkspace *work, DAQPProblem* qp, int nh, int* break_points);

void allocate_daqp_settings(DAQPWorkspace *work);
void allocate_daqp_workspace(DAQPWorkspace *work, int n, int ns);

void free_daqp_ldp(DAQPWorkspace *work);
void free_daqp_workspace(DAQPWorkspace *work);

void daqp_extract_result(DAQPResult* res, DAQPWorkspace* work);
void daqp_default_settings(DAQPSettings *settings);
#endif //ifndef DAQP_API_H
