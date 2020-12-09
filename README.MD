# xcc.c

The overarching goal for this project is to create a [self-hosting](https://en.wikipedia.org/wiki/Self-hosting_(compilers)) compiler. I've chosen to write a compiler in (and that) targets a subset of C. Obviously, in order to make a self-hosting compiler, that subset needs to have sufficient functionality to be able to write a compiler in. As of right now, I've implemented a lot of the basic functionality in the compiler, but there's still a lot more to go. A secondary goal of this project is to make the compiler [singlepass](https://en.wikipedia.org/wiki/One-pass_compiler).

I started this project in late 2019, and then put it on hiatus, and I'm now returning to it in late 2020.

You can see my progress by checking out the main test suite program, [test_1.c](https://github.com/XavierCooney/xcc/blob/master/test_1.c). The code generation targets x86-64 systems with the System V (Linux, etc) ABI.

Please note that in this project I intend to make a compiler that functions, but not necessarily one that produces an optimised output.  Although I plan on some basic optimisations (like constant folding), the goal of making the compiler one-pass precludes a lot of more advanced optimisations.