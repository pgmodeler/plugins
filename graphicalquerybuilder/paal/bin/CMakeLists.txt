include_directories (${PAAL})
file(GLOB_RECURSE bin_srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_custom_target(bins)

add_definitions(-DBOOST_ALL_DYN_LINK=1 -DBOOST_ALL_NO_LIB=1)

file(GLOB_RECURSE boost_program_options "${PAAL}/boost/program_options/src/*.cpp")
add_library(boost_program_options SHARED ${boost_program_options})

# files taken from boost build - it does not compile if one take all of the *.cpp files
set(SERIALIZATION_FNAMES basic_archive basic_iarchive basic_iserializer basic_oarchive
    basic_oserializer basic_pointer_iserializer basic_pointer_oserializer
    basic_serializer_map basic_text_iprimitive basic_text_oprimitive
    basic_xml_archive binary_iarchive binary_oarchive extended_type_info
    extended_type_info_typeid extended_type_info_no_rtti polymorphic_iarchive
    polymorphic_oarchive stl_port text_iarchive text_oarchive void_cast
    archive_exception xml_grammar xml_iarchive xml_oarchive
    xml_archive_exception)

foreach(name ${SERIALIZATION_FNAMES})
    SET(SERIALIZATION_BINS ${SERIALIZATION_BINS} ${PAAL}/boost/serialization/src/${name}.cpp)
endforeach()
add_library(boost_serialization SHARED ${SERIALIZATION_BINS})

# Boost mapped file shared library
file(GLOB_RECURSE boost_iostreams "${PAAL}/boost/iostreams/src/*.cpp")
add_library(boost_iostreams SHARED ${boost_iostreams})
target_link_libraries(boost_iostreams bz2 z)

foreach(bin_src ${bin_srcs})
    get_filename_component(bin_name ${bin_src} NAME_WE)
    message("adding binary" : ${bin_name})
    add_executable(${bin_name} ${bin_src})
    if(${bin_src} MATCHES ".*frequent-directions.*")
        target_link_libraries (${bin_name} ${LIBS} boost_program_options boost_serialization boost_iostreams lapack)
    else()
        target_link_libraries (${bin_name} ${LIBS} boost_program_options boost_serialization boost_iostreams)
    endif()

    add_dependencies(bins ${bin_name})
endforeach()
