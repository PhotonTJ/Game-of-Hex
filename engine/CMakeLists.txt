add_library(hex_engine_core STATIC
  src/board.cpp
  src/ai.cpp
  src/game.cpp
)
target_include_directories(hex_engine_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(hex_engine_core PUBLIC cxx_std_17)

if(HEX_BUILD_TESTS)
  add_executable(hex_engine_tests tests/test_main.cpp)
  target_link_libraries(hex_engine_tests PRIVATE hex_engine_core)
  target_include_directories(hex_engine_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

  enable_testing()
  add_test(NAME hex_engine_tests COMMAND hex_engine_tests)
endif()

if(HEX_BUILD_TOOLS)
  add_executable(hex_benchmark tools/benchmark.cpp)
  target_link_libraries(hex_benchmark PRIVATE hex_engine_core)
endif()

if(HEX_BUILD_PYTHON_BINDINGS)
  add_subdirectory(bindings)
endif()
