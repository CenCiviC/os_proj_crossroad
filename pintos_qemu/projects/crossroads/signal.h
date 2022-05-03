#ifndef __PROJECTS_PROJECT1_SIGNAL_H__
#define __PROJECTS_PROJECT1_SIGNAL_H__

#include "projects/crossroads/vehicle.h"

extern int total;
extern int sum;
extern int running_thread;
extern int blocking_thread;

int semWait(struct vehicle_info *vi, int row, int col);
void init_sema(void);
int semWait(struct vehicle_info *vi, int row, int col);
void semSignal(struct vehicle_info *vi);
int out_crit(int row,int col);
int into_crit(int cur_row ,int cur_col,int next_row,int next_col);

#endif