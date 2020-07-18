SRC_PATH= ~/Desktop/DATA_SCIENCE/GIT-repos/Scientific-Computing/producer-n-consumers_2/app
OUT_PATH= ${PWD}
VPATH=${SRC_PATH}
FILES=main_yield.cpp
SOURCES = $(FILES:%.cpp=$(SRC_PATH)/%.cpp)

CC=clang++
CFLAGS=-g -std=c++17

all:
	${CC} ${CFLAGS} ${SOURCES} -o ${OUT_PATH}/main