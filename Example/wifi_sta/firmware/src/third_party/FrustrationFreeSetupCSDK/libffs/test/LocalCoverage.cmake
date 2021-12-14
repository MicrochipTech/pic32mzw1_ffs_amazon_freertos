# Setup measuring code coverage outside of Brazil.

# Are we using clang/llvm?
if("${CMAKE_C_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
    set(USING_LLVM TRUE)
else()
    set(USING_LLVM FALSE)
endif()

# Set the coverage tool paths and compiler flags for llvm.
if(USING_LLVM)
    if(NOT LLVM_COV_PATH)
        set(LLVM_COV_PATH /Library/Developer/CommandLineTools/usr/bin/llvm-cov)
        set(LLVM_PROFDATA_PATH /Library/Developer/CommandLineTools/usr/bin/llvm-profdata)
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
endif()

# Add a non-Brazil coverage target.
function(add_local_coverage_target target_name)
if(USING_LLVM)
    add_custom_target(all_tests-coverage-preprocessing
        COMMAND LLVM_PROFILE_FILE=${target_name}.profraw $<TARGET_FILE:${target_name}>
        COMMAND ${LLVM_PROFDATA_PATH} merge ${target_name}.profraw -o ${target_name}.profdata
        DEPENDS ${target_name})

    add_custom_target(${target_name}-coverage
        COMMAND ${LLVM_COV_PATH} show $<TARGET_FILE:${target_name}> -instr-profile=${target_name}.profdata --show-line-counts-or-regions -output-dir=coverage -format="html"
        DEPENDS ${target_name}-coverage-preprocessing)
endif()
endfunction(add_local_coverage_target)
