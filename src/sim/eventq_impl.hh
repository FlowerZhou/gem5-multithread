/*
 * Copyright (c) 2012 The Regents of The University of Michigan
 * Copyright (c) 2012-2013 Mark D. Hill and David A. Wood
 * Copyright (c) 2013 Advanced Micro Devices, Inc.
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
 * Authors: Steve Reinhardt
 *          Nathan Binkert
 *          Nilay Vaish
 */

#ifndef __SIM_EVENTQ_IMPL_HH__
#define __SIM_EVENTQ_IMPL_HH__

#include "base/trace.hh"
#include "sim/eventq.hh"

inline void
EventQueue::schedule(Event *event, Tick when, bool global)
{
    //std::lock_guard<EventQueue>lock(*this); //zyc
    //assert(when >= getCurTick());
    assert(!event->scheduled());
    assert(event->initialized());

    event->setWhen(when, this);
    //if(event->Core_ID >= 0)  //zyc
      //printf("enter schedule: Core_id is %d \n", event->Core_ID); //zyc


    // The check below is to make sure of two things
    // a. a thread schedules local events on other queues through the asyncq
    // b. a thread schedules global events on the asyncq, whether or not
    //    this event belongs to this eventq. This is required to maintain
    //    a total order amongst the global events. See global_event.{cc,hh}
    //    for more explanation.
    //if (inParallelMode && (this != curEventQueue() || global)) {//+ !
    if(this != curEventQueue()&&global) {    //zyc
        //printf("This is a 异步插入!\n");//added by zyc
        //if(curEventQueue() == mainEventQueue[0])
         // printf("Insertqueue is 0 \n");
        //else if(curEventQueue() == mainEventQueue[1])
         // printf("Insertqueue is 1 \n");
        //else
          //printf("Insertqueue is yibu\n");
        //if(this != curEventQueue())
          //printf("This is not equal to curqueue! \n");
        //else
          //printf("This is equal to curqueue!\n");
            


        asyncInsert(event);
        //if(this == curEventQueue()) //added by zyc
          //handleAsyncInsertions();//added by zyc

    } else {
        //printf("This is a 同步插入!\n");  //zyc
        //if(curEventQueue() == mainEventQueue[0])//zyc
          //printf("tongbu charu queue0 \n"); //zyc
        //if(curEventQueue() == mainEventQueue[1])
          //printf("tongbu charu queue2 \n"); //zyc
       // else
         // printf("tongbu charu queue1 \n");  //zyc
        insert(event);
    }
    event->flags.set(Event::Scheduled);
    event->acquire();

    if (DTRACE(Event))
        event->trace("scheduled");
}

inline void
EventQueue::deschedule(Event *event)
{
    assert(event->scheduled());
    assert(event->initialized());
    assert(!inParallelMode || this == curEventQueue());// no changes
  if(event->isExitEvent())
    printf ("Delete ExitEvent !\n "); //zyc
    remove(event);

    event->flags.clear(Event::Squashed);
    event->flags.clear(Event::Scheduled);

    if (DTRACE(Event))
        event->trace("descheduled");

    event->release();
}

inline void
EventQueue::reschedule(Event *event, Tick when, bool always)
{
    //std::lock_guard<EventQueue>lock(*this); //zyc
    //printf("Entering Event_implement reschedule !"); //tested by zyc
    //assert(when >= getCurTick());  zyc anotated
    assert(always || event->scheduled());
    assert(event->initialized());
    assert(!inParallelMode || this == curEventQueue());// no changes
    //if(this == mainEventQueue[0])   //zyc
      //printf("now the queue is 0 \n");  //zyc
    //else if (this == mainEventQueue[1])  //zyc
     // printf("now the queue is 1 \n");  //zyc
    //else   //zyc
      //printf("now is asyncq \n");  //zyc

    /*if(curEventQueue() == mainEventQueue[0])
      //printf("cur queue is 0 \n");
    else if(curEventQueue() == mainEventQueue[1])
      //printf("cur queue is 1 \n");
    else
      printf("cur queue is yibu\n");
    if(this == curEventQueue())
      //printf("this == curqueue !\n");
    else
      //printf("this != curqueue !\n");*/



    if (event->scheduled()) {
     // if(this == mainEventQueue[0])  // zyc
     //if(event->isExitEvent())
       //printf("This is the ExitEvent !\n"); //zyc
          remove(event);
    } else {
        event->acquire();
    }

    event->setWhen(when, this);
    insert(event);
    event->flags.clear(Event::Squashed);
    event->flags.set(Event::Scheduled);

    if (DTRACE(Event))
        event->trace("rescheduled");
}

#endif // __SIM_EVENTQ_IMPL_HH__
