
macro(ei_add_property prop value)
  get_property(previous GLOBAL PROPERTY ${prop})  
  if ((NOT previous) OR (previous STREQUAL ""))
    set_property(GLOBAL PROPERTY ${prop} "${value}")
  else()
    set_property(GLOBAL PROPERTY ${prop} "${previous} ${value}")
  endif()  
endmacro(ei_add_property)

#internal. See documentation of ei_add_test for details.
macro(ei_add_test_internal testname testname_with_suffix)
  set(targetname ${testname_with_suffix})
  
  if(STORMEIGEN_ADD_TEST_FILENAME_EXTENSION)
    set(filename ${testname}.${STORMEIGEN_ADD_TEST_FILENAME_EXTENSION})
  else()
    set(filename ${testname}.cpp)
  endif()
  
  if(STORMEIGEN_ADD_TEST_FILENAME_EXTENSION STREQUAL cu)
    cuda_add_executable(${targetname} ${filename})
  else()
    add_executable(${targetname} ${filename})
  endif()
  
  
  if (targetname MATCHES "^eigen2_")
    add_dependencies(eigen2_buildtests ${targetname})
  else()
    add_dependencies(buildtests ${targetname})
  endif()

  if(STORMEIGEN_NO_ASSERTION_CHECKING)
    ei_add_target_property(${targetname} COMPILE_FLAGS "-DSTORMEIGEN_NO_ASSERTION_CHECKING=1")
  else(STORMEIGEN_NO_ASSERTION_CHECKING)
    if(STORMEIGEN_DEBUG_ASSERTS)
      ei_add_target_property(${targetname} COMPILE_FLAGS "-DSTORMEIGEN_DEBUG_ASSERTS=1")
    endif(STORMEIGEN_DEBUG_ASSERTS)
  endif(STORMEIGEN_NO_ASSERTION_CHECKING)
  
  ei_add_target_property(${targetname} COMPILE_FLAGS "-DSTORMEIGEN_TEST_MAX_SIZE=${STORMEIGEN_TEST_MAX_SIZE}")

  ei_add_target_property(${targetname} COMPILE_FLAGS "-DSTORMEIGEN_TEST_FUNC=${testname}")
  
  if(MSVC AND NOT STORMEIGEN_SPLIT_LARGE_TESTS)
    ei_add_target_property(${targetname} COMPILE_FLAGS "/bigobj")
  endif()  

  # let the user pass flags.
  if(${ARGC} GREATER 2)
    ei_add_target_property(${targetname} COMPILE_FLAGS "${ARGV2}")
  endif(${ARGC} GREATER 2)
  
  if(STORMEIGEN_TEST_CUSTOM_CXX_FLAGS)
    ei_add_target_property(${targetname} COMPILE_FLAGS "${STORMEIGEN_TEST_CUSTOM_CXX_FLAGS}")
  endif()

  if(STORMEIGEN_STANDARD_LIBRARIES_TO_LINK_TO)
    target_link_libraries(${targetname} ${STORMEIGEN_STANDARD_LIBRARIES_TO_LINK_TO})
  endif()
  if(EXTERNAL_LIBS)
    target_link_libraries(${targetname} ${EXTERNAL_LIBS})
  endif()
  if(STORMEIGEN_TEST_CUSTOM_LINKER_FLAGS)
    target_link_libraries(${targetname} ${STORMEIGEN_TEST_CUSTOM_LINKER_FLAGS})
  endif()

  if(${ARGC} GREATER 3)
    set(libs_to_link ${ARGV3})
    # it could be that some cmake module provides a bad library string " "  (just spaces),
    # and that severely breaks target_link_libraries ("can't link to -l-lstdc++" errors).
    # so we check for strings containing only spaces.
    string(STRIP "${libs_to_link}" libs_to_link_stripped)
    string(LENGTH "${libs_to_link_stripped}" libs_to_link_stripped_length)
    if(${libs_to_link_stripped_length} GREATER 0)
      # notice: no double quotes around ${libs_to_link} here. It may be a list.
      target_link_libraries(${targetname} ${libs_to_link})
    endif()
  endif() 

  add_test(${testname_with_suffix} "${targetname}")
  
  # Specify target and test labels accoirding to STORMEIGEN_CURRENT_SUBPROJECT
  get_property(current_subproject GLOBAL PROPERTY STORMEIGEN_CURRENT_SUBPROJECT)  
  if ((current_subproject) AND (NOT (current_subproject STREQUAL "")))
    set_property(TARGET ${targetname} PROPERTY LABELS "Build${current_subproject}")
    add_dependencies("Build${current_subproject}" ${targetname})
    set_property(TEST ${testname_with_suffix} PROPERTY LABELS "${current_subproject}")
  endif()

