/*
 * Copyright (c) 2006 The Regents of The University of Michigan
 * Copyright (c) 2013 Advanced Micro Devices, Inc.
 * Copyright (c) 2013 Mark D. Hill and David A. Wood
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Nathan Binkert
 *          Steve Reinhardt
 */

#include "sim/simulate.hh"

#include <mutex>
#include <thread>

#include "base/logging.hh"
#include "base/pollevent.hh"
#include "base/types.hh"
#include "sim/async.hh"
#include "sim/eventq_impl.hh"
#include "sim/sim_events.hh"
#include "sim/sim_exit.hh"
#include "sim/stat_control.hh"
#include "cpu/simple/atomic.hh" //zyc

extern  __thread double t1_lock; //zyc
extern  __thread double t2_unlock; //zyc
extern  __thread long long unsigned int count_lock; //zyc

//! Mutex for handling async events.
std::mutex asyncEventMutex;

//! Global barrier for synchronizing threads entering/exiting the
//! simulation loop.
Barrier *threadBarrier;

bool is_enter = true; //zyc

//! forward declaration
Event *doSimLoop(EventQueue *);

/**
 * The main function for all subordinate threads (i.e., all threads
 * other than the main thread).  These threads start by waiting on
 * threadBarrier.  Once all threads have arrived at threadBarrier,
 * they enter the simulation loop concurrently.  When they exit the
 * loop, they return to waiting on threadBarrier.  This process is
 * repeated until the simulation terminates.
 */
static void
thread_loop(EventQueue *queue)
{
    while (true) {
        threadBarrier->wait();
        doSimLoop(queue);
        printf("lock time is %lf \n",t1_lock); //zyc
        printf("unlock time is %lf \n",t2_unlock); //zyc
        printf("cishu is %llu\n",count_lock); //zyc
        //threadBarrier->wait(); //zyc
        //Event *local_event_ = doSimLoop(queue);  //zyc
        //assert(local_event_ != NULL) ;//zyc
        //BaseGlobalEvent *global_event_ = local_event_->globalEvent(); //zyc
        //assert(global_event_!= NULL); //ZYC
        //threadBarrier->wait(); //zyc
        //GlobalSimLoopExitEvent *global_exit_event_ =
          //        dynamic_cast<GlobalSimLoopExitEvent *>(global_event_);
           // assert(global_exit_event_ != NULL);


    }
}

GlobalSimLoopExitEvent *simulate_limit_event = nullptr;  // annonated by zyc

//std:: vector<GlobalSimLoopExitEvent *> simulate_limit_event;  //zyc

/** Simulate for num_cycles additional cycles.  If num_cycles is -1
 * (the default), do not limit simulation; some other event must
 * terminate the loop.  Exported to Python.
 * @return The SimLoopExitEvent that caused the loop to exit.
 */

