GCC   = riscv64-unknown-linux-gnu-g++
RV_ROOT = $(SS_TOOLS)/riscv64-unknown-elf
MARCH   = rv64imac
ABI     = lp64

DFG = dotp.dfg dotp_16.dfg
SBCONFIG=$(SS)/spatial-scheduler/configs/revel-1x1.sbmodel 

TESTS=main
TESTC=$(TESTS:=.cc)
TEST_OBJS=$(TESTS:=.o)
all: $(TESTS)

DFG_HEADER = $(DFG:.dfg=.dfg.h)

IFLAGS  = -I$(SS_TOOLS)/include               \
	      -I$(SS)/dsa-apps/common/include \
		  -g -ggdb -gdwarf-3 -static -DOPENMP -fopenmp \
		  -L. -lgomp -lpthread -ldl \
		  -I. -static

$(DFG_HEADER): %.dfg.h: %.dfg
	ss_sched -v -e 114514 $(SBCONFIG) $<


$(TEST_OBJS): $(TESTC) $(DFG_HEADER)
	 $(GCC) -c $< $(IFLAGS) -I. -static -o $(@)

# targets is main and dependent on main.o
$(TESTS): $(TEST_OBJS)
	 $(GCC) $(IFLAGS) -static -o $(@) $<


%.s: %.cc $(DFG_HEADER)
	$(GCC) $< $(IFLAGS) -O3 -lm -S -c

clean:
	rm -f *.bc *.ll *.o *.out *.s *.dfg.h
	rm -rf m5out sched viz verif stats
	rm main
