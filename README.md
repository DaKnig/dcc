# Da C Compiler

This is supposed to be a C compiler... but not a very standard/good one.

in constant development.
matrix channel - [#dcc-dev:matrix.org](https://matrix.to/#/%23dcc-dev%3Amatrix.org)

## goals for the project

1. standard compliant C compiler

2. high quality optimization

3. targetting sm83 and 16 bit x86

4. superoptimization for sm83

## progress bar

- [x] C expressions (all except sizeof, alignof and compound expression - basically everything that's not related to types)

- [x] statements (no support for declarations in expr1 in a for loop)

- [ ] declarations (alt syntax) (we are getting there... slowly... very slowly. simple declarations in the next few commits...)

- [ ] functions (minimal progress)

- [ ] structs (not supported in the next few months)

- [ ] symbol table (will start working on it soon)

- [x] IR definition (almost complete, usable.)

- [ ] outputting DCC IR (not done yet)

## other repos in the DCC project

sm83 backend - [vbsm83(name pending)](https://github.com/GreenAndEievui/vbsm83)

custom language frontend - [tinymm8](https://github.com/Oderjunkie/tinymm8)
