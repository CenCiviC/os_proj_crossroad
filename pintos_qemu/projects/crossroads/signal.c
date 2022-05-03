#include "projects/crossroads/vehicle.h"
#include "projects/crossroads/signal.h"

struct sema{
    int value;
    char id;
};

//교차로의 semaphore
struct sema nw;
struct sema sw;
struct sema se;
struct sema ne;

void init_sema(void){
    nw.value = 1;
    nw.id = '0';
    sw.value = 1;
    sw.id = '0';
    se.value = 1;
    se.id = '0';
    ne.value = 1;
    ne.id = '0';
}

void semaphoreOperator(struct sema *s, char id){
    s->id = id;
    s->value =0;
}
int check_semaphore(struct sema *s){  
    return s->value;
}

//nw:1 ne:2 sw:4 se:8 
int check_route(struct vehicle_info *vi){
    char start = vi->start;
    char dest = vi->dest;

    if(start == 'A'){
        if(dest == 'B') return 4;
        else if(dest == 'C') return 12;
        else if(dest == 'D') return 14;
    }
    if(start == 'B'){
        if(dest == 'A') return 11;
        else if(dest == 'C') return 8;
        else if(dest == 'D') return 10;
    }
    if(start == 'C'){
        if(dest == 'B') return 7;
        else if(dest == 'A') return 3;
        else if(dest == 'D') return 2;
    }
    if(start == 'D'){
        if(dest == 'B') return 5;
        else if(dest == 'C') return 13;
        else if(dest == 'A') return 1;
    }
    return 0;
    //start, dest로 return 어디로 가는지 bit manipulation
}



int out_crit(int row,int col){
    if(!((row >=2) && (row <=4)) || !((col >=2) && (col <=4))) return 1;
    else return 0;
}

int into_crit(int cur_row ,int cur_col,int next_row,int next_col){
    if((cur_row == -1) && (cur_col == -1)) return 0;
    
    if(out_crit(cur_row, cur_col)){
        if( ((next_row == 2) || (next_row == 4)) && ((next_col==2) || (next_col ==4))) {
            return 1;
        }
        else return 0;
    }
    else return 0;
}

int semWait(struct vehicle_info *vi, int row, int col){
    switch (check_route(vi)){
        case 1:
            if(check_semaphore(&nw)) {semaphoreOperator(&nw, vi->id); return 0;}
            else return 1;
        case 2:
            if(check_semaphore(&ne)) {semaphoreOperator(&ne, vi->id); return 0;}
            else return 1;
        case 3:
            if(check_semaphore(&nw) && check_semaphore(&ne)) {semaphoreOperator(&nw, vi->id); semaphoreOperator(&ne, vi->id); return 0;}
            else return 1;
        case 4:
            if(check_semaphore(&sw)) {semaphoreOperator(&sw, vi->id); return 0;}
            else return 1;
        case 5:
            if(check_semaphore(&sw)&& check_semaphore(&nw)) {semaphoreOperator(&sw, vi->id); semaphoreOperator(&nw, vi->id);return 0;}
            else return 1;
         case 7:
            if(check_semaphore(&sw)&& check_semaphore(&ne)&& check_semaphore(&nw)) {semaphoreOperator(&sw, vi->id); semaphoreOperator(&ne, vi->id); semaphoreOperator(&nw, vi->id);return 0;}
            else return 1;
         case 8:
            if(check_semaphore(&se)) {semaphoreOperator(&se, vi->id); return 0;}
            else return 1;
         case 10:
            if(check_semaphore(&se)&& check_semaphore(&ne)) {semaphoreOperator(&se, vi->id); semaphoreOperator(&ne, vi->id);return 0;}
            else return 1;
         case 11:
            if(check_semaphore(&se)&& check_semaphore(&ne) && check_semaphore(&nw)) {semaphoreOperator(&nw, vi->id);semaphoreOperator(&se, vi->id); semaphoreOperator(&ne, vi->id);return 0;}
            else return 1;
         case 12:
            if(check_semaphore(&sw)&& check_semaphore(&se)) {semaphoreOperator(&sw, vi->id); semaphoreOperator(&se, vi->id);return 0;}
            else return 1;
         case 13:
            if(check_semaphore(&sw)&& check_semaphore(&nw)&& check_semaphore(&se)) {semaphoreOperator(&nw, vi->id);semaphoreOperator(&sw, vi->id); semaphoreOperator(&nw, vi->id);return 0;}
            else return 1;
         case 14:
            if(check_semaphore(&sw)&& check_semaphore(&se)&& check_semaphore(&ne)) {semaphoreOperator(&ne, vi->id);semaphoreOperator(&sw, vi->id); semaphoreOperator(&se, vi->id);return 0;}
            else return 1;
        default:
            return 2;
    }
    
    return 2;
}

void semSignal(struct vehicle_info *vi){
    if(vi->id == nw.id) {
        nw.id = '0';
        nw.value = 1;
    }
    if(vi->id == sw.id) {
        sw.id = '0';
        sw.value = 1;
    }
    if(vi->id == se.id) {
        se.id = '0';
        se.value = 1;
    }
    if(vi->id == ne.id) {
        ne.id = '0';
        ne.value = 1;
    }
}