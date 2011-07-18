TARGET_LIB = libmpbench.so.1
EXETARGET = mp_bench

CXXSRC = \
	MPBController.cc \
	MPBLogicFactory.cc \
	MPBSampleLogic.cc

OBJCTS = $(patsubst %.cc, %.o, $(CXXSRC))

CXXFLAGS += -fPIC

$(TARGET_LIB) : $(OBJCTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -Wall -shared -Wl,-soname,$@ $^ -o $@

$(EXETARGET) : $(TARGET_LIB) $(EXETARGET).cc
	$(CXX) $(CXXLFAGS) $(LDFLAGS) -Wall -L./ -lmpbench -Wl,-rpath=./ $(EXETARGET).cc -o $@

.PHONY: all
all: $(EXETARGET)

.PHONY: clean
clean:
	rm -f $(OBJCTS) $(TARGET_LIB) $(EXETARGET)