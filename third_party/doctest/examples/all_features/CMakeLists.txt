################################################################################
## BUILD ALL EXAMPLE SOURCES INTO A SINGLE BINARY AND EXECUTE TESTS ON EACH FILE
################################################################################

set(files_with_output
    main.cpp
    doctest_proxy.h
    header.h
    alternative_macros.cpp
    assertion_macros.cpp
    stringification.cpp
    check_doctest_string.cpp
    double_stringification.cpp
    reporters_and_listeners.cpp
    subcases.cpp
    logging.cpp
    templated_test_cases.cpp
    test_cases_and_suites.cpp
    asserts_used_outside_of_tests.cpp
    enums.cpp
    decomposition.cpp
)

set(files_all
    ${files_with_output}
    concurrency.cpp
    coverage_maxout.cpp
    bitfields.cpp
    bitfield_packed_struct.cpp
    namespace1.cpp
    namespace2.cpp
    namespace3.cpp
    namespace4.cpp
    namespace5.cpp
    namespace6.cpp
    namespace7.cpp
    namespace8.cpp
    namespace9.cpp
    no_failures.cpp
)

# add the executable
add_executable(all_features ${files_all})
target_link_libraries(all_features doctest ${CMAKE_THREAD_LIBS_INIT})

# easy way to fix test coverage - disable colors and crash handling
target_compile_definitions(all_features PRIVATE
    DOCTEST_CONFIG_COLORS_NONE
    DOCTEST_CONFIG_NO_POSIX_SIGNALS
    DOCTEST_CONFIG_NO_WINDOWS_SEH)

# omit the version and the num test cases skipped from the summary - this way the output will change less often
set(common_args COMMAND $<TARGET_FILE:all_features> --no-skipped-summary --no-version)

# add per-file tests
foreach(f ${files_with_output})
    doctest_add_test(NAME ${f} ${common_args} -sf=*${f})
endforeach()

# add this separately since it shouldn't have output compared to reference output - due to concurrency
# not adding it for MinGW since it crashes when using mingw-w64-x86_64-8.1.0-release-posix-seh-rt_v6-rev0
# (also disabled for old XCode builds where there is no thread_local support and this is defined in the build matrix)
if(NOT MINGW AND NOT DEFINED DOCTEST_THREAD_LOCAL)
    doctest_add_test(NO_OUTPUT NAME concurrency.cpp ${common_args} -sf=*concurrency.cpp -d) # duration: there is no output anyway
endif()

doctest_add_test(NO_OUTPUT NAME bitfield_packed_struct.cpp ${common_args} -sf=*bitfield_packed_struct.cpp )
doctest_add_test(NO_OUTPUT NAME bitfields.cpp ${common_args} -sf=*bitfields.cpp )
doctest_add_test(NO_OUTPUT NAME namespace1.cpp ${common_args} -sf=*namespace1.cpp )
doctest_add_test(NO_OUTPUT NAME namespace2.cpp ${common_args} -sf=*namespace2.cpp )
doctest_add_test(NO_OUTPUT NAME namespace3.cpp ${common_args} -sf=*namespace3.cpp )
doctest_add_test(NO_OUTPUT NAME namespace4.cpp ${common_args} -sf=*namespace4.cpp )
doctest_add_test(NO_OUTPUT NAME namespace5.cpp ${common_args} -sf=*namespace5.cpp )
doctest_add_test(NO_OUTPUT NAME namespace6.cpp ${common_args} -sf=*namespace6.cpp )
doctest_add_test(NO_OUTPUT NAME namespace7.cpp ${common_args} -sf=*namespace7.cpp )
doctest_add_test(NO_OUTPUT NAME namespace8.cpp ${common_args} -sf=*namespace8.cpp )
doctest_add_test(NO_OUTPUT NAME namespace9.cpp ${common_args} -sf=*namespace9.cpp )

# add this separately since the file has a non-straightforward path
doctest_add_test(NAME coverage_maxout.cpp ${common_args} -sf=*coverage_maxout.cpp)

# queries
doctest_add_test(NAME version           COMMAND $<TARGET_FILE:all_features> -v)
doctest_add_test(NAME help              ${common_args} -h)
doctest_add_test(NO_OUTPUT NAME outfile ${common_args} -c   -out=temp) # just to exercise the output option
doctest_add_test(NAME count             ${common_args} -c   -sf=*coverage*)
doctest_add_test(NAME list_test_cases   ${common_args} -ltc -sf=*coverage*)
doctest_add_test(NAME list_test_suites  ${common_args} -lts -sf=*coverage*)
doctest_add_test(NAME list_reporters    ${common_args} -lr  -sf=*coverage*)

