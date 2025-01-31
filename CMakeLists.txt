# Specify the minimum required version of CMake to build the project.
cmake_minimum_required( VERSION 3.6 )

# export compile commands for linter
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Colored errors
add_compile_options(-fdiagnostics-color=always)

# -------------- sop -----------------
# reference: https://www.sidefx.com/docs/hdk/_h_d_k__intro__compiling.html#HDK_Intro_Compiling_CMake

# add to CMAKE_TOOL_CHAIN_FILE
list( APPEND CMAKE_PREFIX_PATH "$ENV{HFS}/toolkit/cmake" )

project( My_HDK_Project )

# Locate Houdini's libraries and header files.
# Registers an imported library target named 'Houdini'.
find_package( Houdini REQUIRED )

# create library
set( library_name SOP_attribute_connectivity )
add_library( ${library_name} SHARED
    src/SOP_AttributeConnectivity.C
    include/SOP_AttributeConnectivity.h
)

# add header files
target_include_directories(${library_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Link against the Houdini libraries, and add required include directories and compile definitions.
target_link_libraries( ${library_name} Houdini )

# Configure several common target properties, such as its output directory.
# without this outputs in build
houdini_configure_target( ${library_name} )





# ------------ testing -------------
set(tests_name SopTests)
set(test_dir tests/unit-tests)
find_package(GTest CONFIG REQUIRED)
include(GoogleTest)
add_executable(${tests_name})
target_sources(${tests_name} PRIVATE ${test_dir}/mainTests.cpp)

set(HOUDINI_DSOLIB_DIR /opt/hfs20.5.332/dsolib)
target_include_directories(${tests_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
target_sources(${tests_name} PRIVATE
src/SOP_AttributeConnectivity.C
)

target_link_libraries(${tests_name}
    Houdini
    ${HOUDINI_DSOLIB_DIR}/libtbb.so.2
    GTest::gtest
    GTest::gtest_main
)
gtest_discover_tests(${tests_name})


