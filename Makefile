all:
	g++ -std=c++11 -Werror -Wextra -Wall -pedantic -I. tools/database.cc -o bin/database -lhiredis
	g++ -std=c++11 -Werror -Wextra -Wall -pedantic -I. tools/example.cc -o bin/example -lhiredis

clean:
	rm -f bin/example bin/database


### Constants: g++
INC=-Iinclude 
CXX=g++ -std=c++11
CXX_OPT=-Werror -Wextra -Wall -Wfatal-errors -pedantic 

### Constants: gtest
GTEST_ROOT_DIR=ext/googletest/googletest/
GTEST_BUILD_DIR=${GTEST_ROOT_DIR}/build
GTEST_INC_DIR=${GTEST_ROOT_DIR}/include
GTEST_MAIN=${GTEST_BUILD_DIR}/libgtest_main.a
GTEST_INC=-I${GTEST_INC_DIR}
GTEST_LIB=${GTEST_BUILD_DIR}/libgtest.a
GTEST_TARGET=bin/gtest

### Test binaries
TEST_OBJ=\
	test/database.o

### Top-level commands
all: ${GTEST_TARGET}
check: ${GTEST_TARGET}
	${GTEST_TARGET}
submodule:
	git submodule init
	git submodule update
clean:
	rm -rf ${GTEST_BUILD_DIR} ${GTEST_TARGET} ${TEST_OBJ}

### Build rules
%.o: %.cc include/*.h
	${CXX} ${CXX_FLAGS} ${GTEST_INC} ${INC} -c $< -o $@
${GTEST_LIB}:
	mkdir -p ${GTEST_BUILD_DIR}
	cd ${GTEST_BUILD_DIR} && cmake .. && make
${GTEST_TARGET}: ${GTEST_LIB} ${GTEST_MAIN} ${TEST_OBJ}
	${CXX} ${CXX_OPT} -o $@ ${TEST_OBJ} ${GTEST_LIB} ${GTEST_MAIN} -lpthread
