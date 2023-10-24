add_library(ConcordPass MODULE
    Concord.cpp
)

target_compile_features(ConcordPass PRIVATE cxx_range_for cxx_auto_type)

set_target_properties(ConcordPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)
