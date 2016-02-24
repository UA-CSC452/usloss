
srcdir = .
prefix = /home/jhh/452-students
exec_prefix =   ${prefix}

AC_FLAGS =              -DHAVE_CONFIG_H
INSTALL =               /usr/bin/install -c
INSTALL_PROGRAM =       ${INSTALL}
INSTALL_DATA =          ${INSTALL} -m 644
CC = 			gcc
LD = 			@LD@
LIB_DIR = 		/home/jhh/452-students/lib/linux
INC_DIR	= 	    ${prefix}/include
CFLAGS =        -g -O2

COBJS = libuser.o
CFLAGS += -Wall -g -I. -I$(INC_DIR) -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast
TARGET = libuser.a

ifeq ($(shell uname),Darwin)
	CFLAGS += -D_XOPEN_SOURCE
endif


$(TARGET):	$(COBJS)
		$(AR) -r $@ $(COBJS) 

install:	$(TARGET)
		mkdir -p $(INC_DIR) $(LIB_DIR)
		$(INSTALL_DATA) libuser.h $(INC_DIR)
		$(INSTALL_DATA) $(TARGET) $(LIB_DIR)

libuser.o:	libuser.h libuser.c

clean:
	rm -f $(COBJS) $(TARGET) 
