include(CheckCXXCompilerFlag)

get_property(IS_MULTI_CONFIG_ENV GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

find_package(Python COMPONENTS Interpreter)

find_program(VALGRIND_EXE NAMES ${PREFERRED_VALGRIND_NAMES} valgrind)
find_program(FIND_EXE NAMES ${PREFERRED_FIND_NAMES} find)
find_program(LLVM_COV_EXE NAMES ${PREFERRED_LLVM_COV_NAMES} llvm-cov HINTS ${LLVM_BIN_DIR} /Library/Developer/CommandLineTools/usr/bin/)
find_program(LLVM_PROFDATA_EXE NAMES ${PREFERRED_LLVM_PROFDATA_NAMES} llvm-profdata HINTS ${LLVM_BIN_DIR} /Library/Developer/CommandLineTools/usr/bin/)

set(_ORIGINAL_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})

set(CMAKE_REQUIRED_FLAGS -fprofile-instr-generate)
check_cxx_compiler_flag("-fprofile-instr-generate -fcoverage-mapping" LLVM_COVERAGE_COMPILER_FLAGS_OK)

set(CMAKE_REQUIRED_FLAGS ${_ORIGINAL_CMAKE_REQUIRED_FLAGS})

if(VALGRIND_EXE AND Python_Interpreter_FOUND)
  set(VALGRIND_TESTING_AVAILABLE TRUE CACHE INTERNAL "Executable required to run valgrind testing is available.")
endif()
mark_as_advanced(VALGRIND_EXE)


if(LLVM_PROFDATA_EXE AND LLVM_COV_EXE AND FIND_EXE AND LLVM_COVERAGE_COMPILER_FLAGS_OK)
  set(LLVM_COVERAGE_TESTING_AVAILABLE TRUE CACHE INTERNAL "Executables required to run the llvm coverage testing are available.")
endif()
