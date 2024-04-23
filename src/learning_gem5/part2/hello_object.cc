#include "learning_gem5/part2/hello_object.hh"

#include "base/trace.hh"
#include "debug/HelloExample.hh"

namespace gem5
{

HelloObject::HelloObject(const HelloObjectParams &params)
    : SimObject(params),
      event([this] { processEvent(); }, name()),
      goodbye(params.goodbye_object),
      myName(params.name),
      latency(params.time_to_wait),
      timesLeft(params.number_of_fires)
{
    /* std::cout << "Hello World! From a SimObject!" << std::endl; */
    DPRINTF(HelloExample, "Create the hello object with the name %s\n",
            myName);

    panic_if(!goodbye, "HelloObject must have a non-null GoodbyeObject");
}

void
HelloObject::processEvent()
{
    timesLeft--;
    DPRINTF(HelloExample, "Hello World! Processing the event! %d Left\n",
            timesLeft);

    if (timesLeft <= 0) {
        DPRINTF(HelloExample, "Done firing!\n");
        goodbye->sayGoodbye(myName);
    } else {
        schedule(event, curTick() + latency);
    }
}

void
HelloObject::startup()
{
    // This function schedules some instance of an Event for some time in the
    // future (event-driven simulation does not allow events to execute in the
    // past). meaning: schedule the event to execute at tick 100.
    schedule(event, latency);
}

} // namespace gem5
