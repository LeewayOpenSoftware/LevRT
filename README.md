# LevRT

LevRT: Leeway Windows Runtime (WinRT) implementation for LiberNT

## Purpose

LevRT is the Leeway Runtime: an implementation of the Windows Runtime (WinRT) activation, metadata and type layer for LiberNT. The public ABI keeps its Microsoft names (`combase`, `wintypes`, `Windows.*`); LevRT is the project and implementation name.

LevRT is developed as its own repository until it is ready to join the operating
system tree at [LiberNT-LDW](https://github.com/LeewayOpenSoftware/LiberNT-LDW),
the Leeway Open Software fork of LiberNT.

## Status

**Design.** No code yet. The design document for LevRT and LAML together is in
[docs/DESIGN.md](docs/DESIGN.md).

## Clean-room rule

Only public Microsoft documentation, Wine and ReactOS sources, and observable
behaviour are used as references. No Microsoft code, strings, disassembly,
`.winmd` or `.xbf` data is copied into this repository.

## License

Not chosen yet.
