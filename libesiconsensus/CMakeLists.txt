file(GLOB sources "*.cpp" "*.h")

add_library(esiconsensus ${sources})

target_link_libraries(esiconsensus PUBLIC esipbftseal esipbft)
