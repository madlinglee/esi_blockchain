file(GLOB sources "*.cpp" "*.h")

add_library(esipbftseal ${sources})

target_include_directories(esipbftseal PRIVATE ../utils)
target_link_libraries(esipbftseal PUBLIC ethereum)