Tick numcycle; //zyc
GlobalSimLoopExitEvent *
simulate(Tick num_cycles)
{
    // The first time simulate() is called from the Python code, we need to
    // create a thread for each of event queues referenced by the
    // instantiated sim objects.
    static bool threads_initialized = false;
    static std::vector<std::thread *> threads;

    if (!threads_initialized) {
        //numMainEventQueues = 2; //add by zyc
        threadBarrier = new Barrier(numMainEventQueues);

        // the main thread (the one we're currently running on)
        // handles queue 0, so we only need to allocate new threads
        // for queues 1..N-1.  We'll call these the "subordinate" threads.
        for (uint32_t i = 1; i < numMainEventQueues; i++) {
            threads.push_back(new std::thread(thread_loop, mainEventQueue[i]));
            //threads[i-1]->join(); //zyc
            //delete threads[i-1]; //zyc
        }

        printf("The number of thread is : %d\n",numMainEventQueues);//added by zyc

        threads_initialized = true;
        simulate_limit_event =
            new GlobalSimLoopExitEvent(mainEventQueue[0]->getCurTick(),
                                       "simulate() limit reached", 0);
       // simulate_limit_event.push_back(t1);   //zyc
    }
    numcycle = num_cycles; //zyc
    inform("Entering event queue @ %d.  Starting simulation...\n", curTick());
    printf("getCurTick is %lu \n",mainEventQueue[0]->getCurTick()); //addded by zyc

    if (num_cycles < MaxTick - curTick())
        num_cycles = curTick() + num_cycles;
    else // counter would roll over or be set to MaxTick anyhow
        num_cycles = MaxTick;
    printf("num_cycles is %lu \n",num_cycles);//added by zyc
    simulate_limit_event->reschedule(num_cycles); //anoed by zyc
    //simulate_limit_event->reschedule(mainEventQueue[0]->getCurTick()); //zyc
//#########################################################################
  //  tested by zyc #####################################################
    //GlobalSimLoopExitEvent *t2= 
      //      new GlobalSimLoopExitEvent(mainEventQueue[1]->getCurTick(),
        //                                "simulate() limit reached",1);
    //simulate_limit_event.push_back(t2);//zyc
    //inform("Entering event queue @ %d. Starting simulation...\n", curTick());

    //simulate_limit_event[0]->reschedule(num_cycles);


//##########################################################################

    simQuantum = 0; //tested by zyc
    GlobalSyncEvent *quantum_event = NULL;
    //simQuantum = 5; //added by zyc
  if (numMainEventQueues > 1) {   //zyc annonated
        if (simQuantum == 0) {
            //fatal("Quantum for multi-eventq simulation not specified");
        }

        //quantum_event = new GlobalSyncEvent(curTick() + simQuantum, simQuantum,
          //                  EventBase::Progress_Event_Pri, 0);  

        printf("quantum_event runned \n");  //added by zyc
        inParallelMode = true;
    }

    // all subordinate (created) threads should be waiting on the
    // barrier; the arrival of the main thread here will satisfy the
    // barrier, and all threads will enter doSimLoop in parallel


    printf("begin to wait \n"); //zyc
    threadBarrier->wait();   
    printf("wait end \n");  //zyc
    //is_enter = false; //zyc
    Event *local_event = doSimLoop(mainEventQueue[0]);
    /*for (int i =0 ;i < numMainEventQueues-1 ;i++)
    {
      threads[i]->join(); //zyc
    } */
    printf("get local_event \n"); //zyc
    //threads[0]->join();  //added by zyc
    //Event *local_event = doSimLoop(mainEventQueue[0]); //added by zyc
    //threads[1]->join();  //added by zyc
    assert(local_event != NULL);

    printf("lock time is %lf \n", t1_lock); //zyc
    printf("unlock time is %lf \n", t2_unlock); //zyc
    printf("次数 is %llu \n", count_lock); //zyc

    inParallelMode = true;//以前是false

    // locate the global exit event and return it to Python
    BaseGlobalEvent *global_event = local_event->globalEvent();
    assert(global_event != NULL);
    //threadBarrier->wait(); //zyc

    GlobalSimLoopExitEvent *global_exit_event =
        dynamic_cast<GlobalSimLoopExitEvent *>(global_event);
    assert(global_exit_event != NULL);

    //! Delete the simulation quantum event.
    if (quantum_event != NULL) {
        quantum_event->deschedule();
        delete quantum_event;
    }
   // threads[0]->join(); //zyc
    return global_exit_event;
}

/**
 * Test and clear the global async_event flag, such that each time the
 * flag is cleared, only one thread returns true (and thus is assigned
 * to handle the corresponding async event(s)).
 */
static bool
testAndClearAsyncEvent()
{
    bool was_set = false;
    asyncEventMutex.lock();

    if (async_event) {
        was_set = true;
        async_event = false;
    }

    asyncEventMutex.unlock();
    return was_set;
}

/**
 * The main per-thread simulation loop. This loop is executed by all
 * simulation threads (the main thread and the subordinate threads) in
 * parallel.
 */
