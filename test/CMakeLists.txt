find_package(Boost COMPONENTS system program_options log thread REQUIRED)

include_directories (${NODE_DIR}/src
                     ${Boost_INCLUDE_DIRS}
                     ${gtest_SOURCE_DIR}/include
                     )
                     
set(DEFS ${DEFS} -DBOOST_ALL_DYN_LINK)
add_definitions (${DEFS})

add_executable (Test src/test-endpoint.cpp)
target_link_libraries (Test
                       sopmq-nodelib
                       ${Boost_FILESYSTEM_LIBRARY}
                       ${Boost_SYSTEM_LIBRARY}
                        gtest_main
                       )