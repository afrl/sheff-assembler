cmake_minimum_required(VERSION 3.8)

cmake_policy(SET CMP0048 NEW)
project(ASSEMBLER VERSION 1.0.0 LANGUAGES CXX)

# Set package toolchain location for Linux. To enable efficient switching between
# toolchains for cross-compiling, we modify the location where CMake looks for 
# packages using the find_package command by symbolically linking the default location,
# $HOME/.cmake/packages, to a similar location with the folder name given by the 
# compiler family. 
if (UNIX)
    execute_process(COMMAND "mkdir" "-p" "$ENV{HOME}/.cmake/${CMAKE_CXX_COMPILER_ID}"
      RESULT_VARIABLE toolchain_res OUTPUT_VARIABLE toolchain_out)
    if (toolchain_res)
        message(ERROR "Couldn't create toolchain dir: $ENV{HOME}/.cmake/${CMAKE_CXX_COMPILER_ID} ${toolchain_out}")
    endif ()
    execute_process(COMMAND "ln" "-sfn" "$ENV{HOME}/.cmake/${CMAKE_CXX_COMPILER_ID}" "$ENV{HOME}/.cmake/packages"
      RESULT_VARIABLE toolchain_res OUTPUT_VARIABLE toolchain_out)
    if (toolchain_res)
        message(ERROR "Couldn't create toolchain link: $ENV{HOME}/.cmake/packages ${toolchain_out}")
    endif ()
endif()


# Options
option(ASSEMBLER_BuildTests "Build the unit tests." OFF)

# For debugging, print generator, compiler, and options
message(STATUS "Gen: ${CMAKE_GENERATOR}, CXX: ${CMAKE_CXX_COMPILER_ID}, Mode: ${CMAKE_BUILD_TYPE}")
message(STATUS "Tests: ${ASSEMBLER_BuildTests}")

# Add cmake dir to module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

##################
# Checking input #
##################

include(${CMAKE_SOURCE_DIR}/cmake/CommonInputChecks.cmake)

########################
# Declare dependencies #
########################


find_package(Utility CONFIG REQUIRED HINTS 
  "${CMAKE_SOURCE_DIR}/../utility/install_dir" "${CMAKE_SOURCE_DIR}/../../utility/install_dir" "${CMAKE_SOURCE_DIR}/modules/utility/install_dir")
find_package(SimpleMath CONFIG REQUIRED HINTS 
  "${CMAKE_SOURCE_DIR}/../simple_math/install_dir" "${CMAKE_SOURCE_DIR}/../../simple_math/install_dir" "${CMAKE_SOURCE_DIR}/modules/simple_math/install_dir")
find_package(Constitutive CONFIG REQUIRED HINTS 
  "${CMAKE_SOURCE_DIR}/../constitutive/install_dir" "${CMAKE_SOURCE_DIR}/../../constitutive/install_dir" "${CMAKE_SOURCE_DIR}/modules/constitutive/install_dir")
find_package(Mesh CONFIG REQUIRED HINTS 
  "${CMAKE_SOURCE_DIR}/../mesh/install_dir" "${CMAKE_SOURCE_DIR}/../../mesh/install_dir" "${CMAKE_SOURCE_DIR}/modules/mesh/install_dir")
find_package(FeaIo CONFIG REQUIRED HINTS 
  "${CMAKE_SOURCE_DIR}/../fea_io/install_dir" "${CMAKE_SOURCE_DIR}/../../fea_io/install_dir" "${CMAKE_SOURCE_DIR}/modules/fea_io/install_dir")
find_package(FiniteElement CONFIG REQUIRED HINTS 
  "${CMAKE_SOURCE_DIR}/../finite_element/install_dir" "${CMAKE_SOURCE_DIR}/../../finite_element/install_dir" "${CMAKE_SOURCE_DIR}/modules/finite_element/install_dir")
find_package(SparseLinearAlgebra CONFIG REQUIRED HINTS 
  "${CMAKE_SOURCE_DIR}/../sparse_linear_algebra/install_dir" "${CMAKE_SOURCE_DIR}/../../sparse_linear_algebra/install_dir" "${CMAKE_SOURCE_DIR}/modules/sparse_linear_algebra/install_dir")


################################
# Setting up ASSEMBLER target #
################################

# Set the variables for ASSEMBLER
set(ASSEMBLER_INCLUDE ${CMAKE_SOURCE_DIR}/include)
set(ASSEMBLER_SRC ${CMAKE_SOURCE_DIR}/source)
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
# Link dependencies
target_link_libraries(assembler PUBLIC Utility::Utility)
target_link_libraries(assembler PUBLIC SimpleMath::SimpleMath)
target_link_libraries(assembler PUBLIC Constitutive::Constitutive)
target_link_libraries(assembler PUBLIC Mesh::Mesh)
target_link_libraries(assembler PUBLIC FeaIo::FeaIo)
target_link_libraries(assembler PUBLIC FiniteElement::FiniteElement)
target_link_libraries(assembler PUBLIC SparseLinearAlgebra::SparseLinearAlgebra)


# Assign source groups
include(${CMAKE_SOURCE_DIR}/cmake/Utilities.cmake)
assign_source_group("assembler" ${ASSEMBLER_HEADER_LIST})
assign_source_group("assembler" ${ASSEMBLER_SOURCE_LIST})
assign_source_group_Utility()
assign_source_group_SimpleMath()
assign_source_group_Constitutive()
assign_source_group_Mesh()
assign_source_group_FeaIo()
assign_source_group_FiniteElement()
assign_source_group_SparseLinearAlgebra()
assign_source_group_SecondPartyMath()
assign_source_group_SecondPartyMesh()
assign_source_group_SecondPartyMetis()

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

# Register package in user's package registry
export(PACKAGE Assembler)
