CXX=g++
RM=rm -f
CPPFLAGS=-std=c++14 -Wall
LDFLAGS=
LDLIBS=

SRCS=cpu.cc main.cc
OBJS=$(subst .cc,.o,$(SRCS))

all: project1

project1: $(OBJS)
	$(CXX) $(LDFLAGS) -o project1 $(OBJS) $(LDLIBS)

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS) project1

distclean: clean
	$(RM) *~ .depend

include .depend
