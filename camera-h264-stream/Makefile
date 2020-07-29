# variable
CC         = gcc
CFLAGS     = -I./ffmpeg/include
LDFLAGS    = -L./ffmpeg/lib \
             -L./x264/lib \
             -lavdevice -lm -lavfilter \
             -pthread -lm -lswscale -lm -lpostproc -lm -lavformat -lm -lz -lavcodec -pthread -lm -lz \
             -lx264 -pthread -lm -lswresample -lm -lavutil -pthread -lm -ldl
SRC        = ./main.c
BINARYNAME = camera-h264-stream

all: build

fmt:
	clang-format -i *.c

build:
	${CC} ${CFLAGS} -o ${BINARYNAME} ${SRC} ${LDFLAGS}

clean:
	rm -rf *.o *~
	rm -rf ${BINARYNAME}
