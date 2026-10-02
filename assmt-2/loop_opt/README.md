# Loop Optimizations: Interchange, Tiling, Unrolling

Implementation and performance measurement of three classic loop
transformations, on a dense matrix-multiply driver (`C = A*B`, `N x N`,
`double`). Two tracks are provided:

- **Track A — source-level transforms** (`matmul.c`): the transformations are
  written out by hand as separate kernels and benchmarked against a naive
  baseline. Always reproducible, shows the *performance* payoff.
- **Track B — LLVM compiler passes** (`kernel.c` + `make ir`): the naive
  kernel is lowered to LLVM IR and LLVM's **own** loop passes are run with
  `opt`, so you can *see the transformation happen in the IR*.

Toolchain: the locally built LLVM 9 at `../../llvm-project/bld/bin`
(`clang`, `opt`).

## The three optimizations

### 1. Loop Interchange
Swap the order of two loops in a nest to improve spatial locality (make the
innermost loop stride-1) or to enable vectorization.

Naive `matmul` uses order `i, j, k`; the inner `k` loop walks `B` column-wise
(`B[k*N+j]`, stride `N`) — one cache miss per iteration. Interchanging to
`i, k, j` makes the inner `j` loop stride-1 over both `B` and `C`:

```
for i: for k:  a = A[i][k];
         for j:  C[i][j] += a * B[k][j];   // stride-1, cache friendly
```

### 2. Loop Tiling (blocking)
Partition the iteration space into small blocks (`BLK x BLK`) so the working
set of each block fits in cache and is reused before eviction. Applied on top
of the interchanged `i, k, j` loops with `ii, kk, jj` tile loops.

### 3. Loop Unrolling
Replicate the loop body (here the inner `j` loop by a factor of 4) to amortize
loop-control overhead (branch + counter) and expose instruction-level
parallelism. A remainder loop handles counts not divisible by 4.

## Build & run

```
make            # build ./matmul with local LLVM clang
make run        # Track A: benchmark table (default N=128, BLK=32)
make run N=512 BLK=64     # larger problem -> bigger speedups
make ir         # Track B: emit LLVM IR before/after the loop passes
make opt-run    # reference: naive source under clang -O3 -march=native
make clean
```

The baseline is deliberately built with `-O2 -fno-unroll-loops` so the
*source-level* transformations (not the compiler's auto-unroller) are what we
measure.

## Track A — measured results

Measured on this machine with `clang -O2 -fno-unroll-loops`:

**N = 128, BLK = 32**
```
kernel             time(ms)   GFLOP/s   speedup   check
naive                 1.717      2.44     1.00x    ref
interchange           0.576      7.29     2.98x    OK
tiled                 0.456      9.20     3.77x    OK
tiled+unroll          0.636      6.59     2.70x    OK
```

**N = 512, BLK = 64**
```
kernel             time(ms)   GFLOP/s   speedup   check
naive               431.009      0.62     1.00x    ref
interchange          37.292      7.20    11.56x    OK
tiled                25.100     10.69    17.17x    OK
tiled+unroll         38.216      7.02    11.28x    OK
```

Observations:
- **Interchange** alone gives the biggest single jump (≈3x at N=128, ≈12x at
  N=512) — locality dominates at this scale.
- **Tiling** adds further gains (≈3.8x / ≈17x) as the problem grows beyond
  cache: blocking keeps reused data resident.
- **tiled+unroll**: manual unrolling here does *not* help and slightly
  regresses, because at `-O2` it interferes with the compiler's
  auto-vectorization of the clean inner loop. This is a realistic result:
  unrolling pays off mainly when it removes a real control-overhead
  bottleneck or when the compiler cannot vectorize; otherwise the hand-unroll
  can get in the way. (`check = OK` confirms all kernels compute the same
  result.)

Your exact numbers will vary with CPU/cache, but the ordering
naive < interchange < tiled is stable.

## Track B — LLVM's own passes (IR inspection)

`make ir` runs:

1. `clang -O1 -disable-llvm-passes -emit-llvm` → `kernel.ll` (raw IR)
2. `opt -mem2reg -loop-simplify -loop-rotate -indvars` → `kernel.canon.ll`
   (canonical loop form the loop passes expect)
3. **Unrolling:** `opt -loop-unroll -unroll-count=4 -unroll-allow-partial`
   → `unroll.ll`. The inner loop body is replicated — the count of
   `fmul`/`fadd` instructions jumps from 3 to ~2560, i.e. the arithmetic is
   unrolled.
4. **Interchange:** `opt -loop-interchange -enable-loopinterchange
   -loop-interchange-threshold=-1000` → `interchange.ll`. The pass emits the
   optimization remark **`Name: Interchanged`** on `interchange_demo()`,
   confirming the `i`/`j` loops were swapped. (LLVM 9's interchange is
   off-by-default and cost-model-gated; the threshold override lets the legal
   interchange proceed.)

### A note on tiling in LLVM
LLVM has **no standalone loop-tiling pass** in a plain `opt`. Loop/affine
tiling is provided by the **Polly** subproject (`opt -polly -polly-tiling`),
which is **not compiled into this toolchain**. Tiling is therefore
demonstrated and measured at the source level (`mm_tiled`). To enable the
compiler-pass route, LLVM would need to be rebuilt with
`-DLLVM_ENABLE_PROJECTS="...;polly"`.

## Files
- `matmul.c` — four kernels (naive / interchange / tiled / tiled+unroll) +
  timing & correctness harness (Track A).
- `kernel.c` — minimal kernels fed to LLVM's loop passes (Track B).
- `Makefile` — build/run/ir/clean targets wired to the local LLVM.
