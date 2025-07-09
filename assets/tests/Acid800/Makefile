all: cpu_decimal.prg cpu_flags.prg cpu_timing.prg cpu_insn.prg cpu_illegal.prg \
	cpu_timing_ntsc.prg cpu_bugs.prg cpu_clisei.prg

cpu_decimal.prg: cpu_decimal.s common.s
	acme -o cpu_decimal.prg cpu_decimal.s
cpu_flags.prg: cpu_flags.s common.s
	acme -o cpu_flags.prg cpu_flags.s
cpu_timing.prg: cpu_timing.s common.s
	acme -DMODE=0 -o cpu_timing.prg cpu_timing.s
cpu_timing_ntsc.prg: cpu_timing.s common.s
	acme -DMODE=1 -o cpu_timing_ntsc.prg cpu_timing.s
cpu_insn.prg: cpu_insn.s common.s
	acme -o cpu_insn.prg cpu_insn.s
cpu_illegal.prg: cpu_illegal.s common.s
	acme -o cpu_illegal.prg cpu_illegal.s
cpu_clisei.prg: cpu_clisei.s common.s
	acme -o cpu_clisei.prg cpu_clisei.s
cpu_bugs.prg: cpu_bugs.s common.s
	acme -o cpu_bugs.prg cpu_bugs.s

clean:
	$(RM) cpu_decimal.prg cpu_flags.prg cpu_timing.prg cpu_insn.prg cpu_illegal.prg
	$(RM) cpu_timing_ntsc.prg
	$(RM) cpu_bugs.prg cpu_clisei.prg
