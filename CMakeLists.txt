cmake_minimum_required(VERSION 3.8)

cmake_policy(SET CMP0048 NEW)
project(ASSEMBLER VERSION 1.0.0 LANGUAGES CXX)

# Options
option(ASSEMBLER_BuildTests "Build the unit tests." OFF)

# For debugging, print generator, compiler, and options
message(STATUS "Gen: ${CMAKE_GENERATOR}, CXX: ${CMAKE_CXX_COMPILER_ID}, Mode: ${CMAKE_BUILD_TYPE}")
message(STATUS "Tests: ${ASSEMBLER_BuildTests}")

# Add cmake dir to module path
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

##################
# Checking input #
##################

include(${PROJECT_SOURCE_DIR}/cmake/CommonInputChecks.cmake)

########################
# Declare dependencies #
########################

if(NOT TARGET FiniteElement::FiniteElement)
    message(STATUS "FiniteElement did not already exist, finding package.")
    find_package(FiniteElement CONFIG REQUIRED HINTS "${PROJECT_SOURCE_DIR}/../../first_party/finite_element/build")
endif()
if(NOT TARGET SparseLinearAlgebra::SparseLinearAlgebra)
    message(STATUS "SparseLinearAlgebra did not already exist, finding package.")
    find_package(SparseLinearAlgebra CONFIG REQUIRED HINTS "${PROJECT_SOURCE_DIR}/../../first_party/sparse_linear_algebra/build")
endif()

################################
# Setting up ASSEMBLER target #
################################

# Set the variables for ASSEMBLER
set(ASSEMBLER_INCLUDE ${PROJECT_SOURCE_DIR}/include)
set(ASSEMBLER_SRC ${PROJECT_SOURCE_DIR}/source)
file (GLOB_RECURSE ASSEMBLER_HEADER_LIST "${ASSEMBLER_INCLUDE}/*.h")
file (GLOB_RECURSE ASSEMBLER_SOURCE_LIST "${ASSEMBLER_SRC}/*.h" "${ASSEMBLER_SRC}/*.cpp")
    
# Create target
add_library(assembler STATIC)
# Add the header includes and headers to the library
target_include_directories(assembler PUBLIC
    $<BUILD_INTERFACE:${ASSEMBLER_INCLUDE}>
    $<INSTALL_INTERFACE:include>)
# Add headers in the include folder as PUBLIC so downstream
# targets have access to them
target_sources(assembler PUBLIC ${ASSEMBLER_HEADER_LIST})
# Add headers and sources in the source folder as PRIVATE as they
# will not be needed downstream
target_sources(assembler PRIVATE ${ASSEMBLER_SOURCE_LIST})
# Add an alias so that a library can be used inside the build tree, eg when testing
add_library(Assembler::Assembler ALIAS assembler)
# Add C++ 14 flag
target_compile_features(assembler PUBLIC cxx_std_14)
set_target_properties(assembler PROPERTIES FOLDER "sheff")
# Link dependencies
target_link_libraries(assembler PUBLIC Utility::Utility)
target_link_libraries(assembler PUBLIC SimpleMath::SimpleMath)
target_link_libraries(assembler PUBLIC Constitutive::Constitutive)
target_link_libraries(assembler PUBLIC Mesh::Mesh)
target_link_libraries(assembler PUBLIC FeaIo::FeaIo)
target_link_libraries(assembler PUBLIC FiniteElement::FiniteElement)
target_link_libraries(assembler PUBLIC SparseLinearAlgebra::SparseLinearAlgebra)

###########################
# Assigning source groups #
###########################

include("$ENV{SHEFF_TOOLS}/cmake/IncludeSHEFF.cmake")
register_sources_sheff("assembler"
                       "${ASSEMBLER_HEADER_LIST}"
                       "${ASSEMBLER_SOURCE_LIST}")
assign_source_groups_sheff()

#################################
# Add unit test targets #
#################################
    
# Build tests
if(ASSEMBLER_BuildTests)
    include(CTest)
    enable_testing()
    add_subdirectory(unit_tests)
endif()

################
# Installation #
################

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Assembler)

# Install the assembler target (library and headers)
install(
    TARGETS assembler
    EXPORT AssemblerTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)
# This is required so that the exported target has the name Assembler and not assembler
set_target_properties(assembler PROPERTIES EXPORT_NAME Assembler)

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AssemblerConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Create the package config file for find_package(Assembler)
configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/AssemblerConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AssemblerConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Install the config and config version files
# Note: if you write custom find modules, add them here
# Refer to https://github.com/pabloariasal/modern-cmake-sample/blob/cfda7b51ff915409fd61d4f2e75b63032bedb182/libjsonutils/CMakeLists.txt
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AssemblerConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/AssemblerConfigVersion.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/AssemblerTargets.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

###########
# Exports #
###########

# Export just the assembler target (not tests, etc.)
export(
    TARGETS assembler
    FILE ${CMAKE_CURRENT_BINARY_DIR}/AssemblerTargets.cmake
    NAMESPACE Assembler::
)
