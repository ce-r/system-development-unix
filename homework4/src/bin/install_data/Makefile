PROJECT_ROOT = ../../..
INCDIR = $(PROJECT_ROOT)/include
LIBDIR = $(PROJECT_ROOT)/src/lib
BINDIR = $(PROJECT_ROOT)/src/bin
LIBDIR9 = $(PROJECT_ROOT)/lib
BINDIR9 = $(PROJECT_ROOT)/bin

SRCS = install_data.c 
OBJS = $(SRCS:.c=.o)

LIBS = -lshmlib 
INSTALLED_LIBS=$(PROJECT_ROOT)/lib/libshmlib.a
BINS = install_data

FLAGS = -I$(INCDIR) -L$(LIBDIR9)
CC = gcc $(FLAGS)


it: $(BINS)

$(BINS): $(OBJS) $(INSTALLED_LIBS)
	$(CC) $(OBJS) $(LIBS) -o $(BINS)
	chmod 755 $(BINS)

depend: $(SRCS)
	$(CC) -M $(SRCS) > dependlist

install: $(BINS)
	cp $(BINS) $(BINDIR9)

clean:
	rm -f $(OBJS) $(BINS) dependlist logfile

.c.o:
	$(CC) -c $<

-include dependlist