add_library(esiwebthree web_three.cpp web_three.h)
target_link_libraries(esiwebthree PUBLIC esiconsensus webthree ethereum p2p devcore)
