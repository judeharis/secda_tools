

# create binary
include_directories(${secda_tools_SOURCE_DIR}/secda_tools/)
include_directories(${secda_tools_SOURCE_DIR}/)

add_executable( exp exp.cc)
target_link_libraries( exp PUBLIC secda_tools_sim)
# install( TARGETS exp DESTINATION bin)