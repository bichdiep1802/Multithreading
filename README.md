#Traffic Controller

-----------------------
#Description

When there's a construction and a lane is closed of a two-land road, a controlling system is directing traffic coming from the North and the South to maximize the work and to prevent deadlock. A deadlock could either be that the controller does not allow traffic through from either side, or lets traffic through from both sides causing an accident.

- Assuming that when a car arrives, there is an 80% chance another car is following it, but once no car comes, there is a 20 second delay before any new car will come. 
- During the times when no cars are at either end, the controller will be idle, requiring the first car that arrives to blow their horn. 
- When a car arrives at either end, the controller will allow traffic from that side to continue to flow, until there are no more cars, or until there are 10 cars or more lined up on the opposing side, at which time they will be allowed to pass.
- Each car takes 1 second to go through the construction area.

These assumption is to replace sensors that detecting incoming cars.

-------------------------
#Implementation

- Multithreading (3 threads): 2 threads for cars, each of which is working on cars from 1 direction. 
- Consumers and producers idea. Incoming cars are producers and the traffic controller is a consumer.
- Mutex lock + cond variable to prevent deadlock/critical sections
- Coding in C++
- The results are writtent to a .log file
