cmake_minimum_required(VERSION 3.10)
project(jumanlrc_node)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options("/utf-8" "/wd4141" "/wd4018" "/wd4200")
endif()
add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNAPI_DISABLE_CPP_EXCEPTIONS
  -D_SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING)
if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x0600 -D_WIN32_IE=0x0800 -DWINVER=0x0600)
endif()

# ── Paths ─────────────────────────────────────────────────────
set(JPP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(JPP_SRC  "${JPP_ROOT}/src")
set(JPP_LIBS "${JPP_ROOT}/libs")
set(JPP_GEN  "${JPP_ROOT}/build/gen")
set(JPP_INC  "${JPP_ROOT}/include")

# ── Include directories ───────────────────────────────────────
include_directories(
  ${CMAKE_JS_INC}
  "${JPP_INC}"
  "${JPP_SRC}"
  "${JPP_LIBS}"
  "${JPP_LIBS}/pegtl"
  "${JPP_LIBS}/pathie-cpp/include"
  "${JPP_GEN}"
  "${CMAKE_CURRENT_SOURCE_DIR}/node_modules/node-addon-api"
)

# ── Source files (explicit list, matching parent CMakeLists.txt) ──
set(CORE_SRCS
  "${JPP_SRC}/util/mmap.cc"               "${JPP_SRC}/util/memory.cpp"
  "${JPP_SRC}/util/logging.cpp"            "${JPP_SRC}/util/string_piece.cc"
  "${JPP_SRC}/util/status.cpp"             "${JPP_SRC}/util/csv_reader.cc"
  "${JPP_SRC}/util/coded_io.cc"            "${JPP_SRC}/util/characters.cc"
  "${JPP_SRC}/util/printer.cc"             "${JPP_SRC}/util/codegen.cc"
  "${JPP_SRC}/util/assert.cc"              "${JPP_SRC}/util/format.cc"
  "${JPP_SRC}/util/parse_utils.cc"
  "${JPP_LIBS}/pathie-cpp/src/errors.cpp"
  "${JPP_LIBS}/pathie-cpp/src/entry_iterator.cpp"
  "${JPP_LIBS}/pathie-cpp/src/path.cpp"
  "${JPP_LIBS}/pathie-cpp/src/temp.cpp"
  "${JPP_LIBS}/pathie-cpp/src/pathie.cpp"
  "${JPP_SRC}/core/core.cc"                "${JPP_SRC}/core/features_api.cc"
  "${JPP_SRC}/core/env.cc"
  "${JPP_SRC}/core/analysis/analysis_input.cc"
  "${JPP_SRC}/core/analysis/analysis_result.cc"
  "${JPP_SRC}/core/analysis/analyzer.cc"
  "${JPP_SRC}/core/analysis/analyzer_impl.cc"
  "${JPP_SRC}/core/analysis/charlattice.cc"
  "${JPP_SRC}/core/analysis/dic_reader.cc"
  "${JPP_SRC}/core/analysis/dictionary_node_creator.cc"
  "${JPP_SRC}/core/analysis/extra_nodes.cc"
  "${JPP_SRC}/core/analysis/innode_features.cc"
  "${JPP_SRC}/core/analysis/lattice_builder.cc"
  "${JPP_SRC}/core/analysis/lattice_config.cc"
  "${JPP_SRC}/core/analysis/lattice_types.cc"
  "${JPP_SRC}/core/analysis/ngram_computations.cc"
  "${JPP_SRC}/core/analysis/normalized_node_creator.cc"
  "${JPP_SRC}/core/analysis/numeric_creator.cc"
  "${JPP_SRC}/core/analysis/onomatopoeia_creator.cc"
  "${JPP_SRC}/core/analysis/output.cc"
  "${JPP_SRC}/core/analysis/perceptron.cc"
  "${JPP_SRC}/core/analysis/score_processor.cc"
  "${JPP_SRC}/core/analysis/unk_nodes.cc"
  "${JPP_SRC}/core/analysis/unk_nodes_creator.cc"
  "${JPP_SRC}/core/dic/darts_trie.cc"
  "${JPP_SRC}/core/dic/dic_build_detail.cc"
  "${JPP_SRC}/core/dic/dic_builder.cc"
  "${JPP_SRC}/core/dic/dictionary.cc"
  "${JPP_SRC}/core/dic/dic_feature_impl.cc"
  "${JPP_SRC}/core/dic/entry_builder.cc"
  "${JPP_SRC}/core/dic/field_import.cc"
  "${JPP_SRC}/core/impl/feature_computer.cc"
  "${JPP_SRC}/core/impl/feature_debug.cc"
  "${JPP_SRC}/core/impl/feature_impl_combine.cc"
  "${JPP_SRC}/core/impl/feature_impl_compute.cc"
  "${JPP_SRC}/core/impl/feature_impl_ngram_partial.cc"
  "${JPP_SRC}/core/impl/feature_impl_pattern.cc"
  "${JPP_SRC}/core/impl/feature_impl_prim.cc"
  "${JPP_SRC}/core/impl/model_io.cc"
  "${JPP_SRC}/core/input/partial_example.cc"
  "${JPP_SRC}/core/input/partial_example_io.cc"
  "${JPP_SRC}/core/input/pex_stream_reader.cc"
  "${JPP_SRC}/core/input/stream_reader.cc"
  "${JPP_SRC}/core/input/training_io.cc"
  "${JPP_SRC}/core/spec/spec_compiler.cc"
  "${JPP_SRC}/core/spec/spec_dsl.cc"      "${JPP_SRC}/core/spec/spec_hashing.cc"
  "${JPP_SRC}/core/spec/spec_parser.cc"
  "${JPP_SRC}/core/spec/spec_parser_impl.cc"
  "${JPP_SRC}/core/spec/spec_serialization.cc"
  "${JPP_SRC}/core/spec/spec_types.cc"
  "${JPP_SRC}/core/codegen/feature_codegen.cc"
  "${JPP_SRC}/core/codegen/ngram_feature_codegen.cc"
  "${JPP_SRC}/core/codegen/partial_ngram_feature_codegen.cc"
  "${JPP_SRC}/core/codegen/pattern_feature_codegen.cc"
  "${JPP_SRC}/jumandic/shared/jumandic_spec.cc"
  "${JPP_SRC}/jumandic/shared/jumandic_spec_lexdata.cc"
  "${JPP_SRC}/jumandic/shared/jumandic_ids.cc"
  "${JPP_SRC}/jumandic/shared/jumandic_id_resolver.cc"
)
if(WIN32)
  list(APPEND CORE_SRCS "${JPP_SRC}/util/win32_utils.cc")
endif()

# Generated feature code (from codegen step)
set(GEN_SRCS "${JPP_GEN}/jpp_jumandic_cg.cc")

set(ADDON_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/addon.cc"
  "${JPP_SRC}/jumanlrc_capi.cc"
  "${JPP_SRC}/jumanlrc_lib.cc"
  ${CORE_SRCS}
  ${GEN_SRCS}
)

# ── Build the addon ───────────────────────────────────────────
add_library(${PROJECT_NAME} SHARED ${ADDON_SOURCES})
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} ${CMAKE_THREAD_LIBS_INIT})
if(WIN32)
  target_link_libraries(${PROJECT_NAME} shlwapi)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
  PREFIX ""
  SUFFIX ".node"
)
