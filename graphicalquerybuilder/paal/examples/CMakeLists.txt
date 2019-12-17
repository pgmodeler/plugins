#Author Piotr Wygocki

include_directories (${PAAL})
file(GLOB_RECURSE example_srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_custom_target(examples)

#adding examples
foreach(example_src ${example_srcs})
    get_filename_component(example_name ${example_src} NAME_WE)
    message("adding example " : ${example_name})
    add_executable(${example_name} ${example_src})

    if(${example_src} MATCHES ".*iterative_rounding.*"
            OR ${example_src} MATCHES ".*linear_programming.*"
            OR ${example_src} MATCHES ".*fractional_winner_determination_in_MUCA*")
        target_link_libraries (${example_name} ${LIBS} glpk)
    elseif(${example_src} MATCHES ".*frequent_directions.*")
        target_link_libraries (${example_name} ${LIBS} lapack)
    else()
        target_link_libraries (${example_name} ${LIBS})
    endif()

    add_dependencies(examples ${example_name})
endforeach()