Event *
doSimLoop(EventQueue *eventq)
{
    // set the per thread current eventq pointer
    //std::lock_guard<EventQueue> lock(*eventq); //zyc
    curEventQueue(eventq);
    eventq->handleAsyncInsertions();
    //if(eventq != mainEventQueue[0]) //zyc
//    {
  //    printf("queue0 enter dosimLoop! \n"); //zyc
   // }
   // else //zyc
      //printf("queue1 enter dosimLoop ! \n");  //zyc
   // printf("处理了异步插入！\n");//added by zyc

    while (1) {
        // there should always be at least one event (the SimLoopExitEvent
        // we just scheduled) in the queue
        //assert(!eventq->empty());
      /*  if(mainEventQueue[1]->empty())  //zyc
        {
          //printf ("queue1 is empty !\n"); //zyc
         // continue; //zyc
        }
        if(mainEventQueue[0]->empty()) //zyc
        {
          //printf("queue0 is empty !\n"); //zyc
          //continue; //zyc
        }
      //  if(eventq == mainEventQueue[0])  //zyc
        //  printf("queue0 enter while ! \n"); //zyc
      //  else  //zyc
        //  printf("queue1 enter while ! \n"); //zyc*/
        //assert(curTick() <= eventq->nextTick() &&
              // "event scheduled in the past");

        if (async_event && testAndClearAsyncEvent()) {
            // Take the event queue lock in case any of the service
            // routines want to schedule new events.
            std::lock_guard<EventQueue> lock(*eventq);
            if(eventq == mainEventQueue[0]) //zyc
              printf("queue 0 enter if(async_event)!\n") ;//zyc
            else
              printf("queue 1 enter if(async_event)!\n"); //zyc
            //printf("begin to lock \n"); //zyc
            if (async_statdump || async_statreset) {
                printf("enter async_statdump or async_statreset \n"); //zyc
                Stats::schedStatEvent(async_statdump, async_statreset);
                async_statdump = false;
                async_statreset = false;
            }

            if (async_io) {
                if(eventq == mainEventQueue[0]) //zyc
                  printf("queue 0 enter if(async_io)!\n"); //zyc
                else
                  printf("queue 1 enter if(async_io)!\n"); //zyc
                async_io = false;
              //  printf("pollQueue begin to service \n");//zyc
                pollQueue.service();
               // printf("pollQueue service end \n"); //zyc
            }

            if (async_exit) {
                async_exit = false;
                if(eventq == mainEventQueue[1])  //zyc
                  printf ("queue1 interrupt received !\n"); //zyc
                else
                  printf ("queue0 interrupt received !\n"); //zyc
                exitSimLoop("user interrupt received");
            }

            if (async_exception) {
               // printf("enter async_exception \n"); //zyc
                async_exception = false;
                return NULL;
            }
        }
       // printf("begin to call serviceone \n"); //zyc
       // if(eventq == mainEventQueue[1])  //zyc
          //printf("queue 0 enter serviceOne! \n"); //zyc
        //else
         // printf("queue 1 enter serviceOne! \n"); //zyc
        Event *exit_event = eventq->serviceOne();
        //Event *exitevent = NULL ; //zyc
       // printf("serviceone end \n"); //zyc
        int k; //zyc
        if (exit_event != NULL) {
            if(eventq == mainEventQueue[0])
            {
              printf("queue 0 service end ! \n") ; //zyc
              return exit_event; //zyc
            }
            else{
              for(int i=1;i<numMainEventQueues;i++)
              {
                if(eventq == mainEventQueue[i])
                {
                  k=i;
                  break;
                }
              }
              printf("queue %d service end ! \n",k); //zyc
              return exit_event; //zyc
            }
              
            //return exit_event;  //anotated by zyc
        }
       // if(eventq != mainEventQueue[0]&& curTick() == numcycle)
         // return exitevent; //zyc
    }

    // not reached... only exit is return on SimLoopExitEvent
}
