COBJS = pterm.o
CC=gcc
CFLAGS  = -DBSD_COMP -g
TARGET = pterm

ifeq ($(shell uname),Darwin)
    # Add a few things for the Mac
    CFLAGS += -D_XOPEN_SOURCE
    CFLAGS += -Wno-int-to-void-pointer-cast
    OS = macosx
else
	OS = linux
endif


ifndef CS452_STUDENTS
CS452_STUDENTS = $(HOME)/Dropbox/452-students
endif

INSTALL = $(CS452_STUDENTS)

$(TARGET): $(COBJS)
	$(CC) -o $(TARGET) $(COBJS)

clean:
	rm -f $(COBJS) $(TARGET)

install: $(TARGET)
	mkdir -p $(INSTALL)/bin/$(OS)
	cp $(TARGET) $(INSTALL)/bin/$(OS)

