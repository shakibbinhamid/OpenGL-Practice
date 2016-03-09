BIN = coursework1
CC = g++
FLAGS = -std=c++11 -stdlib=libc++
INC = -I/usr/local/include -I ./include
LIB_PATH = ./lib/
LOC_LIB = $(LIB_PATH)libGLEW.a $(LIB_PATH)libglfw3.a
FRAMEWORKS = -framework Cocoa -framework OpenGL -framework IOKit
SRC = main.cpp gl_util.cpp

all: compile

compile:
	${CC} ${FLAGS} ${FRAMEWORKS} -o ${BIN} ${SRC} ${INC} ${LOC_LIB}