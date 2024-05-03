PROJECT_ROOT = ../../..
INCDIR = $(PROJECT_ROOT)/include
LIBDIR = $(PROJECT_ROOT)/src/lib
BINDIR = $(PROJECT_ROOT)/src/bin
LIBDIR9 = $(PROJECT_ROOT)/lib
BINDIR9 = $(PROJECT_ROOT)/bin

SRCS = my_fortune.c 
OBJS = $(SRCS:.c=.o)

# LIBS = -lthread_mgr -llog_mgr 
BINS = my_fortune

FLAGS = -I$(INCDIR) -L$(LIBDIR9)
CC = gcc $(FLAGS)


it: $(BINS)

$(BINS): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(BINS)
	chmod 755 $(BINS)

depend: $(SRCS)
	$(CC) -M $(SRCS) > dependlist

install: $(BINS)
	cp $(BINS) $(BINDIR9)

clean:
	rm -f $(OBJS) $(BINS) dependlist

.c.o:
	$(CC) -c $<

-include dependlist 
