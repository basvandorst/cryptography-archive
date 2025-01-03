IOCTYPE = 0xff00
IOCINFO = 0xff01
IOCCONFIG = 0xff02
TTNAMEMAX = 32
IOCPARM_MASK = 0x7f
IOC_VOID = 0x20000000
IOC_OUT = 0x40000000
IOC_IN = 0x80000000
IOC_INOUT = (IOC_IN|IOC_OUT)
TIOCM_LE = 0001
TIOCM_DTR = 0002
TIOCM_RTS = 0004
TIOCM_ST = 0010
TIOCM_SR = 0020
TIOCM_CTS = 0040
TIOCM_CAR = 0100
TIOCM_CD = TIOCM_CAR
TIOCM_RNG = 0200
TIOCM_RI = TIOCM_RNG
TIOCM_DSR = 0400
TANDEM = 0x00000001
CBREAK = 0x00000002
LCASE = 0x00000004
ECHO = 0x00000008
CRMOD = 0x00000010
RAW = 0x00000020
ODDP = 0x00000040
EVENP = 0x00000080
ANYP = 0x000000c0
CRDELAY = 0x00000300
CR0 = 0x00000000
CR1 = 0x00000100
CR2 = 0x00000200
CR3 = 0x00000300
TBDELAY = 0x00000c00
TAB0 = 0x00000000
TAB1 = 0x00000400
TAB2 = 0x00000800
XTABS = 0x00000c00
BSDELAY = 0x00001000
BS0 = 0x00000000
BS1 = 0x00001000
VTDELAY = 0x00002000
FF0 = 0x00000000
FF1 = 0x00002000
NLDELAY = 0x0000c000
NL0 = 0x00000000
NL1 = 0x00004000
NL2 = 0x00008000
NL3 = 0x0000c000
ALLDELAY = (NLDELAY|TBDELAY|CRDELAY|VTDELAY|BSDELAY)
TOSTOP = 0x00010000
PRTERA = 0x00020000
CRTERA = 0x00040000
TILDE = 0x00080000
FLUSHO = 0x00100000
LITOUT = 0x00200000
CRTBS = 0x00400000
MDMBUF = 0x00800000
NOHANG = 0x01000000
L001000 = 0x02000000
CRTKIL = 0x04000000
PASS8 = 0x08000000
CTLECH = 0x10000000
PENDIN = 0x20000000
DECCTQ = 0x40000000
NOFLUSH = 0x80000000
NOFLSH = NOFLUSH
LCRTBS = (CRTBS>>16)
LPRTERA = (PRTERA>>16)
LCRTERA = (CRTERA>>16)
LTILDE = (TILDE>>16)
LMDMBUF = (MDMBUF>>16)
LLITOUT = (LITOUT>>16)
LTOSTOP = (TOSTOP>>16)
LFLUSHO = (FLUSHO>>16)
LNOHANG = (NOHANG>>16)
LCRTKIL = (CRTKIL>>16)
LPASS8 = (PASS8>>16)
LCTLECH = (CTLECH>>16)
LPENDIN = (PENDIN>>16)
LDECCTQ = (DECCTQ>>16)
LNOFLSH = (NOFLUSH>>16)
TIOCPKT_DATA = 0x00
TIOCPKT_FLUSHREAD = 0x01
TIOCPKT_FLUSHWRITE = 0x02
TIOCPKT_STOP = 0x04
TIOCPKT_START = 0x08
TIOCPKT_NOSTOP = 0x10
TIOCPKT_DOSTOP = 0x20
OTTYDISC = 0
NETLDISC = 1
NTTYDISC = 2
TABLDISC = 3
SLIPDISC = 4
