# libactor

Libactor is an implementation of the actor model for C. It uses libdispatch for
concurrency and is designed for distributed programming.

## Features

* Process creation with C blocks
* Distributed message passing
* Process supervision

## System Requirement

Libactor has been tested in:

* OS X: 10.7 64bit
* FreeBSD: 9.2 64bit
* Ubuntu Linux: 12.04 64bit

Libactor uses **libdispatch** and makes masivly use of the block syntax for C. So it needs
to be compiled with clang/llvm.

## Installl

Currently the included makefile works for OS X. On linux or bsd you need to add some libraries.

## Example

To build the example on OS X type:

    make
    clang -fblocks -o example example.c -L. -lactor
    ./example
