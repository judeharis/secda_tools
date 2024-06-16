include_directories(${secda_tools_SOURCE_DIR}/secda_tools/)

if(NOT BUILD_ARM)
add_library(secda_utils OBJECT acc_helpers.h utils.h)
set_target_properties(secda_utils PROPERTIES LINKER_LANGUAGE CXX)
# target_include_directories(secda_utils PUBLIC .)
target_compile_options(secda_utils PUBLIC -fPIC -DSYSC)

else()
use_cross_compiler()
add_library(secda_utils OBJECT acc_helpers.h utils.h)
set_target_properties(secda_utils PROPERTIES LINKER_LANGUAGE CXX)
# target_include_directories(secda_utils PUBLIC .)
target_compile_options(secda_utils PUBLIC -fPIC)

endif()