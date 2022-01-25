CC = g++11.1
TESTLIBS = -lgtest  -lgtest_main -lpthread -lz

OBJS1 = SimpleHeader.o bouncer.cc
OBJS2 = SimpleHeader.o recv.cc
OBJS3 = SimpleHeader.o test_header.cc
# HEADERDIRS =
CCFLAGS = --std=c++11

all: send recv test

send: $(OBJS1)
	$(CC) -o $@ -L. $(OBJS1) $(TESTLIBS)

recv: $(OBJS2)
	$(CC) -o $@ -L. $(OBJS2) $(TESTLIBS)

test: $(OBJS3)
		$(CC) -o $@ -L. $(OBJS3) $(TESTLIBS)

%.o : %.cc
	$(CC) $(CCFLAGS) -c $<
	$(CC) $(CCFLAGS) -MM -MP -MT $@ $< > $(basename $@).d


.PHONY : clean
clean :
	rm -f *.o *~ *.d


## include the generated dependency files
-include $(addsuffix .d,$(basename $(OBJS1)))
-include $(addsuffix .d,$(basename $(OBJS2)))
