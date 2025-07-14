# this file is part of the C64 Emulator Test Suite. public domain, no copyright

#.SILENT:

COMMONSRC = \
	common/target.inc \
	common/common.asm \
	common/print.asm \
	common/printhb.asm \
	common/showregs.asm \
	common/startup.asm \
	common/target.inc \
	common/waitborder.asm \
	common/waitkey.asm

DISK1PROGS = \
	start.prg \
	ldab.prg \
	ldaz.prg \
	ldazx.prg \
	ldaa.prg \
	ldaax.prg \
	ldaay.prg \
	ldaix.prg \
	ldaiy.prg \
	staz.prg \
	stazx.prg \
	staa.prg \
	staax.prg \
	staay.prg \
	staix.prg \
	staiy.prg \
	ldxb.prg \
	ldxz.prg \
	ldxzy.prg \
	ldxa.prg \
	ldxay.prg \
	stxz.prg \
	stxzy.prg \
	stxa.prg \
	ldyb.prg \
	ldyz.prg \
	ldyzx.prg \
	ldya.prg \
	ldyax.prg \
	styz.prg \
	styzx.prg \
	stya.prg \
	taxn.prg \
	tayn.prg \
	txan.prg \
	tyan.prg \
	tsxn.prg \
	txsn.prg \
	phan.prg \
	plan.prg \
	phpn.prg \
	plpn.prg \
	inxn.prg \
	inyn.prg \
	dexn.prg \
	deyn.prg \
	incz.prg \
	inczx.prg \
	inca.prg \
	incax.prg \
	decz.prg \
	deczx.prg \
	deca.prg \
	decax.prg \
	asln.prg \
	aslz.prg \
	aslzx.prg \
	asla.prg \
	aslax.prg \
	lsrn.prg \
	lsrz.prg \
	lsrzx.prg \
	lsra.prg \
	lsrax.prg \
	roln.prg \
	rolz.prg \
	rolzx.prg \
	rola.prg \
	rolax.prg \
	rorn.prg \
	rorz.prg \
	rorzx.prg \
	rora.prg \
	rorax.prg \
	andb.prg \
	andz.prg \
	andzx.prg \
	anda.prg \
	andax.prg \
	anday.prg \
	andix.prg \
	andiy.prg \
	orab.prg \
	oraz.prg \
	orazx.prg \
	oraa.prg \
	oraax.prg \
	oraay.prg \
	oraix.prg \
	oraiy.prg \
	eorb.prg \
	eorz.prg \
	eorzx.prg \
	eora.prg \
	eorax.prg \
	eoray.prg \
	eorix.prg \
	eoriy.prg \
	clcn.prg \
	secn.prg \
	cldn.prg \
	sedn.prg \
	clin.prg \
	sein.prg \
	clvn.prg \
	adcb.prg \
	adcz.prg \
	adczx.prg \
	adca.prg \
	adcax.prg \
	adcay.prg \
	adcix.prg \
	adciy.prg \
	sbcb.prg \
	sbcz.prg \
	sbczx.prg \
	sbca.prg \
	sbcax.prg \
	sbcay.prg \
	sbcix.prg \
	sbciy.prg \
	cmpb.prg \
	cmpz.prg \
	cmpzx.prg \
	cmpa.prg \
	cmpax.prg \
	cmpay.prg \
	cmpix.prg \
	cmpiy.prg \
	cpxb.prg \
	cpxz.prg \
	cpxa.prg \
	cpyb.prg \
	cpyz.prg \
	cpya.prg \
	bitz.prg \
	bita.prg \
	brkn.prg \
	rtin.prg \
	jsrw.prg \
	rtsn.prg \
	jmpw.prg \
	jmpi.prg

DISK1NEXT = beqr.prg

