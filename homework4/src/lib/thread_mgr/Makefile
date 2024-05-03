PROJECT_ROOT = ../../..
INCDIR = $(PROJECT_ROOT)/include
LIBDIR = $(PROJECT_ROOT)/src/lib
LIBDIR9 = $(PROJECT_ROOT)/lib

SRCS = thread_mgr.c 
OBJS = $(SRCS:.c=.o)

LIBS = libthread_mgr.a
CURRDIR = thread_mgr

FLAGS = -I$(INCDIR) -L$(LIBDIR) -l$(CURRDIR)
CC = gcc $(FLAGS)


it: $(LIBS)

$(LIBS): $(OBJS)
	ar -r $(LIBS) $(OBJS)

depend: $(SRCS)
	$(CC) -M $(SRCS) > dependlist

install: $(LIBS)
	cp $(LIBS) $(LIBDIR9)

clean:
	rm -f $(OBJS) $(LIBS) dependlist

.c.o:
	$(CC) -c $<
	
-include dependlist 