endmacro(ei_add_test_internal)

# Macro to add a test
#
# the unique mandatory parameter testname must correspond to a file
# <testname>.cpp which follows this pattern:
#
# #include "main.h"
# void test_<testname>() { ... }
#
# Depending on the contents of that file, this macro can have 2 behaviors,
# see below.
#
# The optional 2nd parameter is libraries to link to.
#
# A. Default behavior
#
# this macro adds an executable <testname> as well as a ctest test
# named <testname> too.
#
# On platforms with bash simply run:
#   "ctest -V" or "ctest -V -R <testname>"
# On other platform use ctest as usual
#
# B. Multi-part behavior
#
# If the source file matches the regexp
#    CALL_SUBTEST_[0-9]+|STORMEIGEN_TEST_PART_[0-9]+
# then it is interpreted as a multi-part test. The behavior then depends on the
# CMake option STORMEIGEN_SPLIT_LARGE_TESTS, which is ON by default.
#
# If STORMEIGEN_SPLIT_LARGE_TESTS is OFF, the behavior is the same as in A (the multi-part
# aspect is ignored).
#
# If STORMEIGEN_SPLIT_LARGE_TESTS is ON, the test is split into multiple executables
#   test_<testname>_<N>
# where N runs from 1 to the greatest occurence found in the source file. Each of these
# executables is built passing -DSTORMEIGEN_TEST_PART_N. This allows to split large tests
# into smaller executables.
#
# Moreover, targets <testname> are still generated, they
# have the effect of building all the parts of the test.
#
# Again, ctest -R allows to run all matching tests.
macro(ei_add_test testname)
  get_property(STORMEIGEN_TESTS_LIST GLOBAL PROPERTY STORMEIGEN_TESTS_LIST)
  set(STORMEIGEN_TESTS_LIST "${STORMEIGEN_TESTS_LIST}${testname}\n")
  set_property(GLOBAL PROPERTY STORMEIGEN_TESTS_LIST "${STORMEIGEN_TESTS_LIST}")

  if(STORMEIGEN_ADD_TEST_FILENAME_EXTENSION)
    set(filename ${testname}.${STORMEIGEN_ADD_TEST_FILENAME_EXTENSION})
  else()
    set(filename ${testname}.cpp)
  endif()
  
  file(READ "${filename}" test_source)
  set(parts 0)
  string(REGEX MATCHALL "CALL_SUBTEST_[0-9]+|STORMEIGEN_TEST_PART_[0-9]+|STORMEIGEN_SUFFIXES(;[0-9]+)+"
         occurences "${test_source}")
  string(REGEX REPLACE "CALL_SUBTEST_|STORMEIGEN_TEST_PART_|STORMEIGEN_SUFFIXES" "" suffixes "${occurences}")
  list(REMOVE_DUPLICATES suffixes)
  if(STORMEIGEN_SPLIT_LARGE_TESTS AND suffixes)
    add_custom_target(${testname})
    foreach(suffix ${suffixes})
      ei_add_test_internal(${testname} ${testname}_${suffix}
        "${ARGV1} -DSTORMEIGEN_TEST_PART_${suffix}=1" "${ARGV2}")
      add_dependencies(${testname} ${testname}_${suffix})
    endforeach(suffix)
  else(STORMEIGEN_SPLIT_LARGE_TESTS AND suffixes)
    set(symbols_to_enable_all_parts "")
    foreach(suffix ${suffixes})
      set(symbols_to_enable_all_parts
        "${symbols_to_enable_all_parts} -DSTORMEIGEN_TEST_PART_${suffix}=1")
    endforeach(suffix)
    ei_add_test_internal(${testname} ${testname} "${ARGV1} ${symbols_to_enable_all_parts}" "${ARGV2}")
  endif(STORMEIGEN_SPLIT_LARGE_TESTS AND suffixes)