DISK2PROGS = \
	beqr.prg \
	bner.prg \
	bmir.prg \
	bplr.prg \
	bcsr.prg \
	bccr.prg \
	bvsr.prg \
	bvcr.prg \
	nopn.prg \
	nopb.prg \
	nopz.prg \
	nopzx.prg \
	nopa.prg \
	nopax.prg \
	asoz.prg \
	asozx.prg \
	asoa.prg \
	asoax.prg \
	asoay.prg \
	asoix.prg \
	asoiy.prg \
	rlaz.prg \
	rlazx.prg \
	rlaa.prg \
	rlaax.prg \
	rlaay.prg \
	rlaix.prg \
	rlaiy.prg \
	lsez.prg \
	lsezx.prg \
	lsea.prg \
	lseax.prg \
	lseay.prg \
	lseix.prg \
	lseiy.prg \
	rraz.prg \
	rrazx.prg \
	rraa.prg \
	rraax.prg \
	rraay.prg \
	rraix.prg \
	rraiy.prg \
	dcmz.prg \
	dcmzx.prg \
	dcma.prg \
	dcmax.prg \
	dcmay.prg \
	dcmix.prg \
	dcmiy.prg \
	insz.prg \
	inszx.prg \
	insa.prg \
	insax.prg \
	insay.prg \
	insix.prg \
	insiy.prg \
	laxz.prg \
	laxzy.prg \
	laxa.prg \
	laxay.prg \
	laxix.prg \
	laxiy.prg \
	axsz.prg \
	axszy.prg \
	axsa.prg \
	axsix.prg \
	alrb.prg \
	arrb.prg \
	aneb.prg \
	lxab.prg \
	sbxb.prg \
	shaay.prg \
	shaiy.prg \
	shxay.prg \
	shyax.prg \
	shsay.prg \
	ancb.prg \
	lasay.prg \
	sbcb-eb.prg \
	trap1.prg \
	trap2.prg \
	trap3.prg \
	trap4.prg \
	trap5.prg \
	trap6.prg \
	trap7.prg \
	trap8.prg \
	trap9.prg \
	trap10.prg \
	trap11.prg \
	trap12.prg \
	trap13.prg \
	trap14.prg \
	trap15.prg \
	trap16.prg \
	trap17.prg \
	branchwrap.prg \
	mmufetch.prg \
	mmu.prg \
	cpuport.prg \
	cputiming.prg

DISK2PROGS128 = \
	beqr.prg \
	bner.prg \
	bmir.prg \
	bplr.prg \
	bcsr.prg \
	bccr.prg \
	bvsr.prg \
	bvcr.prg \
	nopn.prg \
	nopb.prg \
	nopz.prg \
	nopzx.prg \
	nopa.prg \
	nopax.prg \
	asoz.prg \
	asozx.prg \
	asoa.prg \
	asoax.prg \
	asoay.prg \
	asoix.prg \
	asoiy.prg \
	rlaz.prg \
	rlazx.prg \
	rlaa.prg \
	rlaax.prg \
	rlaay.prg \
	rlaix.prg \
	rlaiy.prg \
	lsez.prg \
	lsezx.prg \
	lsea.prg \
	lseax.prg \
	lseay.prg \
	lseix.prg \
	lseiy.prg \
	rraz.prg \
	rrazx.prg \
	rraa.prg \
	rraax.prg \
	rraay.prg \
	rraix.prg \
	rraiy.prg \
	dcmz.prg \
	dcmzx.prg \
	dcma.prg \
	dcmax.prg \
	dcmay.prg \
	dcmix.prg \
	dcmiy.prg \
	insz.prg \
	inszx.prg \
	insa.prg \
	insax.prg \
	insay.prg \
	insix.prg \
	insiy.prg \
	laxz.prg \
	laxzy.prg \
	laxa.prg \
	laxay.prg \
	laxix.prg \
	laxiy.prg \
	axsz.prg \
	axszy.prg \
	axsa.prg \
	axsix.prg \
	alrb.prg \
	arrb.prg \
	aneb.prg \
	lxab.prg \
	sbxb.prg \
	shaay.prg \
	shaiy.prg \
	shxay.prg \
	shyax.prg \
	shsay.prg \
	ancb.prg \
	lasay.prg \
	sbcb-eb.prg \
	trap1.prg \
	trap2.prg \
	trap3.prg \
	trap4.prg \
	trap5.prg \
	trap6.prg \
	trap7.prg \
	trap8.prg \
	trap9.prg \
	trap10.prg \
	trap11.prg \
	trap12.prg \
	trap13.prg \
	trap14.prg \
	trap15.prg \
	trap16.prg \
	trap17.prg \
	branchwrap.prg \
	mmufetch.prg \
	mmu.prg \
	cpuport128.prg \
	cputiming.prg
	
DISK2NEXT = irq.prg

DISK3PROGSOLD = \
	  irq.prg \
	  nmi.prg \
	cia1tb123.prg \
	cia2tb123.prg \
	cia1pb6.prg \
	cia1pb7.prg \
	cia2pb6.prg \
	\
	cia2pb7.prg \
	  cia1tab.prg \
	loadth.prg \
	cnto2.prg \
	  icr01.prg \
	  imr.prg \
	flipos.prg \
	oneshot.prg \
	cntdef.prg \
	  cia1ta.prg \
	  cia1tb.prg \
	  cia2ta.prg \
	  cia2tb.prg \
	\
	finish.prg \
	\
	template.prg

