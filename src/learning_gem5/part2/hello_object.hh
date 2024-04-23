#ifndef __LEARNING_GEM5_PART2_HELLO_OBJECT_HH__
#define __LEARNING_GEM5_PART2_HELLO_OBJECT_HH__

#include "learning_gem5/part2/goodbye_object.hh"
#include "params/HelloObject.hh"
#include "sim/sim_object.hh"

namespace gem5 {

/**
 * An example class from SimObject declaration
 */
class HelloObject : public SimObject
{
  private:
    /**
     * The Event callback function:
     * will execute every time the event fires
     * @parameters none
     * @return nothing
     */
    void processEvent();

    /**
     * The Event Instance
     * @construction: 2 parameters
     * @1.a function to execute : (std::function<void(void)>)
     * @2.the name : usually the name of the SimObject that owns the event
     */
    EventFunctionWrapper event;

    const std::string myName;

    const Tick latency;

    GoodbyeObject* goodbye;

    /** number of times ti fire */
    int timesLeft;

  public:
    HelloObject(const HelloObjectParams &p);

    /**
     * This function is where SimObjects are allowed to schedule internal
     * events. It does not get executed until the simulation begins for the
     * first time (i.e. the simulate() function is called from a Python config
     * file).
     */
    void startup() override;
};

} // namespace gem5
#endif // !__LEARNING_GEM5_PART2_HELLO_OBJECT_HH__