endmacro(ei_add_test)


# adds a failtest, i.e. a test that succeed if the program fails to compile
# note that the test runner for these is CMake itself, when passed -DSTORMEIGEN_FAILTEST=ON
# so here we're just running CMake commands immediately, we're not adding any targets.
macro(ei_add_failtest testname)
  get_property(STORMEIGEN_FAILTEST_FAILURE_COUNT GLOBAL PROPERTY STORMEIGEN_FAILTEST_FAILURE_COUNT)
  get_property(STORMEIGEN_FAILTEST_COUNT GLOBAL PROPERTY STORMEIGEN_FAILTEST_COUNT)

  message(STATUS "Checking failtest: ${testname}")
  set(filename "${testname}.cpp")
  file(READ "${filename}" test_source)

  try_compile(succeeds_when_it_should_fail
              "${CMAKE_CURRENT_BINARY_DIR}"
              "${CMAKE_CURRENT_SOURCE_DIR}/${filename}"
              COMPILE_DEFINITIONS "-DSTORMEIGEN_SHOULD_FAIL_TO_BUILD")
  if (succeeds_when_it_should_fail)
    message(STATUS "FAILED: ${testname} build succeeded when it should have failed")
  endif()

  try_compile(succeeds_when_it_should_succeed
              "${CMAKE_CURRENT_BINARY_DIR}"
              "${CMAKE_CURRENT_SOURCE_DIR}/${filename}"
              COMPILE_DEFINITIONS)
  if (NOT succeeds_when_it_should_succeed)
    message(STATUS "FAILED: ${testname} build failed when it should have succeeded")
  endif()

  if (succeeds_when_it_should_fail OR NOT succeeds_when_it_should_succeed)
    math(EXPR STORMEIGEN_FAILTEST_FAILURE_COUNT ${STORMEIGEN_FAILTEST_FAILURE_COUNT}+1)
  endif()

  math(EXPR STORMEIGEN_FAILTEST_COUNT ${STORMEIGEN_FAILTEST_COUNT}+1)

  set_property(GLOBAL PROPERTY STORMEIGEN_FAILTEST_FAILURE_COUNT ${STORMEIGEN_FAILTEST_FAILURE_COUNT})
  set_property(GLOBAL PROPERTY STORMEIGEN_FAILTEST_COUNT ${STORMEIGEN_FAILTEST_COUNT})
endmacro(ei_add_failtest)

