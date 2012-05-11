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

To install libactor make shure libdispatch is installed. On OS X it is included since 10.6.
On Linux there are several ports. On Debian based distributions simply type:

    sudo apt-get install libdispatch-dev

Install of libactor:

    sudo make install

## Example

To build the example on OS X type:

    clang -o example example.c -lactor

On Linux:

    clang -fblocks -o example example.c -lactor -ldispatch -lBlocksRuntime

## To be continued

More precise documentation will come...
