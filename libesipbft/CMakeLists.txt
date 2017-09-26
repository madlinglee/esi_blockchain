file(GLOB sources "*.cpp" "*.h")

add_library(esipbft ${sources})

target_link_libraries(esipbft PUBLIC devcrypto devcore)
