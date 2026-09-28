# Local Value Numbering (LVN)

Implementation of the Local Value Numbering assignment from
`Local_Value_Numbering-1.pdf`. Reads quadruples and emits optimized
quadruples using the classic three-table LVN algorithm.

## Optimizations
- Constant folding
- Constant propagation
- Copy propagation
- Common Subexpression Elimination (CSE)
- **Commutativity** handling (`a*b` == `b*a`, `a+b` == `b+a`)
- **Array reference** handling (`x = arr[idx]`, `arr[idx] = x`, with store
  invalidation of cached loads)

## Data structures (as in the PDF)
- `ValnumTable`: name → value number
- `HashTable`: canonical expression → value number
- `NameTable`: value number → { name list, constant value, const flag }

## Build & run
```
make            # builds ./lvn
make run        # runs on input.quad (the PDF example)
make test       # runs both input.quad and input_extra.quad
./lvn <file>    # run on any quadruple file (or read stdin)
```

## Input syntax
One quadruple per line. `#` or `//` begin comments (whole-line or inline).
```
dst = value          # constant / copy      e.g. a = 10 ,  e = i
dst = op1 <op> op2   # binary               e.g. t1 = i * j
dst = arr[idx]       # array load           e.g. t3 = A[t1]
arr[idx] = src       # array store          e.g. A[t1] = t6
```
`<op>` is one of `+ - * /`.

## Output
Two sections:
1. **Annotated** — every input quad after optimization, with notes
   (`// constant`, `// same as t1`, `// deleted`).
2. **Final (renumbered)** — deleted (redundant/constant-temp) quads removed
   and the remaining quads renumbered.

## Verified result (PDF example, `input.quad`)
```
1. a = 10
2. b = 40
3. t1 = i * j
4. c = t1 + 40
5. d = 150 * c
6. e = i
7. t4 = i * 10
8. c = t1 + t4
```
This reproduces the "Quadruples after value numbering" slide (instructions
`t2 = 150` and `t3 = i*j` are deleted).
