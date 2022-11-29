#include "elevator.h"
#include <stdio.h>
#include <pthread.h>
#include <time.h>
//Hadi Usmani
//CS361
//Concurrent Elevator Problem
//sources/citation ---> 
//https://github.com/ecurri3/CS361HW5/blob/master/hw5.c (this doesnt work you can double check if you want I used this for refrence).
// I also refrenced last weeks lab that was giving me 41 seconds which was too slow. I will submit that lab code as well under the this code
//This is the logic I refered to http://laser.cs.umass.edu/verification-examples/elevator_con/elevator_con_1.html
//https://www.cs.rochester.edu/~kshen/csc256-spring2007/assignments/threadtest.cc

typedef struct {
    pthread_mutex_t lock;
    
    pthread_cond_t passenger_has_exited;
    pthread_cond_t passenger_has_entered;
    int is_occupied;
    enum {yes = 1, no = 0} condition;
    enum {yes2 = 1, no2 = 0} condition2;

    int current_floor;
    int direction;
    int occupancy;
    enum {ELEVATOR_ARRIVED=1, ELEVATOR_OPEN=2, ELEVATOR_CLOSED=3} state;

    int pas_req[PASSENGERS];
    int next_requested_index;
    int num_requests;

    int passengers_riding[PASSENGERS];
    int current_passenger;
} ELEV;

int whos_on_what_elevator[PASSENGERS];
int assignment_index = 0;

ELEV e[ELEVATORS];


// The function scheduler init will change according to this
//In this function each elevator is put it into an array

//HELPER FUNCTIONS
/////////////////////////////////////////////////////////////
void helper_init(){
    int j = 0;
    while (j < ELEVATORS) {   //to for scheduler
        e[j].current_floor=0;		//initialzing elevator struct  --> refer to main.c for the strcuct [ELEVATOR]
        e[j].direction=-1;
        e[j].occupancy=0;
        e[j].condition = no;
        e[j].condition2 = no2;
        e[j].state = ELEVATOR_ARRIVED;
        e[j].num_requests = 0;
        e[j].next_requested_index = 0;
        e[j].current_passenger = -1;
        pthread_mutex_init(&e[j].lock,0);
        pthread_cond_init(&e[j].passenger_has_entered, 0);
        pthread_cond_init(&e[j].passenger_has_exited, 0);
        int i = 0;
        while(i < PASSENGERS) {
            e[j].pas_req[i] = -1;
            e[j].passengers_riding[i] = -1;
            i++;
        }
        j++;
    }
}
int help_add(int from, int passenger){   
    whos_on_what_elevator[passenger] = assignment_index++;
    e[whos_on_what_elevator[passenger]].pas_req[passenger] = from;
    return whos_on_what_elevator[passenger];
}
    
int add_start(int from, int passenger) {
    if (assignment_index == ELEVATORS) assignment_index = 0;
    help_add(from, passenger);
}

int help_next(int elev){
    ELEV *_e = &e[elev];
    int n = 0;
    while (_e->pas_req[_e->next_requested_index] == -1) {
            _e->next_requested_index++;
            if (_e->next_requested_index >= PASSENGERS)
                _e->next_requested_index = 0;
            if (++n > PASSENGERS)
                return 0;
        }
        return _e->pas_req[_e->next_requested_index++];
}

int get_next_request(int elev) {
    ELEV *_e = &e[elev];
    if (_e->next_requested_index >= PASSENGERS)
        _e->next_requested_index = 0;
    help_next(elev);
}
void remove_request(int passenger) {
    e[whos_on_what_elevator[passenger]].pas_req[passenger] = -1;
}
//////////////////////////////////////////////////////////////////////////////////////////////

void scheduler_init() {
    
    helper_init();
    for (int j = 0; j < PASSENGERS; j++)
        whos_on_what_elevator[j] = -1;
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
    int x = 0;
    //if the elevator is in the current floor as the passenger  
		//&&  if the elevator door opens for the passenger for pickup 
		//&& if elevator is empty
		//then allow the passenger inside of the elevator and increment of passangers 
    while(waiting) {
        pthread_mutex_lock(&e[elev].lock);
        ELEV *_e = &e[elev];
        if(_e->current_floor == from_floor && _e->state == ELEVATOR_OPEN && _e->occupancy==0) {
            enter(passenger, elev);
            _e->current_passenger = passenger;
            _e->passengers_riding[_e->current_passenger] = to_floor;
            _e->occupancy++;
            waiting=0;
            _e->condition = yes;
            pthread_cond_signal(&_e->passenger_has_entered);
        }
        
        if (_e->current_floor == 0) {
            _e->condition = yes;
            pthread_cond_signal(&_e->passenger_has_entered);
        }
        pthread_mutex_unlock(&e[elev].lock);

        
    }
    int riding = 1;
 
            
    while(riding) {
        pthread_mutex_lock(&e[elev].lock);
        
        ELEV *_e = &e[elev];
            
        if(_e->current_floor == to_floor && _e->state == ELEVATOR_OPEN) {
          
            exit(passenger, elev);
            _e->occupancy--;
            _e->current_passenger = -1;
            _e->num_requests--;
            remove_request(passenger);
            riding=0;
        }
        pthread_mutex_unlock(&e[elev].lock);
        _e->condition2 = yes2;
        pthread_cond_signal(&e[elev].passenger_has_exited);
    }
}

int passenger_is_waiting_at_floor(int elevator, int floor)  {
    for(int i = 0; i < PASSENGERS; i++) {
        if (e[elevator].pas_req[i] == floor)
            return 1;
    }
    return 0;
}

void elevator_ready(int elevator, int at_floor, 
        void(*move_direction)(int, int), 
        void(*door_open)(int), void(*door_close)(int)) {
  
    if(e[elevator].num_requests==0) return;
    ELEV *el = &e[elevator];
    pthread_mutex_lock(&el->lock);
    if(el->state == ELEVATOR_ARRIVED) {
        door_open(elevator);
        el->state=ELEVATOR_OPEN;
    }
    else if(el->state == ELEVATOR_OPEN) {
        if (el->occupancy == 0 && el->num_requests > 0) {
            el->condition = no;
            
            while (el->condition == no && el->num_requests > 0 && passenger_is_waiting_at_floor(elevator, at_floor))
                pthread_cond_wait(&el->passenger_has_entered, &el->lock);
        } else if (el->current_passenger != -1 && el->passengers_riding[el->current_passenger] == at_floor) {
            el->condition2 = no2;
            while (el->condition2 == no2)
                pthread_cond_wait(&el->passenger_has_exited, &el->lock);
        }
        door_close(elevator);
        el->state=ELEVATOR_CLOSED;
    }
    //BASE CASES FOR ELEVATOR LEVEL CHECK
			//if elevator the very bottom of the building
			//----> manupulate the direction
    else {
        int old_direction = el->direction;
        if (el->occupancy != 0) {
            int to_floor = el->passengers_riding[el->current_passenger];
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
        el->current_floor=at_floor+el->direction;
        if (el->direction != 1 && el->direction != -1) {
            el->direction = old_direction;
            if (at_floor == 0)
                el->direction = 1;
            else if (at_floor >= FLOORS-1)
                el->direction = -1;
        }
        el->state=ELEVATOR_ARRIVED;
        
    }
    pthread_mutex_unlock(&el->lock);
}
