UTSRC=..
SRC=../${UTSRC}
include ../../define.mk
LIBS    := \
		   -lp2p \
		   -lethcore \
		   -ldevcrypto  \
		   -ldevcore \
		   -lboost_system \
		   -lboost_thread \
		   -lboost_filesystem \
		   -lboost_random\
		   -lminiupnpc\
		   -lmemenv \
		   -ljson_spirit \
		   -lsecp256k1\
		   -lcryptopp\
		   -lscrypt \
		   -lpthread \
		   -lrt
LDFLAGS := -L${BOOST_LIB} -L${LEVEL_DB_LIB} -L${ETH_JSON_SPIRIT_LIB} -L${ETH_SCRYPT_LIB} -L${ETH_SECP256K1_LIB} \
					  -L${ETH_CRYPTO_LIB} -L${ETH_CORE_LIB} -L${ETH_P2P_LIB} -L${ETH_ETHCORE_LIB}
DEFINES := -Wno-unknown-pragmas -Wno-unused-local-typedefs -std=c++11 -g
INCLUDE := -I.. -I${BOOST_INC} -I${ETH_UTILS_INC} -I${SRC}/ethereum-lib -I${SRC}/src/p2p -I../../pbft_demo/ -I${SRC}/src/consensus/

#引入静态库
GMOCK_LIB := $(LDFLAGS) $(LIBS) ${GMOCK_DIR}/gmock_main.a ${GMOCK_DIR}/gtest_main.a ${GMOCK_DIR}/gtest.a
INCF := -I${GMOCK_INC} ${INCLUDE}


#定义目标文件
#OBJS :=test_tool.o sha_test.o pbft_state_machine.o main.o utils.o msg_collector.o  time_out_ticker.o time_out_ticker_test.o
OBJS := main.o pbft_state_machine_test_data.o key_wrapped.o validator_set.o vote_set.o test_tool.o vote_set_test.o pbft_state_machine.o key_pair_test.o pbft_state_machine_test.o main.o utils.o msg_collector.o  time_out_ticker.o time_out_ticker_test.o consenter_test.o p2p_node.o p2p_capability.o p2p_hostcapability.o
TARGET := test
ALL_TARGETS= ${OBJS} ${TARGET}
#覆盖率参数
COVERAGE := -fprofile-arcs -ftest-coverage

.PHONY:all, target
#生成可执行程序
all : target
target : $(OBJS)
	$(CC) -o ${TARGET} $^ -lpthread -lgcov $(GMOCK_LIB)
%.o : %.cpp
	$(CC) $(DEFINES) $(INCF) -c $^
%.o : ../${UTSRC}/pbft_demo/%.cpp
	$(CC) $(DEFINES) $(INCF) -c $^
testcov:
	-rm *.gcda info output output.tar ut_base.info -rf
	#-./$(TARGET) 忽略执行过程的错误
	./$(TARGET)
	lcov --capture --directory . --output-file ut_base.info --derive-func-data --compat-libtool --checksum
	@genhtml ut_base.info --output-directory output --show-details --legend --show-details
	@find output -name "*.html" -type f|xargs sed -i "s/ISO-8859-1/utf-8/g"
	@tar cvf output.tar output
