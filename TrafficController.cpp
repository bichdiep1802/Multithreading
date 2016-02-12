//Bich Diep
// 07.05.2015

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <queue>
#include <ctime>
#include <signal.h>
#include <sys/types.h>
#include <fstream>
using namespace std;

// define value for NORTH AND SOUTH directions
#define NORTH 0
#define SOUTH 1

// methods
string getTime(); // get current time
int pthread_sleep (int seconds); // pause the current thread for given seconds
void *produceCars (void *q); // produce car for assigned direction
void *worker (void *q); // the worker control the traffic
void handler_SIGINT(); // use a signal handler for CTRL-C
void cleanUp();
bool create_pThread(); // create north and south cars thread and worker thread

// a Car struct that holds info of a car
struct Car
{
    int car_ID;
    string arriveTime;
    string startTime;
    string endTime;

    Car(int i, string arTime)
    {
        car_ID = i;
        arriveTime = arTime;
    }
};

// mutex lock and condition variable
pthread_mutex_t job_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  job_queue_cond = PTHREAD_COND_INITIALIZER;
// create 3 threads: 2 producers(cars at 2 directions) and 1 consumer (flag person)
pthread_t threads_container[3];

// generating car ID, each direction has its own ID
int id[2] = {0, 0};
// chars direction arrays
char direc[2] = {'N','S'};
// array holding 2 queues that store cars from 2 directions
queue<Car> northSouth[2];
// keep track the lifetime of the program
bool done = false;
// output files
ofstream flagpersonFile;
ofstream carFile;

// generate and return the current time under string type
string getTime(){
    int t_len = 200;
    time_t rawtime;
    struct tm * timeinfo;
    char tbuffer[t_len];

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (tbuffer,t_len,"%T",timeinfo);
    string str(tbuffer);
    return str;
}

// pthread_sleep takes an integer number of seconds to pause the current thread
int pthread_sleep (int seconds)
{
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;

    if(pthread_mutex_init(&mutex,NULL))
    {
        return -1;
    }

    if(pthread_cond_init(&conditionvar,NULL))
    {
        return -1;
    }

    //When to expire is an absolute time, so get the current time and add
    //it to our delay time
    timetoexpire.tv_sec = (unsigned int)time(NULL) + seconds;
    timetoexpire.tv_nsec = 0;
    return pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
}

// use a signal handler for CTRL-C
void handler_SIGINT() {
    done = true;
}

// produce cars at the assigned direction
// param: q - the direction needed to produce cars
void *produceCars (void *q)
{
    long direction = (long) q ;

    // keep produce till catch exit request
    while(!done) {
        // 80% cars will be produced, otherwise, sleep for 20s until the new car comes
        if(rand() % 10 < 8)
        {
            /***************************/
            // assign a lock
            pthread_mutex_lock (&job_queue_mutex);
            // produce car
            northSouth[direction].push(Car(id[direction]++, getTime()));
            // notifys the worker
            if (northSouth[direction].size()>0) {
                pthread_cond_signal(&job_queue_cond);
            }
            // release the lock
            pthread_mutex_unlock (&job_queue_mutex);
            /***************************/

        } else{
            pthread_sleep(20);
        }

    }
    return (NULL);
}

// the worker control the traffic
// param: initial direction for the flowing cars - decide which direction can go at the first time
void *worker (void *q)
{
    long currDir = (long)q;

    // keep control the traffic till catch exit request
    while(!done) {

        string worker_sleepTime = "";
        // keep track of the status of the worker
        bool justSlept = false;
        string flagPerson = "";

        /**********************************************************************/
        pthread_mutex_lock (&job_queue_mutex);
        // wait till a car comes
        while (northSouth[0].size()==0 && northSouth[1].size()==0) {
            justSlept = true;
            worker_sleepTime = getTime();
            pthread_cond_wait(&job_queue_cond, &job_queue_mutex);
        }

        // changing direction if there are no more cars,
        // or there are 10 cars or more lined up on the opposite side
        if (northSouth[currDir].size() < 1 || northSouth[1-currDir].size() > 9)
            currDir = 1-currDir;

        // picking the front car from this direction to go
        Car thisCar = northSouth[currDir].front();
        northSouth[currDir].pop();

        pthread_mutex_unlock (&job_queue_mutex);
        /**********************************************************************/

        string startTime = getTime();
        pthread_sleep(1); // takes 1 second to go through the construction area
        string endTime = getTime();

        // only output his/her status, if the worker just wake up from a sleep
        if(justSlept)
        {
            flagpersonFile << worker_sleepTime << " \t sleep\n"; // output the worker's status
            if(worker_sleepTime !=startTime)
                // woken-up time = start time of the car
                flagpersonFile << startTime << " \t woken-up\n";
        }

        // output output all of the necessary events of this car
        carFile << "   " << thisCar.car_ID << "\t    " << direc[currDir] <<  "    \t"<< thisCar.arriveTime
        << "    \t" << startTime << "    \t"  << endTime << "\n";

    }
    return (NULL);
}

// creating pthread for cars and worker
// param: a reference of the container containing the threads
bool create_pThread()
{
    // cars of north direction threat
    if ( -1 == pthread_create (&threads_container[0], NULL, produceCars, (void*) NORTH))
    {
        perror("pthread_create for north");
        return false;
    }
    // cars of south direction threat
    if ( -1 == pthread_create (&threads_container[1], NULL, produceCars, (void*) SOUTH))
    {
        perror("pthread_create for south");
        return false;
    }
    // worker threat
    if ( -1 == pthread_create(&threads_container[2], NULL, worker, ((rand()+1011)%2==1)?(void*)NORTH:(void*)SOUTH))
    {
        perror("pthread_create for consumer");
        return false;
    }
    return true;
}

// cleaning up the threads and closing the output files
// param: a reference of the container containing the threads
void cleanUp()
{
    for (int i = 0; i < 3; i++)
    {
        pthread_cancel(threads_container[i]);
        pthread_join (threads_container[i], NULL);

    }
    //Destroy threads
    pthread_mutex_destroy(&job_queue_mutex);
    pthread_cond_destroy(&job_queue_cond);

    // Close .log files
    flagpersonFile.close();
    carFile.close();
}

int main ()
{
    printf("START THE PROGRAM - USE CTRL-C TO EXIT\n");

    /* set up the signal handler */
    struct sigaction handler;
    handler.sa_handler = (void(*)(int))handler_SIGINT;
    sigaction(SIGINT, &handler, NULL);

    // creating 2 output files to output all of the necessary events of cars and the flag person
    flagpersonFile.open ("flagperson.log");
    flagpersonFile << "Time \t\t State \n";
    carFile.open ("car.log");
    carFile <<"carID   direction\tarrive-time\tstart-time\tend-time \n";

    if(!create_pThread()) return -1;

    // Wait for SIGINT to arrive, clean up, and exit.
    pause();
    printf("\n...SIGNAL RECEIVED...\n");
    // Cleaning up and exit
    cleanUp();
    printf("\n...EXITING...\n");
    printf("\n...END THE PROGRAM...\n");


    return 0;
}
