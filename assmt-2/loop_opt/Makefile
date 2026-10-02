LLVM_BIN := ../../llvm-project/bld/bin
CLANG    := $(LLVM_BIN)/clang
OPT      := $(LLVM_BIN)/opt

CFLAGS   := -O2 -fno-unroll-loops -std=gnu11 -Wall
LDLIBS   := -lm

N   ?= 128
BLK ?= 32

all: matmul

matmul: matmul.c
	$(CLANG) $(CFLAGS) matmul.c -o matmul $(LDLIBS)

run: matmul
	./matmul $(N) $(BLK)

bench: matmul
	@echo "Loop optimization benchmark"      >  benchmark.txt
	@date                                     >> benchmark.txt
	@echo ""                                  >> benchmark.txt
	@./matmul 128 32                          >> benchmark.txt
	@echo ""                                  >> benchmark.txt
	@./matmul 512 64                          >> benchmark.txt
	@cat benchmark.txt

IRFLAGS := -O1 -Xclang -disable-llvm-passes

kernel.ll: kernel.c
	$(CLANG) -S -emit-llvm $(IRFLAGS) kernel.c -o kernel.ll

kernel.canon.ll: kernel.ll
	$(OPT) -S -mem2reg -loop-simplify -loop-rotate -indvars \
	       kernel.ll -o kernel.canon.ll

unroll.ll: kernel.canon.ll
	$(OPT) -S -loop-unroll -unroll-count=4 -unroll-allow-partial \
	       kernel.canon.ll -o unroll.ll

interchange.ll: kernel.canon.ll
	$(OPT) -S -loop-interchange -enable-loopinterchange \
	       -loop-interchange-threshold=-1000 \
	       -pass-remarks-output=interchange.yaml \
	       kernel.canon.ll -o interchange.ll
	@echo "---- loop-interchange remarks ----"
	@grep -E "Name:" interchange.yaml || true

ir: unroll.ll interchange.ll
	@echo ""
	@echo "kernel.canon.ll  - baseline loops"
	@echo "unroll.ll        - after -loop-unroll -unroll-count=4"
	@echo "interchange.ll   - after -loop-interchange"

opt-run: matmul-O3
	./matmul-O3 $(N) $(BLK)

matmul-O3: matmul.c
	$(CLANG) -O3 -march=native -std=gnu11 matmul.c -o matmul-O3 $(LDLIBS)

clean:
	rm -f matmul matmul-O3 *.ll *.yaml benchmark.txt

.PHONY: all run bench ir opt-run clean