DISK3PROGSNEW = \
	  irqnew.prg \
	  nminew.prg \
	cia1tb123.prg \
	cia2tb123.prg \
	cia1pb6.prg \
	cia1pb7.prg \
	cia2pb6.prg \
	cia2pb7.prg \
	  cia1tabnew.prg \
	loadth.prg \
	cnto2.prg \
	  icr01new.prg \
	  imrnew.prg \
	flipos.prg \
	oneshot.prg \
	cntdef.prg \
	  cia1tanew.prg \
	  cia1tbnew.prg \
	  cia2tanew.prg \
	  cia2tbnew.prg \
	\
	finish.prg \
	\
	template.prg

ALLPROGSOLD = $(DISK1PROGS) $(DISK2PROGS) $(DISK3PROGSOLD)
ALLPROGSNEW = $(DISK1PROGS) $(DISK2PROGS) $(DISK3PROGSNEW)
	
ALLPROGS = $(DISK1PROGS) $(DISK2PROGS) $(DISK3PROGSOLD) $(DISK3PROGSNEW)
ALLDISKS = Disk1.d64 Disk2.d64 Disk3.d64 Disk3new.d64 Lorenz.d81 Lorenznew.d81

ALLPROGSOLD128 = $(DISK1PROGS) $(DISK2PROGS128) $(DISK3PROGSOLD)
ALLPROGSNEW128 = $(DISK1PROGS) $(DISK2PROGS128) $(DISK3PROGSNEW)
	
ALLPROGS128 = $(DISK1PROGS) $(DISK2PROGS128) $(DISK3PROGSOLD) $(DISK3PROGSNEW)
ALLDISKS128 = Disk1.d64 Disk2c128.d64 Disk3.d64 Disk3new.d64

ASFLAGS=-C -T -a -q -I ./common -D TARGET=0

all: $(ALLPROGS) $(ALLDISKS) $(ALLPROGS128) $(ALLDISKS128)

cpuport.prg: cpuport.s $(COMMONSRC)
	64tass $(ASFLAGS) -D ISC128C64=0 -i cpuport.s -o cpuport.prg
cpuport128.prg: cpuport.s $(COMMONSRC)
	64tass $(ASFLAGS) -D ISC128C64=1 -i cpuport.s -o cpuport128.prg

trap1.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=1 -i trap1-15.s -o trap1.prg
trap2.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=2 -i trap1-15.s -o trap2.prg
trap3.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=3 -i trap1-15.s -o trap3.prg
trap4.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=4 -i trap1-15.s -o trap4.prg
trap5.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=5 -i trap1-15.s -o trap5.prg
trap6.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=6 -i trap1-15.s -o trap6.prg
trap7.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=7 -i trap1-15.s -o trap7.prg
trap8.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=8 -i trap1-15.s -o trap8.prg
trap9.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=9 -i trap1-15.s -o trap9.prg
trap10.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=10 -i trap1-15.s -o trap10.prg
trap11.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=11 -i trap1-15.s -o trap11.prg
trap12.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=12 -i trap1-15.s -o trap12.prg
trap13.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=13 -i trap1-15.s -o trap13.prg
trap14.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=14 -i trap1-15.s -o trap14.prg
trap15.prg: trap1-15.s $(COMMONSRC)
	64tass $(ASFLAGS) -D TRAP=15 -i trap1-15.s -o trap15.prg

