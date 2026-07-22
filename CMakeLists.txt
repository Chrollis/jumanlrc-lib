cmake_minimum_required(VERSION 3.10)
project(jumanlrc-lib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(JPP_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(JPP_LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")

include_directories(BEFORE "${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/gen")
include_directories("${JPP_SRC_DIR}" "${JPP_LIBS_DIR}" "${JPP_LIBS_DIR}/pegtl" "${JPP_LIBS_DIR}/pathie-cpp/include")

if (MSVC)
  add_compile_options("/utf-8" "/wd4141" "/wd4018" "/wd4200")
endif()
if (WIN32)
  add_definitions("-D_WIN32_WINNT=0x0600" "-D_WIN32_IE=0x0800" "-DWINVER=0x0600")
endif()
add_definitions("-D_CRT_SECURE_NO_WARNINGS")
find_package(Threads REQUIRED)

# Single library with all source files
set(COMBINED_SOURCES
  "${JPP_SRC_DIR}/util/mmap.cc" "${JPP_SRC_DIR}/util/memory.cpp"
  "${JPP_SRC_DIR}/util/logging.cpp" "${JPP_SRC_DIR}/util/string_piece.cc"
  "${JPP_SRC_DIR}/util/status.cpp" "${JPP_SRC_DIR}/util/csv_reader.cc"
  "${JPP_SRC_DIR}/util/coded_io.cc" "${JPP_SRC_DIR}/util/characters.cc"
  "${JPP_SRC_DIR}/util/printer.cc" "${JPP_SRC_DIR}/util/codegen.cc"
  "${JPP_SRC_DIR}/util/assert.cc" "${JPP_SRC_DIR}/util/format.cc"
  "${JPP_SRC_DIR}/util/parse_utils.cc"
  "${JPP_LIBS_DIR}/pathie-cpp/src/errors.cpp"
  "${JPP_LIBS_DIR}/pathie-cpp/src/entry_iterator.cpp"
  "${JPP_LIBS_DIR}/pathie-cpp/src/path.cpp"
  "${JPP_LIBS_DIR}/pathie-cpp/src/temp.cpp"
  "${JPP_LIBS_DIR}/pathie-cpp/src/pathie.cpp"
  "${JPP_SRC_DIR}/core/core.cc" "${JPP_SRC_DIR}/core/features_api.cc"
  "${JPP_SRC_DIR}/core/env.cc"
  "${JPP_SRC_DIR}/core/analysis/analysis_input.cc"
  "${JPP_SRC_DIR}/core/analysis/analysis_result.cc"
  "${JPP_SRC_DIR}/core/analysis/analyzer.cc"
  "${JPP_SRC_DIR}/core/analysis/analyzer_impl.cc"
  "${JPP_SRC_DIR}/core/analysis/charlattice.cc"
  "${JPP_SRC_DIR}/core/analysis/dic_reader.cc"
  "${JPP_SRC_DIR}/core/analysis/dictionary_node_creator.cc"
  "${JPP_SRC_DIR}/core/analysis/extra_nodes.cc"
  "${JPP_SRC_DIR}/core/analysis/innode_features.cc"
  "${JPP_SRC_DIR}/core/analysis/lattice_builder.cc"
  "${JPP_SRC_DIR}/core/analysis/lattice_config.cc"
  "${JPP_SRC_DIR}/core/analysis/lattice_types.cc"
  "${JPP_SRC_DIR}/core/analysis/ngram_computations.cc"
  "${JPP_SRC_DIR}/core/analysis/normalized_node_creator.cc"
  "${JPP_SRC_DIR}/core/analysis/numeric_creator.cc"
  "${JPP_SRC_DIR}/core/analysis/onomatopoeia_creator.cc"
  "${JPP_SRC_DIR}/core/analysis/output.cc"
  "${JPP_SRC_DIR}/core/analysis/perceptron.cc"
  "${JPP_SRC_DIR}/core/analysis/score_processor.cc"
  "${JPP_SRC_DIR}/core/analysis/unk_nodes.cc"
  "${JPP_SRC_DIR}/core/analysis/unk_nodes_creator.cc"
  "${JPP_SRC_DIR}/core/dic/darts_trie.cc"
  "${JPP_SRC_DIR}/core/dic/dic_build_detail.cc"
  "${JPP_SRC_DIR}/core/dic/dic_builder.cc"
  "${JPP_SRC_DIR}/core/dic/dictionary.cc"
  "${JPP_SRC_DIR}/core/dic/dic_feature_impl.cc"
  "${JPP_SRC_DIR}/core/dic/entry_builder.cc"
  "${JPP_SRC_DIR}/core/dic/field_import.cc"
  "${JPP_SRC_DIR}/core/impl/feature_computer.cc"
  "${JPP_SRC_DIR}/core/impl/feature_debug.cc"
  "${JPP_SRC_DIR}/core/impl/feature_impl_combine.cc"
  "${JPP_SRC_DIR}/core/impl/feature_impl_compute.cc"
  "${JPP_SRC_DIR}/core/impl/feature_impl_ngram_partial.cc"
  "${JPP_SRC_DIR}/core/impl/feature_impl_pattern.cc"
  "${JPP_SRC_DIR}/core/impl/feature_impl_prim.cc"
  "${JPP_SRC_DIR}/core/impl/model_io.cc"
  "${JPP_SRC_DIR}/core/input/partial_example.cc"
  "${JPP_SRC_DIR}/core/input/partial_example_io.cc"
  "${JPP_SRC_DIR}/core/input/pex_stream_reader.cc"
  "${JPP_SRC_DIR}/core/input/stream_reader.cc"
  "${JPP_SRC_DIR}/core/input/training_io.cc"
  "${JPP_SRC_DIR}/core/spec/spec_compiler.cc"
  "${JPP_SRC_DIR}/core/spec/spec_dsl.cc" "${JPP_SRC_DIR}/core/spec/spec_hashing.cc"
  "${JPP_SRC_DIR}/core/spec/spec_parser.cc"
  "${JPP_SRC_DIR}/core/spec/spec_parser_impl.cc"
  "${JPP_SRC_DIR}/core/spec/spec_serialization.cc"
  "${JPP_SRC_DIR}/core/spec/spec_types.cc"
  "${JPP_SRC_DIR}/core/codegen/feature_codegen.cc"
  "${JPP_SRC_DIR}/core/codegen/ngram_feature_codegen.cc"
  "${JPP_SRC_DIR}/core/codegen/partial_ngram_feature_codegen.cc"
  "${JPP_SRC_DIR}/core/codegen/pattern_feature_codegen.cc"
  "${JPP_SRC_DIR}/jumandic/shared/jumandic_spec.cc"
  "${JPP_SRC_DIR}/jumandic/shared/jumandic_spec_lexdata.cc"
  "${JPP_SRC_DIR}/jumandic/shared/jumandic_ids.cc"
  "${JPP_SRC_DIR}/jumandic/shared/jumandic_id_resolver.cc"
)
if(WIN32)
  list(APPEND COMBINED_SOURCES "${JPP_SRC_DIR}/util/win32_utils.cc")
endif()

# Object library for core engine (used by both codegen binary and final lib)
add_library(jumanlrc_core_objects OBJECT ${COMBINED_SOURCES})
target_compile_definitions(jumanlrc_core_objects PUBLIC ${CMAKE_CXX_FLAGS})

# codegen: build tool then generate feature code
set(CG_SPEC_INCLUDE "shared/jumandic_spec.h")
set(CG_SPEC_NAME "jumanpp::jumandic::SpecFactory")
configure_file("${JPP_SRC_DIR}/core/codegen/codegen_main.cc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gen/jumandic_codegen_main.cc")
add_executable(jumandic_cg_binary EXCLUDE_FROM_ALL
  "${CMAKE_CURRENT_BINARY_DIR}/gen/jumandic_codegen_main.cc"
  $<TARGET_OBJECTS:jumanlrc_core_objects>)
target_include_directories(jumandic_cg_binary PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include" "${JPP_SRC_DIR}"
  "${JPP_SRC_DIR}/jumandic" "${JPP_LIBS_DIR}")
target_link_libraries(jumandic_cg_binary ${CMAKE_THREAD_LIBS_INIT})
if(WIN32)
  target_link_libraries(jumandic_cg_binary shlwapi)
endif()

set(CG_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
set(CG_OUT_SRC "${CG_OUT_DIR}/jpp_jumandic_cg.cc")
set(CG_OUT_HDR "${CG_OUT_DIR}/jpp_jumandic_cg.h")
add_custom_command(OUTPUT ${CG_OUT_SRC} ${CG_OUT_HDR}
  COMMAND jumandic_cg_binary
  ARGS jpp_jumandic_cg JumandicStatic "${CG_OUT_DIR}"
  DEPENDS jumandic_cg_binary
  COMMENT "Generating static features")
add_custom_target(jumandic_codegen ALL DEPENDS ${CG_OUT_SRC} ${CG_OUT_HDR})

# single combined static library
add_library(jumanlrc_lib STATIC
  $<TARGET_OBJECTS:jumanlrc_core_objects>
  "${CG_OUT_SRC}"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jumanlrc_lib.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jumanlrc_capi.cc")
add_dependencies(jumanlrc_lib jumandic_codegen)
target_include_directories(jumanlrc_lib PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(jumanlrc_lib ${CMAKE_THREAD_LIBS_INIT})
if(WIN32)
  target_link_libraries(jumanlrc_lib shlwapi)
endif()

install(TARGETS jumanlrc_lib DESTINATION lib)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/jumanlrc_capi.h"
  DESTINATION include)

# ─── Example programs ───────────────────────────────────────────
option(JUMANLRC_BUILD_EXAMPLES "Build example programs" OFF)

if(JUMANLRC_BUILD_EXAMPLES)
  add_executable(json_example
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/json_example.cpp")
  target_include_directories(json_example PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/json")
  target_link_libraries(json_example PRIVATE
    jumanlrc_lib
    ${CMAKE_THREAD_LIBS_INIT})
  if(WIN32)
    target_link_libraries(json_example PRIVATE shlwapi)
  endif()
endif()
