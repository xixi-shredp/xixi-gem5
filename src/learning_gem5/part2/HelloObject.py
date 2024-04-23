from m5.params import *
from m5.SimObject import SimObject


# It is not required that the type be the same as the name of the class
class HelloObject(SimObject):
    type = "HelloObject"
    # The type is the C++ class that you are wrapping with this Python SimObject.
    # Only in special circumstances should the type and the class name be different.

    cxx_header = "learning_gem5/part2/hello_object.hh"
    # The cxx_header is the file that contains the declaration of the class used as the type parameter.
    # the convention is to use the name of the SimObject with all lowercase and underscores

    cxx_class = "gem5::HelloObject"
    # The cxx_class is an attribute specifying the newly created SimObject is declared within the gem5 namespace.

    # ------------------ Add Parameters -----------------------
    # Parameters are set by adding new statements to the Python class that include a Param type.
    # Param.<TypeName> declares a parameter of type TypeName.
    # Parameter declaration:
    # 2 Parameters : (default_value : Param.<type>, short_description: string)
    # 1 Parameter  : (short_description: string)
    # if parameter takes no default_value, we need to specify the value in python config file

    # Parameter type : Latency
    # (takes a value as a time value as a string and converts it into simulator ticks. )
    time_to_wait = Param.Latency("Time before firing the event")
    # Parameter type : Int (for integers)
    number_of_fires = Param.Int(
        1, "Number of times to fire the event before goodbye"
    )
    # Parameter type : Float (for floats)
    # Parameter type : Percent
    # Parameter type : Cycles
    # Parameter type : MemorySize
    # Parameter type : another SimObject
    goodbye_object = Param.GoodbyeObject("A goodbye object")
    # ...

    # Once you have declared these parameters in the SimObject file,
    # you need to copy their values to your C++ class in its constructor.


class GoodbyeObject(SimObject):
    type = "GoodbyeObject"
    cxx_header = "learning_gem5/part2/goodbye_object.hh"
    cxx_class = "gem5::GoodbyeObject"

    buffer_size = Param.MemorySize(
        "1kB", "Size of buffer to fill with goodbye"
    )

    write_bandwidth = Param.MemoryBandwidth(
        "100MB/s", "Bandwidth to fill the buffer"
    )
