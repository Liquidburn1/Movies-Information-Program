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
    //assigning using index
    int assignmenting(int f,int pass)
    {

        if(ind==ELEVATORS)
        {
            ind=0;
        }
        which_ele[pass]=ind++;
        elevat[which_ele[pass]].requesting[pass]=f; //putting which floor 
        return which_ele[pass];
        //then returns which elevator the passenger is on


    }

    //removing the request of tje
    void remove_it(int pass)
    {
        elevat[which_ele[pass]].requesting[pass]=-1;

    }

    int tempfunc(int elevtor_on)
    {
        ele *temp = &elevat[elevtor_on];
        int i=0;
        while(temp->requesting[temp->next_one]==-1)
        {
            temp->next_one=temp->next_one+1;
            if(temp->next_one>=PASSENGERS){temp->next_one=0;}
            if(i+1>PASSENGERS){return 0;}



        }
        return temp->requesting[temp->next_one++];

    }
    
    //getting the next request from the passengers
    int get_next_req(int elevator_on)
    {
        ele *temp = &elevat[elevator_on];
        if(temp->next_one>=PASSENGERS){temp->next_one=0;}
        tempfunc(elevator_on);

    }

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


    //this function will check if there is a passenger on the floor or not returns -1 or 1
    int checker(int floor,int el)
    {
        int i =0;
        while(i<PASSENGERS)
        {
            if(elevat[el].requesting[i]==floor)
            {
                return 1;
            }
            i=i+1;
        }
        return -1;
    }




    //each passenger will go as the elevator list
    void passenger_request(int passenger, int from_floor, int to_floor, void (*enter)(int, int), void(*exit)(int, int))
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
            while (checker(elevator, at_floor)==1 && temp->c == n && temp->num_req > 0)
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
                int to_floor = get_next_req(elevator);
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


    