irq.prg: irq.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=0 -i irq.s -o irq.prg
irqnew.prg: irq.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=1 -i irq.s -o irqnew.prg
nmi.prg: nmi.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=0 -i nmi.s -o nmi.prg
nminew.prg: nmi.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=1 -i nmi.s -o nminew.prg
icr01.prg: icr01.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=0 -i icr01.s -o icr01.prg
icr01new.prg: icr01.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=1 -i icr01.s -o icr01new.prg
imr.prg: imr.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=0 -i imr.s -o imr.prg
imrnew.prg: imr.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=1 -i imr.s -o imrnew.prg
cia1tab.prg: cia1tab.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=0 -i cia1tab.s -o cia1tab.prg
cia1tabnew.prg: cia1tab.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=1 -i cia1tab.s -o cia1tabnew.prg
cia1ta.prg: cia1ta.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=0 -i cia1ta.s -o cia1ta.prg
cia1tanew.prg: cia1ta.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=1 -i cia1ta.s -o cia1tanew.prg
cia1tb.prg: cia1tb.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=0 -i cia1tb.s -o cia1tb.prg
cia1tbnew.prg: cia1tb.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=1 -i cia1tb.s -o cia1tbnew.prg
cia2ta.prg: cia2ta.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=0 -i cia2ta.s -o cia2ta.prg
cia2tanew.prg: cia2ta.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=1 -i cia2ta.s -o cia2tanew.prg
cia2tb.prg: cia2tb.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=0 -i cia2tb.s -o cia2tb.prg
cia2tbnew.prg: cia2tb.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEWCIA=1 -i cia2tb.s -o cia2tbnew.prg
	
nextdisk1.prg: nextdisk.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEXT=1 -i nextdisk.s -o nextdisk1.prg
nextdisk2.prg: nextdisk.s $(COMMONSRC)
	64tass $(ASFLAGS) -D NEXT=2 -i nextdisk.s -o nextdisk2.prg

%.prg : %.s $(COMMONSRC)
	64tass $(ASFLAGS) -i "$*.s" -o "$*.prg"

Disk1.d64: $(DISK1PROGS) nextdisk1.prg
	c1541 -format "test suite 2.15+,#1" d64 Disk1.d64 > /dev/null
	for I in $(basename $(DISK1PROGS)); do \
		c1541 Disk1.d64 -write "$$I.prg" "$$I" > /dev/null; \
	done
	c1541 Disk1.d64 -write "nextdisk1.prg" "$(basename $(DISK1NEXT))" > /dev/null
#	c1541 Disk1.d64 -list
Disk2.d64: $(DISK2PROGS) nextdisk2.prg
	c1541 -format "test suite 2.15+,#2" d64 Disk2.d64 > /dev/null
	for I in $(basename $(DISK2PROGS)); do \
		c1541 Disk2.d64 -write "$$I.prg" "$$I" > /dev/null; \
	done
	c1541 Disk2.d64 -write "nextdisk2.prg" "$(basename $(DISK2NEXT))" > /dev/null
#	c1541 Disk2.d64 -list
Disk3.d64: $(DISK3PROGSOLD)
	c1541 -format "test suite 2.15+,#3" d64 Disk3.d64 > /dev/null
	for I in $(basename $(DISK3PROGSOLD)); do \
		c1541 Disk3.d64 -write "$$I.prg" "$$I" > /dev/null; \
	done
#	c1541 Disk3.d64 -list
Disk3new.d64: $(DISK3PROGSNEW)
	c1541 -format "test suite 2.15+,#3" d64 Disk3new.d64 > /dev/null
	for I in $(basename $(DISK3PROGSNEW)); do \
		c1541 Disk3new.d64 -write "$$I.prg" `echo "$$I" | sed -e 's:new$$::'`; \
	done
#	c1541 Disk3new.d64 -list
Lorenz.d81: $(ALLPROGSOLD)
	c1541 -format "test suite 2.15+,#1" d81 Lorenz.d81 > /dev/null
	for I in $(basename $(ALLPROGSOLD)); do \
		c1541 Lorenz.d81 -write "$$I.prg" "$$I" > /dev/null; \
	done
#	c1541 Lorenz.d81 -list
Lorenznew.d81: $(ALLPROGSNEW)
	c1541 -format "test suite 2.15+,#1" d81 Lorenznew.d81 > /dev/null
	for I in $(basename $(ALLPROGSNEW)); do \
		c1541 Lorenznew.d81 -write "$$I.prg" `echo "$$I" | sed -e 's:new$$::'`; \
	done
#	c1541 Lorenznew.d81 -list

Disk2c128.d64: $(DISK2PROGS128) nextdisk2.prg
	c1541 -format "test suite 2.15+,#2" d64 Disk2c128.d64 > /dev/null
	for I in $(basename $(DISK2PROGS128)); do \
		c1541 Disk2c128.d64 -write "$$I.prg" `echo "$$I" | sed -e 's:128$$::'`; \
	done
	c1541 Disk2c128.d64 -write "nextdisk2.prg" "$(basename $(DISK2NEXT))" > /dev/null
#	c1541 Disk2c128.d64 -list

clean:
	rm -f *~

zap: clean
	rm -f *.prg
	rm -f *.d64
	rm -f *.d81
