# Developer notes for lesson 1

Basically all you need to run the code in the lesson plus some useful tips.

## Useful commands

To compile command
`gcc hello_world.c -o hello_world`

To disassemble the code into Assembly
`gcc -S hello_world.c`

To optimize and check the optimizations with the `-S` flag.
`gcc -S -O2 hello_world.c` (capital letter O, not zero)

To check the documentation
`man printf` or `man 3 printf`

## Why the assembly calls `puts` instead of `printf`

Raw notes, to be massaged into proper docs.

### The short version

GCC treats `printf` as a *builtin*: it knows the function's exact semantics.
When the format string is a plain literal with no `%` conversions and ends in
`\n`, GCC rewrites the call to the cheaper, equivalent `puts`. The `\n` is
dropped from the string literal because `puts` appends a newline itself.

Visible in `hello_world.s`:

```asm
.LC0:
        .string "Hello, World!"      ; note: no \n
        ...
        call    puts@PLT
```

### Why it is legal

The C standard lets the compiler assume standard library functions behave as
specified. `printf("Hello, World!\n")` and `puts("Hello, World!")` are
observably identical, so the substitution cannot change program behaviour.

### Why it is worth doing

- `printf` has to scan the format string at runtime looking for `%`.
- `printf` is variadic, so the caller must set up vararg register state
  (e.g. zeroing `%eax` on x86-64 to say "no vector registers used").
- `puts` just writes bytes. Smaller code, faster, no format-parsing pulled in.

### Important: this is NOT an -O2 optimisation

Builtin folding happens in the GCC front end, not in the optimiser passes, so
it applies at every level. `-O0` already emits `call puts@PLT`. There is no
before/after to show by just toggling `-O2`.

Verified on this machine (GCC 13.3.0, Ubuntu 24.04):

| Flags                        | Call emitted          |
|------------------------------|-----------------------|
| `-O0`                        | `puts@PLT`            |
| `-O2`                        | `puts@PLT`            |
| `-O0 -fno-builtin`           | `printf@PLT`          |
| `-O0 -fno-builtin-printf`    | `printf@PLT`          |
| `-O2 -fno-builtin`           | `__printf_chk@PLT`    |
| `-O2 -fno-builtin-printf`    | `puts@PLT` (!)        |

### When the rewrite does NOT happen

- `printf("Hello, World!")` with no trailing newline stays `printf`
  (no `puts` equivalent exists).
- `printf("Hello %d\n", 42)` stays `printf` because there is a conversion.
- A format that is a single character plus newline can become `putchar`.
- `-fno-builtin` turns all builtin folding off.

### Gotcha: `-fno-builtin-printf` at -O2 still gives `puts`

Ubuntu's GCC enables `_FORTIFY_SOURCE` by default when optimising. With it,
`<stdio.h>` turns `printf` into a macro for `__builtin___printf_chk`, which
is a *different* builtin that `-fno-builtin-printf` does not cover. That is
why `-O2 -fno-builtin` shows `__printf_chk@PLT` and the printf-specific flag
appears to do nothing. Use plain `-fno-builtin` for demos.

### How to demonstrate the printf -> puts rewrite

Disable the builtin for the "before" side:

```sh
gcc -S -O0 -fno-builtin hello_world.c -o before.s   # call printf@PLT, string keeps \n
gcc -S -O2 hello_world.c -o after.s                 # call puts@PLT, string loses \n
```

Be honest in the lesson: the flag, not `-O2`, is what makes the difference.
The teaching point is "GCC knows the standard library well enough to replace
one call with a cheaper equivalent, even with optimisation off."

### What -O2 actually changes for hello world

Compare `-O0` and `-O2` without extra flags. Both call `puts`, but the body
of `main` is quite different.

`-O0` (frame pointer kept, value bounces through `%rax`, literal `0` for return):

```asm
main:
        endbr64
        pushq   %rbp
        movq    %rsp, %rbp
        leaq    .LC0(%rip), %rax
        movq    %rax, %rdi
        call    puts@PLT
        movl    $0, %eax
        popq    %rbp
        ret
```

`-O2` (no frame pointer, address loaded straight into `%rdi`, `xorl` for 0):

```asm
main:
        endbr64
        subq    $8, %rsp
        leaq    .LC0(%rip), %rdi
        call    puts@PLT
        xorl    %eax, %eax
        addq    $8, %rsp
        ret
```

Notes on the -O2 version:

- `subq $8, %rsp` / `addq $8, %rsp` is only there to keep the stack 16-byte
  aligned at the `call`, as the x86-64 ABI requires.
- `xorl %eax, %eax` is the idiomatic way to set a register to zero: shorter
  encoding than `movl $0, %eax` and breaks dependency chains.
- `.LFB0` becomes `.LFB23` because the header inlines more functions before
  `main` gets numbered; cosmetic, ignore.

### Useful one-liners

```sh
# Just the call instructions, for quick comparison
gcc -S -O0 -o - hello_world.c | grep call
gcc -S -O2 -o - hello_world.c | grep call

# Only the body of main, without the .cfi noise
gcc -S -O2 -o - hello_world.c | sed -n '/^main:/,/\.cfi_endproc/p' | grep -v '\.cfi'
```
