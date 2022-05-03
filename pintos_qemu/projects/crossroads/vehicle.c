#include "threads/thread.h"
#include "threads/synch.h"
#include "projects/crossroads/vehicle.h"
#include "projects/crossroads/map.h"
#include "projects/crossroads/ats.h"
#include "projects/crossroads/signal.h"

int total;
int sum;
int running_thread;
int blocking_thread;

/* path. A:0 B:1 C:2 D:3 */
const struct position vehicle_path[4][4][10] = {
	/* from A */ {
		/* to A */
		{{-1,-1},},
		/* to B */
		{{4,0},{4,1},{4,2},{5,2},{6,2},{-1,-1},},
		/* to C */
		{{4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{-1,-1},},
		/* to D */
		{{4,0},{4,1},{4,2},{4,3},{4,4},{3,4},{2,4},{1,4},{0,4},{-1,-1}}
	},
	/* from B */ {
		/* to A */
		{{6,4},{5,4},{4,4},{3,4},{2,4},{2,3},{2,2},{2,1},{2,0},{-1,-1}},
		/* to B */
		{{-1,-1},},
		/* to C */
		{{6,4},{5,4},{4,4},{4,5},{4,6},{-1,-1},},
		/* to D */
		{{6,4},{5,4},{4,4},{3,4},{2,4},{1,4},{0,4},{-1,-1},}
	},
	/* from C */ {
		/* to A */
		{{2,6},{2,5},{2,4},{2,3},{2,2},{2,1},{2,0},{-1,-1},},
		/* to B */
		{{2,6},{2,5},{2,4},{2,3},{2,2},{3,2},{4,2},{5,2},{6,2},{-1,-1}},
		/* to C */
		{{-1,-1},},
		/* to D */
		{{2,6},{2,5},{2,4},{1,4},{0,4},{-1,-1},}
	},
	/* from D */ {
		/* to A */
		{{0,2},{1,2},{2,2},{2,1},{2,0},{-1,-1},},
		/* to B */
		{{0,2},{1,2},{2,2},{3,2},{4,2},{5,2},{6,2},{-1,-1},},
		/* to C */
		{{0,2},{1,2},{2,2},{3,2},{4,2},{4,3},{4,4},{4,5},{4,6},{-1,-1}},
		/* to D */
		{{-1,-1},}
	}
};

static int is_position_outside(struct position pos)
{
	return (pos.row == -1 || pos.col == -1);
}

void check_unitstep(int *sum){		
	while(1){
		*sum = running_thread + blocking_thread;
		if(*sum == total){
			crossroads_step +=1;
			running_thread=0;		
			unitstep_changed();
		}
	}
}


/* return 0:termination, 1:success, -1:fail */
static int try_move(int start, int dest, int step, struct vehicle_info *vi)
{
	//running_thread +=1;
	
	struct position pos_cur, pos_next;

	pos_next = vehicle_path[start][dest][step];
	pos_cur = vi->position;

	if (vi->state == VEHICLE_STATUS_RUNNING) {
		/* check termination */
		if (is_position_outside(pos_next)) {
			/* actual move */
			vi->position.row = vi->position.col = -1;
			/* release previous */
			lock_release(&vi->map_locks[pos_cur.row][pos_cur.col]);
			return 0;
		}
	}

	int flag = 0;

	int into = into_crit(pos_cur.row, pos_cur.col, pos_next.row, pos_next.col);
	
	if(into){
		flag = semWait(vi, pos_next.row, pos_next.col);
	} 
	/* lock next position */
	
	if(flag) return 2;
	
	blocking_thread +=1;

	lock_acquire(&vi->map_locks[pos_next.row][pos_next.col]);
	blocking_thread -=1;
		
	//lock을 건 애가 임계 영역에 있으면 다른 애들을 막아야 함
	if (vi->state == VEHICLE_STATUS_READY) {
		/* start this vehicle */
		vi->state = VEHICLE_STATUS_RUNNING;
	} else {
		/* release current position */
		lock_release(&vi->map_locks[pos_cur.row][pos_cur.col]);
		if(out_crit(pos_next.row, pos_next.col)) semSignal(vi);
	}
	/* update position */
	vi->position = pos_next;
	
	return 1;
}

void init_on_mainthread(int thread_cnt){	
	total = thread_cnt;
	running_thread =0;
	blocking_thread =0;
	thread_create("thread M", PRI_MAX, check_unitstep, &sum);
	init_sema();
	/* Called once before spawning threads */
}

void vehicle_loop(void *_vi)
{
	int res;
	int start, dest, step;

	struct vehicle_info *vi = _vi;

	start = vi->start - 'A';
	dest = vi->dest - 'A';

	vi->position.row = vi->position.col = -1;
	vi->state = VEHICLE_STATUS_READY;

	step = 0;
	while (1) {
		/* vehicle main code */
		res = try_move(start, dest, step, vi);
		if (res == 1) {
			step++;
			running_thread +=1;
		}

		/* termination condition. */ 
		if (res == 0) {
			total-=1;			
			break;
		}

		if(res == 2){
			running_thread +=1;
		}
		/* unitstep change! */
		unitstep_changed();
	}	
	/* status transition must happen before sema_up */
	vi->state = VEHICLE_STATUS_FINISHED;
}
