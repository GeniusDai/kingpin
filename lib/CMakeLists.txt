set ( LIB
    Buffer.cc
    Context.cc
    Utils.cc
    AsyncLogger.cc
)

add_library(kingpin_static STATIC ${LIB})
add_library(kingpin_shared SHARED ${LIB})
set_target_properties(kingpin_static PROPERTIES OUTPUT_NAME "kingpin")
set_target_properties(kingpin_shared PROPERTIES OUTPUT_NAME "kingpin")