# print a summary of the different options
macro(ei_testing_print_summary)
  message(STATUS "************************************************************")
  message(STATUS "***    Eigen's unit tests configuration summary          ***")
  message(STATUS "************************************************************")
  message(STATUS "")
  message(STATUS "Build type:        ${CMAKE_BUILD_TYPE}")
  message(STATUS "Build site:        ${SITE}")
  message(STATUS "Build string:      ${BUILDNAME}")
  get_property(STORMEIGEN_TESTING_SUMMARY GLOBAL PROPERTY STORMEIGEN_TESTING_SUMMARY)
  get_property(STORMEIGEN_TESTED_BACKENDS GLOBAL PROPERTY STORMEIGEN_TESTED_BACKENDS)
  get_property(STORMEIGEN_MISSING_BACKENDS GLOBAL PROPERTY STORMEIGEN_MISSING_BACKENDS)
  message(STATUS "Enabled backends:  ${STORMEIGEN_TESTED_BACKENDS}")
  message(STATUS "Disabled backends: ${STORMEIGEN_MISSING_BACKENDS}")

  if(STORMEIGEN_DEFAULT_TO_ROW_MAJOR)
    message(STATUS "Default order:     Row-major")
  else()
    message(STATUS "Default order:     Column-major")
  endif()

  if(STORMEIGEN_TEST_NO_EXPLICIT_ALIGNMENT)
    message(STATUS "Explicit alignment (hence vectorization) disabled")
  elseif(STORMEIGEN_TEST_NO_EXPLICIT_VECTORIZATION)
    message(STATUS "Explicit vectorization disabled (alignment kept enabled)")
  else()
  
  message(STATUS "Maximal matrix/vector size: ${STORMEIGEN_TEST_MAX_SIZE}")

    if(STORMEIGEN_TEST_SSE2)
      message(STATUS "SSE2:              ON")
    else()
      message(STATUS "SSE2:              Using architecture defaults")
    endif()

    if(STORMEIGEN_TEST_SSE3)
      message(STATUS "SSE3:              ON")
    else()
      message(STATUS "SSE3:              Using architecture defaults")
    endif()

    if(STORMEIGEN_TEST_SSSE3)
      message(STATUS "SSSE3:             ON")
    else()
      message(STATUS "SSSE3:             Using architecture defaults")
    endif()

    if(STORMEIGEN_TEST_SSE4_1)
      message(STATUS "SSE4.1:            ON")
    else()
      message(STATUS "SSE4.1:            Using architecture defaults")
    endif()

    if(STORMEIGEN_TEST_SSE4_2)
      message(STATUS "SSE4.2:            ON")
    else()
      message(STATUS "SSE4.2:            Using architecture defaults")
    endif()

    if(STORMEIGEN_TEST_AVX)
      message(STATUS "AVX:               ON")
    else()
      message(STATUS "AVX:               Using architecture defaults")
    endif()

   if(STORMEIGEN_TEST_FMA)
      message(STATUS "FMA:               ON")
    else()
      message(STATUS "FMA:               Using architecture defaults")
    endif()

    if(STORMEIGEN_TEST_ALTIVEC)
      message(STATUS "Altivec:           ON")
    else()
      message(STATUS "Altivec:           Using architecture defaults")
    endif()

    if(STORMEIGEN_TEST_VSX)
      message(STATUS "VSX:               ON")
    else()
      message(STATUS "VSX:               Using architecture defaults")
    endif()

    if(STORMEIGEN_TEST_NEON)
      message(STATUS "ARM NEON:          ON")
    else()
      message(STATUS "ARM NEON:          Using architecture defaults")
    endif()

    if(STORMEIGEN_TEST_NEON64)
      message(STATUS "ARMv8 NEON:        ON")
    else()
      message(STATUS "ARMv8 NEON:        Using architecture defaults")
    endif()
    
    if(STORMEIGEN_TEST_CXX11)
      message(STATUS "C++11:             ON")
    else()
      message(STATUS "C++11:             OFF")
    endif()

  endif() # vectorization / alignment options

  message(STATUS "\n${STORMEIGEN_TESTING_SUMMARY}")

  message(STATUS "************************************************************")
endmacro(ei_testing_print_summary)

