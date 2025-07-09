
CPU related tests ported from "Acid800" (Altirra Acid800 test suite)
--------------------------------------------------------------------------------

cpu_insn.prg:
 - briefly check regular instructions
cpu_illegal.prg:
 - briefly check illegal instructions
cpu_flags.prg:
 - check cpu flags
cpu_decimal.prg:
 - check decimal mode
cpu_timing.prg:
 - checks instruction timing
cpu_bugs.prg:
 - checks BRK and JMP() bugs
cpu_clisei.prg:
 - checks some interrupt flag corner cases

--------------------------------------------------------------------------------

TODO:
-----
cpu_timing.prg:
 - should probably use CIA timers instead of rasterlines
