RV_ROOT = $(SS_TOOLS)/riscv64-unknown-elf
MARCH   = rv64imac
ABI     = lp64
IFLAGS  = -I$(SS_TOOLS)/include                                              \
	      -I$(SS)/ss-workloads/common/include                                \
	      -I$(RV_ROOT)/include/c++/8.1.0/                                    \
	      -I$(RV_ROOT)/include/c++/8.1.0/riscv64-unknown-elf/$(ARCH)/$(ABI)/ \
	      -I$(RV_ROOT)/include/c++/8.1.0/riscv64-unknown-elf/

CLANG_FLAGS = -O2 -fno-inline -fno-vectorize -fno-slp-vectorize \
              -fno-unroll-loops -stream-specialize -emit-llvm -c -std=c++11

CLANG = clang++
OPT   = opt
LLC   = llc
GCC   = riscv64-unknown-elf-g++

# Compile pragma hinted high-level language to encoded metadata.
%.bc: %.cc
	$(CLANG) $< -o $@ $(CLANG_FLAGS) $(IFLAGS) --sysroot=$(RV_ROOT) 

# Transform code regions to be accelerated to decoupled spatial IR.
dsa-%.bc: %.bc
	SBCONFIG=$(SBCONFIG)                                \
	$(OPT) -load $(SS_TOOLS)/lib/libStreamSpecialize.so \
		   -stream-specialize $< -o $@

# Apply general-purpose optimization to clean up the LLVM IR. 
ready-%.bc: %.bc
	$(OPT) -O3 < $< > $@

# Assembly code generation. Corresponding RISCV modeules should
# be enabled in the mattr flags.
%.s: %.bc
	$(LLC) -mattr=+m,+f,+a,+c $<

# Use GCC tools to link the assembly code to executable binaries.
%.out: %.s
	$(GCC) $< -o $@ -march=$(MARCH) -mabi=$(ABI) -lm

clean:
	rm -f *.bc *.ll *.o *.out *.s *.dfg.h *.dfg
