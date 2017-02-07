### Constants: g++
CXX=g++ -std=c++11
CXX_OPT=-Werror -Wextra -Wall -Wfatal-errors -pedantic -O3
INC=-I.
LIB=-lhiredis

### Constants: gtest
GTEST_ROOT_DIR=ext/googletest/googletest
GTEST_BUILD_DIR=${GTEST_ROOT_DIR}/build
GTEST_INC_DIR=${GTEST_ROOT_DIR}/include
GTEST_MAIN=${GTEST_BUILD_DIR}/libgtest_main.a
GTEST_INC=-I${GTEST_INC_DIR}
GTEST_LIB=${GTEST_BUILD_DIR}/libgtest.a
GTEST_TARGET=bin/gtest

### Test binaries
TEST_OBJ=\
	test/adapter.o\
	test/redis.o\
	test/store.o

### Example binaries
BIN=\
	bin/example

### Top-level commands
all: ${BIN}
check: ${GTEST_TARGET}
	${GTEST_TARGET}
clean:
	rm -rf ${GTEST_BUILD_DIR} ${GTEST_TARGET} ${TEST_OBJ} ${BIN}

### Build rules
submodule:
	git submodule init
	git submodule update
bin/%: tools/%.cc include/*.h
	${CXX} ${CXX_FLAGS} ${INC} $< -o $@ ${LIB}
%.o: %.cc include/*.h
	${CXX} ${CXX_FLAGS} ${GTEST_INC} ${INC} -c $< -o $@
${GTEST_LIB}: submodule
	mkdir -p ${GTEST_BUILD_DIR}
	cd ${GTEST_BUILD_DIR} && cmake .. && make
${GTEST_TARGET}: ${GTEST_LIB} ${GTEST_MAIN} ${TEST_OBJ} test/*.h
	${CXX} ${CXX_OPT} -o $@ ${TEST_OBJ} ${GTEST_LIB} ${GTEST_MAIN} ${LIB} -lpthread