# options
doctest_add_test(NAME all_binary  ${common_args} -tc=all?binary* -s) # print all binary asserts - for getAssertString()
doctest_add_test(NAME abort_after ${common_args} -aa=2 -e=off   -sf=*coverage*) # abort after 2 assert fails and parse a negative
doctest_add_test(NAME first_last  ${common_args} -f=2 -l=4      -sf=*coverage*) # run a range
doctest_add_test(NAME filter_1    ${common_args} -ts=none) # should filter out all
# -order-by=name to avoid different output depending on the compiler used. See https://github.com/doctest/doctest/issues/287
doctest_add_test(NAME filter_2        COMMAND $<TARGET_FILE:all_features>   -tse=* -nv -order-by=name) # should filter out all + print skipped
doctest_add_test(NAME filter_3        ${common_args} -sc=from*,sc* -sce=sc2 -sf=*subcases*) # enter a specific subcase - sc1
doctest_add_test(NAME filter_4        ${common_args} -ts=*\\, -tc=*\\: -sc=*\\\\\\,,*:) # escape stuff
doctest_add_test(NAME order_1         ${common_args} -ob=suite -ns          -sf=*test_cases_and_suites*)
doctest_add_test(NAME order_2         ${common_args} -ob=name               -sf=*test_cases_and_suites*)
doctest_add_test(NAME order_3         ${common_args} -ob=rand               -sfe=*) # exclude everything for no output
doctest_add_test(NO_OUTPUT NAME quiet ${common_args} -q -sf=*test_cases_and_suites*) # quiet
doctest_add_test(NAME minimal         ${common_args} -m -sf=*test_cases_and_suites*) # minimal with summary
doctest_add_test(NAME minimal_no_fail ${common_args} -m -sf=*no_failures.cpp) # minimal

add_executable(disabled_but_evaluated assert_returns_disabled.cpp assert_returns_disabled_evaluate.cpp)
target_compile_definitions(disabled_but_evaluated PRIVATE DOCTEST_CONFIG_DISABLE)
target_link_libraries(disabled_but_evaluated doctest ${CMAKE_THREAD_LIBS_INIT})

doctest_add_test_impl(NO_OUTPUT NAME disabled_but_evaluated COMMAND $<TARGET_FILE:disabled_but_evaluated>)

if(MSVC)
    target_compile_options(disabled_but_evaluated PRIVATE /wd4702) # unreachable code
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(disabled_but_evaluated PRIVATE -Wno-global-constructors)
    target_compile_options(disabled_but_evaluated PRIVATE -Wno-unused-variable)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(disabled_but_evaluated PRIVATE -Wno-unused-variable)
endif()

################################################################################
## CONFIG TESTS - TESTS WITH VARIOUS CONFIGURATION OPTIONS
################################################################################

function(add_test_all_features test_name flags)
    add_executable(${test_name} ${files_with_output})
    target_compile_definitions(${test_name} PRIVATE ${flags})
    target_link_libraries(${test_name} doctest ${CMAKE_THREAD_LIBS_INIT})

    doctest_add_test_impl(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}> --no-skipped-summary --no-version -ob=name)
endfunction()

add_test_all_features(no_multithreading DOCTEST_CONFIG_NO_MULTITHREADING)
add_test_all_features(no_multi_lane_atomics DOCTEST_CONFIG_NO_MULTI_LANE_ATOMICS)
add_test_all_features(disabled DOCTEST_CONFIG_DISABLE)
add_test_all_features(std_headers DOCTEST_CONFIG_USE_STD_HEADERS)

# TODO: think about fixing these in a different way! - see issue #61 or commit 6b61e8aa3818c5ea100cedc1bb48a60ea10df6e8
if(MSVC)
    target_compile_options(disabled PRIVATE /wd4505) # unreferenced local function has been removed
    target_compile_options(disabled PRIVATE /wd4100) # unreferenced formal parameter
    target_compile_options(disabled PRIVATE /wd4189) # local variable is initialized but not referenced
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(disabled PRIVATE -Wno-unknown-warning-option)
    target_compile_options(disabled PRIVATE -Wno-unneeded-internal-declaration)
    target_compile_options(disabled PRIVATE -Wno-unused-function)
    target_compile_options(disabled PRIVATE -Wno-unused-parameter)
    target_compile_options(disabled PRIVATE -Wno-unused-variable)
    target_compile_options(disabled PRIVATE -Wno-unused-template)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(disabled PRIVATE -Wno-unused-function)
    target_compile_options(disabled PRIVATE -Wno-unused-parameter)
    target_compile_options(disabled PRIVATE -Wno-unused-variable)
endif()
