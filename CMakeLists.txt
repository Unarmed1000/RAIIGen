cmake_minimum_required(VERSION 4.0)

project(RAIIGen LANGUAGES CXX)

# --- libclang -----------------------------------------------------------------------------------------------------------------------------------
# LLVM does not ship a CMake package for libclang so locate it manually and expose it as an imported target.
# Override the search location with -DLLVM_ROOT=<path> if LLVM is not installed in the default location.
set(LLVM_ROOT "$ENV{ProgramFiles}/LLVM" CACHE PATH "LLVM installation root containing libclang")

find_path(LIBCLANG_INCLUDE_DIR clang-c/Index.h HINTS "${LLVM_ROOT}/include")
find_library(LIBCLANG_LIBRARY NAMES libclang clang HINTS "${LLVM_ROOT}/lib")
if(NOT LIBCLANG_INCLUDE_DIR OR NOT LIBCLANG_LIBRARY)
  message(FATAL_ERROR "libclang not found, set LLVM_ROOT to the LLVM installation directory")
endif()

if(WIN32)
  find_file(LIBCLANG_DLL libclang.dll HINTS "${LLVM_ROOT}/bin" NO_DEFAULT_PATH)
  if(NOT LIBCLANG_DLL)
    message(FATAL_ERROR "libclang.dll not found in ${LLVM_ROOT}/bin")
  endif()
  add_library(LibClang::LibClang SHARED IMPORTED)
  set_target_properties(LibClang::LibClang PROPERTIES IMPORTED_LOCATION "${LIBCLANG_DLL}" IMPORTED_IMPLIB "${LIBCLANG_LIBRARY}")
else()
  add_library(LibClang::LibClang UNKNOWN IMPORTED)
  set_target_properties(LibClang::LibClang PROPERTIES IMPORTED_LOCATION "${LIBCLANG_LIBRARY}")
endif()
target_include_directories(LibClang::LibClang INTERFACE "${LIBCLANG_INCLUDE_DIR}")

# --- RAIIGen ------------------------------------------------------------------------------------------------------------------------------------
file(GLOB_RECURSE RAIIGEN_SOURCES CONFIGURE_DEPENDS source/*.cpp source/*.cc source/*.hpp)
file(GLOB_RECURSE RAIIGEN_HEADERS CONFIGURE_DEPENDS include/*.hpp include/*.h)

add_executable(RAIIGen)
target_sources(RAIIGen
  PRIVATE ${RAIIGEN_SOURCES}
  PRIVATE FILE_SET HEADERS BASE_DIRS include FILES ${RAIIGEN_HEADERS})
target_compile_features(RAIIGen PRIVATE cxx_std_20)
set_target_properties(RAIIGen PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(RAIIGen PRIVATE LibClang::LibClang)

target_compile_definitions(RAIIGen PRIVATE
  $<$<PLATFORM_ID:Windows>:_CONSOLE NOMINMAX UNICODE VC_EXTRALEAN WIN32_LEAN_AND_MEAN FSL_PLATFORM_WINDOWS>)
target_compile_options(RAIIGen PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3 /utf-8>)

# The generator reads its config and writes its output relative to the working directory
set_target_properties(RAIIGen PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
set_property(DIRECTORY "${PROJECT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT RAIIGen)

# Copy libclang.dll next to the executable so it can run without LLVM on the PATH
if(WIN32)
  add_custom_command(TARGET RAIIGen POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different $<TARGET_RUNTIME_DLLS:RAIIGen> $<TARGET_FILE_DIR:RAIIGen>
    COMMAND_EXPAND_LISTS)
endif()
