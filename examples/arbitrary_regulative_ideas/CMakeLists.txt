# examples/arbitrary_regulative_ideas/CMakeLists.txt
# CMake configuration for example programs demonstrating the use of arbitrary regulative ideas.
# This example illustrates Δ‑analysis with non‑standard address spaces (e.g., binary strings for tree‑like structures).

# Create the executable for examples.
# The source file dirichlet_on_strings.cpp implements a Dirichlet‑like construction
# on the space of binary strings, showcasing a regulative idea based on tree addresses.
add_executable(delta_examples 
    dirichlet_on_strings.cpp
)

# Enforce C++20 standard for the example executable.
set_target_properties(delta_examples PROPERTIES CXX_STANDARD 20)

# Add the parent directory (examples/) to the include path,
# allowing examples to include headers from sibling directories if needed.
target_include_directories(delta_examples PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Link against the core library (delta_core).
# delta_core is an INTERFACE library that brings in Boost, Eigen, OpenMP, fmt,
# and all necessary include directories automatically.
target_link_libraries(delta_examples PRIVATE delta_core)