# @Author: fjk
# @Date:   2017-12-29T19:05:24+08:00
# @Email:  sunnyfjk@gmail.com
# @Filename: Makefile
# @Last modified by:   fjk
# @Last modified time: 2017-12-30T14:18:14+08:00

TAG=main
SRC=$(shell find ./ -name "*.c")
INC=-I./include
LIBS= -lpthread
CONFIGS=-D__DEBUG__
CC=gcc
$(TAG):$(patsubst %.c,%.o,$(SRC))
	$(CC) -o $@ $^ $(LIBS)
.c.o:
	$(CC) -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" $(CONFIGS)  $(INC) $(LIBS) -o "$@" "$<"
clean:
	rm -fr $(patsubst %.c,%.o,$(SRC)) $(patsubst %.c,%.d,$(SRC)) $(TAG)
