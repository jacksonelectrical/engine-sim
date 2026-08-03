add_subdirectory(submodules)

# Piranha uses the legacy Boost.Filesystem name for an absolute-path check.
# Keep the pinned submodule source unchanged while building with current Boost.
if (TARGET piranha)
    target_compile_definitions(piranha PRIVATE is_complete=is_absolute)
endif ()

if (DISCORD_ENABLED)
    add_subdirectory(discord)
endif ()
