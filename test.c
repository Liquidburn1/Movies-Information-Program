#include "elevator.h"
#include <stdio.h>
#include <pthread.h>
#include <time.h>

//struct

typedef struct
{

   
    
    
    int this_floor; //current floor
    int direction; //direction of the elevaotr
    int amount; //amount of passengers 
    //elevator has come, elevator is open and it is closed
    enum{elevator_came=1,ele_open=2,ele_close=3} type; //what is happeing to the elevator 
    pthread_mutex_t l; //this will be the lock 
    pthread_cond_t pentered; //passenger entered
    pthread_cond_t pexit; //passenger exited
    int requesting[PASSENGERS]; //passengers requesting
    int next_one; //the next index that we need
    int floor_to_go[PASSENGERS]; //number of passengers inside the elevator
    int pas_inside; //The passenger which is inside right now
    int num_req; //number if requests from the passengers
    enum{y=1,n=0} c;
    enum{y1=1,n1=0} c2;

} ele;

int which_ele[PASSENGERS];
int ind=0; //index

ele elevat[ELEVATORS]; //this will crreate the array of struct 


// Putting the elevators into the array
void sch_initialzer()
{
    int i =0;

    //initializing the structure of the elevators
    while(i<=ELEVATORS-1)
    {
        elevat[i].this_floor=0;		
        elevat[i].direction=-1;
        elevat[i].pas_inside = -1;
        elevat[i].c = n;
        elevat[i].c2 = n1;
        elevat[i].type = elevator_came;
        elevat[i].next_one = 0;
        elevat[i].num_req = 0;
        elevat[i].amount=0;
        pthread_mutex_init(&elevat[i].l,0);
        pthread_cond_init(&elevat[i].pentered, 0);
        pthread_cond_init(&elevat[i].pexit, 0);
        int k=0;
        while(k<=PASSENGERS-1)
        {
            elevat[i].floor_to_go[i] =-1;
            elevat[i].requesting[k] = -1;

            k=k+1;
        }
        i=i+1;

    }
}
    int help_add(int from, int passenger){   
    which_ele[passenger] = ind++;
    elevat[which_ele[passenger]].requesting[passenger] = from;
    return which_ele[passenger];
}
    
int add_start(int from, int passenger) {
    if (ind == ELEVATORS) ind = 0;
    help_add(from, passenger);
}


//after asignmenting in my code



int help_next(int elev){
    ele *_e = &elevat[elev];
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
    ele *_e = &elevat[elev];
    if (_e->next_one >= PASSENGERS)
        _e->next_one = 0;
    help_next(elev);
}
void remove_request(int passenger) {
    elevat[which_ele[passenger]].requesting[passenger] = -1;
}
//////////////////////////////////////////////////////////////////////////////////////////////

void scheduler_init()
    {
        sch_initialzer();
        int i=0;
        while(i<PASSENGERS)
        {

            which_ele[i]=-1;
            i=i+1;
        }

    }

int passenger_is_waiting_at_floor(int elevator, int floor)  {
    for(int i = 0; i < PASSENGERS; i++) {
        if (elevat[elevator].requesting[i] == floor)
            return 1;
    }
    return 0;
}


   //each passenger will go as the elevator list
    void passenger_request(int passenger, int from_floor, int to_floor, void (*enter)(int, int), void(*exit)(int, int))
    {

        int temp=add_start(from_floor,passenger);
        elevat[temp].num_req=elevat[temp].num_req+1;
        
        //if the passenger is at the floor it should waut for the passenger to get in
        int idle=1;
        
        while(idle==1)
        {
            pthread_mutex_lock(&elevat[temp].l);
            ele *tem = &elevat[temp];
            if(tem->this_floor==0)
            {
               tem->c=y; 
                pthread_cond_signal(&tem->pentered);
            }


            if(tem->type==ele_open && tem->amount==0 && tem->this_floor==from_floor)
            {
                tem->pas_inside=passenger;
                tem->floor_to_go[tem->pas_inside]=to_floor;
                tem->amount=tem->amount+1;
                enter(passenger,temp);
                idle=-1;
                tem->c=y;
                pthread_cond_signal(&tem->pentered);
            }
            pthread_mutex_unlock(&elevat[temp].l);


        } 

        //when the passenger is riding and going to the desired floor
        int inside_it=1;

        while(inside_it==1)
        {
            pthread_mutex_lock(&elevat[temp].l);
            ele *tem=&elevat[temp];
            //if its at the desired floor 
            if(tem->type==ele_open && tem->this_floor==to_floor)
            {
            exit(passenger,temp);
            tem->amount=tem->amount-1;
            tem->num_req=tem->num_req-1;
            tem->pas_inside=-1;
            remove_request(passenger);
            inside_it=-1;
            }
        
        pthread_mutex_unlock(&elevat[temp].l);

        tem->c2=y1;
        pthread_cond_signal(&elevat[temp].pexit);
        }


    }

    void elevator_ready(int elevator, int at_floor, void(*move_direction)(int, int), void(*door_open)(int), void(*door_close)(int))
    {
        if(elevat[elevator].num_req==0)
        {
            return;
        }

        ele *temp = &elevat[elevator];
        pthread_mutex_lock(&temp->l);
        
        //checks for the passenger and weather the pasenger is in or not
        if(temp->type==ele_open)
        {

            if (temp->amount == 0 && temp->num_req > 0) {
            temp->c = n;
            
            //if passenger is at the floor
            while (passenger_is_waiting_at_floor(elevator, at_floor)==1 && temp->c == n && temp->num_req > 0)
                pthread_cond_wait(&temp->pentered, &temp->l);
        }

        //checking if there is a passenger inside we are using -1 since pas inside holds the passenger num
            else if(temp->floor_to_go[temp->pas_inside]==at_floor && temp->pas_inside!=-1) 
            {
                temp->c2=n1;
                while(temp->c2==n1)
                {
                    pthread_cond_wait(&temp->pexit,&temp->l);

                }
                

            }
            temp->type=ele_close;
            door_close(elevator);

        }
        //open the door if elevator is there
        else if(temp->type==elevator_came)
        {
            temp->type=ele_open;
            door_open(elevator);
        }
        
        else
        {
            int direc = temp->direction;
            
            if(at_floor<1 || FLOORS-1<=at_floor)
            {
                temp->direction=temp->direction * -1;
            }

            if(temp->amount!=0)
            {
                int to_floor=temp->floor_to_go[temp->pas_inside];
                temp->direction=to_floor-at_floor;
                

            }

            else if(temp->num_req>0)
            {
                int to_floor = get_next_request(elevator);
                temp->direction=to_floor-at_floor;

            }


            if(at_floor+temp->direction>-1 && at_floor+temp->direction<FLOORS)
            {move_direction(elevator,temp->direction);}
            temp->this_floor=at_floor+temp->direction;
            if(temp->direction != 1 && temp->direction !=-1)
            {
               temp->direction = direc;
            if (at_floor == 0)
                temp->direction = 1;
            else if (at_floor >= FLOORS-1)
                temp->direction = -1;
        }
        temp->type=elevator_came;




         
        }
        pthread_mutex_unlock(&temp->l);
    } 



    