macro(ei_init_testing)
  define_property(GLOBAL PROPERTY STORMEIGEN_CURRENT_SUBPROJECT BRIEF_DOCS " " FULL_DOCS " ")
  define_property(GLOBAL PROPERTY STORMEIGEN_TESTED_BACKENDS BRIEF_DOCS " " FULL_DOCS " ")
  define_property(GLOBAL PROPERTY STORMEIGEN_MISSING_BACKENDS BRIEF_DOCS " " FULL_DOCS " ")
  define_property(GLOBAL PROPERTY STORMEIGEN_TESTING_SUMMARY BRIEF_DOCS " " FULL_DOCS " ")
  define_property(GLOBAL PROPERTY STORMEIGEN_TESTS_LIST BRIEF_DOCS " " FULL_DOCS " ")

  set_property(GLOBAL PROPERTY STORMEIGEN_TESTED_BACKENDS "")
  set_property(GLOBAL PROPERTY STORMEIGEN_MISSING_BACKENDS "")
  set_property(GLOBAL PROPERTY STORMEIGEN_TESTING_SUMMARY "")
  set_property(GLOBAL PROPERTY STORMEIGEN_TESTS_LIST "")

  define_property(GLOBAL PROPERTY STORMEIGEN_FAILTEST_FAILURE_COUNT BRIEF_DOCS " " FULL_DOCS " ")
  define_property(GLOBAL PROPERTY STORMEIGEN_FAILTEST_COUNT BRIEF_DOCS " " FULL_DOCS " ")

  set_property(GLOBAL PROPERTY STORMEIGEN_FAILTEST_FAILURE_COUNT "0")
  set_property(GLOBAL PROPERTY STORMEIGEN_FAILTEST_COUNT "0")
  
  # uncomment anytime you change the ei_get_compilerver_from_cxx_version_string macro
  # ei_test_get_compilerver_from_cxx_version_string()
endmacro(ei_init_testing)

macro(ei_set_sitename)
  # if the sitename is not yet set, try to set it
  if(NOT ${SITE} OR ${SITE} STREQUAL "")
    set(eigen_computername $ENV{COMPUTERNAME})
	set(eigen_hostname $ENV{HOSTNAME})
    if(eigen_hostname)
      set(SITE ${eigen_hostname})
	elseif(eigen_computername)
	  set(SITE ${eigen_computername})
    endif()
  endif()
  # in case it is already set, enforce lower case
  if(SITE)
    string(TOLOWER ${SITE} SITE)
  endif()  
endmacro(ei_set_sitename)

macro(ei_get_compilerver VAR)
    if(MSVC)
      # on windows system, we use a modified CMake script  
      include(EigenDetermineVSServicePack)
      EigenDetermineVSServicePack( my_service_pack )

      if( my_service_pack )
        set(${VAR} ${my_service_pack})
      else()
        set(${VAR} "na")
      endif()
    else()
    # on all other system we rely on ${CMAKE_CXX_COMPILER}
    # supporting a "--version" or "/version" flag
    
    if(WIN32 AND ${CMAKE_CXX_COMPILER_ID} EQUAL "Intel")
      set(STORMEIGEN_CXX_FLAG_VERSION "/version")
    else()
      set(STORMEIGEN_CXX_FLAG_VERSION "--version")
    endif()
    
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} ${STORMEIGEN_CXX_FLAG_VERSION}
                    OUTPUT_VARIABLE eigen_cxx_compiler_version_string OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "[\n\r].*"  ""  eigen_cxx_compiler_version_string  ${eigen_cxx_compiler_version_string})
    
    ei_get_compilerver_from_cxx_version_string("${eigen_cxx_compiler_version_string}" CNAME CVER)
    set(${VAR} "${CNAME}-${CVER}")
    
  endif()
endmacro(ei_get_compilerver)

