#include "elevator.h"
#include <stdio.h>
#include <pthread.h>
#include <time.h>
//Hassan Ali Qadir

//sources/citation ---> 
//https://github.com/ecurri3/CS361HW5/blob/master/hw5.c (this doesnt work you can double check if you want I used this for refrence).
// I also refrenced last weeks lab that was giving me 41 seconds which was too slow. I will submit that lab code as well under the this code
//This is the logic I refered to http://laser.cs.umass.edu/verification-examples/elevator_con/elevator_con_1.html
//https://www.cs.rochester.edu/~kshen/csc256-spring2007/assignments/threadtest.cc


//Structure for 
typedef struct {
    pthread_mutex_t l;
    
    pthread_cond_t pexit;
    pthread_cond_t pentered;
    enum {yes = 1, no = 0} c;
    enum {yes2 = 1, no2 = 0} c2;

    int from_floor;
    int direction;
    int amount;
    enum {elevator_came=1, ele_open=2, ele_close=3} type;

    int requesting[PASSENGERS];
    int next_one;
    int num_requests;

    int amount_inside[PASSENGERS];
    int pas_inside;
} ELEV;

int which_ele[PASSENGERS];
int ind = 0;

ELEV e[ELEVATORS];


// The function scheduler init will change according to this
//In this function each elevator is put it into an array

//HELPER FUNCTIONS
/////////////////////////////////////////////////////////////
void helper_init(){
    int j = 0;
    while (j < ELEVATORS) {   //to for scheduler
        e[j].from_floor=0;		//initialzing elevator struct  --> refer to main.c for the strcuct [ELEVATOR]
        e[j].direction=-1;
        e[j].amount=0;
        e[j].c = no;
        e[j].c2 = no2;
        e[j].type = elevator_came;
        e[j].num_requests = 0;
        e[j].next_one = 0;
        e[j].pas_inside = -1;
        pthread_mutex_init(&e[j].l,0);
        pthread_cond_init(&e[j].pentered, 0);
        pthread_cond_init(&e[j].pexit, 0);
        int i = 0;
        while(i < PASSENGERS) {
            e[j].requesting[i] = -1;
            e[j].amount_inside[i] = -1;
            i++;
        }
        j++;
    }
}
int help_add(int from, int passenger){   
    which_ele[passenger] = ind++;
    e[which_ele[passenger]].requesting[passenger] = from;
    return which_ele[passenger];
}
    
int add_start(int from, int passenger) {
    if (ind == ELEVATORS) ind = 0;
    help_add(from, passenger);
}


//after asignmenting in my code



int help_next(int elev){
    ELEV *_e = &e[elev];
    int n = 0;
    while (_e->requesting[_e->next_one] == -1) {
            _e->next_one++;
            if (_e->next_one >= PASSENGERS)
                _e->next_one = 0;
            if (++n > PASSENGERS)
                return 0;
        }
        return _e->requesting[_e->next_one++];
}

int get_next_request(int elev) {
    ELEV *_e = &e[elev];
    if (_e->next_one >= PASSENGERS)
        _e->next_one = 0;
    help_next(elev);
}
void remove_request(int passenger) {
    e[which_ele[passenger]].requesting[passenger] = -1;
}
//////////////////////////////////////////////////////////////////////////////////////////////

void scheduler_init() {
    
    helper_init();
    for (int j = 0; j < PASSENGERS; j++)
        which_ele[j] = -1;
}

// Now each pasanger needs to run accordinly to the elevator list
// In this case we will use the code proivded in the main.c and run it with the multiple evelator
void passenger_request(int passenger, int from_floor, int to_floor, 
        void (*enter)(int, int), 
        void(*exit)(int, int))
{	
    int elev = add_start(from_floor, passenger);
    e[elev].num_requests++;
    int waiting = 1;
    
    //if the elevator is in the current floor as the passenger  
		//&&  if the elevator door opens for the passenger for pickup 
		//&& if elevator is empty
		//then allow the passenger inside of the elevator and increment of passangers 
    while(waiting) {
        pthread_mutex_l(&e[elev].l);
        ELEV *_e = &e[elev];
        if(_e->from_floor == from_floor && _e->type == ele_open && _e->amount==0) {
            enter(passenger, elev);
            _e->pas_inside = passenger;
            _e->amount_inside[_e->pas_inside] = to_floor;
            _e->amount++;
            waiting=0;
            _e->c = yes;
            pthread_cond_signal(&_e->pentered);
        }
        
        if (_e->from_floor == 0) {
            _e->c = yes;
            pthread_cond_signal(&_e->pentered);
        }
        pthread_mutex_unl(&e[elev].l);

        
    }
    int riding = 1;
 
            
    while(riding) {
        pthread_mutex_l(&e[elev].l);
        
        ELEV *_e = &e[elev];
            
        if(_e->from_floor == to_floor && _e->type == ele_open) {
          
            exit(passenger, elev);
            _e->amount--;
            _e->pas_inside = -1;
            _e->num_requests--;
            remove_request(passenger);
            riding=0;
        }
        pthread_mutex_unl(&e[elev].l);
        _e->c2 = yes2;
        pthread_cond_signal(&e[elev].pexit);
    }
}

int passenger_is_waiting_at_floor(int elevator, int floor)  {
    for(int i = 0; i < PASSENGERS; i++) {
        if (e[elevator].requesting[i] == floor)
            return 1;
    }
    return 0;
}

void elevator_ready(int elevator, int at_floor, void(*move_direction)(int, int), void(*door_open)(int), void(*door_close)(int)) {
  
    if(e[elevator].num_requests==0) return;
    ELEV *el = &e[elevator];
    pthread_mutex_l(&el->l);
    if(el->type == elevator_came) {
        door_open(elevator);
        el->type=ele_open;
    }
    else if(el->type == ele_open) {
        if (el->amount == 0 && el->num_requests > 0) {
            el->c = no;
            
            while (el->c == no && el->num_requests > 0 && passenger_is_waiting_at_floor(elevator, at_floor))
                pthread_cond_wait(&el->pentered, &el->l);
        } else if (el->pas_inside != -1 && el->amount_inside[el->pas_inside] == at_floor) {
            el->c2 = no2;
            while (el->c2 == no2)
                pthread_cond_wait(&el->pexit, &el->l);
        }
        door_close(elevator);
        el->type=ele_close;
    }
    //BASE CASES FOR ELEVATOR LEVEL CHECK
			//if elevator the very bottom of the building
			//----> manupulate the direction
    else {
        int old_direction = el->direction;
        if (el->amount != 0) {
            int to_floor = el->amount_inside[el->pas_inside];
            el->direction = to_floor-at_floor;
        } 
        else if (el->num_requests > 0) {
            int to_floor = get_next_request(elevator);
            
            el->direction = to_floor-at_floor;
        }
        else if (at_floor<=0 || at_floor>=FLOORS-1) {
            el->direction *=- 1;
        }
        //if the elevator has to down up i.e if our next pick up is on the floor below us 
			//&& if we are moving down with an empty elevator
		   //|| the pickup is in the range of all the possible floor we can go to
			// Decrement  the direction of elevator a floor above
        if (at_floor + el->direction < FLOORS && at_floor+ el->direction > -1)
            move_direction(elevator,el->direction);
        el->from_floor=at_floor+el->direction;
        if (el->direction != 1 && el->direction != -1) {
            el->direction = old_direction;
            if (at_floor == 0)
                el->direction = 1;
            else if (at_floor >= FLOORS-1)
                el->direction = -1;
        }
        el->type=elevator_came;
        
    }
    pthread_mutex_unl(&el->l);
}
