GCC   = riscv64-unknown-linux-gnu-g++
RV_ROOT = $(SS_TOOLS)/riscv64-unknown-elf
MARCH   = rv64imac
ABI     = lp64

DFG = dotp.dfg
DFG_HEADER = $(DFG:.dfg=.dfg.h)
SBCONFIG=$(SS)/spatial-scheduler/configs/revel-1x1.sbmodel 

TESTS=main
TESTC=$(TESTS:=.cpp)
TEST_OBJS=$(TESTS:=.o)

all: $(DFG_HEADER) $(TESTS)


OPT?=-O0 #-O3
CFLAGS=$(OPT) -g -ggdb -gdwarf-3  -DOPENMP -fopenmp -I$(SS)/ss-workloads/common/include -I$(SS_TOOLS)/include -I$(SS_TOOLS)/dsa-apps/common/include
LIB=-L. -lgomp -lpthread -ldl # -lm5
LFLAGS = -lm

$(DFG_HEADER): %.dfg.h: %.dfg
	ss_sched -v -e 114514 $(SBCONFIG) $<

$(TEST_OBJS): $(TESTC) $(DFG_HEADERS)
	 $(GCC) -c $< $(LIB) $(CFLAGS) $(MACROS) -I. -static -o $(@)

# targets is main and dependent on main.o
$(TESTS): $(TEST_OBJS)
	 $(GCC) $(LIB) $(CFLAGS) $(MACROS) -static -o $(@) $< $(LFLAGS)

clean:
	rm -f *.bc *.ll *.o *.out *.s *.dfg.h
	rm -rf m5out sched viz verif stats
	rm main

cleandfg:
	rm -f *.dfg.h
