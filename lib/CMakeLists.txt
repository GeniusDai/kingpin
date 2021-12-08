set ( LIB
    Buffer.cc
    Context.cc
    Utils.cc
    AsyncLogger.cc
)

add_library(kingpin_static STATIC ${LIB})

set_target_properties(kingpin_static PROPERTIES OUTPUT_NAME "kingpin")
