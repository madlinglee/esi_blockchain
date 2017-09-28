set(
    sources
    genesis_info.h
    main.cpp
)

add_executable(esi ${sources})
target_include_directories(esi PRIVATE ../utils)
target_link_libraries(
    esi
    PRIVATE esiwebthree esiconsensus ethereum web3jsonrpc devcore
)

install(TARGETS esi DESTINATION bin)
