add_subdirectory(submodules)

# Delta Studio lists Release and Debug D3DX libraries together. Keep Release
# application builds on the redistributable D3DX runtime names.
if (TARGET delta-core)
    get_target_property(delta_core_libraries delta-core INTERFACE_LINK_LIBRARIES)
    list(FILTER delta_core_libraries EXCLUDE REGEX "d3dx(9|10|11)d\\.lib$")
    set_target_properties(delta-core PROPERTIES
        INTERFACE_LINK_LIBRARIES "${delta_core_libraries}")
endif ()

# Piranha uses the legacy Boost.Filesystem name for an absolute-path check.
# Keep the pinned submodule source unchanged while building with current Boost.
if (TARGET piranha)
    target_compile_definitions(piranha PRIVATE is_complete=is_absolute)
endif ()

if (DISCORD_ENABLED)
    add_subdirectory(discord)
endif ()