# Extract compiler name and version from a raw version string
# WARNING: if you edit thid macro, then please test it by  uncommenting
# the testing macro call in ei_init_testing() of the EigenTesting.cmake file.
# See also the ei_test_get_compilerver_from_cxx_version_string macro at the end of the file
macro(ei_get_compilerver_from_cxx_version_string VERSTRING CNAME CVER)
  # extract possible compiler names  
  string(REGEX MATCH "g\\+\\+"      ei_has_gpp    ${VERSTRING})
  string(REGEX MATCH "llvm|LLVM"    ei_has_llvm   ${VERSTRING})
  string(REGEX MATCH "gcc|GCC"      ei_has_gcc    ${VERSTRING})
  string(REGEX MATCH "icpc|ICC"     ei_has_icpc   ${VERSTRING})
  string(REGEX MATCH "clang|CLANG"  ei_has_clang  ${VERSTRING})
  
  # combine them
  if((ei_has_llvm) AND (ei_has_gpp OR ei_has_gcc))
    set(${CNAME} "llvm-g++")
  elseif((ei_has_llvm) AND (ei_has_clang))
    set(${CNAME} "llvm-clang++")
  elseif(ei_has_icpc)
    set(${CNAME} "icpc")
  elseif(ei_has_gpp OR ei_has_gcc)
    set(${CNAME} "g++")
  else()
    set(${CNAME} "_")
  endif()
  
  # extract possible version numbers
  # first try to extract 3 isolated numbers:
  string(REGEX MATCH " [0-9]+\\.[0-9]+\\.[0-9]+" eicver ${VERSTRING})
  if(NOT eicver)
    # try to extract 2 isolated ones:
    string(REGEX MATCH " [0-9]+\\.[0-9]+" eicver ${VERSTRING})
    if(NOT eicver)
      # try to extract 3:
      string(REGEX MATCH "[^0-9][0-9]+\\.[0-9]+\\.[0-9]+" eicver ${VERSTRING})
      if(NOT eicver)
        # try to extract 2:
        string(REGEX MATCH "[^0-9][0-9]+\\.[0-9]+" eicver ${VERSTRING})
      else()
        set(eicver " _")
      endif()
    endif()
  endif()
  
  string(REGEX REPLACE ".(.*)" "\\1" ${CVER} ${eicver})
  
endmacro(ei_get_compilerver_from_cxx_version_string)

macro(ei_get_cxxflags VAR)
  set(${VAR} "")
  ei_is_64bit_env(IS_64BIT_ENV)
  if(STORMEIGEN_TEST_NEON)
    set(${VAR} NEON)
  elseif(STORMEIGEN_TEST_NEON64)
    set(${VAR} NEON)
  elseif(STORMEIGEN_TEST_VSX)
    set(${VAR} VSX)
  elseif(STORMEIGEN_TEST_ALTIVEC)
    set(${VAR} ALVEC)
  elseif(STORMEIGEN_TEST_FMA)
    set(${VAR} FMA)
  elseif(STORMEIGEN_TEST_AVX)
    set(${VAR} AVX)
  elseif(STORMEIGEN_TEST_SSE4_2)
    set(${VAR} SSE42)
  elseif(STORMEIGEN_TEST_SSE4_1)
    set(${VAR} SSE41)
  elseif(STORMEIGEN_TEST_SSSE3)
    set(${VAR} SSSE3)
  elseif(STORMEIGEN_TEST_SSE3)
    set(${VAR} SSE3)
  elseif(STORMEIGEN_TEST_SSE2 OR IS_64BIT_ENV)
    set(${VAR} SSE2)  
  endif()

  if(STORMEIGEN_TEST_OPENMP)
    if (${VAR} STREQUAL "")
	  set(${VAR} OMP)
	else()
	  set(${VAR} ${${VAR}}-OMP)
	endif()
  endif()
  
  if(STORMEIGEN_DEFAULT_TO_ROW_MAJOR)
    if (${VAR} STREQUAL "")
	  set(${VAR} ROW)
	else()
	  set(${VAR} ${${VAR}}-ROWMAJ)
	endif()  
  endif()
endmacro(ei_get_cxxflags)

