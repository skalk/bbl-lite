RISC-V Boot Loader (Lite)
=============================

About
---------

This package contains the BBL-Lite, which is a minimal supervisor execution
environment for RISC-V systems. It is designed to host the RISC-V Linux port.

Build Steps
---------------

We assume that the RISCV environment variable is set to the RISC-V tools
install path, and that the riscv-gnu-toolchain package is installed.

    $ mkdir build
    $ cd build
    $ ../configure --prefix=$RISCV --host=riscv64-unknown-elf --with-payload=linux-4.6.2/vmlinux
    $ make

By default, the 64-bit (RV64) version of `bbl` is built.  To build a 32-bit
(RV32) version, supply a `--enable-32bit` flag to the configure command.

References
---------------

BBL-Lite is derived from [riscv-pk](https://github.com/riscv/riscv-pk/).
