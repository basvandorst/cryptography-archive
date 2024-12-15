
CPLUSFLAGS = -I../ -Wall -g 
CPLUS = g++ $(CPLUSFLAGS)

OBJS = 	NewsServer.o NewsGroup.o \

CONTAINERS = NewsGroup.List.o NewsTreeList.o
CONTAINERLIB = libcontain.a

.cc.o: $<
	$(CPLUS) -c $<

all: news++

$(CONTAINERLIB): $(CONTAINERS)
	ar -r $(CONTAINERLIB) $(CONTAINERS) $(OBJS)
	ranlib $@

news++: test.o $(OBJS) $(CONTAINERLIB) ../libnews.a
	$(CPLUS) test.o $(OBJS) -o news++ $(CONTAINERLIB) -L../ -lnews


clean:
	rm -f *.o *.lib news++

depend:
	$(CPLUS) -MM *.cc > .depend

#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