macro(ei_set_build_string)
  ei_get_compilerver(LOCAL_COMPILER_VERSION)
  ei_get_cxxflags(LOCAL_COMPILER_FLAGS)
  
  include(EigenDetermineOSVersion)
  DetermineOSVersion(OS_VERSION)

  set(TMP_BUILD_STRING ${OS_VERSION}-${LOCAL_COMPILER_VERSION})

  if (NOT ${LOCAL_COMPILER_FLAGS} STREQUAL  "")
    set(TMP_BUILD_STRING ${TMP_BUILD_STRING}-${LOCAL_COMPILER_FLAGS})
  endif()

  ei_is_64bit_env(IS_64BIT_ENV)
  if(NOT IS_64BIT_ENV)
    set(TMP_BUILD_STRING ${TMP_BUILD_STRING}-32bit)
  else()
    set(TMP_BUILD_STRING ${TMP_BUILD_STRING}-64bit)
  endif()
  
  if(STORMEIGEN_TEST_CXX11)
    set(TMP_BUILD_STRING ${TMP_BUILD_STRING}-cxx11)
  endif()
  
  if(STORMEIGEN_BUILD_STRING_SUFFIX)
    set(TMP_BUILD_STRING ${TMP_BUILD_STRING}-${STORMEIGEN_BUILD_STRING_SUFFIX})
  endif()

  string(TOLOWER ${TMP_BUILD_STRING} BUILDNAME)
endmacro(ei_set_build_string)

macro(ei_is_64bit_env VAR)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(${VAR} 1)
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(${VAR} 0)
  else()
    message(WARNING "Unsupported pointer size. Please contact the authors.")
  endif()
endmacro(ei_is_64bit_env)


# helper macro for testing ei_get_compilerver_from_cxx_version_string
# STR: raw version string
# REFNAME: expected compiler name
# REFVER: expected compiler version
macro(ei_test1_get_compilerver_from_cxx_version_string STR REFNAME REFVER)
  ei_get_compilerver_from_cxx_version_string(${STR} CNAME CVER)
  if((NOT ${REFNAME} STREQUAL ${CNAME}) OR (NOT ${REFVER} STREQUAL ${CVER}))
    message("STATUS ei_get_compilerver_from_cxx_version_string error:")
    message("Expected \"${REFNAME}-${REFVER}\", got \"${CNAME}-${CVER}\"")
  endif()
endmacro(ei_test1_get_compilerver_from_cxx_version_string)

# macro for testing ei_get_compilerver_from_cxx_version_string
# feel free to add more version strings
macro(ei_test_get_compilerver_from_cxx_version_string)
  ei_test1_get_compilerver_from_cxx_version_string("g++ (SUSE Linux) 4.5.3 20110428 [gcc-4_5-branch revision 173117]" "g++" "4.5.3")
  ei_test1_get_compilerver_from_cxx_version_string("c++ (GCC) 4.5.1 20100924 (Red Hat 4.5.1-4)" "g++" "4.5.1")
  ei_test1_get_compilerver_from_cxx_version_string("icpc (ICC) 11.0 20081105" "icpc" "11.0")
  ei_test1_get_compilerver_from_cxx_version_string("g++-3.4 (GCC) 3.4.6" "g++" "3.4.6")
  ei_test1_get_compilerver_from_cxx_version_string("SUSE Linux clang version 3.0 (branches/release_30 145598) (based on LLVM 3.0)" "llvm-clang++" "3.0")
  ei_test1_get_compilerver_from_cxx_version_string("icpc (ICC) 12.0.5 20110719" "icpc" "12.0.5")
  ei_test1_get_compilerver_from_cxx_version_string("Apple clang version 2.1 (tags/Apple/clang-163.7.1) (based on LLVM 3.0svn)" "llvm-clang++" "2.1")
  ei_test1_get_compilerver_from_cxx_version_string("i686-apple-darwin11-llvm-g++-4.2 (GCC) 4.2.1 (Based on Apple Inc. build 5658) (LLVM build 2335.15.00)" "llvm-g++" "4.2.1")
  ei_test1_get_compilerver_from_cxx_version_string("g++-mp-4.4 (GCC) 4.4.6" "g++" "4.4.6")
  ei_test1_get_compilerver_from_cxx_version_string("g++-mp-4.4 (GCC) 2011" "g++" "4.4")
endmacro(ei_test_get_compilerver_from_cxx_version_string)
