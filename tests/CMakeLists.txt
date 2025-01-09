# create binary
include_directories(${secda_tools_SOURCE_DIR}/secda_tools/)
include_directories(${secda_tools_SOURCE_DIR}/)

if(BUILD_ARM)
  use_cross_compiler()
  add_executable(exp exp.cc)
  target_link_libraries(exp PUBLIC secda_toolsv5)

else()
  add_executable(exp exp.cc)
  target_link_libraries(exp PUBLIC secda_tools_simv5)
endif()

# install( TARGETS exp DESTINATION bin)