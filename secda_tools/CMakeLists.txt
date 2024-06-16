add_subdirectory(secda_profiler)
add_subdirectory(secda_integrator)
add_subdirectory(axi_support)
add_subdirectory(secda_utils)
add_subdirectory(ap_sysc)

# Create for secda_tools
function(bundle_static_library tgt_name bundled_tgt_name)
  list(APPEND static_libs ${tgt_name})

  function(_recursively_collect_dependencies input_target)
    set(_input_link_libraries LINK_LIBRARIES)
    get_target_property(_input_type ${input_target} TYPE)
    if (${_input_type} STREQUAL "INTERFACE_LIBRARY")
      set(_input_link_libraries INTERFACE_LINK_LIBRARIES)
    endif()
    get_target_property(public_dependencies ${input_target} ${_input_link_libraries})
    foreach(dependency IN LISTS public_dependencies)
      if(TARGET ${dependency})
        get_target_property(alias ${dependency} ALIASED_TARGET)
        if (TARGET ${alias})
          set(dependency ${alias})
        endif()
        get_target_property(_type ${dependency} TYPE)
        if (${_type} STREQUAL "STATIC_LIBRARY")
          list(APPEND static_libs ${dependency})
        endif()

        get_property(library_already_added
          GLOBAL PROPERTY _${tgt_name}_static_bundle_${dependency})
        if (NOT library_already_added)
          set_property(GLOBAL PROPERTY _${tgt_name}_static_bundle_${dependency} ON)
          _recursively_collect_dependencies(${dependency})
        endif()
      endif()
    endforeach()
    set(static_libs ${static_libs} PARENT_SCOPE)
  endfunction()

  _recursively_collect_dependencies(${tgt_name})

  list(REMOVE_DUPLICATES static_libs)

  set(bundled_tgt_full_name 
    ${CMAKE_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${bundled_tgt_name}${CMAKE_STATIC_LIBRARY_SUFFIX})

  if (CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    file(WRITE ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in
      "CREATE ${bundled_tgt_full_name}\n" )
        
    foreach(tgt IN LISTS static_libs)
      file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in
        "ADDLIB $<TARGET_FILE:${tgt}>\n")
    endforeach()
    
    file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in "SAVE\n")
    file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in "END\n")

    file(GENERATE
      OUTPUT ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar
      INPUT ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in)

    set(ar_tool ${CMAKE_AR})
    if (CMAKE_INTERPROCEDURAL_OPTIMIZATION)
      set(ar_tool ${CMAKE_CXX_COMPILER_AR})
    endif()

    add_custom_command(
      COMMAND ${ar_tool} -M < ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar
      DEPENDS ${static_libs}  
      OUTPUT ${bundled_tgt_full_name}
      COMMENT "Bundling ${bundled_tgt_name}"
      VERBATIM)
  elseif(MSVC)
    find_program(lib_tool lib)

    foreach(tgt IN LISTS static_libs)
      list(APPEND static_libs_full_names $<TARGET_FILE:${tgt}>)
    endforeach()

    add_custom_command(
      COMMAND ${lib_tool} /NOLOGO /OUT:${bundled_tgt_full_name} ${static_libs_full_names}
      DEPENDS ${static_libs}          
      OUTPUT ${bundled_tgt_full_name}
      COMMENT "Bundling ${bundled_tgt_name}"
      VERBATIM)
  else()
    message(FATAL_ERROR "Unknown bundle scenario!")
  endif()

  add_custom_target(bundling_target ALL DEPENDS ${bundled_tgt_full_name})
  add_dependencies(bundling_target ${tgt_name})

  add_library(${bundled_tgt_name} STATIC IMPORTED)
  set_target_properties(${bundled_tgt_name} 
    PROPERTIES 
      IMPORTED_LOCATION ${bundled_tgt_full_name}
      INTERFACE_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:${tgt_name},INTERFACE_INCLUDE_DIRECTORIES>)
  add_dependencies(${bundled_tgt_name} bundling_target)

endfunction()


# Combine a list of library targets into a single output archive
# Usage:
# combine_archives(output_archive_name input_target1 input_target2...)
function(combine_archives output_archive)
    # Generate the MRI file for ar to consume.
    # Note that a separate file must be generated for each build configuration.
    set(mri_file ${CMAKE_BINARY_DIR}/$<CONFIG>/${output_archive}.mri)
    set(mri_file_content "create ${CMAKE_BINARY_DIR}/$<CONFIG>/lib${output_archive}.a\n")
    FOREACH(in_target ${ARGN})
        string(APPEND mri_file_content "addlib $<TARGET_FILE:${in_target}>\n")
    ENDFOREACH()
    string(APPEND mri_file_content "save\n")
    string(APPEND mri_file_content "end\n")
    file(GENERATE
            OUTPUT ${mri_file}
            CONTENT ${mri_file_content}
            )

    # Create a dummy file for the combined library
    # This dummy file depends on all the input targets so that the combined library is regenerated if any of them changes.
    set(output_archive_dummy_file ${CMAKE_BINARY_DIR}/${output_archive}.dummy.cpp)
    add_custom_command(OUTPUT ${output_archive_dummy_file}
            COMMAND touch ${output_archive_dummy_file}
            DEPENDS ${ARGN})

    add_library(${output_archive} STATIC ${output_archive_dummy_file})

    # Add a custom command to combine the archives after the static library is "built".
    add_custom_command(TARGET ${output_archive}
            POST_BUILD
            COMMAND ar -M < ${mri_file}
            COMMENT "Combining static libraries for ${output_archive}"
            )
endfunction(combine_archives)



if(BUILD_ARM)
    use_cross_compiler()
    add_library(secda_tools STATIC .)
    target_include_directories(secda_tools PUBLIC )
    target_link_libraries(secda_tools PUBLIC profiler api_v2 secda_utils)

    add_library(secda_toolsv3 STATIC .)
    target_include_directories(secda_toolsv3 PUBLIC )
    target_link_libraries(secda_toolsv3 PUBLIC profiler api_v3 secda_utils)
else()

    # use_host_compiler()
    # add_library(secda_tools_sim STATIC . $<TARGET_OBJECTS:secda_integrator>)
    # bundle_static_library(secda_tools_sim secda_tools_sim_bundled)

    # combine_archives(secda_tools_sim secda_integrator api_sysc_v2)
    add_library(secda_tools_sim STATIC .)
    target_include_directories(secda_tools_sim PUBLIC ${SYSC_INCLUDES})
    target_link_libraries(secda_tools_sim PUBLIC secda_integrator api_sysc_v2 profiler ap_sysc secda_utils ${SYSC_LIBRARIES})
    target_compile_options(secda_tools_sim PUBLIC -fPIC -DSYSC)
    # INSTALL(TARGETS secda_tools_sim 
    #     LIBRARY DESTINATION build/secda_tools
    #     PUBLIC_HEADER DESTINATION build/secda_tools
    # )


    add_library(secda_tools_simv3 STATIC .)
    target_include_directories(secda_tools_simv3 PUBLIC ${SYSC_INCLUDES})
    target_link_libraries(secda_tools_simv3 PUBLIC secda_integrator api_sysc_v3 profiler ap_sysc secda_utils ${SYSC_LIBRARIES})
    target_compile_options(secda_tools_simv3 PUBLIC -fPIC -DSYSC)
endif()
