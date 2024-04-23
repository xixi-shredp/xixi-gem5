import m5
from m5.objects import *

# instantiate the Root object, as required by all gem5 instances.
root = Root(full_system=False)

# instantiate the HelloObject you created
# to call the Python “constructor”
# make sure that it is a child of the root object
# Only SimObjects that are children of the Root object are instantiated in C++.
root.hello = HelloObject(time_to_wait="2us", number_of_fires=5)
root.hello.goodbye_object = GoodbyeObject(buffer_size="100B")


# call instantiate on the m5 module and actually run the simulation
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()
print(f"Exiting @ tick {m5.curTick()} because {exit_event.getCause()}")
