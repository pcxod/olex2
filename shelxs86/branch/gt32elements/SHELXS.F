C
C   ** SHELXS-86 **   CRYSTAL STRUCTURE SOLUTION  -  UNIX VERSION
C
C ** NEXT STATEMENT MUST HAVE C IN COLUMN 1 IF NOT FORTRAN-77 **
      CHARACTER*1 IH,IT,NAME*40,tim*8
C
      COMMON F(126),G(126),SL,TL,LM,LR,LG,LH,LI,LP,LF,LA,LB,
     +LW,LY,LL,LQ,LE,LV,LX,LD,LZ,LJ,HA,HD,A(50000)
      COMMON/VAX/IVX,NAME,IPF
      COMMON/WORD/IH(50),IT(120)
      IPF=0
C
C START TIMING
C
      CALL SXTI(SL)
      stim=sl
C
C ** LM = DIMENSION OF A (NOT LESS THAN 30000, BETTER CA. 50000) **
C
      LM=50000
C
C ** SET UNITS FOR INPUT (LR), HKL DATA (LH), PRINTER (LI)   **
C ** AND 'CARDPUNCH' (LP) (USUALLY 5, 3, 6 AND THEN 1 OR 7). **
C ** EXPLICIT OPEN STATEMENTS MAY BE NEEDED HERE, BUT IT MAY **
C ** BE BETTER TO OPEN UNIT LH NEAR THE END OF SX2A, SO THAT **
C ** IT IS ONLY OPENED IF AND WHEN REQUIRED.                 **
C
      LR=5
C
      call getarg(iargc(),name)
      n=0
      do 500 i=1,40
      if(name(i:i).eq.' ')goto 500
      if(name(i:i).ge.'A'.and.name(i:i).le.'Z')name(i:i)=
     +char(ichar(name(i:i))+32)
      n=n+1
      name(n:n)=name(i:i)
 500  continue
      name(n+1:)=' '
C
      OPEN(UNIT=5,FILE=NAME(:N)//'.ins',STATUS='OLD',ERR=5)
      LH=3
      OPEN(UNIT=3,FILE=NAME(:N)//'.hkl',STATUS='OLD',ERR=5)
      LI=6
C      OPEN(UNIT=6,FILE=NAME(:N)//'.lst',STATUS='UNKNOWN',ERR=5)
      LP=7
      OPEN(UNIT=7,FILE=NAME(:N)//'.res',STATUS='UNKNOWN',ERR=5)
C
C ** SET BINARY (UNFORMATTED) SCRATCH UNITS (LA, LB, LF AND LG) **
C ** (USUALLY 2, 4, 8 AND 9 RESP.).  ALL UNIT NUMBERS MUST BE   **
C ** DIFFERENT, AND ALL FILES USED BY SHELX-86 ARE SEQUENTIAL.  **
C ** EXPLICIT OPEN STATEMENTS MAY NEED TO BE INSERTED HERE.     **
C
      LA=2
      OPEN(UNIT=2,STATUS='SCRATCH',FORM='UNFORMATTED',ERR=5)
      LB=4
      OPEN(UNIT=4,STATUS='SCRATCH',FORM='UNFORMATTED',ERR=5)
      LF=8
      OPEN(UNIT=8,STATUS='SCRATCH',FORM='UNFORMATTED',ERR=5)
      LG=9
      OPEN(UNIT=9,STATUS='SCRATCH',FORM='UNFORMATTED',ERR=5)
C
C ** SET CHARS/INCH ACROSS (HA) AND LINES/INCH DOWN PAGE (HD) **
C ** (USUALLY HA=10. AND HD=6. OR 8.)                         **
C
      HA=10.
      HD=6.
C
C ** SET DEFAULT MAXTIME FOR TIDYING UP AND FINISHING OFF **
C
      TL=999999.
C
C CALL SUBROUTINES (OVERLAYS)
C
      CALL SX2A
      IF(LD.NE.0)GOTO 3
      CALL SX2B
   1  CALL SX2C
      CALL SX2D
      CALL SX2E
   2  CALL SX2F
      CALL SX2G
      IF(A(54).GT.6.5)GOTO 2
      IF(ABS(A(54)-2.).GT.1.5)GOTO 3
      CALL SX2I
      CALL SX2L
      GOTO 1
   3  CALL SX2H
      call time(tim)
      call sxti(t)
      write(li,6)tim,t-stim
      CALL SXIT
   4  FORMAT(/' ** CANNOT OPEN FILE')
   5  WRITE(*,4)
      CALL SXIT
   6  format(/' ',61('+')/' +  Finished at ',a8,4x,
     +'Total CPU time:',f11.2,' secs  +'/' ',61('+')/)
      STOP
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SXIT
C
C TERMINATE JOB IN MANNER APPROPRIATE TO OPERATING SYSTEM
C E.G. OUTPUT A NEW PAGE USING FORMAT('1') THEN STOP
C
      STOP
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SXTI(T)
C
C SETS T TO THE CPU TIME USED SO FAR IN SECONDS (NOT NECESSARILY
C ZERO AT THE START OF SHELXS).  IF THE OPERATING SYSTEM DOES
C NOT PROVIDE THIS INFORMATION, THE SUBROUTINE SHOULD SET T TO -1.
C
      t=0.01*real(mclock())
C
      RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SXTM(SL,LI)
C
C PRINT CPU TIME USED SINCE LAST CALLED
C
      T=SL
      CALL SXTI(SL)
      IF(SL.LT.0.)GOTO 1
      T=SL-T
      WRITE(LI,'(//F10.2,A/)')T,' seconds CPU time'
   1  RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SXPW(X,IC)
C
C PACK 4 CHARACTERS INTO ONE REAL
C
C ** NEXT STATEMENT MUST HAVE C IN COLUMN 1 IF NOT FORTRAN-77 **
      CHARACTER*1 IH,IT,IC
C
      COMMON/WORD/IH(50),IT(120)
      DIMENSION IC(4)
      X=0.
        DO 2 I=1,4
        J=1
   1    IF(IC(I).EQ.IH(J))GOTO 2
        J=J+1
        IF(J.LT.50)GOTO 1
   2    X=X*50.+FLOAT(J-1)
      RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SXUW(X,IC)
C
C UNPACK 4 CHARACTERS OUT OF ONE REAL
C
C ** NEXT STATEMENT MUST HAVE C IN COLUMN 1 IF NOT FORTRAN-77 **
      CHARACTER*1 IH,IT,IC
C
      COMMON/WORD/IH(50),IT(120)
      DIMENSION IC(4)
      Y=X+.5
      J=INT(8.E-6*Y)+1
      IC(1)=IH(J)
      J=INT(AMOD(Y*4.E-4,50.))+1
      IC(2)=IH(J)
      J=INT(AMOD(Y*2.E-2,50.))+1
      IC(3)=IH(J)
      J=INT(AMOD(Y,50.))+1
      IC(4)=IH(J)
      RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SXH2(W,X,Y,Z)
C
C UNPACK REFLECTION INDICES STORED IN ONE POSITIVE REAL
C
      Z=AINT(2.5E-5*W+.5)
      X=W-40000.*Z
      X=AINT(X+SIGN(.5,X+.5))
      Y=AINT(5.E-3*X+SIGN(.5,X+.5))
      X=X-200.*Y
      X=AINT(X+SIGN(.5,X+.5))
      RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SX2A
C
C INTERPRET INSTRUCTIONS
C
C ** NEXT STATEMENT MUST HAVE C IN COLUMN 1 IF NOT FORTRAN-77 **
      CHARACTER*1 IH,IT,IR,IA,IB,IG,ID,NAME*40,dat*9,tim*8
C
      COMMON F(126),G(126),SL,TL,LM,LR,LG,LH,LI,LP,LF,LA,LB,
     +LW,LY,LL,LQ,LE,LV,LX,LD,LZ,LJ,HA,HD,A(50000)
      COMMON/WORD/IH(50),IT(40),IR(80)
      COMMON/VAX/IVX,NAME,IPF
      DIMENSION IA(94),IB(94),C(34),AN(94),AR(94),AM(94),
     +AO(94),AU(94),IG(50),ID(4),AG(94)
C
C ** FOLLOWING DATA STATEMENTS TO INITIALISE CHARACTER VARIABLES **
C ** MAY REQUIRE MODIFICATION (E.G. 'X' TO 1HX AND '''' TO 1H')  **
C ** FOR COMPILERS WHICH DO NOT CONFORM TO FORTRAN-77 STANDARD.  **
C ** THE CHARACTER " IS NOT IN THE FORTRAN-77 STANDARD SET       **
C
      DATA IG/'0','1','2','3','4','5','6','7','8','9','.','-',
     +'+','X','Y','Z',',','=','/',' ','*','''','"','(',')',':',
     +'A','B','C','D','E','F','G','H','I','J','K','L','M','N',
     +'O','P','Q','R','S','T','U','V','W','$'/
C      DATA IA/'H','B','C','N','O','F','N','A','S','P','S',
C     +'C','K','C','M','F','C','N','C','A','S','B','M','R',
C     +'A','S','S','I','O','P','A','H'/
C      DATA IB/' ',' ',' ',' ',' ',' ','A','L','I',' ',' ',
C     +'L',' ','R','N','E','O','I','U','S','E','R','O','U',
C     +'G','N','B',' ','S','T','U','G'/
C
C ENCODED COMMANDS
C
      DATA C/1952193.,5712287.,3576887.,4692295.,5536938.,5578828.,
     +5849245.,5218839.,5096745.,3861537.,3971341.,4109229.,
     +4851880.,5604239.,3983832.,5734031.,5700691.,4216881.,
     +5208844.,4852380.,5192295.,3848969.,5711930.,5616394.,
     +5236530.,4712245.,4829245.,3329213.,6081695.,4827182.,
     +3993843.,2423469.,4976480.,5452245./
C
C ATOMIC NUMBER, RADIUS, WEIGHT, CU AND MO ABSORPTION COEFFICIENTS
C
C      DATA AR/.37,.80,.77,.75,.73,.71,1.3,1.15,1.05,1.03,
C     +1.02,1.00,1.96,1.25,1.30,1.24,1.25,1.25,1.28,1.21,1.17,
C     +1.14,1.36,1.33,1.40,1.40,1.43,1.33,1.38,1.35,1.44,1.50/
C
C      DATA AN/1.,5.,6.,7.,8.,9.,11.,13.,14.,15.,16.,17.,
C     +19.,24.,25.,26.,27.,28.,29.,33.,34.,35.,42.,44.,
C     +47.,50.,51.,53.,76.,78.,79.,80./
C
C      DATA AM/1.0079,10.81,12.011,14.0067,15.9994,18.9984,22.9898,
C     +26.9815,28.0855,30.9738,32.064,35.453,39.0983,51.996,54.938,
C     +55.847,58.9332,58.70,63.546,74.9216,78.96,79.904,95.94,101.07,
C     +107.868,118.69,121.75,126.9045,190.2,195.09,196.9665,200.59/
C
C      DATA AU/.6548,38.45,84.13,166.1,293.1,503.2,1157.,2249.,
C     +3047.,3975.,4927.,6429.,9637.,21780.,24860.,28230.,33130.,
C     +4759.,5437.,9411.,10870.,11980.,25210.,30350.,39070.,
C     +49930.,53890.,61470.,58030.,64210.,68030.,72010./
C
C      DATA AO/.6238,6.194,10.67,18.37,30.48,49.99,112.2,225.8,304.7,
C     +404.8,512.5,685.3,1052.,2526.,2906.,3500.,4014.,4604.,5206.,
C     +8207.,9023.,9910.,2937.,3581.,4725.,6145.,6675.,7654.,
C     +31660.,35180.,36450.,38210./
C
C Atomic radius, weight, Cu, Mo and Ag absorption coefficients
C
      DATA IA(1)/'H'/,IB(1)/' '/,AR(1)/.32/,AM(1)/1.008/,
     +AU(1)/.655/,AO(1)/.624/,AG(1)/.614/
C
      DATA IA(2)/'H'/,IB(2)/'E'/,AR(2)/1.50/,AM(2)/4.00/,
     +AU(2)/1.94/,AO(2)/1.34/,AG(2)/1.28/
C
      DATA IA(3)/'L'/,IB(3)/'I'/,AR(3)/1.52/,AM(3)/6.94/,
     +AU(3)/5.76/,AO(3)/2.28/,AG(3)/2.06/
C
      DATA IA(4)/'B'/,IB(4)/'E'/,AR(4)/1.11/,AM(4)/9.01/,
     +AU(4)/16.6/,AO(4)/3.83/,AG(4)/3.13/
C
      DATA IA(5)/'B'/,IB(5)/' '/,AR(5)/0.82/,AM(5)/10.81/,
     +AU(5)/41.5/,AO(5)/6.61/,AG(5)/4.79/
C
      DATA IA(6)/'C'/,IB(6)/' '/,AR(6)/0.77/,AM(6)/12.01/,
     +AU(6)/89.9/,AO(6)/11.5/,AG(6)/7.45/
C
      DATA IA(7)/'N'/,IB(7)/' '/,AR(7)/0.70/,AM(7)/14.01/,
     +AU(7)/173./,AO(7)/19.6/,AG(7)/11.7/
C
      DATA IA(8)/'O'/,IB(8)/' '/,AR(8)/0.66/,AM(8)/16.00/,
     +AU(8)/304./,AO(8)/32.5/,AG(8)/18.2/
C
      DATA IA(9)/'F'/,IB(9)/' '/,AR(9)/0.64/,AM(9)/19.00/,
     +AU(9)/498./,AO(9)/51.5/,AG(9)/27.7/
C
      DATA IA(10)/'N'/,IB(10)/'E'/,AR(10)/1.50/,AM(10)/20.18/,
     +AU(10)/768./,AO(10)/78.6/,AG(10)/41.2/
C
      DATA IA(11)/'N'/,IB(11)/'A'/,AR(11)/1.86/,AM(11)/22.99/,
     +AU(11)/1140./,AO(11)/116./,AG(11)/59.6/
C
      DATA IA(12)/'M'/,IB(12)/'G'/,AR(12)/1.60/,AM(12)/24.31/,
     +AU(12)/1610./,AO(12)/165./,AG(12)/84.2/
C
      DATA IA(13)/'A'/,IB(13)/'L'/,AR(13)/1.25/,AM(13)/26.98/,
     +AU(13)/2220./,AO(13)/229./,AG(13)/116./
C
      DATA IA(14)/'S'/,IB(14)/'I'/,AR(14)/1.17/,AM(14)/28.09/,
     +AU(14)/2970./,AO(14)/310./,AG(14)/156./
C
      DATA IA(15)/'P'/,IB(15)/' '/,AR(15)/1.10/,AM(15)/30.97/,
     +AU(15)/3880./,AO(15)/410./,AG(15)/206./
C
      DATA IA(16)/'S'/,IB(16)/' '/,AR(16)/1.03/,AM(16)/32.06/,
     +AU(16)/4970./,AO(16)/532./,AG(16)/267./
C
      DATA IA(17)/'C'/,IB(17)/'L'/,AR(17)/0.99/,AM(17)/35.45/,
     +AU(17)/6240./,AO(17)/678./,AG(17)/341./
C
      DATA IA(18)/'A'/,IB(18)/'R'/,AR(18)/1.50/,AM(18)/39.95/,
     +AU(18)/7720./,AO(18)/851./,AG(18)/429./
C
      DATA IA(19)/'K'/,IB(19)/' '/,AR(19)/2.27/,AM(19)/39.10/,
     +AU(19)/9400./,AO(19)/1050./,AG(19)/532./
C
      DATA IA(20)/'C'/,IB(20)/'A'/,AR(20)/1.97/,AM(20)/40.08/,
     +AU(20)/11300./,AO(20)/1290./,AG(20)/652./
C
      DATA IA(21)/'S'/,IB(21)/'C'/,AR(21)/1.61/,AM(21)/44.96/,
     +AU(21)/13500./,AO(21)/1560./,AG(21)/789./
C
      DATA IA(22)/'T'/,IB(22)/'I'/,AR(22)/1.45/,AM(22)/47.90/,
     +AU(22)/15900./,AO(22)/1860./,AG(22)/947./
C
      DATA IA(23)/'V'/,IB(23)/' '/,AR(23)/1.31/,AM(23)/50.94/,
     +AU(23)/18500./,AO(23)/2200./,AG(23)/1120./
C
      DATA IA(24)/'C'/,IB(24)/'R'/,AR(24)/1.24/,AM(24)/52.00/,
     +AU(24)/21300./,AO(24)/2580./,AG(24)/1330./
C
      DATA IA(25)/'M'/,IB(25)/'N'/,AR(25)/1.37/,AM(25)/54.94/,
     +AU(25)/24600./,AO(25)/3020./,AG(25)/1550./
C
      DATA IA(26)/'F'/,IB(26)/'E'/,AR(26)/1.24/,AM(26)/55.85/,
     +AU(26)/28000./,AO(26)/3490./,AG(26)/1800./
C
      DATA IA(27)/'C'/,IB(27)/'O'/,AR(27)/1.25/,AM(27)/58.93/,
     +AU(27)/31400./,AO(27)/4010./,AG(27)/2070./
C
      DATA IA(28)/'N'/,IB(28)/'I'/,AR(28)/1.25/,AM(28)/58.71/,
     +AU(28)/4760./,AO(28)/4570./,AG(28)/2380./
C
      DATA IA(29)/'C'/,IB(29)/'U'/,AR(29)/1.28/,AM(29)/63.54/,
     +AU(29)/5470./,AO(29)/5180./,AG(29)/2710./
C
      DATA IA(30)/'Z'/,IB(30)/'N'/,AR(30)/1.33/,AM(30)/65.37/,
     +AU(30)/6290./,AO(30)/5860./,AG(30)/3070./
C
      DATA IA(31)/'G'/,IB(31)/'A'/,AR(31)/1.26/,AM(31)/69.72/,
     +AU(31)/7190./,AO(31)/6600./,AG(31)/3460./
C
      DATA IA(32)/'G'/,IB(32)/'E'/,AR(32)/1.22/,AM(32)/72.59/,
     +AU(32)/8190./,AO(32)/7380./,AG(32)/3870./
C
      DATA IA(33)/'A'/,IB(33)/'S'/,AR(33)/1.21/,AM(33)/74.92/,
     +AU(33)/9290./,AO(33)/8220./,AG(33)/4330./
C
      DATA IA(34)/'S'/,IB(34)/'E'/,AR(34)/1.17/,AM(34)/78.96/,
     +AU(34)/10500./,AO(34)/9110./,AG(34)/4820./
C
      DATA IA(35)/'B'/,IB(35)/'R'/,AR(35)/1.14/,AM(35)/79.91/,
     +AU(35)/11800./,AO(35)/10000./,AG(35)/5350./
C
      DATA IA(36)/'K'/,IB(36)/'R'/,AR(36)/1.50/,AM(36)/83.80/,
     +AU(36)/13200./,AO(36)/11000./,AG(36)/5920./
C
      DATA IA(37)/'R'/,IB(37)/'B'/,AR(37)/2.48/,AM(37)/85.47/,
     +AU(37)/14800./,AO(37)/12100./,AG(37)/6520./
C
      DATA IA(38)/'S'/,IB(38)/'R'/,AR(38)/2.15/,AM(38)/87.62/,
     +AU(38)/16500./,AO(38)/13200./,AG(38)/7150./
C
      DATA IA(39)/'Y'/,IB(39)/' '/,AR(39)/1.78/,AM(39)/88.91/,
     +AU(39)/18300./,AO(39)/14300./,AG(39)/7800./
C
      DATA IA(40)/'Z'/,IB(40)/'R'/,AR(40)/1.59/,AM(40)/91.22/,
     +AU(40)/20300./,AO(40)/2470./,AG(40)/8470./
C
      DATA IA(41)/'N'/,IB(41)/'B'/,AR(41)/1.43/,AM(41)/92.91/,
     +AU(41)/22300./,AO(41)/2730./,AG(41)/9220./
C
      DATA IA(42)/'M'/,IB(42)/'O'/,AR(42)/1.36/,AM(42)/95.94/,
     +AU(42)/24600./,AO(42)/3000./,AG(42)/11500./
C
      DATA IA(43)/'T'/,IB(43)/'C'/,AR(43)/1.35/,AM(43)/98.00/,
     +AU(43)/27000./,AO(43)/3320./,AG(43)/10700./
C
      DATA IA(44)/'R'/,IB(44)/'U'/,AR(44)/1.33/,AM(44)/101.07/,
     +AU(44)/29500./,AO(44)/3640./,AG(44)/1920./
C
      DATA IA(45)/'R'/,IB(45)/'H'/,AR(45)/1.35/,AM(45)/102.91/,
     +AU(45)/32300./,AO(45)/3990./,AG(45)/2100./
C
      DATA IA(46)/'P'/,IB(46)/'D'/,AR(46)/1.38/,AM(46)/106.40/,
     +AU(46)/35200./,AO(46)/4360./,AG(46)/2300./
C
      DATA IA(47)/'A'/,IB(47)/'G'/,AR(47)/1.44/,AM(47)/107.87/,
     +AU(47)/38200./,AO(47)/4760./,AG(47)/2510./
C
      DATA IA(48)/'C'/,IB(48)/'D'/,AR(48)/1.49/,AM(48)/112.40/,
     +AU(48)/41500./,AO(48)/5180./,AG(48)/2730./
C
      DATA IA(49)/'I'/,IB(49)/'N'/,AR(49)/1.44/,AM(49)/114.82/,
     +AU(49)/45000./,AO(49)/5630./,AG(49)/2970./
C
      DATA IA(50)/'S'/,IB(50)/'N'/,AR(50)/1.40/,AM(50)/118.69/,
     +AU(50)/48600./,AO(50)/6110./,AG(50)/3230./
C
      DATA IA(51)/'S'/,IB(51)/'B'/,AR(51)/1.41/,AM(51)/121.75/,
     +AU(51)/52500./,AO(51)/6620./,AG(51)/3500./
C
      DATA IA(52)/'T'/,IB(52)/'E'/,AR(52)/1.37/,AM(52)/127.60/,
     +AU(52)/56500./,AO(52)/7160./,AG(52)/3780./
C
      DATA IA(53)/'I'/,IB(53)/' '/,AR(53)/1.33/,AM(53)/126.90/,
     +AU(53)/60700./,AO(53)/7730./,AG(53)/4090./
C
      DATA IA(54)/'X'/,IB(54)/'E'/,AR(54)/1.50/,AM(54)/131.30/,
     +AU(54)/65200./,AO(54)/8340./,AG(54)/4410./
C
      DATA IA(55)/'C'/,IB(55)/'S'/,AR(55)/2.65/,AM(55)/132.91/,
     +AU(55)/70000./,AO(55)/8980./,AG(55)/4750./
C
      DATA IA(56)/'B'/,IB(56)/'A'/,AR(56)/2.17/,AM(56)/137.34/,
     +AU(56)/75000./,AO(56)/9650./,AG(56)/5110./
C
      DATA IA(57)/'L'/,IB(57)/'A'/,AR(57)/1.87/,AM(57)/138.91/,
     +AU(57)/80300./,AO(57)/10400./,AG(57)/5490./
C
      DATA IA(58)/'C'/,IB(58)/'E'/,AR(58)/1.83/,AM(58)/140.12/,
     +AU(58)/85700./,AO(58)/11100./,AG(58)/5880./
C
      DATA IA(59)/'P'/,IB(59)/'R'/,AR(59)/1.82/,AM(59)/140.91/,
     +AU(59)/91200./,AO(59)/11900./,AG(59)/6300./
C
      DATA IA(60)/'N'/,IB(60)/'D'/,AR(60)/1.81/,AM(60)/144.24/,
     +AU(60)/96800./,AO(60)/12700./,AG(60)/6740./
C
      DATA IA(61)/'P'/,IB(61)/'M'/,AR(61)/1.81/,AM(61)/147.00/,
     +AU(61)/102000./,AO(61)/13500./,AG(61)/7200./
C
      DATA IA(62)/'S'/,IB(62)/'M'/,AR(62)/1.80/,AM(62)/150.35/,
     +AU(62)/108000./,AO(62)/14400./,AG(62)/7680./
C
      DATA IA(63)/'E'/,IB(63)/'U'/,AR(63)/2.00/,AM(63)/151.96/,
     +AU(63)/110000./,AO(63)/15400./,AG(63)/8190./
C
      DATA IA(64)/'G'/,IB(64)/'D'/,AR(64)/1.79/,AM(64)/157.25/,
     +AU(64)/105000./,AO(64)/16300./,AG(64)/8720./
C
      DATA IA(65)/'T'/,IB(65)/'B'/,AR(65)/1.76/,AM(65)/158.92/,
     +AU(65)/84700./,AO(65)/17400./,AG(65)/9270./
C
      DATA IA(66)/'D'/,IB(66)/'Y'/,AR(66)/1.75/,AM(66)/162.50/,
     +AU(66)/97700./,AO(66)/18400./,AG(66)/9850./
C
      DATA IA(67)/'H'/,IB(67)/'O'/,AR(67)/1.74/,AM(67)/164.93/,
     +AU(67)/34700./,AO(67)/19500./,AG(67)/10400./
C
      DATA IA(68)/'E'/,IB(68)/'R'/,AR(68)/1.73/,AM(68)/167.26/,
     +AU(68)/36700./,AO(68)/20700./,AG(68)/11100./
C
      DATA IA(69)/'T'/,IB(69)/'M'/,AR(69)/1.72/,AM(69)/168.93/,
     +AU(69)/39300./,AO(69)/21900./,AG(69)/11700./
C
      DATA IA(70)/'Y'/,IB(70)/'B'/,AR(70)/1.94/,AM(70)/173.04/,
     +AU(70)/41000./,AO(70)/23100./,AG(70)/12400./
C
      DATA IA(71)/'L'/,IB(71)/'U'/,AR(71)/1.72/,AM(71)/174.97/,
     +AU(71)/45000./,AO(71)/24400./,AG(71)/13100./
C
      DATA IA(72)/'H'/,IB(72)/'F'/,AR(72)/1.56/,AM(72)/178.49/,
     +AU(72)/46000./,AO(72)/25800./,AG(72)/13900./
C
      DATA IA(73)/'T'/,IB(73)/'A'/,AR(73)/1.43/,AM(73)/180.95/,
     +AU(73)/48500./,AO(73)/27200./,AG(73)/14600./
C
      DATA IA(74)/'W'/,IB(74)/' '/,AR(74)/1.37/,AM(74)/183.85/,
     +AU(74)/51300./,AO(74)/28600./,AG(74)/15400./
C
      DATA IA(75)/'R'/,IB(75)/'E'/,AR(75)/1.37/,AM(75)/186.20/,
     +AU(75)/57200./,AO(75)/30100./,AG(75)/16200./
C
      DATA IA(76)/'O'/,IB(76)/'S'/,AR(76)/1.34/,AM(76)/190.20/,
     +AU(76)/58000./,AO(76)/31600./,AG(76)/17100./
C
      DATA IA(77)/'I'/,IB(77)/'R'/,AR(77)/1.36/,AM(77)/192.20/,
     +AU(77)/62400./,AO(77)/33100./,AG(77)/18000./
C
      DATA IA(78)/'P'/,IB(78)/'T'/,AR(78)/1.37/,AM(78)/195.09/,
     +AU(78)/63400./,AO(78)/34800./,AG(78)/18900./
C
      DATA IA(79)/'A'/,IB(79)/'U'/,AR(79)/1.44/,AM(79)/196.9655/,
     +AU(79)/66900./,AO(79)/36500./,AG(79)/19900./
C
      DATA IA(80)/'H'/,IB(80)/'G'/,AR(80)/1.50/,AM(80)/200.59/,
     +AU(80)/66800./,AO(80)/38200./,AG(80)/20900./
C
      DATA IA(81)/'T'/,IB(81)/'L'/,AR(81)/1.64/,AM(81)/204.37/,
     +AU(81)/75400./,AO(81)/40100./,AG(81)/21900./
C
      DATA IA(82)/'P'/,IB(82)/'B'/,AR(82)/1.60/,AM(82)/207.19/,
     +AU(82)/79800./,AO(82)/41900./,AG(82)/22900./
C
      DATA IA(83)/'B'/,IB(83)/'I'/,AR(83)/1.60/,AM(83)/208.98/,
     +AU(83)/84300./,AO(83)/43800./,AG(83)/24000./
C
      DATA IA(84)/'P'/,IB(84)/'O'/,AR(84)/1.60/,AM(84)/210.00/,
     +AU(84)/88100./,AO(84)/45800./,AG(84)/25100./
C
      DATA IA(85)/'A'/,IB(85)/'T'/,AR(85)/1.60/,AM(85)/210.00/,
     +AU(85)/86500./,AO(85)/40700./,AG(85)/26200./
C
      DATA IA(86)/'R'/,IB(86)/'N'/,AR(86)/1.80/,AM(86)/222.00/,
     +AU(86)/97200./,AO(86)/39800./,AG(86)/27300./
C
      DATA IA(87)/'F'/,IB(87)/'R'/,AR(87)/2.80/,AM(87)/223.00/,
     +AU(87)/102000./,AO(87)/32200./,AG(87)/28500./
C
      DATA IA(88)/'R'/,IB(88)/'A'/,AR(88)/2.20/,AM(88)/226.00/,
     +AU(88)/102000./,AO(88)/33000./,AG(88)/29800./
C
      DATA IA(89)/'A'/,IB(89)/'C'/,AR(89)/1.90/,AM(89)/227.00/,
     +AU(89)/143000./,AO(89)/54000./,AG(89)/31100./
C
      DATA IA(90)/'T'/,IB(90)/'H'/,AR(90)/1.85/,AM(90)/232.04/,
     +AU(90)/118000./,AO(90)/37000./,AG(90)/32300./
C
      DATA IA(91)/'P'/,IB(91)/'A'/,AR(91)/1.80/,AM(91)/231.00/,
     +AU(91)/106000./,AO(91)/38700./,AG(91)/34200./
C
      DATA IA(92)/'U'/,IB(92)/' '/,AR(92)/1.80/,AM(92)/238.03/,
     +AU(92)/112000./,AO(92)/40300./,AG(92)/35000./
C
      DATA IA(93)/'N'/,IB(93)/'P'/,AR(93)/1.80/,AM(93)/237.00/,
     +AU(93)/123000./,AO(93)/25700./,AG(93)/29900./
C
      DATA IA(94)/'P'/,IB(94)/'U'/,AR(94)/1.80/,AM(94)/242.00/,
     +AU(94)/113000./,AO(94)/16200./,AG(94)/22700./
C
   1  FORMAT(///' ',59('+')/' +',57X,'+'/' +  SHELXS-86 - ',
     +'CRYSTAL STRUCTURE SOLUTION - UNIX VERSION  +',
     +/' +',57X,'+'/' +  ',A18,'   STARTED AT ',A8,
     +' ON ',A9,'  +'/' +',57X,'+'/' ',59('+')//)
C
      call time(tim)
      call date(dat)
      write(li,1)name(:18),tim,dat
C
C DEFAULT PARAMETERS
C
        DO 2 I=1,50
   2    IH(I)=IG(I)
      LX=25
      LL=1
      LZ=0
      LQ=0
      LY=65
      KG=0
      LJ=0
      LW=0
      JQ=0
      FG=.1
      PM=0.
      HS=0.
        DO 3 I=1,76
   3    A(I)=0.
      A(14)=1.
      A(16)=1.
      A(19)=1.
      A(26)=1.2
      A(27)=5.
      A(28)=.005
      A(29)=.7
      A(32)=1.5
        DO 4 I=33,35
        A(I)=-2.
   4    A(I+3)=2.
      A(43)=4.
      A(52)=2.
      A(53)=1.
      A(56)=28.
      A(58)=.5
      A(59)=1.5
      A(64)=1.
      A(65)=1.
      A(69)=1.
      A(73)=1.
        DO 5 I=121,123
        F(I)=1.
   5    F(I+3)=0.
C
C READ INSTRUCTION
C
   6  READ(LR,9)IR
      do 600 i=1,80
      if(ir(i).eq.char(13))ir(i)=char(32)
 600  continue
      JR=4
        DO 7 I=5,80
        IF(IR(I).NE.IH(20))JR=I
   7    CONTINUE
      WRITE(LI,10)(IR(I),I=1,JR)
      CALL SXPW(X,IR)
      A(LX)=X
        DO 8 NK=1,34
        IF(ABS(X-C(NK)).LT..5)GOTO 11
   8    CONTINUE
      NK=1
      GOTO 13
   9  FORMAT(80A1)
  10  FORMAT(1X,80A1)
  11  IF(NK.LT.14)WRITE(LP,9)(IR(I),I=1,JR)
      IF(NK.EQ.32)GOTO 6
      IF(NK.EQ.1)GOTO 6
      IF(NK.NE.2)GOTO 13
        DO 12 I=1,40
  12    IT(I)=IR(I+4)
      GOTO 6
C
C DECODE INSTRUCTION
C
  13  NA=0
        DO 14 I=1,126
  14    G(I)=0.
      JD=0
      ID(1)=IH(14)
      ID(2)=IH(20)
      ID(3)=IH(20)
      ID(4)=IH(20)
      NJ=LY+7
      L=LY+21
      N=4
      IF(NK.NE.5)GOTO 16
      J=LY+10
        DO 15 I=J,L
  15    A(I+2)=0.
  16  W=1.
  17  V=0.
      NB=0
      Y=1.
      U=10.
      Z=1.
      GOTO 19
  18  Z=Y*Z
      V=U*ABS(V)+Z*X
      NB=1
      IF(V.EQ.0.)GOTO 19
      V=SIGN(V,W)
      W=V
  19  N=N+1
      K=10
      IF(N.GT.JR)GOTO 22
      X=0.
        DO 20 K=1,19
        IF(IR(N).EQ.IH(K))GOTO 21
  20    X=X+1.
      K=1
      GOTO 22
  21  IF(K.LT.11)GOTO 18
      K=K-9
  22  IF(NK.NE.5)GOTO 31
C
C SYMM
C
      GOTO(19,23,33,33,24,24,24,25,26,25),K
  23  U=1.
      Y=.1
      GOTO 19
  24  K=K+NJ
      A(K)=W
      GOTO 16
  25  A(L)=A(L)+AINT(24.5*V)/24.
      L=L+1
      NJ=NJ+3
      IF(NJ+8.LT.L)GOTO 16
      LY=LY+12
      IF(LZ)29,6,29
C
C CONTINUATION LINES AND ERRORS
C
  26  IF(NK.EQ.5)GOTO 29
      IF(NK.NE.6)GOTO 27
      IF(NA.LT.1)GOTO 29
  27  READ(LR,9)IR
      JR=4
        DO 28 I=5,80
        IF(IR(I).NE.IH(20))JR=I
  28    CONTINUE
      WRITE(LI,10)(IR(I),I=1,JR)
      IF(NK.LT.14)WRITE(LP,9)(IR(I),I=1,JR)
      N=4
      CALL SXPW(X,IR)
      IF(ABS(X-C(32)).LT..5)GOTO 16
  29  WRITE(LI,30)
      CALL SXIT
  30  FORMAT(/' ** NONSENSE')
  31  IF(K.EQ.2)GOTO 23
      IF(NB.EQ.0)GOTO 35
      NA=NA+1
      IF(NA.GT.126)GOTO 29
      G(NA)=V
  32  IF(K-9)34,26,37
  33  A(L)=AINT(24.5*V)/24.
  34  IF(K.NE.3)GOTO 16
      W=-1.
      GOTO 17
  35  IF(K.EQ.1)K=6
      IF(IABS(K-6).GT.1)GOTO 32
      IF(IR(N).EQ.IH(20))GOTO 32
      JD=JD+1
        DO 36 K=1,4
        IF(N.GT.JR)GOTO 37
        IF(IR(N).EQ.IH(17))GOTO 16
        IF(IR(N).EQ.IH(18))GOTO 27
        IF(IR(N).EQ.IH(19))GOTO 37
        IF(IR(N).EQ.IH(20))GOTO 16
        IF(JD.EQ.1)ID(K)=IR(N)
  36    N=N+1
      GOTO 16
C
C CELL
C
  37  IF(NK.GT.26)GOTO 6
      IF(NK.NE.3)GOTO 40
      IF(NA.NE.7)GOTO 29
        DO 38 J=1,7
        IF(.01.GT.G(J))GOTO 29
  38    A(J)=G(J)
      U=2.*A(2)*A(3)*A(4)
        DO 39 J=2,4
        X=1.74533E-2*A(J+3)
        G(J)=COS(X)
        G(J+3)=SIN(X)
        A(J+9)=U*G(J)/A(J)
  39    A(J+6)=A(J)*A(J)
      X=(G(2)*G(3)-G(4))/(G(5)*G(6))
      Y=SQRT(ABS(1.-X*X))
      A(46)=1./(A(2)*G(6)*Y)
      A(48)=1./(A(3)*G(5))
      A(47)=X*A(48)/Y
      A(49)=(-G(6)*G(2)*X-G(5)*G(3))/(A(4)*G(5)*G(6)*Y)
      A(51)=1./A(4)
      A(50)=-G(2)*A(51)/G(5)
      A(60)=1./(A(46)*A(48)*A(51))
      GOTO 6
C
C LATT
C
  40  IF(NK.NE.4)GOTO 41
      IF(G(1).LT.0.)A(23)=1.
      IF(LL.NE.1)GOTO 29
      LL=INT(.5+ABS(G(1)))
      IF(LL.EQ.0)GOTO 29
      IF(LL-8)6,29,29
  41  IF(NK.NE.6)GOTO 63
      IF(LX.NE.25)GOTO 29
      IF(LL.GT.65)GOTO 58
      N=3*LL
      L=INT(4.1-2.*A(23))
        DO 42 I=4,12
  42    F(I)=.5
        DO 43 I=1,3
  43    F(I)=0.
      IF(N-12)46,44,48
  44    DO 45 I=4,12,4
  45    F(I)=0.
  46  IF(N.NE.9)GOTO 49
        DO 47 I=4,9
  47    F(I)=.6666667
      F(5)=.3333333
      F(6)=.3333333
      F(7)=.3333333
      GOTO 49
  48  F(LL-1)=0.
      N=4
  49  LL=LY+8
        DO 50 K=2,L,2
          DO 50 J=1,N,3
          LL=LL+4
          A(LL)=3.-FLOAT(K)
          A(LL+1)=F(J)+99.5
          A(LL+2)=F(J+1)+99.5
  50      A(LL+3)=F(J+2)+99.5
      LQ=LL-1
      F(1)=1.1
C
C CHECK LATT/SYMM
C
      IF(A(1).LT..001)GOTO 29
      M=LY+12
      N=LQ+2
        DO 51 K=65,LY,12
          DO 51 L=M,LL,4
          N=N+3
          A(N)=AMOD(A(L)*(.143*A(K)+.277*A(K+1)+
     +    .811*A(K+2)+A(K+9))+A(L+1)+.5,1.)
          A(N+1)=AMOD(A(L)*(.143*A(K+3)+.277*A(K+4)+
     +    .811*A(K+5)+A(K+10))+A(L+2)+.5,1.)
  51      A(N+2)=AMOD(A(L)*(.143*A(K+6)+.277*A(K+7)+
     +    .811*A(K+8)+A(K+11))+A(L+3)+.5,1.)
        DO 57 K=65,LY,12
          DO 56 L=M,LL,4
          I=LQ+2
  52      I=I+3
          IF(I.GT.N)GOTO 56
          X=AMOD(A(L)*(A(I)*A(K)+A(I+1)*A(K+1)+
     +    A(I+2)*A(K+2)+A(K+9))+A(L+1)+.5,1.)
          Y=AMOD(A(L)*(A(I)*A(K+3)+A(I+1)*A(K+4)+
     +    A(I+2)*A(K+5)+A(K+10))+A(L+2)+.5,1.)
          Z=AMOD(A(L)*(A(I)*A(K+6)+A(I+1)*A(K+7)+
     +    A(I+2)*A(K+8)+A(K+11))+A(L+3)+.5,1.)
          NB=0
          J=LQ+2
  53      J=J+3
          IF(J.GT.N)GOTO 54
          IF(ABS(X-A(J))+ABS(Y-A(J+1))+ABS(Z-A(J+2)).LT..001)NB=NB+1
          GOTO 53
  54      IF(NB.EQ.1)GOTO 52
          WRITE(LI,55)
          CALL SXIT
  55      FORMAT(/' ** INCONSISTENT LATT/SYMM')
  56      CONTINUE
  57    CONTINUE
C
C SFAC
C
  58  IF(NA.GT.0)GOTO 62
      K=4
  59  K=K+1
      IF(K.GT.79)GOTO 6
      IF(IR(K).EQ.IH(19))GOTO 6
      IF(IR(K).EQ.IH(20))GOTO 59
      IF(IR(K).EQ.IH(18))GOTO 29
        DO 60 J=1,32
C Altered here
        IF(IR(K).NE.IA(J))GOTO 60
        IF(IR(K+1).EQ.IB(J))GOTO 61
  60    CONTINUE
      GOTO 29
  61  LQ=LQ+5
      ID(1)=IA(J)
      ID(2)=IB(J)
      ID(3)=IH(20)
      ID(4)=IH(20)
      A(LQ)=AN(J)
      A(LQ+1)=AR(J)
      LZ=LZ+1
      CALL SXPW(A(LQ+2),ID)
      A(LQ+3)=AU(J)
      IF(A(1).LT.1.)A(LQ+3)=AO(J)
      A(LQ+4)=AM(J)
      K=K+1
      GOTO 59
  62  LQ=LQ+5
      A(LQ)=AINT(.5+G(1)+G(3)+G(5)+G(7)+G(9))
      A(LQ+1)=G(13)
      LZ=LZ+1
      CALL SXPW(A(LQ+2),ID)
      A(LQ+3)=G(12)
      A(LQ+4)=G(14)
      IF(NA-14)29,6,29
C
C END
C
  63  IF(NK.NE.22)GOTO 64
      IF(NA.EQ.0)GOTO 101
      LR=INT(G(1))
      IF(LR.LT.0)GOTO 29
      IF(LR.EQ.LH)GOTO 29
      IF(LR.EQ.LI)GOTO 29
      IF(LR.EQ.LP)GOTO 29
      IF(LR.EQ.LF)GOTO 29
      IF(LR.EQ.LA)GOTO 29
      IF(LR.EQ.LB)GOTO 29
      IF(LR.EQ.LG)GOTO 29
      IF(1-NA)29,6,6
C
C UNIT
C
  64  IF(NK.NE.7)GOTO 68
      IF(NA.NE.LZ)GOTO 29
      J=LL+4
      LE=LQ+3
      LX=LQ+5
      U=0.
      V=0.
      P=0.
      Q=0.
      R=0.
      HS=0.
      Z=0.
      Y=0.
        DO 65 I=1,NA
        IF(A(J).GT.1.5)Z=Z+G(I)
        W=A(J)*G(I)
        P=P+W
        Q=Q+A(J+4)*G(I)
        A(J+4)=G(I)
        R=R+A(J+3)*G(I)
        HS=AMAX1(HS,A(J)*A(J))
        U=U+W*A(J)
        V=V+W*A(J)*A(J)
        Y=Y+W*SQRT(A(J))
  65    J=J+5
      T=Q*1.66052/A(60)
      R=R*.1/A(60)
      PM=HS*999.1/U
      Z=A(60)/Z
      X=FLOAT(LL-LY-8)
      WRITE(LI,67)A(60),Z,P,R,PM,Q,T
      A(24)=SQRT(.25*X/U)
C
C DEFAULT PARAMETERS FOR DIRECT METHODS
C
      T=(2.-A(23))/X
      A(45)=V/(U*SQRT(T*U))
      Y=Y**2/(T*P**3)
      X=15.*(250.+A(60)/X)/FLOAT(LY-53)
      FF=AINT(AMIN1(X,150.+.5*X,300.+.25*X))
      A(44)=-AMIN1(20.+ABS(FF)*.5,160.1+40.*A(23))
      JF=0
      L=0
      M=0
      N=0
      T=30.
        DO 66 K=65,LY,12
        IF(A(K).LT.-.5)L=1
        IF(A(K+4).LT.-.5)M=1
        IF(A(K+8).LT.-.5)N=1
        IF(ABS(A(K+9))+ABS(A(K+10))+ABS(A(K+11)).GT..1)T=20.
  66    CONTINUE
      A(42)=AMAX1(-1.1+.3*A(23),AMIN1(-.2,-T*Y*(2.-A(23))))
      IF(T.GT.25.)GOTO 6
      IF(A(23).LT..5)L=3
      IF(L+M+N.GT.2)A(44)=ABS(A(44))
      GOTO 6
  67  FORMAT(/' V =',F10.2,5X,'AT VOL =',F6.1,5X,'F(000) =',F8.1,
     +5X,'MU =',F7.2,' MM-1'//' MAX SINGLE PATTERSON VECTOR =',
     +F6.1,'    CELL WT =',F10.2,'    RHO =',F7.3/)
C
C PHAS
C
  68  IF(NK.NE.19)GOTO 69
      IF(LX.EQ.25)GOTO 29
      IF(LX.NE.LE+2)GOTO 29
      LE=LE+2
      A(LE)=ABS(G(1)+200.*(G(2)+200.*G(3)))
      A(LE+1)=G(4)
      LX=LE+2
      IF(NA-4)29,6,29
C
C OMIT
C
  69  IF(NK.NE.9)GOTO 71
      IF(NA.EQ.3)GOTO 70
      A(52)=.5*ABS(G(1))
      IF(NA.EQ.2)A(53)=(SIN(8.726646E-3*G(2)))**2
      IF(2-NA)29,6,6
  70  IF(F(1).GT.119.5)GOTO 29
      F(1)=F(1)+1.
      I=INT(F(1))
      F(I)=G(1)+200.*(G(2)+200.*G(3))
      GOTO 6
C
C SPIN
C
  71  IF(NK.NE.14)GOTO 73
        DO 72 I=1,3
        F(I+120)=COS(G(I))
  72    F(I+123)=SIN(G(I))
      A(60)=-ABS(A(60))
      IF(NA-3)29,6,29
C
C FRAG
C
  73  IF(NK.NE.15)GOTO 77
      KG=1
      IF(NA.LT.2)GOTO 75
        DO 74 I=2,4
        IF(G(I).LT..1)GOTO 29
        X=1.74533E-2*G(I+3)
        G(I+3)=COS(X)
        IF(G(I+3).GT..99)GOTO 29
  74    G(I+6)=SIN(X)
      X=(G(5)*G(6)-G(7))/(G(8)*G(9))
      Y=SQRT(ABS(1.-X*X))
      A(14)=1./(G(2)*G(9)*Y)
      A(16)=1./(G(3)*G(8))
      A(15)=A(16)*X/Y
      A(17)=(-G(9)*G(5)*X-G(8)*G(6))/(G(4)*G(8)*G(9)*Y)
      A(19)=1./G(4)
      A(18)=-G(5)*A(19)/G(8)
      IF(NA-7)29,6,29
  75    DO 76 I=14,19
  76    A(I)=0.
      A(14)=1.
      A(16)=1.
      A(19)=1.
      GOTO 6
C
C TIME
C
  77  IF(NK.NE.23)GOTO 78
      IF(NA.GT.0)TL=G(1)
      GOTO 6
C
C OTHER INSTRUCTIONS
C
  78  IF(NK.NE.10)GOTO 82
      IF(NA.EQ.0)G(1)=1.2
      IF(G(2).LT.1.)G(2)=1.
      G(4)=G(4)+10.*AINT(G(5))
      IF(NA.GT.4)NA=4
      IF(G(5).LT.0.)GOTO 29
      IF(G(5).GT.3.1)GOTO 29
      JQ=NA
      J=26
  79  K=4
  80  IF(NA.LT.1)NA=1
        DO 81 I=1,NA
        A(J)=G(I)
  81    J=J+1
      IF(K-NA)29,6,6
  82  IF(NK.NE.24)GOTO 85
      IF(NA.LT.1)GOTO 6
      A(43)=G(1)
      K=INT(A(43))
      IF(K.EQ.5)GOTO 83
      IF(IABS(K-2).GT.1)GOTO 84
      A(44)=AINT(2.*FF/A(K+1))
  83  IF(JQ.EQ.0)A(26)=1.
      IF(K.EQ.5)A(44)=.4*FF
      A(44)=-ABS(A(44))
      IF(JF.NE.0)GOTO 84
      FF=FF-.5*A(44)
      A(40)=FF
  84  IF(K.EQ.0)A(44)=10.
      IF(NA.GT.1)A(44)=G(2)
      GOTO 6
  85  IF(NK.NE.11)GOTO 87
      J=54
      G(1)=ABS(G(1))
  86  K=3
      GOTO 80
  87  IF(NK.NE.12)GOTO 88
      J=33
      K=6
      GOTO 80
  88  IF(NK.NE.8)GOTO 89
      J=57
      GOTO 86
  89  IF(NK.NE.13)GOTO 90
      FG=.1+AMIN1(ABS(G(1)),99.)
      GOTO 6
  90  IF(NK.NE.21)GOTO 91
      IF(A(21).NE.0.)GOTO 29
      IF(NA.EQ.0)G(1)=-2.
      A(21)=G(1)
      IF(ABS(A(21)).LT..99)GOTO 29
      IF(NA.LT.2)G(2)=AMIN1(PM*.175*FLOAT(LL-LY-8)/
     +(2.-A(23)),25.)
      A(20)=G(2)
      IF(NA.LT.3)G(3)=1.6
      IF(NA.LT.4)G(4)=AMIN1(360.1,12.01*FLOAT(LY-53))
      A(22)=AINT(ABS(G(4)))+.01*AMIN1(ABS(G(3)),99.)
      GOTO 6
  91  IF(NK.NE.26)GOTO 92
      IF(G(1).LT.2.5)LW=INT(G(1))
      IF(LW.LT.0)GOTO 29
      IF(G(1).GT.2.5)LJ=INT(G(1))
      IF(LJ-4)6,6,29
  92  IF(NK.NE.25)GOTO 93
      IF(A(21).NE.0.)GOTO 29
      A(20)=-9.E9
      IF(NA.EQ.0)G(1)=200.
      A(21)=G(1)
      IF(ABS(A(21)).LT..99)GOTO 29
      A(22)=SIN(8.726646E-3*G(2))
      IF(NA.LT.2)A(22)=.5*A(1)
      GOTO 6
  93  IF(NK.NE.16)GOTO 94
      IF(NA.EQ.0)G(1)=50
      IF(NA.GT.1)JF=1
      IF(NA.LT.2)G(2)=FF
      IF(NA.LT.3)G(3)=-4.-3.*A(23)
      IF(NA.LT.4)G(4)=.1
      IF(ABS(G(3)).LT..99)GOTO 29
      G(3)=SIGN(AINT(ABS(G(3)))+AMIN1(ABS(G(4)),.999),G(3))
      G(4)=ABS(G(5))
      NA=MAX0(3,NA-1)
      J=39
      GOTO 79
  94  IF(NK.NE.17)GOTO 95
      J=30
      GOTO 86
  95  IF(NK.NE.20)GOTO 96
      A(64)=1.
      J=61
      GOTO 79
C
C ATOMS
C
  96  IF(NK.NE.1)GOTO 101
      IF(LX.EQ.25)GOTO 29
      A(LX+1)=FG+1000.*G(1)
      J=INT(ABS(G(1)))
      IF(J.LT.1)GOTO 29
      IF(J.GT.LZ)GOTO 29
      LX=LX+2
      IF(KG.EQ.0)GOTO 98
      IF(A(60).GT.0.)GOTO 29
      G(2)=G(2)/A(14)
      G(3)=(G(3)-G(2)*A(15))/A(16)
      G(4)=(G(4)-G(2)*A(17)-G(3)*A(18))/A(19)
      J=2
      K=3
        DO 97 I=121,123
        J=J+1
        IF(J.GT.4)J=J-3
        K=K+1
        IF(K.GT.4)K=K-3
        W=G(J)
        G(J)=W*F(I)-G(K)*F(I+3)
  97    G(K)=W*F(I+3)+G(K)*F(I)
      G(4)=G(2)*A(49)+G(3)*A(50)+G(4)*A(51)
      G(3)=G(2)*A(47)+G(3)*A(48)
      G(2)=G(2)*A(46)
  98    DO 99 J=2,4
        IF(KG.EQ.0)G(J)=AMOD(G(J)+5.,10.)-5.
  99    G(J)=A(64)*G(J)+A(J+59)
      IF(NA.LT.5)G(5)=1.
      G(5)=AMOD(G(5)+5.,10.)-5.
        DO 100 J=2,7
        A(LX)=G(J)
 100    LX=LX+1
      IF(IABS(NA-5).LT.2)GOTO 6
      IF(11-NA)29,6,29
C
C HKLF OR END OF INSTRUCTIONS
C
 101  IF(LX.EQ.25)GOTO 29
      A(60)=ABS(A(60))
      IF(G(1).LT.0.)LH=LR
C
C ** IF LH IS NOT EQUAL TO LR, IT MAY BE NECESSARY TO OPEN THE **
C ** HKLF DATA FILE (UNIT LH) HERE EXPLICITELY.  SINCE IT WILL **
C ** NO LONGER BE NEEDED, LR MAY ALSO BE 'CLOSED' IF LH IS NOT **
C ** EQUAL TO LR.  THIS ENABLES IT TO BE EDITED TO PREPARE THE **
C ** NEXT JOB ETC.                                             **
C
      K=IABS(INT(G(1)))
      IF(K.GT.7)GOTO 29
      IF(K.EQ.6)GOTO 29
      IF(LH.EQ.LR)GOTO 102
      CLOSE(UNIT=LR,STATUS='KEEP')
C      IF(K.NE.0)OPEN(UNIT=LH,FILE=NAME//'.hkl',STATUS='OLD',ERR=29)
C      IF(K.EQ.0)OPEN(UNIT=LH,FILE=NAME//'.hkl',STATUS='OLD',ERR=29,
C     +FORM='UNFORMATTED')
 102  K=IABS(INT(G(1)))
      IF(K.GT.8)GOTO 29
      A(54)=ABS(A(54))
      J=INT(A(54))
      IF(IABS(J-4).EQ.1)GOTO 105
      IF(K.EQ.7)GOTO 105
      IF(IABS(K-4).EQ.1)WRITE(LI,103)
      IF(J.EQ.1)GOTO 106
      IF(J.EQ.6)GOTO 106
      IF(ABS(A(21)).LT..5)GOTO 105
      IF(J.NE.0)GOTO 29
      J=6
      GOTO 106
 103  FORMAT(/' ** WARNING - IT WOULD BE BETTER TO INPUT ',
     +'F-SQUARED THAN F SO'/' ** THAT ZERO AND NEGATIVE ',
     +'INTENSITIES ARE TREATED CORRECTLY')
 104  FORMAT(/' ** HKLF MATRIX CHANGES HAND OF AXES')
 105  IF(IABS(J-4).NE.1)GOTO 107
      IF(J+J+K.NE.13)GOTO 29
      IF(NA.GT.1)GOTO 29
 106  IF(ABS(A(39)).GT..1)GOTO 29
      IF(ABS(A(30)).GT..1)GOTO 29
      IF(LE.GT.LQ+4)GOTO 29
      IF(J.EQ.6)A(54)=2.
      GOTO 108
 107  IF(K.GT.6)GOTO 29
 108  IF(J.EQ.0)GOTO 110
      IF(J.EQ.4)GOTO 109
      IF(J.LT.7)GOTO 112
 109  IF(ABS(A(39)).GT..1)GOTO 112
      IF(ABS(A(30)).GT..1)GOTO 112
      A(30)=9.E9
      A(32)=0.
      GOTO 112
 110  IF(ABS(A(39)).LT..1)GOTO 111
      A(54)=7.
      IF(HS.GT.290.)A(54)=4.
      GOTO 112
 111  IF(LE.GT.LQ+4)A(54)=8.
      IF(ABS(A(30)).GT..1)A(54)=8.
 112  IF(J.EQ.5)A(26)=9.E9
      IF(A(20).LT.-8.E9)GOTO 114
      IF(ABS(A(21)).LT..5)GOTO 113
      IF(LX.GT.LE+2)A(21)=FLOAT((LX-LE)/8)
      IF(JQ.EQ.0)A(26)=1.
      GOTO 114
 113  IF(ABS(A(54)-2.).LT.1.1)A(26)=SIGN(9.E9,A(26))
 114  IF(ABS(A(57)).GT..1)GOTO 116
      IF(A(54).LT..1)GOTO 116
      U=195.
      IF(A(20).LT.-8.E9)U=45.
      IF(A(54).LT.3.5)GOTO 115
      U=.28*(2.-A(23))*A(60)/FLOAT(LL-LY-8)
      IF(A(26).LT.0.)GOTO 115
      U=U*12./(FLOAT(LY-53)*(2.-A(23)))
 115  A(57)=5.-ABS(A(31))+AINT(U)
 116  IF(NA.GT.10)GOTO 117
      IF(NA.GT.2)GOTO 29
      G(3)=1.
      G(7)=1.
      G(11)=1.
      IF(NA.LT.2)G(2)=1.
 117  LV=LE-6
      X=G(3)*G(7)-G(4)*G(6)
      Y=G(4)*G(9)-G(3)*G(10)
      Z=G(6)*G(10)-G(7)*G(9)
      X=Z*G(5)+Y*G(8)+X*G(11)
      IF(ABS(X).LT..01)GOTO 29
      IF(X.LT.0.)WRITE(LI,104)
      LX=LX-8
      WRITE(LG)F
      REWIND LG
      U=2.*A(2)*A(3)*A(4)
        DO 118 J=2,4
        F(J)=U*COS(1.74533E-2*A(J+3))/A(J)
 118    F(J+3)=F(J)*F(J)
      V=U*U
      U=.5*A(1)*A(1)/(V-A(8)*F(5)-A(9)*F(6)-A(10)*F(7)+
     +F(2)*F(3)*F(4))
        DO 119 J=8,10
        A(J+6)=.5*U*((V/A(J))-F(J-3))
 119    A(J+9)=-2.*U*A(J)*A(J+3)
      A(17)=A(17)+U*A(12)*A(13)
      A(18)=A(18)+U*A(11)*A(13)
      A(19)=A(19)+U*A(11)*A(12)
      LD=LX
      IF(NK.NE.22)LD=0
      M=INT(A(57))
      IF(LW.NE.0)WRITE(LP,120)M
 120  FORMAT('FMAP 5'/'PLAN',I5/'HKLF -3')
      RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SX2B
C
C READ REFLECTION DATA, GENERATE AND SORT E-VALUES
C
      COMMON F(126),G(126),SL,TL,LM,LR,LG,LH,LI,LP,LF,LA,LB,
     +LW,LY,LL,LQ,LE,LV,LX,LD,LZ,LJ,HA,HD,A(50000)
      DIMENSION IP(20),E(87),RS(13),SO(14),SU(14)
      DATA RS/5.,3.5,2.5,2.,1.7,1.5,1.4,1.3,1.2,1.1,1.,.9,.8/
   1  FORMAT(3I4,2F8.2)
   2  FORMAT(20I4)
   3  FORMAT(//I8,'  REFLECTIONS READ, OF WHICH',I6,'  REJECTED'
     +//'    MAXIMUM H, K, L AND 2-THETA =',3F6.0,F8.2)
      NX=0
      LZ=LX+7
        DO 4 I=1,14
        SO(I)=0.
   4    SU(I)=0.
      ML=LY+12
        DO 5 J=ML,LL,4
        A(J+1)=A(J+1)-99.5
        A(J+2)=A(J+2)-99.5
   5    A(J+3)=A(J+3)-99.5
C
C READ DATA FOR FMAP 3 AND 5
C
        DO 6 JP=61,64
   6    A(JP)=0.
      SB=1.
      NR=-1
      M=0
      I=INT(A(54))
      IF(IABS(I-4).NE.1)GOTO 10
   7  N=1
   8  F(N+2)=0.
      NR=NR+1
      IF(I.EQ.3)READ(LH,1)J,K,L,F(N+1)
      IF(I.EQ.5)READ(LH,1)J,K,L,F(N+1),F(N+2)
      IF(MAX0(IABS(J),IABS(K)).GT.99)GOTO 8
      X=FLOAT(J)
      Y=FLOAT(K)
      Z=FLOAT(L)
      IF(ABS(X).GT.A(61))A(61)=ABS(X)
      IF(ABS(Y).GT.A(62))A(62)=ABS(Y)
      IF(ABS(Z).GT.A(63))A(63)=ABS(Z)
      F(N)=X+200.*(Y+200.*Z)
      Q=X*X*A(14)+Y*Y*A(15)+Z*Z*A(16)+Y*Z*A(17)+X*Z*A(18)+X*Y*A(19)
      IF(Q.GT.1.)GOTO 8
      IF(Q.GT.A(64))A(64)=Q
      IF(ABS(F(N)).LT..5)GOTO 9
      M=M+1
      N=N+3
      IF(N.LT.126)GOTO 8
      WRITE(LA)F
      GOTO 7
   9  WRITE(LA)F
      REWIND LA
      NR=NR-M
      Q=114.5916*ATAN2(SQRT(A(64)),SQRT(1.-A(64)))
      WRITE(LI,3)M,NR,A(61),A(62),A(63),Q
      GOTO 141
C
C READ SHELX-76 BINARY DATA FILE
C
  10  N=1
        DO 11 I=52,87
  11    E(I)=0.
      NR=0
      ND=0
      NU=0
      M=IABS(INT(G(1)))+1
      G(1)=.5
      IF(M.GT.1)GOTO 15
  12  READ(LH)(G(I),I=12,95)
      I=6
  13  I=I+6
      IF(I.GT.95)GOTO 12
      IF(ABS(G(I)).LT..5)GOTO 50
      UM=G(I)
      T=G(I+1)
      S=G(I+2)
  14  Q=ABS(UM)
      Z=AINT(.5+.0001*Q)
      X=(AINT(.5+Q)-10000.*Z)*.01
      Y=AINT(X+SIGN(.5,X+.1))
      X=AINT(101.*(X-Y))
      T=T*G(2)
      S=2.*T*G(2)*SQRT(S)
      T=T*T
      GOTO 34
C
C READ CONDENSED DATA
C
  15  IF(M.NE.2)GOTO 29
      UM=G(13)
      KI=0
      KK=0
  16  READ(LH,2)IP
      KK=MOD(KK,99)+1
      JP=0
      GOTO 19
  17  X=1.
  18  UM=UM+X*ABS(FLOAT(IP(JP)))
      UM=AINT(UM+SIGN(.3,UM))
  19  JP=JP+1
      X=100.
      IF(JP.GT.20)GOTO 16
      KI=INT(AMOD(FLOAT(KI)+FLOAT(KK)*FLOAT(IP(JP)),10000.))
      IF(IP(JP))17,20,23
  20  IF(20.GT.JP)GOTO 21
      READ(LH,2)IP
      JP=0
  21  JP=JP+1
      IF(IP(JP).EQ.0)GOTO 50
      IF(IP(JP).NE.KI)GOTO 26
      WRITE(LI,22)
      GOTO 50
  22  FORMAT(/' CHECKSUM O.K.')
  23  L=IP(JP)/1000
      IF(L)17,18,24
  24  UM=UM+FLOAT(L)
      L=MOD(IP(JP)/100,10)-5
      S=1./(FLOAT(MOD(IP(JP),100))*(10.**L))
      JP=JP+1
      IF(JP.LT.21)GOTO 25
      READ(LH,2)IP
      KK=MOD(KK,99)+1
      JP=1
  25  L=(IP(JP)/1000)-4
      KI=INT(AMOD(FLOAT(KI)+FLOAT(KK)*FLOAT(IP(JP)),10000.))
      T=FLOAT(MOD(IP(JP),1000))*(10.**L)
      GOTO 14
  26  WRITE(LI,27)
      CALL SXIT
  27  FORMAT(' ** BAD CONDENSED DATA')
C
C READ H,K,L,E OR H,K,L,F,SIGMA
C
  28  FORMAT(13X,3I3,9X,F9.2,9X,F5.1)
  29  IF(M.NE.6)GOTO 30
C
C ** MAY HAVE TO DELETE ',END=50' IN FOLLOWING STATEMENTS IF **
C ** NOT FORTRAN-77, IN WHICH CASE IT WILL BE NECESSARY TO   **
C ** TERMINATE XRAY SYSTEM REFLECTION FILE IN SOME OTHER WAY **
C
      READ(LH,28,END=50)J,K,L,T,S
      GOTO 31
  30  READ(LH,1,END=50)J,K,L,T,S
C
  31  T=T*G(2)
      S=S*G(2)
      IF(M.EQ.6)GOTO 32
      IF(M.NE.4)GOTO 33
  32  S=2.*S*ABS(T)
      IF(T.LT.0.)T=0.
      T=T*T
  33  IF(S.LT.1.E-4)S=.1
      IF(T.LT..5*S)T=AMIN1(.25*S,.5*SB)
      SB=.8*SB+.2*S
      IF(IABS(J)+IABS(K)+IABS(L).EQ.0)GOTO 50
      X=FLOAT(J)
      Y=FLOAT(K)
      Z=FLOAT(L)
  34  F(N+1)=T
      F(N+2)=S
C
C REORIENTATE, REJECT LATTICE ABSENCES
C
      IF(T.LT.1.E-6)GOTO 47
      U=X*G(3)+Y*G(4)+Z*G(5)
      V=X*G(6)+Y*G(7)+Z*G(8)
      W=X*G(9)+Y*G(10)+Z*G(11)
      IF(ABS(AMOD(U+999.5,1.)-.5)+ABS(AMOD(V+999.5,1.)-.5)+
     +ABS(AMOD(W+999.5,1.)-.5).GT..01)GOTO 45
      J=ML
  35  J=J+4
      IF(J.GT.LL)GOTO 36
      IF(ABS(AMOD(U*A(J+1)+V*A(J+2)+
     +W*A(J+3)+999.5,1.)-.5)-.01)35,45,45
C
C MAXIMISE INDICES
C
  36  F(N)=0.
        DO 37 K=65,LY,12
        X=U*A(K)+V*A(K+3)+W*A(K+6)
        Y=U*A(K+1)+V*A(K+4)+W*A(K+7)
        Z=U*A(K+2)+V*A(K+5)+W*A(K+8)
        IF(AMAX1(ABS(X),ABS(Y),ABS(Z)).GT.99.5)GOTO 47
        X=AINT(1.001*X)
        Y=AINT(1.001*Y)
        Z=AINT(1.001*Z)
        F(N)=AMAX1(F(N),ABS(X+200.*(Y+200.*Z)))
        A(61)=AMAX1(A(61),ABS(X))
        A(62)=AMAX1(A(62),ABS(Y))
        A(63)=AMAX1(A(63),ABS(Z))
  37    CONTINUE
      CALL SXH2(F(N),X,Y,Z)
      IF(E(52).GT.X)E(52)=X
      IF(E(53).LT.X)E(53)=X
      IF(E(54).GT.Y)E(54)=Y
      IF(E(55).LT.Y)E(55)=Y
      IF(E(56).GT.Z)E(56)=Z
      IF(E(57).LT.Z)E(57)=Z
C
C REJECT SYSTEMATIC ABSENCES
C
      K=65
  38  K=K+12
      IF(K.GT.LY)GOTO 39
      Q=AINT(1.001*(X*A(K)+Y*A(K+3)+Z*A(K+6)))+
     +200.*(AINT(1.001*(X*A(K+1)+Y*A(K+4)+Z*A(K+7)))+
     +200.*AINT(1.001*(X*A(K+2)+Y*A(K+5)+Z*A(K+8))))
      IF(A(23).LT..5)Q=ABS(Q)
      IF(Q+.5.LT.F(N))GOTO 38
      IF(ABS(AMOD(.5+ABS(X*A(K+9)+Y*A(K+10)+Z*A(K+11)),1.)
     +-.5)-.01)38,45,45
  39  Q=X*X*A(14)+Y*Y*A(15)+Z*Z*A(16)+Y*Z*A(17)+X*Z*A(18)+X*Y*A(19)
      IF(Q.GT.A(64))A(64)=Q
      IF(Q.LT.1.)GOTO 40
      IF(NU.EQ.0)WRITE(LI,42)
      IF(NU.LT.50)WRITE(LI,41)U,V,W,F(N+1),F(N+2)
      GOTO 46
  40  N=N+3
      IF(N.LT.126)GOTO 48
      WRITE(LA)F
      N=1
      GOTO 48
  41  FORMAT(3F8.2,F12.2,F10.2,'     SIN(THETA) GREATER THAN 1')
  42  FORMAT(/5X,'H',7X,'K',7X,'L',10X,'F*F      SIGMA',
     +'     WHY REJECTED'/)
  43  FORMAT(3F8.2,F12.2,F10.2,
     +'     OBSERVED BUT SHOULD BE SYSTEMATICALLY ABSENT')
  44  FORMAT(/' ** ETC. **')
  45  IF(F(N+1).LT.A(52)*F(N+2))GOTO 47
      IF(NU.EQ.0)WRITE(LI,42)
      IF(NU.LT.50)WRITE(LI,43)U,V,W,F(N+1),F(N+2)
  46  NU=NU+1
      IF(NU.EQ.50)WRITE(LI,44)
  47  NR=NR+1
  48  ND=ND+1
      IF(M-2)13,19,29
C
C END OF DATA - UNIT LH MAY BE CLOSED AT THIS POINT
C
  49  FORMAT('HKLF',I3)
  50  F(N)=0.
      WRITE(LA)F
      REWIND LA
      CLOSE(LH,STATUS='KEEP')
      IF(A(64).GT.1.)A(64)=1.
      X=114.5916*ATAN2(SQRT(A(64)),SQRT(1.-A(64)))
      WRITE(LI,3)ND,NR,A(61),A(62),A(63),X
      IF(A(20).LT.-8.E9)A(22)=AMIN1(A(22)*A(22),A(64))
      NU=0
      NR=0
      L=1
      QH=E(53)-E(52)+1.
      QK=E(55)-E(54)+1.
      QL=FLOAT(LX)+7.3
      QC=QL+.8-E(52)-QH*(E(54)-QK*E(56))
      RA=0.
      RB=.0001
      RC=0.
      RD=.0001
      N=-LJ
      IF(LJ.NE.0)WRITE(LP,49)N
C
C SORT/MERGE REFLECTION DATA
C
  51  QM=FLOAT(LM)+.3
      JF=0
      N=INT(AMIN1(QC+E(53)+QH*(E(55)+QK*E(57)),QM))
      NF=0
      M=LX+8
  52    DO 53 I=M,N
  53    A(I)=0.
      IF(LZ.LT.N)LZ=N
  54  READ(LA)F
      I=-2
  55  I=I+3
      IF(I.GT.126)GOTO 54
      IF(F(I).LT..5)GOTO 58
      CALL SXH2(F(I),X,Y,Z)
      Q=QC+QH*(Y+QK*Z)+X
      IF(Q.LT.QL)GOTO 55
      IF(Q.GT.QM)GOTO 55
      J=INT(Q)
      IF(NF.GT.0)GOTO 56
      A(J)=1.
      GOTO 55
  56  K=INT(A(J))
      IF(NF.GT.1)GOTO 57
      W=AMAX1(F(I+1)/F(I+2),3.)/F(I+2)
      A(K)=F(I)
      A(K+1)=A(K+1)+W
      A(K+2)=A(K+2)+W*F(I+1)
      A(K+3)=A(K+3)+1.
      A(K+4)=A(K+4)+F(I+1)
      GOTO 55
  57  A(K+1)=A(K+1)+ABS(F(I+1)-A(K+2))
      A(K+4)=A(K+4)+1./F(I+2)**2
      GOTO 55
  58  REWIND LA
      IF(NF.GT.0)GOTO 62
      NF=1
      Q=.3
      K=M
        DO 59 I=M,N
        IF(A(I).LT..5)GOTO 59
        A(I)=Q
        Q=Q+5.
        IF(I+INT(Q).GT.LM)GOTO 60
        K=I
  59    CONTINUE
      JF=1
      Q=Q+5.
  60  QM=FLOAT(K)+1.
        DO 61 J=M,K
  61    A(J)=A(J)+QM
      M=K+1
      QM=QM-.7
      N=K+INT(Q-5.)
      GOTO 52
  62  I=M-5
      IF(NF.EQ.2)GOTO 65
  63  I=I+5
      IF(I.GT.N)GOTO 64
      A(I+2)=A(I+2)/A(I+1)
      A(I+1)=0.
      IF(A(I+3).GT.1.5)RB=RB+A(I+4)
      A(I+4)=0.
      GOTO 63
  64  NF=2
      GOTO 54
  65  READ(LG)F
      REWIND LG
      JU=INT(F(1))
  66  I=I+5
      IF(I.GT.N)GOTO 84
      NR=NR+1
C
C LIST REFLECTIONS
C
      G(L)=A(I)
      V=A(I+2)
      G(L+1)=SQRT(AMAX1(1.E-8,V))
      W=1./SQRT(A(I+4))
      CALL SXH2(G(L),X,Y,Z)
      J=INT(X)
      K=INT(Y)
      NI=INT(Z)
      IF(A(I+3).LT.1.5)GOTO 68
      RA=RA+A(I+1)
      P=A(I+1)/(A(I+3)*SQRT(A(I+3)-1.))
      IF(P.LT.5.*W)GOTO 67
      IF(NX.EQ.0)WRITE(LI,70)
      NX=NX+1
      IF(NX.LT.51)WRITE(LI,71)J,K,NI,V,W,P
      IF(NX.EQ.50)WRITE(LI,44)
  67  W=AMAX1(P,W)
  68  IF(LJ.EQ.0)GOTO 75
      P=V
      T=W
      IF(LJ.EQ.4)GOTO 69
      P=G(L+1)
      T=.5*T/P
  69  IF(P.GT.99999.99)GOTO 74
      IF(T.GT.99999.99)GOTO 74
      WRITE(LP,72)J,K,NI,P,T
      GOTO 75
  70  FORMAT(///' INCONSISTENT EQUIVALENTS'//
     +'   H   K   L       F*F    SIGMA(F*F)  ESD OF MEAN(F*F)'/)
  71  FORMAT(3I4,F12.2,2F10.2)
  72  FORMAT(3I4,2F8.2)
  73  FORMAT(3I4,2F8.0)
  74  WRITE(LP,73)J,K,NI,P,T
C
C FIND EPSILON AND RESTRICTED PHASES
C
  75  P=0.
      T=0.
        DO 76 K=65,LY,12
        Q=AINT(1.001*(X*A(K)+Y*A(K+3)+Z*A(K+6)))+
     +  200.*(AINT(1.001*(X*A(K+1)+Y*A(K+4)+Z*A(K+7)))+
     +  200.*AINT(1.001*(X*A(K+2)+Y*A(K+5)+Z*A(K+8))))
        S=SIGN(1.,Q)*(X*A(K+9)+Y*A(K+10)+Z*A(K+11))
        IF(A(23).LT..5)Q=ABS(Q)
        IF(Q+.5.GE.G(L))P=P+1.
        IF(.5-Q.GE.G(L))T=10.*AINT(12.*AMOD(400.01-S,1.)+12.)
  76    CONTINUE
C
C SIN(THETA), PARITY, R(INT) AND R(SIGMA)
C
      Q=X*X*A(14)+Y*Y*A(15)+Z*Z*A(16)+Y*Z*A(17)+X*Z*A(18)+X*Y*A(19)
      G(L+2)=Q+AINT(AMOD(X+998.01,2.)+AMOD(Y+998.01,2.)*2.+
     +AMOD(Z+998.01,2.)*4.)
      G(L+3)=T+(1./P)
      J=58+INT(AMIN1(14.1,33.3333*Q/(A(1)*A(1))))
      E(J)=E(J)+1.
      E(J+15)=E(J+15)+V/P
      S=.5*A(1)/SQRT(Q)
      K=13
  77  IF(RS(K).GT.S)GOTO 78
      K=K-1
      IF(K.GT.0)GOTO 77
  78  K=K+1
      SU(K)=SU(K)+1.
      IF(Q.GT.A(53))GOTO 81
      J=1
  79  J=J+1
      IF(JU.LT.J)GOTO 80
      IF(ABS(G(L)-F(J)).LT..5)GOTO 81
      GOTO 79
  80  IF(V.GT.W*A(52))GOTO 82
  81  G(L+1)=-G(L+1)
      SO(K)=SO(K)+1.
      NU=NU+1
  82  RC=RC+W
      RD=RD+V
      L=L+4
      IF(L.LT.124)GOTO 66
      WRITE(LB)G
      L=1
      GOTO 66
  83  FORMAT(/I8,'  UNIQUE REFLECTIONS, OF WHICH',I7,
     +'  OBSERVED'//'    R(INT) =',F7.4,'     R(SIGMA) =',F7.4,
     +'      FRIEDEL OPPOSITES MERGED')
  84  QC=QC-QM+QL
      IF(JF.EQ.0)GOTO 51
      G(L)=0.
      WRITE(LB)G
      REWIND LB
      NU=NR-NU
      RA=RA/RB
      RC=RC/RD
      WRITE(LI,83)NR,NU,RA,RC
      I=0
      X=0.
      IF(LJ.NE.0)WRITE(LP,72)I,I,I,X,X
C
C NUMBER OF UNIQUE DATA IN SHELLS
C
      NQ=1
        DO 85 I=1,13
        IF(SU(I).GT..5)NQ=I
  85    CONTINUE
        DO 86 I=2,4
        X=AINT(A(I)/RS(NQ))
        J=I+I+18
        G(J-1)=-X
  86    G(J)=X+.5
        DO 87 I=1,NQ
        P=.5*A(1)/RS(I)
        G(I+30)=P*P
        G(I+50)=114.592*ATAN2(P,SQRT(AMAX1(0.,1.-G(I+30))))
        SO(I)=SU(I)-SO(I)
  87    G(I)=0.
      G(14)=0.
      Z=0.
      IF(LY.EQ.77)GOTO 93
      IF(LY-90)96,95,95
  88  FORMAT(///' NUMBER OF UNIQUE DATA AS A FUNCTION OF RESOLUTION ',
     +'IN ANGSTROMS'//' RESOLUTION  INF',13F8.2)
  89  FORMAT(/' N(OBSERVED) ',13F8.0)
  90  FORMAT(/' N(MEASURED) ',13F8.0)
  91  FORMAT(/' N(THEORY)   ',13F8.0)
  92  FORMAT(/' TWO-THETA   0.0',13F8.1)
  93  IF(A(81)*A(85).LT.0.)GOTO 95
  94  G(21)=0.
      GOTO 96
  95  G(23)=0.
      IF(LY.NE.101)GOTO 96
      IF(ABS(A(78))+ABS(A(90)).LT..1)GOTO 94
  96  Y=G(23)
  97  X=G(21)
  98  J=ML
  99  J=J+4
      IF(J.GT.LL)GOTO 100
      IF(ABS(AMOD(X*A(J+1)+Y*A(J+2)+Z*A(J+3)+999.5,1.)-.5)-
     +.01)99,105,105
 100  W=X+200.*(Y+200.*Z)+.5
      K=65
 101  K=K+12
      IF(K.GT.LY)GOTO 102
      Q=AINT(1.001*(X*A(K)+Y*A(K+3)+Z*A(K+6)))+
     +200.*(AINT(1.001*(X*A(K+1)+Y*A(K+4)+Z*A(K+7)))+
     +200.*AINT(1.001*(X*A(K+2)+Y*A(K+5)+Z*A(K+8))))
      IF(ABS(Q).GT.W)GOTO 105
      IF(A(23).LT..5)Q=ABS(Q)
      IF(Q+1..LT.W)GOTO 101
      IF(ABS(AMOD(.5+ABS(X*A(K+9)+Y*A(K+10)+Z*A(K+11)),1.)-.5)-
     +.01)101,105,105
 102  Q=X*X*A(14)+Y*Y*A(15)+Z*Z*A(16)+Y*Z*A(17)+X*Z*A(18)+X*Y*A(19)
      IF(Q.LT..0001)GOTO 105
      K=30+NQ
 103  IF(G(K).LT.Q)GOTO 104
      K=K-1
      IF(K.GT.30)GOTO 103
 104  K=K-29
      G(K)=G(K)+1.
 105  X=X+1.
      IF(X.LT.G(22))GOTO 98
      Y=Y+1.
      IF(Y.LT.G(24))GOTO 97
      Z=Z+1.
      IF(Z.LT.G(26))GOTO 96
      WRITE(LI,88)(RS(I),I=1,NQ)
      WRITE(LI,89)(SO(I),I=1,NQ)
      WRITE(LI,90)(SU(I),I=1,NQ)
      WRITE(LI,91)(G(I),I=1,NQ)
      WRITE(LI,92)(G(I+50),I=1,NQ)
C
C THETA DEPENDENCE
C
        DO 106 J=1,5
 106    G(J)=0.
      Q=.015
        DO 107 I=58,72
        IF(E(I).LT.9.5)GOTO 107
        P=ALOG(E(I+15)/E(I))
        G(1)=G(1)+1.
        G(2)=G(2)+Q
        G(3)=G(3)+Q*Q
        G(4)=G(4)+P
        G(5)=G(5)+P*Q
 107    Q=Q+.03
      P=20.
      IF(G(1).LT..5)GOTO 108
      Q=G(1)*G(3)-G(2)*G(2)
      IF(Q.LT.1.E-6)GOTO 108
      P=(G(2)*G(4)-G(1)*G(5))/Q
 108  P=P/(A(1)*A(1))
C
C NORMALISE E-VALUES
C
      T=10./(A(64)+.001)
      NR=1
      ND=LX+39
      LH=ND
      I=INT(A(29)*.1)
      IF(I.GT.0)LH=LH+2*INT(1.5+A(I+60))
      LD=LH-2
      W=1./(A(27)*A(27))
      W=W*W
      U=78.9568*A(28)/(A(1)*A(1))
        DO 140 M=1,4
        IF(M.NE.1)GOTO 110
          DO 109 I=1,38
 109      G(I)=0.
 110    IF(M.NE.2)GOTO 116
        J=LX+27
          DO 111 K=J,LH,2
          A(K)=0.
 111      A(K+1)=0.
        R=0.
        Q=0.
          DO 112 K=12,19
          R=R+G(K)
 112      Q=Q+G(K+19)
        R=R/AMAX1(Q,.01)
          DO 113 K=31,38
 113      G(K)=G(K)*R
        J=LX+8
          DO 114 K=1,19
          A(J)=G(K)/AMAX1(G(K+19),.01)
 114      J=J+1
          DO 115 K=12,19
 115      A(J)=AMOD(A(29),10.)*(A(J)-1.)+1.
        IF(A(29).LT.5.)GOTO 140
 116    IF(M.NE.3)GOTO 118
        IF(A(29).LT.5.)GOTO 118
          DO 117 K=ND,LH,2
 117      A(K)=AMOD(A(29),10.)*(A(K)/AMAX1(A(K+1),.01)-1.)+1.
 118    IF(M.NE.4)GOTO 120
        J=LX+27
        L=J+3
          DO 119 K=J,L
 119      A(K+4)=A(K)/AMAX1(A(K+4),.01)
 120    READ(LB)F
          DO 136 I=1,124,4
          IF(F(I).LT..5)GOTO 139
          QC=AMOD(F(I+2),1.)
          R=F(I+1)**2*EXP(P*QC)*AMOD(F(I+3),10.)
          Q=QC*T
          N=INT(Q)
          Q=Q-FLOAT(N)
          N=N+1
          S=1.-Q
          L=INT(12.+F(I+2))
          IF(M.NE.1)GOTO 121
          G(L)=G(L)+1.
          G(L+19)=G(L+19)+R
          G(N)=G(N)+S
          G(N+1)=G(N+1)+Q
          G(N+19)=G(N+19)+R*S
          G(N+20)=G(N+20)+R*Q
          GOTO 136
 121      K=N+LX+7
          L=L+LX+7
          R=R*A(L)*(A(K)*S+A(K+1)*Q)
          CALL SXH2(F(I),XX,YY,ZZ)
          E(1)=ABS(XX)
          E(2)=ABS(YY)
          E(3)=ABS(ZZ)
          IF(A(29).LT.5.)GOTO 123
          J=INT(A(29)*.1)
          J=INT(.5+2.*E(J))+LX+39
          IF(M.NE.2)GOTO 122
          A(J)=A(J)+1.
          A(J+1)=A(J+1)+R
          GOTO 136
 122      R=R*A(J)
 123      IF(IABS(N-5).GT.2)GOTO 128
          IF(E(1).GT..5)GOTO 124
          IF(AMIN1(E(2),E(3)).LT..5)GOTO 128
          J=LX+27
          GOTO 126
 124      IF(E(2).GT..5)GOTO 125
          IF(E(3).LT..5)GOTO 128
          J=LX+28
          GOTO 126
 125      J=LX+29
          IF(E(3).GT..5)J=J+1
 126      IF(M.NE.3)GOTO 127
          A(J)=A(J)+1.
          A(J+4)=A(J+4)+R
          GOTO 128
 127      A(J+8)=A(J+8)+ABS(1.-R*A(J+4))
 128      IF(M.NE.4)GOTO 136
          IF(R.GT..001)R=SQRT(SQRT(1./((1./(R*R))+W)))*EXP(U*QC)
C
C TRICLINIC EXPANSION
C
          IF(A(26).GE.0.)GOTO 131
          L=3
          K=53
 129      K=K+12
          IF(K.GT.LY)GOTO 134
          X=XX*A(K)+YY*A(K+3)+ZZ*A(K+6)
          Y=XX*A(K+1)+YY*A(K+4)+ZZ*A(K+7)
          Z=XX*A(K+2)+YY*A(K+5)+ZZ*A(K+8)
          IF(AMAX1(ABS(X),ABS(Y),ABS(Z)).GT.99.5)GOTO 129
          G(NR)=ABS(AINT(1.001*X)+200.*(AINT(1.001*Y)+
     +    200.*AINT(1.001*Z)))
          L=L+1
          E(L)=G(NR)
          N=3
 130      N=N+1
          IF(N.GE.L)GOTO 132
          IF(ABS(E(N)-E(L))-.5)129,130,130
 131      G(NR)=F(I)
          L=4
          E(4)=F(I)
 132      G(NR+1)=F(I+1)
          G(NR+2)=R
          NR=NR+3
          IF(NR.LT.126)GOTO 133
          WRITE(LA)G
          NR=1
 133      IF(A(26).LT.0.)GOTO 129
 134      IF(R.LT.ABS(A(26)))GOTO 136
          IF(F(I+1).LT.0.)GOTO 136
            DO 135 K=4,L
            LD=LD+4
            A(LD)=E(K)
            IF(LD.GT.LM-2000)GOTO 138
            A(LD+1)=AMIN1(R,9.)
            A(LD+2)=0.
            IF(A(26).GT.0.)A(LD+2)=AINT(F(I+3)*.1)*10.
            A(LD+3)=1.
            IF(A(26).GT.0.)A(LD+3)=1./AMOD(F(I+3),10.)
 135        CONTINUE
 136      CONTINUE
        GOTO 120
 137    FORMAT(/' ** TOO MANY LARGE E - REDUCE DELTA(U) ',
     +  'OR INCREASE E(MIN)')
 138    WRITE(LI,137)
        CALL SXIT
 139    REWIND LB
 140    CONTINUE
      G(NR)=0.
      WRITE(LA)G
      REWIND LA
 141  RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SX2C
C
C E-LISTS AND E-CALC
C
C ** NEXT STATEMENT MUST HAVE C IN COLUMN 1 IF NOT FORTRAN-77 **
      CHARACTER*1 IH,IT,IR
C
      COMMON F(126),G(126),SL,TL,LM,LR,LG,LH,LI,LP,LF,LA,LB,
     +LW,LY,LL,LQ,LE,LV,LX,LD,LZ,LJ,HA,HD,A(50000)
      COMMON/WORD/IH(50),IT(40),IR(80)
      DIMENSION IP(20)
   1  FORMAT(//18X,' CENTRIC ACENTRIC    0KL      H0L      HK0',
     +6X,'REST'//' MEAN ABS(E*E-1)    0.968    0.736',4F9.3//
     +' HIGHEST MEMORY FOR DATA REDUCTION =',I6)
   2  FORMAT(///' OBSERVED E .GT.',10F6.3//' NUMBER  ',6X,10I6)
      NA=LX+4
      IF(A(54).GT.98.)GOTO 50
      AP=ABS(A(30))
      M=LY+12
        DO 3 I=M,LL,4
        A(I+1)=A(I+1)+99.5
        A(I+2)=A(I+2)+99.5
   3    A(I+3)=A(I+3)+99.5
      JJ=IABS(INT(A(54)))
      IF(IABS(JJ-4).EQ.1)GOTO 94
      M=0
      K=0
      I=0
        DO 4 J=65,LY,12
        IF(ABS(A(J+1)).GT..5)I=1
        IF(ABS(A(J+2)).GT..5)K=1
        IF(ABS(A(J+5)).GT..5)M=1
   4    CONTINUE
      J=LX+27
      IF(I+K+M.NE.3)GOTO 7
        DO 6 M=1,2
        K=J+2
        X=A(J)+A(J+1)+A(K)
          DO 5 I=J,K
   5      A(I)=X
   6    J=J+8
      GOTO 12
   7  IF(I.EQ.0)GOTO 8
      I=LX+28
      GOTO 10
   8  IF(K.EQ.0)GOTO 9
      I=LX+29
      GOTO 10
   9  IF(M.EQ.0)GOTO 12
      J=LX+28
      I=LX+29
  10  M=J+8
        DO 11 K=J,M,8
        X=A(K)+A(I)
        A(K)=X
        A(I)=X
  11    I=I+8
  12  J=LX+27
        DO 13 I=1,4
        G(I)=A(J+8)/AMAX1(A(J),.01)
  13    J=J+1
      IF(LZ.LT.LD+3)LZ=LD+3
C
C FILE UNIQUE LARGE E
C
      T=ABS(A(26))
      S=T
      IF(T.GT.8.E9)GOTO 20
        DO 14 I=5,14
        IP(I)=0
        G(I)=T
  14    T=T+.1
      I=LX+4
      K=LH-2
      IF(LD.GT.LH)GOTO 16
      WRITE(LI,15)S
      CALL SXIT
  15  FORMAT(/' ** NO OBSERVED E ABOVE',F7.3)
  16  M=1
  17  K=K+4
      IF(K.GT.LD)GOTO 19
      Q=A(K+1)
        DO 18 L=5,14
        IF(Q.GT.G(L))IP(L)=IP(L)+1
  18    CONTINUE
      I=I+4
      A(I)=A(K)
      F(M)=A(K)
      F(M+1)=A(K+2)+Q
      F(M+2)=A(K+3)
      A(I+1)=F(M+1)
      M=M+3
      A(I+2)=-1.
      A(I+3)=A(K+3)
      IF(M.LT.126)GOTO 17
      WRITE(LF)F
      GOTO 16
  19  LD=I
      F(M)=0.
      WRITE(LF)F
      REWIND LF
      WRITE(LI,2)(G(I),I=5,14),(IP(I),I=5,14)
  20  WRITE(LI,1)(G(I),I=1,4),LZ
      T=SL
      CALL SXTM(SL,LI)
C
C SUMMARISE PARAMETERS
C
      WRITE(LI,24)IT
      P=2.*A(52)
      Q=114.59*ATAN2(SQRT(A(53)),SQRT(ABS(1.-A(53))))
      J=AINT(A(29)*.1)
      U=A(29)-10.*FLOAT(J)
      IF(ABS(A(26)).LT.8.E9)WRITE(LI,25)A(26),A(27),A(28),U,J
      WRITE(LI,26)P,Q
      IF(ABS(A(39)).LT..5)GOTO 21
      J=INT(A(40))
      K=INT(A(41))
      Q=AMOD(ABS(A(41)),1.)
      WRITE(LI,27)A(39),J,K,Q,A(42)
      L=INT(A(43))
      M=INT(A(44))
      WRITE(LI,28)L,M
  21  IF(ABS(A(21)).LT..1)GOTO 22
      IF(A(20).LT.-8.E9)GOTO 22
      I=INT(A(21))
      J=INT(A(22))
      Q=100.*(A(22)-FLOAT(J))
      WRITE(LI,30)I,A(20),Q,J
  22  IF(ABS(A(30)).LT..5)GOTO 23
      I=INT(A(30))
      J=INT(A(31))
      WRITE(LI,29)I,J,A(32)
  23  I=INT(A(54))
      IF(I.EQ.2)I=6
      J=INT(A(57))
      WRITE(LI,31)I,J,A(58),A(59)
      WRITE(LI,32)TL
  24  FORMAT(' REM ',40A1/)
  25  FORMAT(' ESEL  EMIN',F7.3,'   EMAX',F7.3,'   DELU',F6.3,
     +'   RENORM',F6.3,'   AXIS',I2)
  26  FORMAT(' OMIT  S',F6.2,'   2THETA(MAX)',F7.1)
  27  FORMAT(' TREF  NP',F10.0,'   NE',I6,'   NTAN',I4,
     +'   TW',F7.3,'   WN',F7.3)
  28  FORMAT(' SUBS  TYPE',I4,'   NS',I5)
  29  FORMAT(' TEXP  NA',I5,'   NH',I4,'   EK',F6.3)
  30  FORMAT(' PATT  M',I4,'   THRESH',F6.1,'   RMIN',F6.2,
     +'   MAXVECS',I6)
  31  FORMAT(' FMAP  CODE',I3/' PLAN  NPEAKS',I5,
     +'   DEL1',F6.3,'   DEL2',F6.3)
  32  FORMAT(' TIME  T',F8.0)
  33  FORMAT(' PSEE  M',F7.0,'   2THETA(MAX)',F7.1)
      TL=TL+T
      IF(ABS(A(26)).GT.8.E9)GOTO 42
C
C DUMP LARGEST E-VALUES (PSEE)
C
      IF(A(20).GT.-8.E9)GOTO 42
      NN=0
      J=LX
  34  J=J+4
      IF(J.GT.LD)GOTO 35
      CALL SXH2(A(J),X,Y,Z)
      IF(X*X*A(14)+Y*Y*A(15)+Z*Z*A(16)+Y*Z*A(17)+
     +X*Z*A(18)+X*Y*A(19).GT.A(22))A(J)=-A(J)
      IF(A(J).GT.0.)NN=NN+1
      GOTO 34
  35  X=114.5916*ATAN2(SQRT(A(22)),SQRT(1.-A(22)))
      NN=MIN0(NN,INT(.1+ABS(A(21))))
      WRITE(LI,33)A(21),X
      WRITE(LP,39)NN,IT,X
      M=0
  36  J=LX+4
      L=0
      Q=0.
  37  J=J+4
      IF(J.GT.LD)GOTO 40
      IF(A(J).LT.0.)GOTO 37
      P=AMOD(A(J+1),10.)
      IF(Q.GT.P)GOTO 37
      Q=P
      L=J
      GOTO 37
  38  FORMAT(3(3I3,F6.3,I2,2X),3I3,F6.3,I2)
  39  FORMAT('READ',I5,' HKLE FOR ',40A1,' 2THETA.LT.',F7.2)
  40  CALL SXH2(A(L),X,Y,Z)
      IF(M.LT.4)GOTO 41
      WRITE(LP,38)(IP(I),IP(I+4),IP(I+8),F(I),IP(I+12),I=1,4)
      M=0
  41  M=M+1
      IP(M)=INT(X)
      IP(M+4)=INT(Y)
      IP(M+8)=INT(Z)
      F(M)=Q
      IP(M+12)=INT(A(L+3))
      A(L)=-A(L)
      NN=NN-1
      IF(NN.GT.0)GOTO 36
      WRITE(LP,38)(IP(I),IP(I+4),IP(I+8),F(I),IP(I+12),I=1,M)
  42  L=LX+4
  43  L=L+4
      IF(L.GT.LD)GOTO 44
      A(L)=ABS(A(L))
      A(L+3)=0.
      GOTO 43
C
C CONVERT SYMOPS TO TRICLINIC ACENTRIC
C
  44  IF(A(26).GT.0.)GOTO 58
      A(23)=1.
      I=LY+8
      J=73
  45  I=I+4
      IF(A(I).LT.0.)GOTO 46
      J=J+4
      A(J)=A(I)
      A(J+1)=A(I+1)
      A(J+2)=A(I+2)
      A(J+3)=A(I+3)
      IF(I.LT.LL)GOTO 45
  46  I=I+3
      LL=J
      LY=65
      J=J+3
  47  I=I+1
      IF(I.GT.LE+1)GOTO 48
      J=J+1
      A(J)=A(I)
      GOTO 47
  48  LQ=LQ+J-LE-1
      LE=J-1
      GOTO 58
  49  FORMAT(///' PARAMETERS FOR PARTIAL STRUCTURE EXPANSION'/)
C
C HEAVY ATOMS FROM PATTERSON - READ BACK E'S
C
  50  A(20)=0.
      A(21)=0.
      A(54)=8.
      A(55)=0.
      JJ=8
      LE=LV+6
      LQ=LE
      Q=0.
      A(57)=5.-1.5*A(31)+AINT(3.5*A(60)/FLOAT((LL-LY-8)*(LY-53)))
      IF(A(57).LT.8.)A(57)=8.
      LD=NA
  51  READ(LF)F
        DO 52 I=1,124,3
        IF(F(I).LT..5)GOTO 53
        IF(LD.GT.LM-2000)GOTO 53
        P=AMOD(F(I+1),10.)
        LD=LD+4
        A(LD)=F(I)
        A(LD+1)=F(I+1)
        IF(P.GT.A(32))Q=Q+.5
        A(LD+3)=-1.
  52    A(LD+4)=0.
      GOTO 51
  53  REWIND LF
      A(30)=AMIN1(200.,AINT(Q))
  54  READ(LA)F
        DO 56 I=1,124,3
        IF(F(I).LT..5)GOTO 57
        IF(F(I+1).LT.0.)GOTO 56
        CALL SXH2(F(I),X,Y,Z)
        P=0.
          DO 55 K=65,LY,12
          Q=AINT(1.001*(X*A(K)+Y*A(K+3)+Z*A(K+6)))+
     +    200.*(AINT(1.001*(X*A(K+1)+Y*A(K+4)+Z*A(K+7)))+
     +    200.*AINT(1.001*(X*A(K+2)+Y*A(K+5)+Z*A(K+8))))
          IF(A(23).LT..5)Q=ABS(Q)
          IF(Q+.5.GT.F(I))P=P+1.
  55      CONTINUE
        F(I+1)=F(I+2)
        F(I+2)=P
  56    CONTINUE
      WRITE(LF)F
      GOTO 54
  57  WRITE(LF)F
      REWIND LF
      REWIND LA
      WRITE(LI,49)
      AP=A(30)
      I=INT(AP)
      J=INT(A(31))
      WRITE(LI,29)I,J,A(32)
      I=INT(A(57))
      WRITE(LI,31)JJ,I,A(58),A(59)
C
C E-CALC
C
  58  LJ=LL-1
      IF(IABS(JJ-1).LT.2)GOTO 94
      IF(A(40).LT.-.1)GOTO 59
      IF(AP.LT..5)GOTO 68
  59  IF(LX.EQ.LV)GOTO 68
      M=0
        DO 60 I=1,3
  60    F(I)=0.
      I=LX+4
  61  I=I+4
      IF(I.GT.LD)GOTO 67
      R=AMOD(A(I+1),10.)
      IF(A(40).LT.-.1)GOTO 62
      IF(R.LT.A(32))GOTO 61
  62  CALL SXH2(A(I),X,Y,Z)
      X=X*6.283185
      Y=Y*6.283185
      Z=Z*6.283185
      O=0.
      P=0.
      J=LV
  63  J=J+8
      IF(J.GT.LX)GOTO 65
      K=INT(.001*A(J+1))*5+LJ
      Q=A(K)*A(J+5)
        DO 64 K=65,LY,12
        U=X*A(K)+Y*A(K+3)+Z*A(K+6)
        V=X*A(K+1)+Y*A(K+4)+Z*A(K+7)
        W=X*A(K+2)+Y*A(K+5)+Z*A(K+8)
        T=U*A(J+2)+V*A(J+3)+W*A(J+4)+X*A(K+9)+
     +  Y*A(K+10)+Z*A(K+11)
        O=O+Q*SIN(T)
  64    P=P+Q*COS(T)
      GOTO 63
  65  O=O*A(23)
      Q=SQRT(O*O+P*P)
      IF(Q.LT.1.E-6)GOTO 61
      A(I+2)=57.2958*ATAN2(O,P)
      IF(A(I+2).LT.0.)A(I+2)=A(I+2)+360.
      IF(R.LT.A(32))GOTO 61
      F(1)=F(1)+R*R
      F(2)=F(2)+R*Q
      F(3)=F(3)+Q*Q
      M=M+1
      A(I+3)=Q/R
      GOTO 61
  66  FORMAT(///' RE =',F7.4,' FOR',I4,' ATOMS AND',
     +I5,' E GREATER THAN',F7.3)
  67  J=(LX-LV)/8
      R=SQRT(ABS(1.-F(2)*F(2)/(F(1)*F(3))))
      WRITE(LI,66)R,J,M,A(32)
      CALL SXTM(SL,LI)
  68  IF(ABS(A(39)).GT..5)GOTO 69
      IF(AP.GT..5)GOTO 69
      IF(LE.LT.LQ+4)GOTO 94
C
C PHAS REFLECTIONS
C
  69  I=LQ+3
  70  I=I+2
      IF(I.GT.LE)GOTO 74
      J=NA
  71  J=J+4
      IF(J.GT.LD)GOTO 72
      IF(ABS(A(J)-A(I)).GT..5)GOTO 71
      NA=NA+4
      P=A(NA)
      A(NA)=A(J)
      A(J)=P
      P=A(J+1)
      A(J+1)=A(NA+1)
      A(NA+1)=P
      A(J+2)=A(NA+2)
      A(NA+2)=A(I+1)
      A(J+3)=A(NA+3)
      P=.1*P
      IF(AMOD(15.*AINT(P)-A(I+1)+7200.5,180.).LT.1.)GOTO 70
      IF(A(23).GT.P)GOTO 70
  72  CALL SXH2(A(I),X,Y,Z)
      WRITE(LI,73)X,Y,Z,A(I+1)
      GOTO 70
  73  FORMAT(/' ** BAD PHAS ',4F6.0)
  74  IF(LX.LT.LV+8)GOTO 82
  75  FORMAT(/' ** CANNOT PHASE ENOUGH REFLECTIONS')
  76  FORMAT(//' FIXED PHASES'//4('    CODE   ',
     +'H   K   L   E   PHI')/)
  77  FORMAT(4(I7,'.',3I4,F6.3,I4))
C
C SELECT PARTIAL STRUCTURE PHASES
C
  78  IF(AP.LT..5)GOTO 82
      Q=.001
      I=NA
      J=0
  79  I=I+4
      IF(I.GT.LD)GOTO 80
      IF(Q.GT.A(I+3))GOTO 79
      J=I
      Q=A(I+3)
      GOTO 79
  80  IF(J.GT.0)GOTO 81
      WRITE(LI,75)
      GOTO 84
  81  NA=NA+4
      P=A(NA)
      A(NA)=A(J)
      A(J)=P
      P=A(NA+1)
      A(NA+1)=A(J+1)
      A(J+1)=P
      P=A(NA+2)
      A(NA+2)=A(J+2)
      A(J+2)=P
      A(J+3)=A(NA+3)
      A(NA+3)=0.
      AP=AP-1.
      GOTO 78
C
C PRINT FIXED PHASES
C
  82  I=LX+4
  83  I=I+4
      IF(I.GT.LD)GOTO 84
      A(I+3)=AMOD(A(I+1),10.)
      GOTO 83
  84  IF(NA.EQ.LX+4)GOTO 88
      IF(A(30).GT.-.5)GOTO 88
      WRITE(LI,76)
      N=0
      I=LX+4
  85  I=I+4
      IF(I.GT.NA)GOTO 87
      IF(N.LT.4)GOTO 86
      WRITE(LI,77)(IP(J),IP(J+4),IP(J+8),IP(J+12),
     +G(J),IP(J+16),J=1,4)
      N=0
  86  CALL SXH2(A(I),X,Y,Z)
      N=N+1
      IP(N)=(I-LX-4)/4
      IP(N+4)=INT(X)
      IP(N+8)=INT(Y)
      IP(N+12)=INT(Z)
      G(N)=AMOD(A(I+1),10.)
      IP(N+16)=INT(AMOD(720.5+A(I+2),360.))
      GOTO 85
  87  WRITE(LI,77)(IP(J),IP(J+4),IP(J+8),IP(J+12),
     +G(J),IP(J+16),J=1,N)
C
C SORT REFLECTIONS ACCORDING TO E
C
  88  N=(LD-NA)/4
      M=1
  89  M=3*M+1
      IF(M.LT.N)GOTO 89
  90  M=M/3
      N=4*M
      NJ=NA+N
      NI=NJ+4
        DO 93 I=NI,LD,4
        Q=A(I)
        R=A(I+1)
        S=A(I+2)
        T=A(I+3)
        J=I
  91    K=J-N
        IF(A(K+3).GT.T)GOTO 92
        A(J)=A(K)
        A(J+1)=A(K+1)
        A(J+2)=A(K+2)
        A(J+3)=A(K+3)
        J=K
        IF(J.GT.NJ)GOTO 91
  92    A(J)=Q
        A(J+1)=R
        A(J+2)=S
  93    A(J+3)=T
      IF(M.GT.2)GOTO 90
  94  LZ=NA
      RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SX2D
C
C DERIVE PHASE RELATIONS
C
C ** NEXT STATEMENT MUST HAVE C IN COLUMN 1 IF NOT FORTRAN-77 **
      CHARACTER*1 IH,IT,IR
C
      COMMON F(126),G(126),SL,TL,LM,LR,LG,LH,LI,LP,LF,LA,LB,
     +LW,LY,LL,LQ,LE,LV,LX,LD,LZ,LJ,HA,HD,A(50000)
      COMMON/WORD/IH(50),IT(40),IR(80)
      DIMENSION IP(27)
      NA=LZ
      IF(ABS(A(39)).LT..5)GOTO 132
      IF(ABS(ABS(A(54))-1.5).LT..8)GOTO 132
      A(22)=A(43)
      A(28)=0.
      PA=.1
      PS=.1
      PF=.1
      NF=1
      NB=NA
      NY=INT(AMAX1(ABS(A(40)),ABS(A(30))+10.))
      NZ=MIN0(LD,INT(.5*(ABS(A(44))+FLOAT(NY))+1.1)*4+LX)
C
C MOVE SPECIAL REFLECTIONS TO TOP OF LIST (TYPE 1, 2, 3 OR 5)
C
      MT=INT(A(43))
      I=NA
      J=NA
      IF(MT.EQ.5)GOTO 1
      IF(MT.GT.3)GOTO 4
      IF(MT.LT.1)GOTO 4
   1  I=I+4
      IF(I.GT.LD)GOTO 3
      CALL SXH2(A(I),F(1),F(2),F(3))
      F(5)=AMOD(ABS(F(1))+.1,2.)+AMOD(ABS(F(2))+.1,2.)+
     +AMOD(ABS(F(3))+.1,2.)
      IF(ABS(F(MT)).GT..5)GOTO 1
      K=I+3
        DO 2 L=I,K
        P=A(L)
        A(L)=A(J+4)
        A(J+4)=P
   2    J=J+1
      GOTO 1
   3  NZ=MIN0(NZ,J)
   4  MA=LX+4
      NS=0
      ME=LD+4
      IF(MT.GE.0)GOTO 75
C
C EXCLUDE STRONGEST E FROM SUBSET (TYPE NEGATIVE)
C
      N=MIN0(LX+4-4*MT,NZ)
      I=NA
      J=MIN0(NZ-4*MT,LD)
   5  I=I+4
      IF(I.GT.N)GOTO 75
      K=I+3
        DO 6 L=I,K
        P=A(L)
        A(L)=A(J)
        A(J)=P
   6    J=J+1
      J=J-8
      IF(J.GT.NZ)GOTO 5
      NZ=NZ-4
      N=MIN0(N,NZ)
      GOTO 5
C
C SCAN PART OF REFLECTION LIST
C
   7  L=NA
      MB=NT+((NZ-LX)/4)-3
   8  L=L+4
      IF(L.GT.NZ)GOTO 45
      R=AMOD(A(L+1),10.)*A(45)
      NQ=MB
      IZ=(L-LX-4)/4
C
C FIND TPR (AS SUM)
C
      W=A(L)-.5
   9  PQ=.0001
      RR=.0001
      Q=1.
      RA=0.
      RB=0.
      PI=0.
      NN=NT+2
      I=ME
      K=I
      MZ=NT
  10  J=MZ
  11  MZ=K+2*((J-K)/4)
      IF(A(MZ).GT.W)GOTO 10
      K=MZ
      IF(J.GT.K+2)GOTO 11
      W=W+1.
  12  I=I+2
  13  IF(I.GE.J)GOTO 19
      X=W-A(I)-A(J)
      IF(X.GT.1.)GOTO 12
      IF(X.GT.0.)GOTO 14
      J=J-2
      GOTO 13
  14  NI=INT(A(I+1)*Q)
      NJ=INT(A(J+1))
C
C REJECT SIGMA-1 TPR
C
      IF(IABS(NJ).GT.IABS(NI))GOTO 15
      K=NI
      NI=NJ
      NJ=K
  15  IF(A(23).GT..5)GOTO 16
      NJ=-IABS(NJ)
      NI=IABS(NI)
  16  IF(NI+NJ.EQ.0)GOTO 18
      N=IABS(NJ)
      M=IABS(NI)
      IF(M.EQ.IZ)GOTO 18
      IF(N.EQ.IZ)GOTO 18
C
C ROUGH ALPHA SUMS TO SELECT SUBSET REFLECTIONS
C
      IF(NS.EQ.2)GOTO 17
      IF(NS.EQ.4)GOTO 17
      K=N*4+LX+5
      NK=M*4+LX+5
      P=(AMOD(A(K),10.)*AMOD(A(NK),10.))**2
      PQ=PQ+P
      IF(NS.NE.1)GOTO 18
      N=N+NT+1
      A(N)=A(N)+P
      M=M+NT+1
      A(M)=A(M)+P
      GOTO 18
C
C PHASE SHIFT
C
  17  IF(NQ+3.GT.LM)GOTO 91
      NQ=NQ+2
      A(NQ)=FLOAT(NI)
      Y=AMOD(ABS(A(I+1)),1.)
      IF(Q.LT.0.)Y=AMOD(1.008-Y,1.)
      A(NQ+1)=SIGN(FLOAT(N)+AMOD(ABS(A(J+1))+Y,1.),FLOAT(NJ))
  18  IF(Q)20,12,12
C
C FIND TPR (AS DIFFERENCE)
C
  19  Q=-1.
      I=ME
      J=MZ
  20  I=I+2
  21  X=W+A(I)-A(J)
      IF(X.LT.0.)GOTO 20
      IF(X.LT.1.)GOTO 14
      J=J+2
      IF(J.LT.NN)GOTO 21
      IF(NS.EQ.3)GOTO 22
      IF(NS.GT.0)GOTO 23
  22  A(L+3)=R*SQRT(PQ)
      GOTO 8
  23  IF(NS.EQ.1)GOTO 48
C
C FIND STATISTICAL WEIGHTS FOR TPR
C
      I=MB
  24  I=I+2
      IF(I.GT.NQ)GOTO 31
      Q=1.
      N=IABS(INT(A(I)))
      NI=4*N+LX+5
      M=IABS(INT(A(I+1)))
      NJ=4*M+LX+5
      J=I
  25  J=J+2
  26  IF(J.GT.NQ)GOTO 29
      IF(IABS(INT(A(J))).NE.N)GOTO 25
      IF(IABS(INT(A(J+1))).NE.M)GOTO 25
      Q=Q+1.
      T=A(J)
      V=A(J+1)
      A(J)=A(NQ)
      A(J+1)=A(NQ+1)
      NQ=NQ-2
      IF(A(L+1).LT.10.)GOTO 25
      K=720+INT(12.*AMOD(ABS(A(I+1)),1.))-INT(12.*AMOD(ABS(V),1.))
      IF(A(I)*T.GT.0.)GOTO 27
      IF(A(NI).LT.10.)GOTO 26
      K=K+INT(SIGN(.1*A(NI),A(I)))
  27  IF(A(I+1)*V.GT.0.)GOTO 28
      IF(A(NJ).LT.10.)GOTO 26
      K=K+INT(SIGN(.1*A(NJ),A(I+1)))
  28  IF(MOD(K,12).NE.0)Q=-9.E5
      GOTO 26
  29  IF(Q.LT.0.)GOTO 24
C
C COUNT TPR AND SUM FOR ALPHA(EST)
C
      IF(NS.NE.4)GOTO 30
      IF(M.GT.IZ)GOTO 30
      PA=PA+1.
      IF(L.LE.NC)PS=PS+1.
  30  Q=SQRT(Q)
      X=AMOD(A(NJ),10.)*AMOD(A(NI),10.)
      IF(NJ.EQ.NI)Q=Q*(X-1.)/X
      RR=RR+(X*Q)**2
      A(I)=SIGN(ABS(A(I))+Q*.1,A(I))
      X=X*Q*R
      Y=X*SQRT(2.-A(23))
      Y=AMIN1(Y*(.5658+Y*(Y*.0106-.1304)),Y/(.56+Y))
      PQ=PQ+X*(X+2.*Y*PI)
      PI=PI+X*Y
      IF(NS.NE.2)GOTO 24
      IF(A(40).GT.0.)GOTO 24
      Y=.0174533*(SIGN(A(NI+1),A(I))+SIGN(A(NJ+1),A(I+1))+
     +30.*AINT(12.*AMOD(ABS(A(I+1)),1.)))
      RA=RA+X*COS(Y)
      RB=RB+X*SIN(Y)
      GOTO 24
C
C WRITE TPR TO FILE
C
  31  A(L+3)=SQRT(PQ)
      IF(NS.EQ.2)GOTO 34
      NQ=NQ+2
      A(NQ)=0.
      A(NQ+1)=FLOAT(IZ)
      I=MB
  32  I=I+2
      IF(I.GT.NQ)GOTO 33
      F(NF)=A(I)
      F(NF+1)=A(I+1)
      NF=NF+2
      IF(NF.LT.126)GOTO 32
      WRITE(LB)F
      NF=1
      GOTO 32
  33  IF(PA.GT.2.*PS+.5)GOTO 92
      GOTO 8
C
C REFLECTION AND TPR LISTS
C
  34  A(28)=A(28)+A(L+3)**2
      IF(L.GT.NB)GOTO 41
      IF(A(40).GT.0.)GOTO 41
      Q=W-.5
      CALL SXH2(Q,X,Y,Z)
      IF(L.EQ.MA+4)WRITE(LI,85)
      I=INT(X)
      J=INT(Y)
      K=INT(Z)
      Q=AMOD(A(L+1),10.)
      IF(A(L+1).GT.10.)GOTO 36
      WRITE(LI,86)IZ,I,J,K,Q,A(L+3)
      GOTO 37
  35  FORMAT(' PHI(CALC) =',F6.0,'     ALPHA(CALC) =',F8.1)
  36  N=15*INT(.1*A(L+1))
      M=N-180
      WRITE(LI,86)IZ,I,J,K,Q,A(L+3),IH(20),M,N
  37  M=0
      Z=SQRT(RA**2+RB**2)
      IF(A(L+2).GT.-.5)WRITE(LI,35)A(L+2),Z
      WRITE(LI,87)
      N=MB
  38  N=N+2
      IF(N.GT.NQ)GOTO 40
      IF(M.LT.27)GOTO 39
      WRITE(LI,87)IP
      M=0
  39  M=M+3
      IP(M-2)=INT(12.*AMOD(ABS(A(N+1)),1.))*30
      IP(M-1)=INT(A(N))
      IP(M)=INT(A(N+1))
      GOTO 38
  40  IF(M.GT.0)WRITE(LI,87)(IP(I),I=1,M)
C
C SPECIAL TPR LIST
C
  41  I=MB
  42  I=I+2
      IF(I.GT.NQ)GOTO 44
      IF(L.GT.NB)GOTO 43
      IF(INT(ABS(A(I+1))).LT.IZ)PF=PF+1.
  43  MB=MB+2
      A(MB)=A(I)
      A(MB+1)=A(I+1)
      GOTO 42
  44  MB=MB+2
      IF(MB.GT.LM-2000)GOTO 91
      A(MB)=0.
      MB=MB+1
      A(MB)=FLOAT(IZ)
      MB=MB+1
      A(MB)=A(L+3)
      A(MB+1)=125./RR
      GOTO 8
C
C IF TYPE 0 OR 4, TAKE SIGMA-2 SUMS FOR WEAK E INTO ACCOUNT
C
  45  IF(NS.NE.0)GOTO 51
      NS=1
      IF(IABS(MT-2).NE.2)GOTO 51
      IZ=0
      I=NT+2
        DO 46 J=I,MB
  46    A(J)=1.
  47  READ(LA)F
      NF=-2
  48  NF=NF+3
      IF(NF.GT.124)GOTO 47
      IF(.5.GT.F(NF))GOTO 49
      CALL SXH2(F(NF),X,Y,Z)
      R=X*X*A(14)+Y*Y*A(15)+Z*Z*A(16)+Y*Z*A(17)+X*Z*A(18)+X*Y*A(19)
      IF(.005.GT.R)GOTO 48
      IF(EXP(2.*R/A(1)**2)*F(NF+2).GT..65+.05*A(23))GOTO 48
      W=F(NF)-.5
      GOTO 9
  49  I=LX+4
      REWIND LA
      J=NT+1
  50  I=I+4
      IF(I.GT.NZ)GOTO 51
      J=J+1
      A(I+3)=A(I+3)*A(J)
      GOTO 50
C
C SIN-THETA WEIGHTING IF TYPE GREATER THAN 6
C
  51  IF(NS.NE.1)GOTO 66
      IF(MT.LT.7)GOTO 53
      T=FLOAT(6-MT)/A(1)**2
      I=NA
  52  I=I+4
      IF(I.GT.NZ)GOTO 53
      CALL SXH2(A(I),X,Y,Z)
      A(I+3)=A(I+3)*EXP(T*(X*X*A(14)+Y*Y*A(15)+Z*Z*A(16)+
     +Y*Z*A(17)+X*Z*A(18)+X*Y*A(19)))
      GOTO 52
C
C DEFINE SUBSET
C
  53  NB=NA
      NC=LX+4*INT(ABS(A(44))+1.1)
  54  Q=.01
      I=NB
  55  I=I+4
      IF(I.GT.NZ)GOTO 56
      IF(Q.GT.A(I+3))GOTO 55
      Q=A(I+3)
      M=I
      GOTO 55
  56  IF(Q.LT..1)GOTO 58
      K=M+3
        DO 57 I=M,K
        P=A(I)
        A(I)=A(NB+4)
        A(NB+4)=P
  57    NB=NB+1
      IF(NB.LT.NC)GOTO 54
C
C SORT REMAINING REFLECTIONS ON E
C
  58  I=NB
      NZ=MIN0(NZ,NC)
  59  I=I+4
      IF(I.GT.LD)GOTO 60
      A(I+3)=AMOD(A(I+1),10.)
      GOTO 59
  60  N=(LD-NB)/4
      M=1
  61  M=3*M+1
      IF(M.LT.N)GOTO 61
  62  M=M/3
      N=4*M
      NJ=NB+N
      NI=NJ+4
        DO 65 I=NI,LD,4
        Q=A(I)
        R=A(I+1)
        S=A(I+2)
        T=A(I+3)
        J=I
  63    K=J-N
        IF(A(K+3).GT.T)GOTO 64
        A(J)=A(K)
        A(J+1)=A(K+1)
        A(J+2)=A(K+2)
        A(J+3)=A(K+3)
        J=K
        IF(J.GT.NJ)GOTO 63
  64    A(J)=Q
        A(J+1)=R
        A(J+2)=S
  65    A(J+3)=T
      IF(M.GT.2)GOTO 62
      NS=2
      NZ=NB
      MB=NT
      IF(MT.NE.0)GOTO 75
C
C MOVE UP SUBSET SIGMA-2 LIST
C
  66  IF(NS.NE.2)GOTO 68
      K=MB+3
      A(K-1)=0.
      A(K)=0.
      J=NT+2
        DO 67 I=J,K
        A(ME)=A(I)
  67    ME=ME+1
      NS=3
      NC=MIN0(LD,LX+4+4*NY)
      GOTO 74
C
C OPTIMISE REFLECTION SET FOR PHASE REFINEMENT
C
  68  IF(NS.NE.3)GOTO 92
      NF=1
      NC=NB
  69  Q=.01
      I=NC
  70  I=I+4
      IF(I.GT.NZ)GOTO 71
      IF(Q.GT.A(I+3))GOTO 70
      Q=A(I+3)
      M=I
      GOTO 70
  71  IF(Q.LT..1)GOTO 73
      K=M+3
        DO 72 I=M,K
        P=A(I)
        A(I)=A(NC+4)
        A(NC+4)=P
  72    NC=NC+1
      IF(NC.LT.LX+4+4*NY)GOTO 69
  73  NS=4
  74  NZ=NC
C
C EXPANDED LIST
C
  75  L=MA
      NT=ME
      A(NT)=0.
      P=0.
  76  L=L+4
      NG=NT
      P=P+1.
      CALL SXH2(A(L),X,Y,Z)
        DO 79 M=65,LY,12
        W=AINT(1.001*(X*A(M)+Y*A(M+3)+Z*A(M+6)))+
     +  200.*(AINT(1.001*(X*A(M+1)+Y*A(M+4)+Z*A(M+7)))+
     +  200.*AINT(1.001*(X*A(M+2)+Y*A(M+5)+Z*A(M+8))))
        Q=1.-A(23)*(1.-SIGN(1.,W))
        W=ABS(W)
        J=NG
  77    J=J+2
        IF(J.GT.NT)GOTO 78
        IF(ABS(W-A(J))-.5)79,79,77
  78    NT=NT+2
        A(NT)=W
        A(NT+1)=Q*(P+AMOD(900.004-Q*(X*A(M+9)+Y*A(M+10)+
     +  Z*A(M+11)),1.))
  79    CONTINUE
      IF(NT.GT.LM-3000)GOTO 91
      IF(L.LT.NZ)GOTO 76
C
C SORT EXPANDED LIST
C
      N=(NT-ME)/2
      M=1
  80  M=3*M+1
      IF(M.LT.N)GOTO 80
  81  M=M/3
      N=M*2
      NJ=ME+N
      NI=NJ+2
        DO 84 I=NI,NT,2
        Q=A(I)
        T=A(I+1)
        J=I
  82    K=J-N
        IF(A(K).LT.Q)GOTO 83
        A(J)=A(K)
        A(J+1)=A(K+1)
        J=K
        IF(J.GT.NJ)GOTO 82
  83    A(J)=Q
  84    A(J+1)=T
      IF(M.GT.2)GOTO 81
      IF(NS.EQ.0)GOTO 7
      L=NA
      MB=NT
      IF(NS.LT.3)GOTO 8
      NZ=MIN0(LD,NC+2*NY)
      IF(NS.NE.4)GOTO 8
      L=MA
      NZ=NC
      IF(A(41).LT.0.)NZ=LD
      A(41)=ABS(A(41))
      GOTO 8
C
C SUMMARISE PHASE RELATIONS
C
  85  FORMAT(///' TPR FOR SUBSET'//' PHASE SHIFT ',
     +'(DEGREES), +/-CODE(2), +/-CODE(3), WHERE  PHI(1)',
     +' = SHIFT +/-PHI(2) +/-PHI(3)')
  86  FORMAT(//' CODE(1) =',I5,5X,'H =',I3,'  K =',I3,
     +'  L =',I3,'    E =',F7.3,'    ALPHA(EST) =',F8.1,A1,
     +4X,'RESTRICTED TO',I4,' OR',I4)
  87  FORMAT(9(I6,2I4))
  88  FORMAT(/I6,' LARGE E-VALUES REFINED USING',F9.0,' UNIQUE TPR',
     +/I6,' REFLECTIONS AND',F9.0,' UNIQUE TPR FOR R(ALPHA)')
  89  FORMAT(//I6,' SUBSET REFLECTIONS AND',
     +F8.0,' UNIQUE TPR FOR FILTER')
  90  FORMAT(//' ** NOT ENOUGH MEMORY TO FIND ALL TPR - ',
     +'REDUCE NS OR NE')
  91  WRITE(LI,90)
      CALL SXIT
  92  F(NF)=0.
      F(NF+1)=0.
      WRITE(LB)F
      REWIND LB
      J=(NB-LX-4)/4
      WRITE(LI,89)J,PF
      J=(NC-LX-4)/4
      WRITE(LI,88)J,PS,IZ,PA
      CALL SXTM(SL,LI)
      NY=NT+4
      M=LD-1
      NQ=M
      IF(A(42).LT..9999)GOTO 93
      IF(A(44).GT.0.)GOTO 131
C
C SMALL E FOR NQR
C
  93  R=.75+.05*A(23)
      A(NT+2)=9.E9
      NQ=NT+3
      A(NQ)=-1.
      MZ=INT(AMIN1(.3*FLOAT(LM)+.7*FLOAT(NQ),FLOAT(NQ)+10000.))
  94  READ(LA)F
      I=-2
  95  I=I+3
      IF(I.GT.124)GOTO 94
      IF(.5.GT.F(I))GOTO 101
      CALL SXH2(F(I),X,Y,Z)
      P=X*X*A(14)+Y*Y*A(15)+Z*Z*A(16)+Y*Z*A(17)+X*Z*A(18)+X*Y*A(19)
      IF(.005.GT.P)GOTO 95
      P=EXP(2.*P/A(1)**2)*F(I+2)
      IF(P.GT.R)GOTO 95
      M=NQ
        DO 98 N=65,LY,12
        Q=ABS(AINT(1.001*(X*A(N)+Y*A(N+3)+Z*A(N+6)))+
     +  200.*(AINT(1.001*(X*A(N+1)+Y*A(N+4)+Z*A(N+7)))+
     +  200.*AINT(1.001*(X*A(N+2)+Y*A(N+5)+Z*A(N+8)))))
        J=M
  96    J=J+2
        IF(J.GT.NQ)GOTO 97
        IF(ABS(Q-A(J))-.5)98,96,96
  97    NQ=NQ+2
        A(NQ)=Q
        A(NQ+1)=P
  98    CONTINUE
  99  IF(NQ.LT.MZ)GOTO 95
      R=R-.01
      J=NT+3
      K=NQ
      NQ=J
 100  J=J+2
      IF(J.GT.K)GOTO 99
      IF(A(J+1).GT.R)GOTO 100
      NQ=NQ+2
      A(NQ)=A(J)
      A(NQ+1)=A(J+1)
      GOTO 100
 101  REWIND LA
      NY=NQ+1
C
C SORT SMALL E VALUES
C
      N=(NQ-NT-3)/2
      M=1
 102  M=3*M+1
      IF(M.LT.N)GOTO 102
 103  M=M/3
      N=M+M
      NJ=NT+3+N
      NI=NJ+2
        DO 106 I=NI,NQ,2
        Q=A(I)
        T=A(I+1)
        J=I
 104    K=J-N
        IF(A(K).LT.Q)GOTO 105
        A(J)=A(K)
        A(J+1)=A(K+1)
        J=K
        IF(J.GT.NJ)GOTO 104
 105    A(J)=Q
 106    A(J+1)=T
      IF(M.GT.2)GOTO 103
      A(NQ+2)=9.E9
C
C DERIVE NQR
C
      MZ=5*MIN0((LM-NQ-111)/13,1000)+NQ+10
      NY=MAX0(MZ+1,NY)
      PZ=0.
      M=NQ+2
      MP=M+5
      KZ=0
      NI=NT+2
      JN=NI+1
      NL=0
      L=LX+4
 107  L=L+4
      IF(L.GT.NC)GOTO 130
      NL=NL+1
      I=NQ+2
      N=MZ
      NU=-1
      S=1.
      Q=-1.
      R=A(L)
      X=A(NQ)-R+.5
      J=ME+2
      IF(X.LT.A(J))GOTO 110
      NM=NT
 108  K=NM
 109  NM=J+2*((K-J)/4)
      IF(A(NM).GT.X)GOTO 108
      J=NM
      IF(K.GT.J+2)GOTO 109
 110  I=I-2
 111  X=A(J)-A(I)+R
      IF(X.LT.-.5)GOTO 110
      IF(X.LT..5)GOTO 124
      J=J-2
      IF(J.GT.ME)GOTO 111
      S=-1.
      NU=0
 112  J=J+2
 113  X=R-A(I)-A(J)
      IF(X.GT..5)GOTO 112
      IF(X.GT.-.5)GOTO 124
      I=I-2
      IF(I.GT.JN)GOTO 113
      Q=1.
      NU=1
 114  I=I+2
 115  X=R+A(I)-A(J)
      IF(X.LT.-.5)GOTO 114
      IF(X.LT..5)GOTO 124
      J=J+2
      IF(J.LT.NI)GOTO 115
 116  J=MZ
 117  J=J+3
      IF(J.GT.N)GOTO 107
      S=A(J)+R
      I=J+3
      K=N+3
 118  K=K-3
 119  IF(K.LE.I)GOTO 117
      X=S+A(I)+A(K)
      IF(X.LT.-.5)GOTO 118
      IF(X.LT..5)GOTO 120
      I=I+3
      GOTO 119
C
C SORT NQR
C
 120  G(1)=FLOAT(NL)+AMOD(ABS(A(I+1))+ABS(A(J+1))+
     +ABS(A(K+1)),1.)
      G(2)=A(J+1)
      G(3)=A(I+1)
      G(4)=A(K+1)
      KZ=KZ+1
      P=2.-A(I+2)-A(J+2)-A(K+2)
        DO 121 NJ=1,4
        NM=INT(ABS(G(NJ)))*4+LX+5
 121    P=P*AMOD(A(NM),10.)
      IF(P.LT.PZ)GOTO 118
      IF(M.LT.MP)M=MP
      A(MP)=P
      A(MP+1)=G(1)
      A(MP+2)=G(2)
      A(MP+3)=G(3)
      A(MP+4)=G(4)
      IF(M.LT.MZ-8)GOTO 123
      PZ=P
      NJ=NQ+2
 122  NJ=NJ+5
      IF(NJ.GT.M)GOTO 118
      IF(A(NJ).GT.PZ)GOTO 122
      PZ=A(NJ)
      MP=NJ
      GOTO 122
 123  MP=MP+5
      GOTO 118
C
C INTERMEDIATE LIST FOR NQR SEARCH
C
 124  NM=INT(ABS(A(J+1)))
      IF(NM.GE.NL)GOTO 127
      K=MZ
 125  K=K+3
      IF(K.GT.N)GOTO 126
      IF(NM.EQ.INT(ABS(A(K+1))))GOTO 127
      GOTO 125
 126  IF(N.GT.LM-5)GOTO 116
      N=N+3
      IF(NY.LT.N+2)NY=N+2
      A(N)=A(J)*S
      X=AMOD(ABS(A(J+1)),1.)
      IF(S.LT.0.)X=AMOD(1.008-X,1.)
      A(N+1)=SIGN(AINT(ABS(A(J+1)))+X,A(J+1)*S)
      A(N+2)=A(I+1)**2
 127  IF(NU)110,112,114
 128  FORMAT(/' HIGHEST MEMORY USED TO DERIVE ',
     +'PHASE RELATIONS =',I6)
 129  FORMAT(/I8,' NEGATIVE QUARTETS FOUND,',I5,' USED')
 130  I=(M-NQ)/5
      A(43)=64.*A(45)/SQRT(FLOAT(I)+24.)
      IF(I.EQ.0)A(43)=0.
      WRITE(LI,129)KZ,I
      CALL SXTM(SL,LI)
 131  LZ=M
      LQ=NQ+2
      N=((M-LQ)/5)*8+M+5
      IF(N.GT.NY)NY=N
      WRITE(LI,128)NY
      A(27)=PF
      A(29)=FLOAT(ME)
      LR=NB
      LH=NC
 132  LE=NA
      RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SX2E
C
C TANGENT REFINEMENT
C
C ** NEXT STATEMENT MUST HAVE C IN COLUMN 1 IF NOT FORTRAN-77 **
      CHARACTER*1 IH,IT,IR
C
      COMMON F(126),G(126),SL,TL,LM,LR,LG,LH,LI,LP,LF,LA,LB,
     +LW,LY,LL,LQ,LE,LV,LX,LD,LZ,LJ,HA,HD,A(50000)
      COMMON/WORD/IH(50),IT(40),IR(80)
      DIMENSION IP(20),ID(31),SN(15),B(64),C(64),D(126),
     +E(126),FB(64),FC(64),FD(64),FE(64),PM(64),PR(64)
      RR=1.E8
      W=0.
      S=0.
      NA=LE
      IF(ABS(A(39)).LT..5)GOTO 132
      TW=AMOD(A(41),1.)
      MB=-1
      IF(A(27).LT..5)MB=0
      IF(MB.NE.0)TW=5.*TW
      ME=INT(A(29))
      NB=LR
      NC=LH
C
C PREPARE NQR INDEX TABLES
C
      NI=ME+1
      NJ=(NC-LX-4)/4+ME
        DO 1 IZ=NI,NJ
   1    A(IZ)=.1
      MP=LZ+5
      NE=MP
      A(NE)=0.
      J=LQ
   2  J=J+5
      IF(J.GT.LZ)GOTO 4
      K=J+3
        DO 3 L=J,K
        M=INT(ABS(A(L+1)))
        NE=NE+2
        A(NE)=-FLOAT(M)
        A(NE+1)=FLOAT(J)
        M=M+ME
   3    A(M)=A(M)+2.
      GOTO 2
   4  ML=MP
        DO 5 I=NI,NJ
        J=INT(A(I))
        A(I)=FLOAT(ML)
   5    ML=ML+J
      I=MP
   6  I=I+2
      IF(I.GT.NE)GOTO 8
   7  IF(A(I).GT.0.)GOTO 6
      M=INT(.1-A(I))+ME
      A(M)=A(M)+2.
      M=INT(A(M))
      Q=A(M)
      A(M)=-A(I)
      IF(I.EQ.M)GOTO 6
      A(I)=Q
      Q=A(M+1)
      A(M+1)=A(I+1)
      A(I+1)=Q
      GOTO 7
C
C WRITE EXPANDED NQRS
C
   8  L=0
      NF=1
      K=LX+4
   9  K=K+4
      IF(K.GT.NC)GOTO 13
      L=L+1
      NJ=L+ME
      NJ=INT(A(NJ))
  10  IF(INT(A(NJ)).LT.L)GOTO 9
      J=INT(A(NJ+1))
      NJ=NJ-2
      T=AMOD(A(J+1),1.)
      W=AINT(A(J+1))
      X=AINT(A(J+2))
      Y=AINT(A(J+3))
      Z=AINT(A(J+4))
  11  IF(INT(ABS(W)).NE.L)GOTO 12
      S=SIGN(1.,W)
      G(NF)=T+FLOAT(L)
      G(NF+1)=S*X
      G(NF+2)=S*Y
      G(NF+3)=S*Z
      NF=NF+4
      IF(NF.LT.122)GOTO 10
      WRITE(LG)G
      NF=1
      GOTO 10
  12  S=W
      W=X
      X=Y
      Y=Z
      Z=S
      GOTO 11
  13  G(NF)=0.
      WRITE(LG)G
      REWIND LG
C
C RETAIN UP TO 100 STRONGEST NQR FOR FILTER
C
      WF=0.
      MS=ME-3
      IF(MB.EQ.0)GOTO 24
      IF(A(44).GT.0.)GOTO 24
      M=(NB-LX-4)/4
      N=LZ
      LZ=LQ
      I=LQ
  14  I=I+5
      IF(I.GT.N)GOTO 17
      IF(INT(A(I+1)).GT.M)GOTO 14
      LZ=LZ+5
      A(LZ)=A(I)
      A(LZ+1)=A(I+1)
      A(LZ+2)=A(I+2)
      A(LZ+3)=A(I+3)
      A(LZ+4)=A(I+4)
      GOTO 14
  15  FORMAT(///' NEGATIVE QUARTETS FOR SUBSET'//'  PROB.   PHI(1)',
     +' PHI(2) PHI(3) PHI(4) = PHI  ERROR'/)
  16  FORMAT(F7.3,4I7,'  =',I5,I6)
  17  Q=-1.
      N=0
      I=LQ
  18  I=I+5
      IF(I.GT.LZ)GOTO 19
      IF(A(I).LT.Q)GOTO 18
      N=I
      Q=A(I)
      GOTO 18
  19  IF(N.EQ.0)GOTO 24
      ME=ME+4
      A(ME-3)=A(N+1)
      A(ME-2)=A(N+2)
      A(ME-1)=A(N+3)
      A(ME)=A(N+4)
      IF(A(40).GT.0.)GOTO 22
      IF(ME.EQ.MS+7)WRITE(LI,15)
      IP(1)=INT(A(ME-3))
      IP(2)=INT(A(ME-2))
      IP(3)=INT(A(ME-1))
      IP(4)=INT(A(ME))
      IP(5)=INT(12.*AMOD(9999.52-A(N+1),1.))*30
      J=5
      Q=7379.5-FLOAT(IP(5))
      K=N
        DO 20 M=1,4
        K=K+1
        L=INT(ABS(A(K)))*4+LX+6
        P=A(L)
        IF(P.LT.-.5)GOTO 21
        IF(A(K).LT.0.)P=-P
  20    Q=Q+P
      J=6
      IP(6)=INT(AMOD(Q,360.))-179
  21  R=.5+.5*TANH(A(N)*A(45)**2/(1.+A(23)))
      WRITE(LI,16)R,(IP(L),L=1,J)
  22  Q=1.
      K=ME-3
        DO 23 I=K,ME
        J=INT(ABS(A(I)))*4+LX+5
  23    Q=Q*AMOD(A(J),10.)
      WF=WF+Q
      LQ=LQ+5
      A(N)=A(LQ)
      A(N+1)=A(LQ+1)
      A(N+2)=A(LQ+2)
      A(N+3)=A(LQ+3)
      A(N+4)=A(LQ+4)
      IF(ME.LT.MS+400)GOTO 17
  24  I=(ME-MS)/4
      WRITE(LI,27)I
C
C SEMINVARIANTS
C
      TS=0.
      TZ=0.
      NS=ME
      I=LX+4
  25  I=I+4
      IF(I.GT.LD)GOTO 31
      TS=TS+A(I+3)
      TZ=TZ+A(I+3)**2/(A(I+3)+5.)
      IF(I.GT.NC)GOTO 25
      IF(A(23).LT..5)GOTO 26
      IF(A(I+1).LT.10.)GOTO 25
      IF(A(I+1).LT.120.)GOTO 25
      IF(A(I+1).GT.130.)GOTO 25
  26  IF(NS-ME.GT.64)GOTO 25
      CALL SXH2(A(I),X,Y,Z)
      IF(AMOD(900.1+Z,2.)+AMOD(98.1+Y,2.)+
     +AMOD(98.1+X,2.).GT..5)GOTO 25
      NS=NS+1
      A(NS)=FLOAT((I-LX-4)/4)
      J=INT(X)
      K=INT(Y)
      L=INT(Z)
      T=AMOD(A(I+1),10.)
      IF(NS.EQ.ME+1)WRITE(LI,28)
      WRITE(LI,29)J,K,L,T
      IF(MOD(NS-ME,5).EQ.0)WRITE(LI,29)
      GOTO 25
  27  FORMAT(//I4,' NQR INCLUDED IN FILTER')
  28  FORMAT(//' ONE-PHASE SEMINVARIANTS'//'   H   K   L     E'/)
  29  FORMAT(3I4,F8.3)
C
C SET UP PHASE DETERMINATION
C
  30  FORMAT(//I4,' /',I3,' PARALLEL REFINEMENTS,  HIGHEST MEMORY =',I7)
  31  NQ=1
      MQ=1
      IF(A(39).LT.0.)GOTO 32
      X=2.*FLOAT(LM-NS)
      I=INT(A(39)-.5)
      NQ=MIN0(64,I+1,INT(X/FLOAT(NC-LX-4)))
      K=I+NQ
      NQ=I/(K/NQ)+1
      MQ=MIN0(126,INT(X/FLOAT(NB-LX-4)),(1-MB)*NQ)
  32  ML=NS+MAX0(((NC-LX-4)/2)*NQ,((NB-LX-4)/2)*MQ)
      I=-MQ*MB
      WRITE(LI,30)I,NQ,ML
      MT=INT(A(41))
      NE=NS+1
      NM=2*NQ
      NL=NE-NM
      MH=2*MQ
      ML=NE-MH
      CALL SXTM(SL,LI)
      WRITE(LI,116)IT
      RN=2097152.*AMOD(SQRT(.4321*ABS(A(40))),1.)
      PQ=9.E9
      TN=.3
      PS=0.
        DO 33 NJ=1,15
  33    SN(NJ)=SIN(FLOAT(NJ-1)*.523598)
        DO 34 NG=1,31
  34    ID(NG)=0
  35    DO 36 I=1,NQ
        FB(I)=0.
        FC(I)=.0001
        FD(I)=0.
        FE(I)=0.
        B(I)=0.
        C(I)=0.
  36    PM(I)=9.E9
        DO 37 I=1,MQ
        D(I)=-1.E-6
  37    E(I)=0.
      NZ=MB
      MM=MQ
      PN=.1
C
C STARTING PHASES
C
  38    DO 39 I=1,MQ
        RN=AMOD((1.+2.*AINT(RN/2.+.3))*5.,2097152.)
        F(I)=0.
        IF(MB.NE.0)GOTO 39
        PM(I)=0.
        PR(I)=RN
  39    G(I)=RN
      IF(A(39).LT.0.)G(1)=-A(39)
  40  N=NE
      M=NB
      IF(NZ.EQ.0)M=NC
        DO 44 I=1,MM
        R=G(I)
        R=AMOD(7169.*SQRT(R/2097152.),1.)
        K=N
        L=LX+4
  41    L=L+4
        IF(L.GT.M)GOTO 44
        P=AMOD(A(L+1),10.)
        IF(MB.NE.0)P=P*.2
        IF(L.GT.NB)P=P*TW
        R=AMOD((1.+2.*AINT(1048576.*R+.3))*5.,2097152.)/2097152.
        Q=R
        IF(L.LE.NA)Q=A(L+2)/360.
        IF(A(23).GT..5)GOTO 42
        Q=SIGN(1.,AMOD(Q+.75,1.)-.5)
        A(K)=P*Q
        A(K+1)=-A(K)
        GOTO 43
  42    IF(A(L+1).GT.10.)Q=AMOD(AINT(.1*A(L+1))/24.+9.25-Q,.5)-.25+Q
        Q=Q*6.28319
        A(K)=P*COS(Q)
        A(K+1)=P*SIN(Q)
  43    K=K+MM*2
        GOTO 41
  44    N=N+2
C
C INITIAL PHASING
C
      MU=1
      IF(MB.EQ.0)GOTO 47
      M=LD+2
      IZ=5
      IF(A(22).LT.3.5)GOTO 62
        DO 45 K=65,LY,12
        IF(ABS(A(K+9))+ABS(A(K+10))+ABS(A(K+11)).GT..1)IZ=MIN0(IZ+1,8)
  45    CONTINUE
      GOTO 62
  46  REWIND LB
      REWIND LG
  47  MU=NZ
      NZ=NZ+1
      IF(NZ.GT.MT)GOTO 108
      READ(LG)G
      NG=1
      MP=0
  48  READ(LB)F
      M=-1
  49  M=M+2
      IF(M.GT.125)GOTO 48
      P=F(M)
      T=ABS(P)
      XQ=F(M+1)
      Z=ABS(XQ)
  50  I=INT(T)*NM+NL
      J=INT(Z)*NM+NL
      IF(I.EQ.NL)GOTO 63
      IF(MU.EQ.0)GOTO 49
  51  KI=INT(T)*4+LX+5
      KJ=INT(Z)*4+LX+5
      T=10.*AMOD(T,1.)
      Z=AMOD(Z,1.)
      IF(A(23).GT..5)GOTO 55
      IF(Z.GT..25)J=J+1
      IF(ABS(T-1.).GT..001)GOTO 53
C
C ** VERY CRITICAL LOOP **
C
        DO 52 K=1,MM
        D(K)=D(K)+A(I)*A(J)
        I=I+2
  52    J=J+2
C
      GOTO 61
C
C ** VERY CRITICAL LOOP **
C
  53    DO 54 K=1,MM
        D(K)=D(K)+T*A(I)*A(J)
        I=I+2
  54    J=J+2
C
      GOTO 61
  55  R=SIGN(1.,P)
      S=SIGN(1.,XQ)
      K=INT(12.*Z)
      V=SN(K+1)*T
      U=SN(K+4)*T
C
C SIMPLIFY TPR SUMS INVOLVING TWO RESTRICTED PHASES
C
      KJ=INT(.1*A(KJ))
      IF(KJ.LT.12)GOTO 59
      IF(ABS(V).GT..001)GOTO 59
      KI=INT(.1*A(KI))
      IF(KI.NE.12)GOTO 58
      T=U
      IF(KJ.EQ.12)GOTO 53
      IF(KJ.NE.18)GOTO 59
      T=T*S
      J=J+1
C
C ** VERY CRITICAL LOOP **
C
  56    DO 57 K=1,MM
        E(K)=E(K)+T*A(I)*A(J)
        I=I+2
  57    J=J+2
C
      GOTO 61
  58  IF(KI.NE.18)GOTO 59
      T=U*R
      I=I+1
      IF(KJ.EQ.12)GOTO 56
      T=-T*S
      J=J+1
      IF(KJ.EQ.18)GOTO 53
      J=J-1
      I=I-1
C
C ** VERY CRITICAL LOOP **
C
  59    DO 60 K=1,MM
        Y=R*A(I+1)
        X=U*A(I)-V*Y
        Y=U*Y+V*A(I)
        Q=S*A(J+1)
        D(K)=D(K)+X*A(J)-Y*Q
        E(K)=E(K)+X*Q+Y*A(J)
        I=I+2
  60    J=J+2
C
  61  IF(NZ.GT.0)GOTO 49
  62  M=M+2
      P=A(M)
      XQ=A(M+1)
      T=ABS(P)
      Z=ABS(XQ)
      IF(NZ.EQ.0)GOTO 50
      I=INT(T)*MH+ML
      J=INT(Z)*MH+ML
      IF(I.GT.ML)GOTO 51
      IF(J-ML)96,96,69
C
C NQR SUMS
C
  63  IF(J.EQ.NL)GOTO 96
      IF(NZ.EQ.0)GOTO 69
      IF(MU.EQ.0)GOTO 70
      MP=MP+1
      IF(NZ.EQ.MT)GOTO 64
      IF(A(43).LT.1.E-6)GOTO 70
  64  IF(MP.NE.INT(G(NG)))GOTO 70
      N=INT(ABS(G(NG+1)))*NM+NL
      K=INT(ABS(G(NG+2)))*NM+NL
      L=INT(ABS(G(NG+3)))*NM+NL
      IF(A(23).GT..5)GOTO 66
      IF(AMOD(G(NG),1.).GT..25)N=N+1
C
C ** VERY CRITICAL LOOP **
C
        DO 65 I=1,NQ
        B(I)=B(I)+A(N)*A(K)*A(L)
        N=N+2
        K=K+2
  65    L=L+2
C
      GOTO 68
  66  I=INT(12.*AMOD(G(NG),1.))
      Q=SN(I+1)
      P=SN(I+4)
      R=SIGN(1.,G(NG+1))
      S=SIGN(1.,G(NG+2))
      T=SIGN(1.,G(NG+3))
C
C ** VERY CRITICAL LOOP **
C
        DO 67 I=1,NQ
        U=A(N)*A(K)-A(N+1)*A(K+1)*R*S
        V=A(N)*A(K+1)*S+A(N+1)*A(K)*R
        X=U*A(L)-V*A(L+1)*T
        Y=U*A(L+1)*T+V*A(L)
        B(I)=B(I)+P*X+Q*Y
        C(I)=C(I)-P*Y+Q*X
        N=N+2
        K=K+2
  67    L=L+2
C
  68  NG=NG+4
      IF(NG.LT.122)GOTO 64
      READ(LG)G
      NG=1
      GOTO 64
C
C PHASE RESTRICTIONS
C
  69  M=M+2
      Z=A(M)
      RR=A(M+1)
  70  L=4*INT(XQ)+LX+4
      K=MU
      IF(L.GE.NB)MU=1
      IF(K.EQ.0)GOTO 49
      P=AMOD(A(L+1),10.)
      IF(A(L+1).LT.10.)GOTO 74
      Q=.261799*AINT(.1*A(L+1))
      U=COS(Q)
      V=SIN(Q)
      IF(NZ.GT.0)GOTO 72
        DO 71 K=1,MM
        X=D(K)*U+E(K)*V
        E(K)=X*V
  71    D(K)=X*U
      GOTO 74
  72    DO 73 K=1,NQ
        X=B(K)*U+C(K)*V
        Y=D(K)*U+E(K)*V
        C(K)=X*V
        E(K)=Y*V
        B(K)=X*U
  73    D(K)=Y*U
C
C SUM FOR RALPHA(0)
C
  74  T=P*A(45)
      IF(NZ.GT.0)Z=A(L+3)
      ZG=1./(Z+5.)
      IF(NZ)75,81,84
  75  IF(IZ.GT.0)GOTO 81
      IF(A(23).GT..5)GOTO 79
C
C ** CRITICAL LOOP **
C
        DO 76 K=1,MQ
        F(K)=F(K)+(Z-T*ABS(D(K)))**2
  76    D(K)=-1.E-6
      GOTO 62
C
C ** CRITICAL LOOP **
C
  77    DO 78 K=1,MM
        W=SIGN(P,D(K))*AMIN1(1.,D(K)**2*RR)
        D(K)=-1.E-6
        A(J+1)=-W
        A(J)=W
  78    J=J+2
C
      GOTO 83
C
C ** CRITICAL LOOP **
C
  79    DO 80 K=1,MQ
        F(K)=F(K)+(Z-T*SQRT(D(K)**2+E(K)**2))**2
        E(K)=0.
  80    D(K)=-1.E-6
      GOTO 62
C
C SELECT NEW PHASES FOR SUBSET
C
  81  IF(A(23).LT..5)GOTO 77
C
C ** CRITICAL LOOP **
C
        DO 82 K=1,MM
        W=D(K)**2+E(K)**2
        W=P*AMIN1(1.,W*RR)/SQRT(W)
        A(J+1)=W*E(K)
        E(K)=0.
        A(J)=W*D(K)
        D(K)=-1.E-6
  82    J=J+2
C
  83  IF(L.LT.NB)GOTO 62
      GOTO 96
C
C NEW PHASES FOR FULL REFINEMENT
C
  84  ZZ=(Z/T)**2
      ZT=ZZ
      IF(A(L+1).GT.10.)ZT=9.E9
      IF(NZ.LT.MT)GOTO 89
      IF(A(23).GT..5)GOTO 86
C
C ** CRITICAL LOOP **
C
        DO 85 K=1,NQ
        W=B(K)*D(K)
        FB(K)=FB(K)+W
        FC(K)=FC(K)+ABS(W)
        W=T*ABS(D(K))
        FD(K)=FD(K)+ZG*(Z-W)**2
  85    FE(K)=FE(K)+W
C
      GOTO 88
C
C ** CRITICAL LOOP **
C
  86    DO 87 K=1,NQ
        FB(K)=FB(K)+B(K)*D(K)+C(K)*E(K)
        W=SQRT(D(K)**2+E(K)**2)
        FC(K)=FC(K)+W*SQRT(B(K)**2+C(K)**2)
        FD(K)=FD(K)+ZG*(Z-W*T)**2
  87    FE(K)=FE(K)+W*T
C
  88  IF(L.GT.NC)GOTO 94
  89  IF(L.LE.NA)GOTO 94
      IF(A(23).LT..5)GOTO 92
C
C ** CRITICAL LOOP **
C
        DO 91 K=1,NQ
        U=D(K)-A(43)*B(K)
        V=E(K)-A(43)*C(K)
        W=U**2+V**2
        X=D(K)**2+E(K)**2
        IF(X.LT.ZT)GOTO 90
        X=ZZ/X
        Y=SIGN(SQRT(ABS(1.-X)),B(K)*E(K)-D(K)*C(K))
        X=SQRT(X)
        U=D(K)*X-E(K)*Y
        V=E(K)*X+D(K)*Y
  90    W=P/SQRT(W)
        A(J+1)=W*V
        A(J)=W*U
  91    J=J+2
      GOTO 94
C
C ** CRITICAL LOOP **
C
  92    DO 93 K=1,NQ
        W=SIGN(P,D(K)-A(43)*B(K))
        A(J+1)=-W
        A(J)=W
  93    J=J+2
C
C ** CRITICAL LOOP **
C
  94    DO 95 K=1,NQ
        B(K)=0.
        C(K)=0.
        D(K)=-1.E-6
  95    E(K)=0.
C
      IF(NZ.EQ.MT)GOTO 49
      IF(L.LT.NC)GOTO 61
C
C NQR SUMS FOR FILTER
C
  96  IF(NZ.GT.0)GOTO 46
      M=LD+2
      IZ=IZ-1
      IF(IZ.GT.NZ)GOTO 62
      IF(NZ.EQ.0)GOTO 47
      J=MS
  97  J=J+4
      IF(J.GT.ME)GOTO 101
      K=INT(A(J))*MH+ML
      L=INT(ABS(A(J+1)))*MH+ML
      M=INT(ABS(A(J+2)))*MH+ML
      N=INT(ABS(A(J+3)))*MH+ML
      IF(A(23).GT..5)GOTO 99
      IF(AMOD(A(J),1.).GT..25)K=K+1
C
C ** CRITICAL LOOP **
C
        DO 98 I=1,MQ
        E(I)=E(I)+A(K)*A(L)*A(M)*A(N)
        K=K+2
        L=L+2
        M=M+2
  98    N=N+2
C
      GOTO 97
  99  I=INT(12.*AMOD(A(J),1.))
      Q=SN(I+1)
      P=SN(I+4)
      R=SIGN(1.,A(J+1))
      S=SIGN(1.,A(J+2))
      T=SIGN(1.,A(J+3))
C
C ** CRITICAL LOOP **
C
        DO 100 I=1,MQ
        U=A(K)*A(L)-A(K+1)*A(L+1)*R
        V=A(K)*A(L+1)*R+A(K+1)*A(L)
        X=U*A(M)-V*A(M+1)*S
        Y=U*A(M+1)*S+V*A(M)
        E(I)=E(I)+P*(X*A(N)-V*A(N+1)*T)-Q*(Y*A(N+1)*T+X*A(N))
        K=K+2
        L=L+2
        M=M+2
 100    N=N+2
C
      GOTO 97
C
C APPLY FILTER
C
 101  J=0
      Q=-9.E9
 102    DO 103 I=1,NQ
        IF(PM(I).LT.Q)GOTO 103
        Q=PM(I)
        M=I
 103    CONTINUE
 104  J=J+1
      IF(J.GT.MQ)GOTO 105
      P=F(J)/A(28)
      IF(WF.GT..1)P=P+AMAX1(0.,E(J)/WF+.25)**2
      F(J)=0.
      E(J)=0.
      IF(P.GT.Q)GOTO 104
      PM(M)=P
      PR(M)=G(J)
      Q=P
      GOTO 102
 105  PN=PN+1.
      IF(A(39).LT.0.)GOTO 106
      IF(Q.LT..125)GOTO 106
      IF(PN.LT.5.)GOTO 38
 106  MM=NQ
        DO 107 I=1,NQ
 107    G(I)=PR(I)
      NZ=0
      GOTO 40
C
C CALCULATE AND COMPARE CFOM
C
 108  Q=9.E9
      R=0.
      V=0.
      T=0.
      M=0
        DO 109 K=1,NQ
        D(K)=FB(K)/FC(K)
        E(K)=FE(K)/TS
        C(K)=FD(K)/TZ
        P=C(K)
        IF(ABS(FC(K)).GT..001)P=P+AMAX1(0.,D(K)-A(42))**2
        I=INT(AMIN1(31.5,1.+50.*P))
        ID(I)=ID(I)+1
        IF(P.GT.Q)GOTO 109
        Q=P
        R=C(K)
        S=D(K)
        W=PR(K)
        V=PM(K)
        T=E(K)
        M=K
 109    B(K)=P
      IF(TN.LT..8)GOTO 110
      IF(A(40).GT.0.)GOTO 113
 110    DO 112 K=1,NQ
        J=1
        IR(1)=IH(20)
        IF(K.EQ.M)IR(1)=IH(21)
        L=K+K-2+NL
        N=ME
 111    N=N+1
        IF(N.GT.NS)GOTO 112
        I=INT(A(N))*NM+L
        J=J+1
        IR(J)=IH(13)
        IF(A(I).LT.0.)IR(J)=IH(12)
        GOTO 111
 112    WRITE(LI,120)PR(K),PM(K),C(K),D(K),E(K),B(K),(IR(I),I=1,J)
      WRITE(LI,120)
 113  M=M+M-2+NL
      K=0
      J=1
      IR(1)=IH(20)
      N=ME
 114  N=N+1
      IF(N.GT.NS)GOTO 121
      I=INT(A(N))*NM+M
      J=J+1
      IR(J)=IH(13)
      IF(A(I).LT.0.)IR(J)=IH(12)
      GOTO 114
C
C PRINT AND SAVE RESULTS
C
 115  FORMAT(//F10.0,' PHASE SETS REFINED - BEST SOLUTION IS CODE',
     +F10.0,'  WITH CFOM =',F8.4)
 116  FORMAT(///' STRUCTURE SOLUTION FOR ',40A1//
     +'   TRY  FILTER RALPHA  NQUAL  M(ABS)  CFOM   SEMINVARIANTS'/)
 117  FORMAT(//' CFOM RANGE   FREQUENCY'/31(/F6.3,' -',F6.3,I7))
 118  FORMAT(//' PHASES FOR BEST SOLUTION'//
     +4('    CODE   ','H   K   L   E   PHI')/)
 119  FORMAT(4(I7,'.',3I4,F6.3,I4))
 120  FORMAT(F9.0,2F6.3,3F7.3,A1,1X,13(1X,5A1))
 121  IF(TN.LT..8)GOTO 122
      IF(A(40).LT.0.)GOTO 122
      WRITE(LI,120)W,V,R,S,T,Q,(IR(I),I=1,J)
 122  IF(PQ.LT.Q)GOTO 125
      I=LX+4
 123  I=I+4
      IF(I.GT.NC)GOTO 124
      M=M+NM
      U=-9.E9
      IF(ABS(A(M))+ABS(A(M+1)).GT..01)U=
     +57.29578*ATAN2(A(M+1)*A(23),A(M))
      IF(U.LT.0.)U=U+360.
      A(I+2)=U
      GOTO 123
 124  PQ=Q
      PS=W
C
C TERMINATE TANGENT REFINEMENT
C
 125  TN=TN+FLOAT(NQ)
      CALL SXTI(T)
      IF(T.GT.TL)GOTO 126
      IF(TN.LT.A(39))GOTO 35
 126    DO 127 I=1,31
 127    F(I)=FLOAT(I-1)*.02
      F(32)=9.999
      WRITE(LI,117)(F(I),F(I+1),ID(I),I=1,31)
      IF(TN.LT.1.1)TN=1.1
      WRITE(LI,115)TN,PS,PQ
      IF(A(40).LT.0.)WRITE(LI,118)
C
C SET UP TANGENT EXPANSION
C
      K=0
      I=NA
 128  M=0
 129  I=I+4
      IF(I.GT.NC)GOTO 131
      K=K+1
      A(I)=ABS(A(I))
      IF(A(I+2).LT.-8.E9)GOTO 129
      NA=NA+4
      P=A(NA)
      A(NA)=A(I)
      A(I)=P
      P=A(NA+1)
      A(NA+1)=A(I+1)
      A(I+1)=P
      P=A(NA+2)
      A(NA+2)=A(I+2)
      A(I+2)=P
      CALL SXH2(A(NA),X,Y,Z)
      M=M+1
      IP(M)=K
      IP(M+4)=INT(X)
      IP(M+8)=INT(Y)
      IP(M+12)=INT(Z)
      IP(M+16)=INT(A(NA+2)+.5)
      F(M)=AMOD(A(NA+1),10.)
      Q=.0174533*A(NA+2)
      P=F(M)*COS(Q)
      Q=F(M)*SIN(Q)
      IF(LW.EQ.1)WRITE(LP,130)IP(M+4),IP(M+8),IP(M+12),P,Q
      IF(A(40).GT.0.)GOTO 128
      IF(M.LT.4)GOTO 129
      WRITE(LI,119)(IP(J),IP(J+4),IP(J+8),IP(J+12),
     +F(J),IP(J+16),J=1,4)
      GOTO 128
 130  FORMAT(3I4,2F8.2)
 131  IF(M.GT.0)WRITE(LI,119)(IP(J),IP(J+4),
     +IP(J+8),IP(J+12),F(J),IP(J+16),J=1,M)
      M=0
      P=0.
      IF(LW.EQ.1)WRITE(LP,130)M,M,M,P,P
      CALL SXTM(SL,LI)
 132  LE=NA
      RETURN
      END
C
C -----------------------------------------------------------
C
      SUBROUTINE SX2F
C
C PARTIAL STRUCTURE EXPANSION
C
      COMMON F(126),G(126),SL,TL,LM,LR,LG,LH,LI,LP,LF,LA,LB,
     +LW,LY,LL,LQ,LE,LV,LX,LD,LZ,LJ,HA,HD,A(50000)
      NA=LE
      NZ=INT(A(54))
      IF(NZ.LT.4)GOTO 33
      IF(NZ.EQ.5)GOTO 33
      IF(NA.LE.LX-4)GOTO 33
      NT=LD+4
      A(NT)=0.
      PQ=10.
      NB=LX+4
      I=NB
   1  I=I+4
      P=AMOD(A(I+1),10.)
      Q=1.74533E-2*A(I+2)
      A(I+2)=P*COS(Q)
      A(I+3)=P*SIN(Q)
      IF(I.LT.NA)GOTO 1
      IF(A(30).GT.8.E9)GOTO 29
C
C EXPANDED LIST
C
   2  IF(NT.GT.LM-6)GOTO 12
      NQ=NT
      I=NB
   3  I=I+4
      IF(I.GT.NA)GOTO 7
      CALL SXH2(A(I),X,Y,Z)
        DO 6 J=65,LY,12
        P=6.283185*(X*A(J+9)+Y*A(J+10)+Z*A(J+11))
        U=COS(P)
        V=SIN(P)
        P=U*A(I+2)+V*A(I+3)
        Q=U*A(I+3)-V*A(I+2)
        W=AINT(1.001*(X*A(J)+Y*A(J+3)+Z*A(J+6)))+
     +  200.*(AINT(1.001*(X*A(J+1)+Y*A(J+4)+Z*A(J+7)))+
     +  200.*AINT(1.001*(X*A(J+2)+Y*A(J+5)+Z*A(J+8))))
        IF(W.LT.0.)Q=-Q
        W=ABS(W)
        K=NQ
   4    K=K+3
        IF(K.GT.NT)GOTO 5
        IF(ABS(W-A(K))-.5)6,4,4
   5    NT=NT+3
        A(NT)=W
        A(NT+1)=P
        A(NT+2)=Q*A(23)
        IF(NT.GT.LM-6)GOTO 7
   6    CONTINUE
      NQ=NT
      GOTO 3
C
C SORT EXPANDED LIST
C
   7  LZ=NT+3
      A(LZ)=9.E9
      M=(LZ-LD)/3
   8  M=M/2
      IF(M.LT.1)GOTO 12
      N=M*3
      K=LZ-N
      J=LD+4
   9  I=J
  10  L=I+N
      IF(A(I).LT.A(L))GOTO 11
      Q=A(L)
      A(L)=A(I)
      A(I)=Q
      Q=A(L+1)
      A(L+1)=A(I+1)
      A(I+1)=Q
      Q=A(L+2)
      A(L+2)=A(I+2)
      A(I+2)=Q
      I=I-N
      IF(I.GT.LD+4)GOTO 10
  11  J=J+3
      IF(J-K)9,9,8
C
C TANGENT EXPANSION
C
  12  NB=NA
      S=0.
      L=NB
  13  L=L+4
      IF(L.GT.LD)GOTO 25
      P=AMOD(A(L+1),10.)
      U=0.
      V=0.
      Z=A(L)-.5
      Q=1.
      I=LD+4
      K=I
      M=NT
  14  J=M
  15  M=K+3*((J-K)/6)
      IF(A(M).GT.Z)GOTO 14
      K=M
      IF(J.GT.K+3)GOTO 15
      Z=Z+.5
  16  I=I+3
  17  IF(I.GT.J)GOTO 19
      X=Z-A(I)-A(J)
      IF(X.GT..5)GOTO 16
      IF(X.GT.-.5)GOTO 18
      J=J-3
      GOTO 17
  18  U=U+A(I+1)*A(J+1)-A(I+2)*A(J+2)*Q
      V=V+A(I+1)*A(J+2)+A(I+2)*A(J+1)*Q
      IF(Q)20,16,16
  19  I=LD+4
      J=M
      Q=-1.
  20  I=I+3
  21  X=Z+A(I)-A(J)
      IF(X.LT.-.5)GOTO 20
      IF(X.LT..5)GOTO 18
      J=J+3
      IF(J.LT.LZ)GOTO 21
C
C ACCEPT NEW PHASES
C
      IF(A(L+1).LT.10.)GOTO 22
      W=.261799*AINT(.1*A(L+1))
      X=COS(W)
      W=SIN(W)
      V=X*U+W*V
      U=X*V
      V=W*V
  22  W=SQRT(U*U+V*V)
      X=P*W*A(45)
      IF(S.LT.X)S=X
      IF(X.LT.PQ)GOTO 13
      NA=NA+4
      Q=A(L)
      T=A(L+1)
      J=L
  23  J=J-4
      IF(J.LT.NA)GOTO 24
      A(J+4)=A(J)
      A(J+5)=A(J+1)
      GOTO 23
  24  A(NA)=Q
      A(NA+1)=T
      A(NA+2)=P*U/W
      A(NA+3)=P*V*A(23)/W
      IF(NA-NB.LT.400)GOTO 13
  25  PQ=PQ*.8
      IF(S.LT..1)GOTO 28
      IF(NA.EQ.LD)GOTO 28
      IF(NA.GT.NB)GOTO 2
      PQ=.8*S
      GOTO 12
C
C RESULTS TO FILE
C
  26  FORMAT(//' TANGENT EXPANDED TO',I5,'  OUT OF',I5,
     +'  E GREATER THAN',F7.3/' HIGHEST MEMORY USED =',I6)
  27  FORMAT(' TPR SEARCH RESTRICTED BY MEMORY')
  28  I=(NA-LX-4)/4
      J=(LD-LX-4)/4
      P=ABS(A(26))
      WRITE(LI,26)I,J,P,LZ
      IF(LZ.GT.LM-4)WRITE(LI,27)
      CALL SXTM(SL,LI)
  29  I=LX+4
  30  N=-2
  31  I=I+4
      IF(I.GT.NA)GOTO 32
      N=N+3
      F(N)=A(I)
      F(N+1)=A(I+2)
      F(N+2)=A(I+3)
      IF(N.LT.124)GOTO 31
      WRITE(LA)F
      GOTO 30
  32  N=N+3
      F(N)=0.
      WRITE(LA)F
      REWIND LA
      LE=-1
  33  IF(NZ.EQ.0)GOTO 94
      IF(ABS(A(55)).GT..5)GOTO 57
C
C FIND ASYMMETRIC UNIT OF FOURIER
C
      WP=1.
      IF(NZ.LT.4)WP=0.
      XJ=A(2)
      YJ=A(3)
      ZJ=A(4)
      IX=0
      IY=0
      IZ=0
      KP=1
      IF(A(23).LT..5)GOTO 34
      IF(NZ.LT.4)KP=2
  34  ML=LY+12
        DO 35 I=1,3
        J=I+30
          DO 35 K=I,J,6
          F(K)=9.E9
  35      F(K+3)=1.
        DO 46 L=ML,LL,4
          DO 45 N=65,LY,12
          W=A(L)
            DO 44 K=1,KP
            X=AMOD(A(N+9)*W*WP+A(L+1)+.501,1.)-.001
            Y=AMOD(A(N+10)*W*WP+A(L+2)+.501,1.)-.001
            Z=AMOD(A(N+11)*W*WP+A(L+3)+.501,1.)-.001
            IX=9
            IF(AMAX1(ABS(A(N+1)),ABS(A(N+2))).GT..01)GOTO 36
            IX=1
            IF(ABS(X).GT..01)GOTO 36
            IF(A(N)*W.LT..5)GOTO 36
            IX=0
  36        IY=9
            IF(AMAX1(ABS(A(N+3)),ABS(A(N+5))).GT..01)GOTO 37
            IY=1
            IF(ABS(Y).GT..01)GOTO 37
            IF(A(N+4)*W.LT..5)GOTO 37
            IY=0
  37        IZ=9
            IF(AMAX1(ABS(A(N+6)),ABS(A(N+7))).GT..01)GOTO 40
            IZ=1
            IF(ABS(Z).GT..01)GOTO 38
            IF(A(N+8)*W.LT..5)GOTO 38
            IZ=0
  38        IF(A(N+8)*W.LT.0.)GOTO 39
            IF(IZ.EQ.0)GOTO 40
            IF(IX+IY.LT.1)F(6)=AMIN1(F(6),Z)
            IF(IX.LT.1)F(12)=AMIN1(F(12),Z)
            IF(IY.LT.1)F(18)=AMIN1(F(18),Z)
            IF(F(30).GT.Z)F(30)=Z
            GOTO 40
  39        IF(IX+IY.LT.1)F(3)=AMIN1(F(3),Z)
            IF(IX.LT.1)F(9)=AMIN1(F(9),Z)
            IF(IY.LT.1)F(15)=AMIN1(F(15),Z)
            IF(F(27).GT.Z)F(27)=Z
  40        IF(IY.GT.1)GOTO 42
            IF(A(N+4)*W.LT.0.)GOTO 41
            IF(IY.EQ.0)GOTO 42
            IF(IX+IZ.LT.1)F(11)=AMIN1(F(11),Y)
            IF(IX.LT.1)F(5)=AMIN1(F(5),Y)
            IF(IZ.LT.1)F(35)=AMIN1(F(35),Y)
            IF(F(17).GT.Y)F(17)=Y
            GOTO 42
  41        IF(IX+IZ.LT.1)F(8)=AMIN1(F(8),Y)
            IF(IX.LT.1)F(2)=AMIN1(F(2),Y)
            IF(IZ.LT.1)F(32)=AMIN1(F(32),Y)
            IF(F(14).GT.Y)F(14)=Y
  42        IF(IX.GT.1)GOTO 44
            IF(A(N)*W.LT.0.)GOTO 43
            IF(IX.EQ.0)GOTO 44
            IF(IY+IZ.LT.1)F(16)=AMIN1(F(16),X)
            IF(IY.LT.1)F(22)=AMIN1(F(22),X)
            IF(IZ.LT.1)F(28)=AMIN1(F(28),X)
            IF(F(4).GT.X)F(4)=X
            GOTO 44
  43        IF(IY+IZ.LT.1)F(13)=AMIN1(F(13),X)
            IF(IY.LT.1)F(19)=AMIN1(F(19),X)
            IF(IZ.LT.1)F(25)=AMIN1(F(25),X)
            IF(F(1).GT.X)F(1)=X
  44        W=-W
  45      CONTINUE
  46    CONTINUE
        DO 47 I=1,27,13
        F(I+6)=F(I)
  47    F(I+9)=F(I+3)
        DO 48 I=3,13,5
        F(I+18)=F(I)
  48    F(I+21)=F(I+3)
        DO 49 I=1,31,6
        J=I+2
          DO 49 K=I,J
          F(K)=.5*F(K)
          IF(F(K).LT.1.)F(K+3)=.5*F(K+3)
          IF(F(K).GT.1.)F(K)=0.
  49      CONTINUE
C
C SET UP FMAP AND GRID
C
      RE=19.6*A(1)/SQRT(A(64))
      U=9.E9
        DO 55 M=1,31,6
          DO 54 N=1,3
          L=N
  50      K=1
          IF(.501.GT.F(M+5))GOTO 51
          K=2
  51      J=1
          IF(.501.GT.F(M+4))GOTO 52
          J=2
  52      V=FLOAT(J*K)*F(M+3)
          IF(V.GT.U)GOTO 53
          IF(U.GT.V+.01)W=9.E9
          U=V+.001
          X=YJ*FLOAT(J)-RE
          Y=ZJ*FLOAT(K)-RE
          Z=X*X+Y*Y
          IF(Z.GT.W)GOTO 53
          W=Z
          A(55)=FLOAT(L)
          IX=INT(100.01*F(M+1))-J
          IY=INT(100.01*F(M+2))-K
          IZ=J
          KP=K
          A(56)=INT(3.5+100.*F(M+3)*XJ/RE)
          A(36)=F(M+3)*100./(A(56)-3.)
          A(33)=100.*F(M)-A(36)
  53      X=YJ
          YJ=ZJ
          ZJ=X
          X=F(M+1)
          F(M+1)=F(M+2)
          F(M+2)=X
          X=F(M+4)
          F(M+4)=F(M+5)
          F(M+5)=X
          L=-L
          IF(L.LT.0)GOTO 50
          X=XJ
          XJ=YJ
          YJ=ZJ
          ZJ=X
          X=F(M)
          F(M)=F(M+1)
          F(M+1)=F(M+2)
          F(M+2)=X
          X=F(M+3)
          F(M+3)=F(M+4)
          F(M+4)=F(M+5)
  54      F(M+5)=X
  55    CONTINUE
      J=INT(A(55))
      K=INT(A(56))
      L=NZ
      IF(NZ.EQ.2)L=6
      WRITE(LI,56)L,J,K,A(33),IX,IY,A(36),IZ,KP
  56  FORMAT(/' FMAP AND GRID SET BY PROGRAM'//' FMAP',3I4/
     +' GRID',2(F10.3,2I4))
      A(34)=IX
      A(35)=IY
      A(37)=IZ
      A(38)=KP
  57  IF(NZ.LT.7)GOTO 94
      IF(LE.LT.0)GOTO 94
      A(54)=A(54)-1.
C
C SET UP PEAKLIST OPTIMISATION
C
      LZ=LD+8
      MB=LZ+1251
      NB=MB-9
      RC=0.
      HM=0.
      S=27.*(A(57)-5.)
      R=S
      T=0.
      I=LV
  58  I=I+8
      IF(I.GT.LD)GOTO 59
      J=INT(A(I+1)*.001)*5+LJ
      A(I+6)=A(J)
      IF(I.GT.LX)GOTO 58
      IF(HM.LT.A(J))HM=A(J)
      R=R+A(I+5)*A(J)*A(J)
      GOTO 58
  59  READ(LF)F
        DO 64 I=1,124,3
        IF(F(I).LT..5)GOTO 65
        IF(F(I+1).LT.0.)GOTO 64
        P=AMOD(F(I+1),10.)
        IF(ABS(P**2-.8).LT.T)GOTO 64
  60    IF(NB+16.LT.LM)GOTO 63
        T=T+.05
        J=MB
  61    J=J+9
  62    IF(J.GT.NB)GOTO 60
        IF(ABS(A(NB+5)**2-.8).GT.T)GOTO 61
        A(J+4)=A(NB+4)
        A(J+5)=A(NB+5)
        A(J+6)=A(NB+6)
        NB=NB-9
        GOTO 62
  63    NB=NB+9
        A(NB+6)=F(I)
        A(NB+5)=P
        A(NB+4)=F(I+2)
  64    CONTINUE
      GOTO 59
  65  REWIND LF
        DO 68 I=MB,NB,9
        CALL SXH2(A(I+6),X,Y,Z)
        A(I)=X
        A(I+1)=Y
        A(I+2)=Z
        Q=SQRT(X*X*A(14)+Y*Y*A(15)+Z*Z*A(16)+Y*Z*A(17)+
     +  X*Z*A(18)+X*Y*A(19))/A(1)
        A(I+7)=47.*Q*SQRT(Q)
        T=S*(3.834/(3.834+A(I+7)))**2
        J=LV
  66    J=J+8
        IF(J.GT.LX)GOTO 67
        K=INT(A(J+1)*.001)*5+LJ
        W=SQRT(A(K)*SQRT(A(K)))
        T=T+A(J+5)*(W*A(K)/(W+A(I+7)))**2
        GOTO 66
  67    P=A(I+5)
        A(I+8)=SQRT(R/T)
        P=P/A(I+8)
        A(I+3)=P
        RC=RC+P*P
        A(I+5)=0.
  68    A(I+6)=0.
      J=LZ
        DO 69 I=1,1251
        A(J)=SIN(6.283185E-3*FLOAT(I-1))
  69    J=J+1
      IX=(LD-LX)/16
      IZ=LV+1
      NA=LD+8
      KP=(LD-LV)/8
      SM=0.
C
C SCAN PEAKS
C
      IY=0
  70  I=NA
  71  I=I-8
      IF(I.LT.IZ)GOTO 85
      IF(A(I+5).EQ.0.)GOTO 71
      NK=-2
        DO 72 J=65,LY,12
        NK=NK+3
        G(NK)=1000.*(A(I+2)*A(J)+A(I+3)*A(J+1)+
     +  A(I+4)*A(J+2)+A(J+9))
        G(NK+1)=1000.*(A(I+2)*A(J+3)+A(I+3)*A(J+4)+
     +  A(I+4)*A(J+5)+A(J+10))
  72    G(NK+2)=1000.*(A(I+2)*A(J+6)+A(I+3)*A(J+7)+
     +  A(I+4)*A(J+8)+A(J+11))
  73  RA=0.
      RB=0.
C
C TRICLINIC INNER LOOP
C
      X=G(1)
      Y=G(2)
      Z=G(3)
      W=SQRT(A(I+6)*SQRT(A(I+6)))
      T=W*A(I+6)*A(I+5)*A(24)
      IF(LY.NE.65)GOTO 75
        DO 74 J=MB,NB,9
        KZ=LZ+INT(AMOD(1000000.5+A(J)*X+A(J+1)*Y+A(J+2)*Z,1000.))
        O=T/(W+A(J+7))
        U=A(J+5)-A(KZ+250)*O
        V=A(J+6)-A(KZ)*O*A(23)
        S=(V*V+U*U)/A(J+4)
        RA=RA+S
        IF(IX.LT.IY)GOTO 74
        A(J+6)=V
        A(J+5)=U
  74    RB=RB+A(J+3)*SQRT(S)
      GOTO 82
C
C MONOCLINIC INNER LOOP
C
  75  XJ=G(4)
      YJ=G(5)
      ZJ=G(6)
      IF(LY.NE.77)GOTO 77
        DO 76 J=MB,NB,9
        KZ=LZ+INT(AMOD(1000000.5+A(J)*X+A(J+1)*Y+A(J+2)*Z,1000.))
        KY=LZ+INT(AMOD(1000000.5+A(J)*XJ+A(J+1)*YJ+A(J+2)*ZJ,1000.))
        O=T/(W+A(J+7))
        U=A(J+5)-O*(A(KZ+250)+A(KY+250))
        V=A(J+6)-A(23)*O*(A(KZ)+A(KY))
        S=(V*V+U*U)/A(J+4)
        RA=RA+S
        IF(IX.LT.IY)GOTO 76
        A(J+6)=V
        A(J+5)=U
  76    RB=RB+SQRT(S)*A(J+3)
      GOTO 82
C
C ORTHORHOMBIC INNER LOOP
C
  77  IF(LY.NE.101)GOTO 79
      XK=G(7)
      YK=G(8)
      ZK=G(9)
      XL=G(10)
      YL=G(11)
      ZL=G(12)
        DO 78 J=MB,NB,9
        KZ=LZ+INT(AMOD(1000000.5+A(J)*X+A(J+1)*Y+A(J+2)*Z,1000.))
        KY=LZ+INT(AMOD(1000000.5+A(J)*XJ+A(J+1)*YJ+A(J+2)*ZJ,1000.))
        KX=LZ+INT(AMOD(1000000.5+A(J)*XK+A(J+1)*YK+A(J+2)*ZK,1000.))
        KW=LZ+INT(AMOD(1000000.5+A(J)*XL+A(J+1)*YL+A(J+2)*ZL,1000.))
        O=T/(W+A(J+7))
        U=A(J+5)-O*(A(KZ+250)+A(KY+250)+A(KX+250)+A(KW+250))
        V=A(J+6)-A(23)*O*(A(KZ)+A(KY)+A(KX)+A(KW))
        S=(V*V+U*U)/A(J+4)
        RA=RA+S
        IF(IX.LT.IY)GOTO 78
        A(J+6)=V
        A(J+5)=U
  78    RB=RB+SQRT(S)*A(J+3)
      GOTO 82
C
C INNER LOOP FOR OTHER CRYSTAL SYSTEMS
C
  79    DO 81 J=MB,NB,9
        X=0.
        Y=0.
          DO 80 K=1,NK,3
          KZ=LZ+INT(AMOD(1000000.5+A(J)*G(K)+A(J+1)*G(K+1)+
     +    A(J+2)*G(K+2),1000.))
          X=X+A(KZ+250)
  80      Y=Y+A(KZ)
        O=T/(W+A(J+7))
        V=A(J+6)-Y*A(23)*O
        U=A(J+5)-X*O
        S=(U*U+V*V)/A(J+4)
        RA=RA+S
        IF(IX.LT.IY)GOTO 81
        A(J+6)=V
        A(J+5)=U
  81    RB=RB+SQRT(S)*A(J+3)
  82  W=RB*RB/(RA*RC)
      IF(IY)84,71,83
  83  IF(W.LT.SM)GOTO 71
      IY=-IY
      GOTO 73
C
C ELIMINATE PEAK
C
  84  A(I+5)=0.
      KP=KP-1
      IX=IX-1
      IY=-IY
      SM=W
  85  IF(IY.EQ.IX+1)GOTO 88
      IF(IY.NE.0)GOTO 87
      SM=W
      IZ=LX+1
        DO 86 J=MB,NB,9
        A(J+5)=-A(J+5)
  86    A(J+6)=-A(J+6)
  87  IF(IX.LT.1)GOTO 88
      IF(I.GT.IZ-1)GOTO 71
      IY=IX+1
      GOTO 70
C
C WRITE WEIGHTED FOURIER FILE, PRINT R-INDEX
C
  88  I=1
      G(1)=-.5
      G(2)=.8
      G(3)=-.02
      G(4)=-.5
      G(5)=1.9
      G(6)=-.02
      G(7)=1.
      G(8)=2.4
      G(9)=-.01
      X=0.
      Y=0.
        DO 89 J=MB,NB,9
        X=X+A(J+5)*A(J+5)+A(J+6)*A(J+6)
  89    Y=Y+A(J+3)*A(J+3)
      X=SQRT(Y/X)
        DO 92 J=MB,NB,9
        F(I)=A(J)+200.*(A(J+1)+200.*A(J+2))
        W=1.
        IF(HM.GT.18.5)GOTO 91
        U=A(14)*A(J)*A(J)+A(15)*A(J+1)*A(J+1)+A(16)*A(J+2)*A(J+2)+
     +  A(17)*A(J+1)*A(J+2)+A(18)*A(J)*A(J+2)+A(19)*A(J)*A(J+1)
        U=U*78.9568/(A(1)*A(1))
        S=SQRT(U+U)
          DO 90 K=1,7,3
          V=G(K+1)*S
  90      W=W+G(K)*SIN(V)*EXP(U*G(K+2))/V
        W=SQRT(W)
  91    U=X*A(J+5)
        V=X*A(J+6)
        S=SQRT(U*U+V*V)
        O=SQRT(1./A(J+4))
        T=100.*W*(2.*A(J+3)*TANH(S*O*A(J+3)*A(J+8)**2)/S-O)
        IF(A(54).GT.6.5)T=T*A(J+8)
        F(I+1)=U*T
        F(I+2)=V*T
        KI=INT(A(J))
        KJ=INT(A(J+1))
        KK=INT(A(J+2))
        IF(NZ-LW.EQ.5)WRITE(LP,96)KI,KJ,KK,F(I+1),F(I+2)
        IF(I.LT.124)GOTO 92
        WRITE(LA)F
        I=-2
  92    I=I+3
      F(I)=0.
      WRITE(LA)F
      REWIND LA
  93  FORMAT(//' PEAK LIST OPTIMISATION'
     +/' RE =',F6.3,' FOR',I4,' SURVIVING ATOMS AND',I5,
     +' E-VALUES'/'  HIGHEST MEMORY USED =',I6)
      S=SQRT(1.-SM)
      J=(NB-MB+9)/9
      I=NB+8
      WRITE(LI,93)S,KP,J,I
      M=0
      P=0.
      IF(NZ-LW.EQ.5)WRITE(LP,96)M,M,M,P,P
      CALL SXTM(SL,LI)
  94  LE=0
      I=LV
  95  I=I+8
      IF(I.GT.LX)GOTO 97
      J=INT(.001*A(I+1))*5+LJ
      A(I+7)=(.1+A(J+1))**2
      GOTO 95
  96  FORMAT(3I4,2F8.2)
  97  RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SX2G
C
C PATTERSONS, E-MAPS AND PEAKSEARCH
C
C ** NEXT STATEMENT MUST HAVE C IN COLUMN 1 IF NOT FORTRAN-77 **
      CHARACTER*1 IH,IT,IR,IC
C
      COMMON F(126),G(126),SL,TL,LM,LR,LG,LH,LI,LP,LF,LA,LB,
     +LW,LY,LL,LQ,LE,LV,LX,LD,LZ,LJ,HA,HD,A(50000)
      COMMON/WORD/IH(50),IT(40),IR(80)
      DIMENSION IP(53),IC(4)
      IF(A(57).LT.0.)LX=LV
      RR=.1
      IF(AMAX1(A(5),A(6),A(7)).GT.110.)RR=.3
      NZ=INT(A(54))
      IF(NZ.EQ.6)NZ=4
      IF(NZ.GT.6)NZ=6
      IF(NZ.LT.4)GOTO 1
      IF(NZ.EQ.5)GOTO 1
      LX=MIN0(LX,LV+8*INT(ABS(A(31))))
      RR=.5
   1  LD=LX
      IF(NZ.EQ.0)GOTO 89
      IF(ABS(A(57)).LT..5)GOTO 89
      TP=A(LX+6)
      A(LX+6)=9.E9
      MP=(INT(ABS(A(57)))*8)+LX
      A(MP+6)=AMAX1(0.,.7*A(20))
      ML=LY+12
      NX=INT(A(56))
      LZ=MP+16
      IF(LZ+8600.GT.LM)GOTO 26
      NL=0
      NH=LZ+8134
        DO 2 I=LZ,NH
   2    A(I)=0.
      MA=MAX0(MIN0(INT(ABS(A(55))),3),1)
      NS=LZ+8427
      NA=NH-3
      SS=0.
      ZZ=A(33)
      KP=1
      NF=1
      NG=1
      WP=1.
      IF(NZ.GT.3)GOTO 6
      WP=0.
      IF(A(23).LT..5)GOTO 3
      KP=2
C
C DUMP PATTERSON PARAMETERS
C
   3  IF(A(20).GT.-8.E9)GOTO 6
      I=INT(A(55))
      J=INT(A(56))-2
      M=INT(A(37))
      N=INT(A(38))
      K=M+INT(A(34))
      L=N+INT(A(35))
      P=A(33)+A(36)
      WRITE(LP,4)I,J,P,K,L,A(36),M,N
   4  FORMAT('PATT',2I4,'      GRID',2(F8.3,2I3))
   5  FORMAT(I3,I2,25I3/26I3)
C
C READ REFLECTIONS
C
   6  READ(LA)F
      I=-2
   7  I=I+3
      IF(I.GT.124)GOTO 6
      IF(ABS(F(I)).LT..5)GOTO 23
      S=F(I+1)*F(I+1)+F(I+2)*F(I+2)
      IF(S.LT.1.E-10)GOTO 7
      U=SQRT(S)
      CALL SXH2(F(I),X,Y,Z)
      EZ=0.
      L=NA
C
C LOCATING SCHEME
C
        DO 13 K=65,LY,12
        P=AINT(1.001*(X*A(K)+Y*A(K+3)+Z*A(K+6)))
        Q=AINT(1.001*(X*A(K+1)+Y*A(K+4)+Z*A(K+7)))
        R=AINT(1.001*(X*A(K+2)+Y*A(K+5)+Z*A(K+8)))
        W=P+200.*(Q+200.*R)
        A(L+7)=ABS(W)
        IF(ABS(W-F(I)).LT..5)EZ=EZ+1.
        IF(ABS(W+F(I)).LT..5-A(23))EZ=EZ+1.
        J=NA
   8    J=J+4
        IF(J.GT.L)GOTO 9
        IF(.5-ABS(A(J+3)-A(L+7)))8,8,13
   9      DO 10 J=1,MA
          A(L+4)=P
          P=Q
          Q=R
  10      R=A(L+4)
        IF(A(55).LT.0.)GOTO 11
        T=P
        P=Q
        Q=T
  11    IF(ABS(Q).GT.63.5)GOTO 13
        T=127.*P+Q
        IF(ABS(T).GT.8134.5)GOTO 13
        M=INT(ABS(T)+.001)+LZ
        L=L+4
        IF(NF.GT.1)GOTO 12
        A(M)=1.1
        GOTO 13
  12    A(L+2)=100.*(X*A(K+9)+Y*A(K+10)+Z*A(K+11))
        A(L+1)=T
  13    CONTINUE
      IF(NF.EQ.1)GOTO 7
C
C FOURIER TYPE
C
      R=0.
      P=F(I+1)
      K=NA
      T=SQRT(EZ)
      GOTO(15,14,18,19,19,19),NZ
  14  P=T*F(I+2)
  15  P=ABS(P*F(I+1))
      IF(A(20).GT.-8.E9)GOTO 16
      IF(A(21))17,18,18
  16  IF(ABS(A(21)).LT..5)GOTO 18
  17  P=SQRT(P)*T*F(I+2)
  18  Q=0.
      GOTO 20
  19  Q=F(I+2)
      IF(NZ.EQ.5)GOTO 20
      Q=Q*T
      P=P*T
  20  K=K+4
      IF(K.GT.L)GOTO 7
      Y=SIGN(1.,A(K+1)+.1)
      X=Y*A(K)
      Z=Q*Y
      SS=SS+SQRT(P*P+Q*Q)
      T=ABS(A(K+1))+.001
      M=INT(T)+LZ
      N=INT(A(M))
      IF(NZ.GT.3)R=Y*A(K+2)
      IF(NF.NE.3)GOTO 21
      G(NG)=FLOAT(N)
      G(NG+1)=T
      G(NG+2)=X
      G(NG+3)=R
      G(NG+4)=P
      G(NG+5)=Z
      NG=NG+6
      IF(NG.LT.126)GOTO 21
      WRITE(LG)G
      NG=1
  21  N=N*NH+NS
      A(N)=T
C
C HX SUMS
C
      S=.0628319*(X*ZZ+R)
      O=SIN(S)
      S=COS(S)
      T=P*S+Z*O
      S=P*O-Z*S
      Z=.0628319*X*A(36)
      W=COS(Z)
      Z=SIN(Z)
      M=N+1
C
C ** CRITICAL LOOP **
C
        DO 22 N=3,NH,2
        A(M)=A(M)+T
        A(M+1)=A(M+1)+S
        O=T*W-S*Z
        S=T*Z+S*W
        T=O
  22    M=M+2
      GOTO 20
C
C ALLOCATE MEMORY
C
  23  REWIND LA
      IF(NF.NE.1)GOTO 32
      NF=2
      M=0
        DO 24 I=LZ,NH
  24    M=M+INT(A(I))
      IF(M.GT.0)GOTO 25
      WRITE(LI,31)
      GOTO 88
  25  NH=MIN0(NX,((LM-NS)/M-1)/2)
      IF(NH.LT.NX)NF=3
      NH=NH+NH+1
      IF(NH.GT.2)GOTO 28
  26  WRITE(LI,27)
      GOTO 88
  27  FORMAT(/' ** INSUFFICIENT MEMORY FOR FOURIER')
  28  NL=NS+(NH*M)
        DO 29 I=NS,NL
  29    A(I)=0.
      K=0
      L=LZ+8134
        DO 30 I=LZ,L
        IF(A(I).LT..5)GOTO 30
        A(I)=FLOAT(K)
        K=K+1
  30    CONTINUE
      GOTO 6
  31  FORMAT(/' ** NO DATA FOR FOURIER'///)
C
C INITIATE SCANS
C
  32  NC=-1
      NB=-1
      NA=LZ
      G(NG)=-1.
      NG=0
      EZ=9999.
      A(44)=0.
      YM=AMOD(A(34)+1000.1,100.)-.1
      ZM=AMOD(A(35)+1000.1,100.)-.1
      A(NL)=1000000.
      SS=999.1/SS
      IF(NF.EQ.2)GOTO 33
      WRITE(LG)G
      REWIND LG
  33    DO 34 I=1,126
  34    G(I)=SIN(6.283185E-2*FLOAT(I-1))
      GOTO 41
C
C READ BACK DATA (DISK MODE)
C
  35  N=NL-1
        DO 36 I=NS,N
  36    A(I)=0.
      NH=MIN0(NH,1+2*(NX-NG))
  37  READ(LG)F
      I=-5
  38  I=I+6
      IF(I.GT.121)GOTO 37
      N=INT(F(I))
      IF(N.LT.0)GOTO 40
      N=NH*N+NS
      A(N)=F(I+1)
      S=.0628319*(F(I+2)*ZZ+F(I+3))
      O=SIN(S)
      S=COS(S)
      T=F(I+4)*S+F(I+5)*O
      S=F(I+4)*O-F(I+5)*S
      Z=.0628319*F(I+2)*A(36)
      W=COS(Z)
      Z=SIN(Z)
      M=N+1
C
C ** CRITICAL LOOP **
C
        DO 39 N=3,NH,2
        A(M)=A(M)+T
        A(M+1)=A(M+1)+S
        O=T*W-S*Z
        S=T*Z+S*W
        T=O
  39    M=M+2
      GOTO 38
C
C FOURIER SUMMATIONS
C
  40  REWIND LG
  41  MS=NS+1
  42  Z=0.
      W=63.5
      NK=NA+2808
        DO 43 J=NA,NK
  43    A(J)=0.
        DO 44 J=1,106
  44    F(J)=0.
      NK=MS
        DO 50 I=NS,NL,NH
        IF(W.GT.A(I))GOTO 48
        K=INT(AMOD(Z*A(38),100.))
        L=INT(AMOD(Z*ZM,100.))
        N=NA
          DO 46 J=1,53
          L=MOD(L,100)
          W=G(L+1)
          Z=G(L+26)
C
C ** CRITICAL LOOP **
C
            DO 45 M=1,53
            A(N)=A(N)+F(M)*Z+F(M+53)*W
  45        N=N+1
  46      L=L+K
        IF(I.EQ.NL)GOTO 50
        Z=AINT(A(I)/127.+.5)+.0001
        W=127.*Z+63.5
          DO 47 J=1,53
          F(J)=0.
  47      F(J+53)=0.
  48    U=AMOD(A(I)+63.,127.)+37.
        K=INT(AMOD(U*A(37),100.))
        L=INT(AMOD(U*YM,100.))
        U=A(NK)*SS
        V=A(NK+1)*SS
C
C ** CRITICAL LOOP **
C
          DO 49 J=1,53
          L=MOD(L,100)
          F(J)=F(J)+U*G(L+26)-V*G(L+1)
          F(J+53)=F(J+53)-V*G(L+26)-U*G(L+1)
  49      L=L+K
  50    NK=NK+NH
C
C DUMP ENCODED PATTERSON
C
      IF(NC.LT.0)GOTO 65
      IF(NZ.GT.3)GOTO 53
      IF(A(20).GT.-8.E9)GOTO 53
      IP(1)=NG-1
        DO 52 I=53,2703,53
          DO 51 K=1,51
          N=I+K+NB
  51      IP(K+2)=MAX0(-99,MIN0(999,INT(A(N))))
        IP(2)=I/53
  52    WRITE(LP,5)IP
C
C LOCATE MAXIMA
C
  53  Z=A(35)
        DO 64 I=53,2703,53
        Z=Z+A(38)
        Y=A(34)
          DO 64 K=1,51
          Y=Y+A(37)
          NK=I+K
          M=NK+NB
          IF(EZ.GT.A(M))EZ=A(M)
          P=A(M)
          IF(P*1.2.LT.A(MP+6))GOTO 64
          IF(A(M-1).GT.P)GOTO 64
          IF(A(M+1).GT.P)GOTO 64
          IF(AMAX1(A(M-54),A(M-53),A(M-52)).GT.P)GOTO 64
          IF(AMAX1(A(M+52),A(M+53),A(M+54)).GT.P)GOTO 64
          L=NK+NC
          IF(AMAX1(A(L-53),A(L-1),A(L),A(L+1),A(L+53)).GT.P)GOTO 64
          N=NK+NA
          IF(AMAX1(A(N-53),A(N-1),A(N),A(N+1),A(N+53)).GT.P)GOTO 64
          Q=P+P
          U=A(L)-A(N)
          V=A(M-1)-A(M+1)
          W=A(M-53)-A(M+53)
          R=U/(A(N)+A(L)-Q)
          S=V/(A(M-1)+A(M+1)-Q)
          T=W/(A(M-53)+A(M+53)-Q)
          H=P-(U*R+V*S+W*T)*.0416667
          IF(H.GT.A(44))A(44)=H
          IF(H.LT.A(20))GOTO 64
C
C ELIMINATE EQUIVALENTS, FIND S.O.F.
C
          W=.01*ZZ+A(36)*(.005*R-.01)
          V=.005*(Y+Y+A(37)*S)
          U=.005*(Z+Z+A(38)*T)
          IF(A(55).LT.0.)GOTO 54
          T=U
          U=V
          V=T
  54        DO 55 NK=1,MA
            T=W
            W=V
            V=U
  55        U=T
          SK=0.
          XS=0.
          YS=0.
          ZS=0.
          CS=1.
            DO 59 NK=1,KP
              DO 58 L=65,LY,12
              XA=U*A(L)+V*A(L+1)+W*A(L+2)+WP*A(L+9)
              YA=U*A(L+3)+V*A(L+4)+W*A(L+5)+WP*A(L+10)
              ZA=U*A(L+6)+V*A(L+7)+W*A(L+8)+WP*A(L+11)
                DO 57 M=ML,LL,4
                O=CS*A(M)*XA+A(M+1)
                P=CS*A(M)*YA+A(M+2)
                Q=CS*A(M)*ZA+A(M+3)
                N=LV
                IF(NZ.LT.4)N=LX
                R=AMOD(O-U,1.)-.5
                S=AMOD(P-V,1.)-.5
                T=AMOD(Q-W,1.)-.5
                IF(R*R*A(8)+S*S*A(9)+T*T*A(10)+S*T*A(11)+
     +          R*T*A(12)+R*S*A(13).GT.RR)GOTO 56
                XS=XS+R+U
                YS=YS+S+V
                ZS=ZS+T+W
                SK=SK+1.
  56            N=N+8
                IF(N.GT.LD)GOTO 57
                R=AMOD(O-A(N+2),1.)-.5
                S=AMOD(P-A(N+3),1.)-.5
                T=AMOD(Q-A(N+4),1.)-.5
                IF(R*R*A(8)+S*S*A(9)+T*T*A(10)+S*T*A(11)+
     +          R*T*A(12)+R*S*A(13)-A(N+7))64,56,56
  57            CONTINUE
  58          CONTINUE
  59        CS=-1.
C
C SORT PEAKS
C
          J=LD+8
          NK=J
  60      J=J-8
          IF(A(J+6)-H)60,63,63
  61      N=NK+7
            DO 62 L=NK,N
  62        A(L+8)=A(L)
  63      NK=NK-8
          IF(J.LT.NK)GOTO 61
          A(J+8)=0.
          A(J+9)=1000.1
          SK=1./SK
          A(J+10)=XS*SK
          A(J+11)=YS*SK
          A(J+12)=ZS*SK
          A(J+13)=SK
          A(J+14)=H
          A(J+15)=RR
          LD=MIN0(LD+8,MP)
  64      CONTINUE
C
C RECYCLE
C
  65  NC=NB
      NB=NA
      NA=NA+2809
      IF(NA.GT.LZ+8426)NA=LZ
      MS=MS+2
      NG=NG+1
      ZZ=ZZ+A(36)
      IF(NX.EQ.NG)GOTO 71
      IF(MS-NH-NS)42,35,35
  66  FORMAT(//' SHARPENED PATTERSON FOR ',40A1)
  67  FORMAT(//' SUPER-SHARP PATTERSON FOR ',40A1)
  68  FORMAT(//' PATTERSON FOR ',40A1)
  69  FORMAT(//' FOURIER FOR ',40A1)
  70  FORMAT(' MAXIMUM =',F8.2,',  MINIMUM =',F8.2,
     +6X,' HIGHEST MEMORY USED =',I6,A1,5X,'DISC MODE')
  71  A(LX+6)=TP
      IF(NZ.GT.3)GOTO 76
      IF(NZ.NE.2)GOTO 75
      IF(A(20).GT.-8.E9)GOTO 73
      IF(A(21).GE.0.)GOTO 74
  72  WRITE(LI,67)IT
      GOTO 77
  73  IF(ABS(A(21)).GT..5)GOTO 72
  74  WRITE(LI,66)IT
      GOTO 77
  75  WRITE(LI,68)IT
      GOTO 77
  76  WRITE(LI,69)IT
  77  IF(NF.EQ.2)WRITE(LI,70)A(44),EZ,NL
      IF(NF.EQ.3)WRITE(LI,70)A(44),EZ,NL,IH(20)
      CALL SXTM(SL,LI)
      IF(NZ.EQ.6)GOTO 89
C
C CHARACTER CONVERSION
C
      I=LX
      X=48469.
        DO 78 J=1,9
        X=X+125000.
        I=I+8
  78    A(I)=X
      X=125969.
        DO 79 J=10,99
        I=I+8
        A(I)=X
        IF(MOD(J,10).EQ.9)X=X+100000.
  79    X=X+2500.
      X=125019.
        DO 80 J=100,999
        I=I+8
        IF(I.GT.LD)GOTO 84
        A(I)=X
        IF(MOD(J,100).EQ.99)X=X+100000.
        IF(MOD(J,10).EQ.9)X=X+2000.
  80    X=X+50.
      X=125000.
        DO 81 J=1000,9999
        I=I+8
        IF(I.GT.LD)GOTO 84
        A(I)=X
        IF(MOD(J,1000).EQ.999)X=X+100000.
        IF(MOD(J,100).EQ.99)X=X+2000.
        IF(MOD(J,10).EQ.9)X=X+40.
  81    X=X+1.
      LD=I
  82  FORMAT(/10X,'X',7X,'Y',7X,'Z',4X,'WEIGHT PEAK VECTOR'/)
  83  FORMAT(1X,4A1,3F8.4,F6.0,F7.0,F7.2)
  84  IF(NZ.GT.3)GOTO 89
C
C PATTERSON OUTPUT
C
      WRITE(LI,82)
      I=LX
  85  I=I+8
      IF(I.GT.LD)GOTO 87
      Q=9.E9
      X=A(I+2)
      Y=A(I+3)
      Z=A(I+4)
        DO 86 M=ML,LL,4
        U=AMOD(X+A(M+1),1.)-.5
        V=AMOD(Y+A(M+2),1.)-.5
        W=AMOD(Z+A(M+3),1.)-.5
        P=U*U*A(8)+V*V*A(9)+W*W*A(10)+V*W*A(11)+
     +  U*W*A(12)+U*V*A(13)
        IF(P.GT.Q)GOTO 86
        Q=P
        A(I+2)=AMOD(99.+U,1.)
        A(I+3)=AMOD(99.+V,1.)
        A(I+4)=AMOD(99.+W,1.)
  86    CONTINUE
      A(I+5)=1./A(I+5)
      K=I+7
      A(K)=SQRT(Q)
      J=I+2
      CALL SXUW(A(I),IC)
      WRITE(LI,83)IC,(A(M),M=J,K)
      GOTO 85
  87  IF(A(20).GT.-8.E9)GOTO 89
  88  CALL SXIT
  89  RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SX2H
C
C MOLECULE ASSEMBLY, GEOMETRY AND PROJECTION
C
C ** NEXT STATEMENT MUST HAVE C IN COLUMN 1 IF NOT FORTRAN-77 **
      CHARACTER*1 IH,IR,IC,IB
C
      COMMON F(126),G(126),SL,TL,LM,LR,LG,LH,LI,LP,LF,LA,LB,
     +LW,LY,LL,LQ,LE,LV,LX,LD,LZ,LJ,HA,HD,A(50000)
      COMMON/WORD/IH(50),IR(120)
      DIMENSION IB(4),IC(4)
      A(44)=AMAX1(A(44),1.)*1.1
      ML=LY+12
C
C ASSEMBLE MOLECULES
C
      Q=.5
      IF(LD+300.GT.LM)GOTO 13
      T=.1
      I=LV
      MA=I
      MB=0
   1  I=I+8
      IF(I.GT.LD)GOTO 2
      A(I+7)=AMOD(A(I+1),1000.)
      IF(T.LT.A(I+7))T=A(I+7)
      IF(I.LE.LX)A(I+6)=0.
      A(I+5)=AINT(1./A(I+5)+.2)+A(I+6)/A(44)
      K=INT(.001*A(I+1))*5+LJ+1
      A(I+6)=A(K)
      GOTO 1
   2  IF(T-.5)4,5,5
   3  IF(MB.LT.1)GOTO 25
   4  MB=-1.
      Q=T+.4
      T=T+1.
   5  I=LV
   6  I=I+8
      IF(I.GT.LD)GOTO 3
      IF(A(I+7).GT..5)GOTO 6
      IF(MB.GT.-1)GOTO 7
      A(I+7)=T
      MB=0
      MA=I
      GOTO 6
   7  P=A(I+6)+A(58)
      K=LD+5
      NK=K+3
        DO 8 L=65,LY,12
        X=A(I+2)*A(L)+A(I+3)*A(L+1)+A(I+4)*A(L+2)+A(L+9)
        Y=A(I+2)*A(L+3)+A(I+3)*A(L+4)+A(I+4)*A(L+5)+A(L+10)
        Z=A(I+2)*A(L+6)+A(I+3)*A(L+7)+A(I+4)*A(L+8)+A(L+11)
          DO 8 M=ML,LL,4
          K=K+3
          A(K)=A(M)*X+A(M+1)
          A(K+1)=A(M)*Y+A(M+2)
   8      A(K+2)=A(M)*Z+A(M+3)
      J=LV
      MB=LD
      IF(I.GT.MA)GOTO 9
      J=MA
      MB=J+8
   9  J=J+8
      IF(J.GT.MB)GOTO 6
      IF(A(J+7).LT.Q)GOTO 9
      S=(P+A(J+6))**2
        DO 10 L=NK,K,3
        U=AMOD(A(L)-A(J+2),1.)-.5
        V=AMOD(A(L+1)-A(J+3),1.)-.5
        W=AMOD(A(L+2)-A(J+4),1.)-.5
        IF(U*U*A(8)+V*V*A(9)+W*W*A(10)+V*W*A(11)+U*W*A(12)+
     +  U*V*A(13).LT.S)GOTO 11
  10    CONTINUE
      GOTO 9
  11  A(I+2)=A(J+2)+U
      A(I+3)=A(J+3)+V
      A(I+4)=A(J+4)+W
      A(I+7)=A(J+7)
      MA=I-8
      GOTO 5
  12  FORMAT(/' ** NOT ENOUGH MEMORY FOR PLAN'/)
  13  WRITE(LI,12)
      GOTO 77
C
C SET UP PLOTS
C
  14  FORMAT(///' MOLECULE',I3,5X,'SCALE',F6.3,
     +' INCHES =',F6.3,' CM PER ANGSTROM')
  15  FORMAT(///' MOLECULE',I3)
  16  FORMAT(1X,120A1)
  17  FORMAT(//' ATOM PEAK     X',7X,'Y',7X,'Z',5X,
     +'SOF  HEIGHT  DISTANCES AND ANGLES')
  18  FORMAT(/1X,4A1,F5.0,3F8.4,F7.3,F6.2,I4,1X,4A1,F6.3)
  19  FORMAT(47X,I4,1X,4A1,F6.3,11F6.1)
  20  FORMAT(//' ATOM CODE   X       Y       Z    HEIGHT',
     +'  SYMMETRY TRANSFORMATION'/)
  21  FORMAT(1X,4A1,I3,3F8.4,F7.2,1X,3(F8.4,4A1))
  22  FORMAT(4A1,I3,3F8.4,F10.6,'  0.05')
  23  FORMAT('MOLE',I4)
  24  FORMAT('END '//)
  25  A(50)=A(2)*COS(.0174533*A(6))
      A(49)=A(3)*SIN(.0174533*A(5))
      A(51)=A(3)*COS(.0174533*A(5))
      A(48)=(A(2)*A(3)*COS(.0174533*A(7))-A(51)*A(50))/A(49)
      A(47)=SQRT(A(2)*A(2)-A(50)*A(50)-A(48)*A(48))
      A(52)=A(4)
C
C WRITE MOLE NUMBERS AND PEAKS TO RESULTS FILE
C
      NK=INT(T)
      NX=0
  26  NX=NX+1
      IF(NX.GT.NK)GOTO 30
      WRITE(LP,23)NX
      I=LV
  27  I=I+8
      IF(I.GT.LD)GOTO 26
      IF(INT(A(I+7)).NE.NX)GOTO 27
      M=INT(.001*A(I+1))
      S=10.+1./AINT(A(I+5))
      CALL SXUW(A(I),IC)
        DO 28 KI=1,10
        IF(IC(1).EQ.IH(KI))GOTO 29
  28    CONTINUE
      WRITE(LP,22)IC,M,A(I+2),A(I+3),A(I+4),S
      GOTO 27
  29  WRITE(LP,22)IH(43),IC(1),IC(2),IC(3),M,
     +A(I+2),A(I+3),A(I+4),S
      GOTO 27
  30  WRITE(LP,24)
C
C ENVIRONMENT
C
      NX=0
  31  NX=NX+1
      IF(NX.GT.NK)GOTO 76
      N=LD
      MB=4
      I=LV
  32  I=I+8
      IF(I.GT.LD)GOTO 40
      IF(A(I+7).GT.999.)A(I+7)=0.
      IF(INT(A(I+7)).EQ.NX)MB=MB+2
      T=A(I+6)+A(58)
      S=AMAX1(T,A(I+6)+ABS(A(59)))
        DO 39 L=65,LY,12
        U=A(I+2)*A(L)+A(I+3)*A(L+1)+A(I+4)*A(L+2)+A(L+9)
        V=A(I+2)*A(L+3)+A(I+3)*A(L+4)+A(I+4)*A(L+5)+A(L+10)
        W=A(I+2)*A(L+6)+A(I+3)*A(L+7)+A(I+4)*A(L+8)+A(L+11)
          DO 38 M=ML,LL,4
          X=A(M)*U+A(M+1)
          Y=A(M)*V+A(M+2)
          Z=A(M)*W+A(M+3)
          J=LV
  33      J=J+8
          IF(J.GT.LD)GOTO 38
          IF(INT(A(J+7)).NE.NX)GOTO 33
          P=AMOD(X-A(J+2),1.)-.5
          Q=AMOD(Y-A(J+3),1.)-.5
          R=AMOD(Z-A(J+4),1.)-.5
          O=P*P*A(8)+Q*Q*A(9)+R*R*A(10)+Q*R*A(11)+P*R*A(12)+P*Q*A(13)
          IF(O.GT.(S+A(J+6))**2)GOTO 33
          P=P+A(J+2)
          Q=Q+A(J+3)
          R=R+A(J+4)
          IF(INT(A(I+7)).NE.NX)GOTO 34
          IF(ABS(P-A(I+2))+ABS(Q-A(I+3))+ABS(R-A(I+4)).LT..01)
     +    GOTO 33
  34      IF(O.LT.(T+A(J+6))**2)GOTO 35
          IF(AMAX1(A(I+1),A(J+1)).LT.SIGN(2500.,A(59)))GOTO 33
  35      K=N+8
  36      K=K-8
          IF(K.EQ.LD)GOTO 37
          IF(ABS(P-A(K+2))+ABS(Q-A(K+3))+ABS(R-A(K+4)).LT..01)
     +    GOTO 33
          GOTO 36
  37      N=N+8
          IF(N+59.GT.LM)GOTO 13
          A(N)=FLOAT(I)
          A(N+1)=FLOAT(L)
          A(N+2)=P
          A(N+3)=Q
          A(N+4)=R
          A(N+5)=FLOAT(M)
          A(N+6)=A(I+6)
          A(N+7)=FLOAT(NX)
          GOTO 33
  38      CONTINUE
  39    CONTINUE
      GOTO 32
C
C FIND SCALE AND ORIENTATION
C
  40  IF(MB.EQ.4)GOTO 31
      IF(MB.LT.14)GOTO 61
      MA=N+8
      IF(MA+((N-LD)/4)+MB.GT.LM)GOTO 13
      A(MA)=-9.E9
        DO 41 M=1,9
  41    G(M)=0.
      G(10)=9.E9
      G(11)=-9.E9
      G(12)=9.E9
      G(13)=-9.E9
      G(14)=9.E9
        DO 53 K=4,13,3
        IF(K.LT.10)S=.01
        I=LV
  42    I=I+8
        IF(I.GT.N)GOTO 51
        IF(INT(A(I+7)).NE.NX)GOTO 42
        U=A(47)*A(I+2)
        V=A(48)*A(I+2)+A(49)*A(I+3)
        W=A(50)*A(I+2)+A(51)*A(I+3)+A(52)*A(I+4)
        IF(K.GT.7)GOTO 45
        J=I
  43    J=J+8
        IF(J.GT.N)GOTO 42
        IF(INT(A(J+7)).NE.NX)GOTO 43
        X=A(47)*A(J+2)-U
        Y=A(48)*A(J+2)+A(49)*A(J+3)-V
        Z=A(50)*A(J+2)+A(51)*A(J+3)+A(52)*A(J+4)-W
        IF(K.EQ.4)GOTO 44
        P=Y*G(6)-Z*G(5)
        Q=Z*G(4)-X*G(6)
        Z=X*G(5)-Y*G(4)
        X=P
        Y=Q
  44    R=X*X+Y*Y+Z*Z
        IF(R.LT.S)GOTO 43
        S=R
        R=1./SQRT(R)
        G(K)=X*R
        G(K+1)=Y*R
        G(K+2)=Z*R
        GOTO 43
  45    X=U*G(1)+V*G(2)+W*G(3)
        Y=U*G(4)+V*G(5)+W*G(6)
        Z=U*G(7)+V*G(8)+W*G(9)
        IF(K.GT.10)GOTO 46
        IF(G(10).GT.X)G(10)=X
        IF(G(11).LT.X)G(11)=X
        IF(G(12).GT.Y)G(12)=Y
        IF(G(13).LT.Y)G(13)=Y
        IF(G(14).GT.Z)G(14)=Z
        GOTO 43
  46    U=X-G(10)
        V=Y-G(12)
        IF(T.GT.0.)GOTO 47
        U=V
        V=G(11)-X
  47    Q=200.*AINT(R*U+.5)+S*V+201.5
        J=MA+2
        L=J
  48    J=J-2
        IF(A(J)-Q)50,50,48
  49    A(L+2)=A(L)
        A(L+3)=A(L+1)
  50    L=L-2
        IF(J.LT.L)GOTO 49
        A(J+2)=Q
        A(J+3)=FLOAT(I)
        IF(I.GT.LD)A(J+3)=A(I)
        MA=MA+2
        A(I+7)=1000.+(Z-G(14))*ABS(T)
        GOTO 42
  51    IF(K.NE.7)GOTO 52
        G(1)=G(5)*G(9)-G(6)*G(8)
        G(2)=G(6)*G(7)-G(4)*G(9)
        G(3)=G(4)*G(8)-G(5)*G(7)
  52    IF(K.NE.10)GOTO 53
        R=G(13)-G(12)
        S=G(11)-G(10)
        T=AMIN1(1.,116./(HA*R),57./(HD*S))
        IF(T.LT..8)T=-AMIN1(1.,120./(HD*R),116./(HA*S))
        R=HD*ABS(T)
        S=HA*ABS(T)
  53    CONTINUE
C
C PLOT ATOMS
C
      S=2.54*ABS(T)
      WRITE(LI,14)NX,ABS(T),S
      M=0
        DO 54 J=1,120
  54    IR(J)=IH(20)
      K=1
      I=N+8
  55  I=I+2
      IF(I.GT.MA)GOTO 60
      NC=INT(A(I)*.005)
  56  IF(NC.EQ.M)GOTO 58
      WRITE(LI,16)(IR(J),J=1,K)
        DO 57 J=1,K
  57    IR(J)=IH(20)
      K=1
      M=M+1
      GOTO 56
  58  L=INT(A(I+1))
      CALL SXUW(A(L),IC)
      L=INT(AMOD(A(I),200.))
        DO 59 J=1,4
        IF(IC(J).EQ.IH(20))GOTO 55
        IF(IR(L).NE.IH(20))IC(J)=IH(21)
        IR(L)=IC(J)
        IF(K.LT.L)K=L
  59    L=L+1
      GOTO 55
  60  WRITE(LI,16)(IR(J),J=1,K)
      GOTO 63
C
C DISTANCES AND ANGLES
C
  61  WRITE(LI,15)NX
      N=LD
      I=LV
  62  I=I+8
      IF(I.GT.N)GOTO 63
      IF(INT(A(I+7)).EQ.NX)A(I+7)=1000.001
      GOTO 62
  63  WRITE(LI,17)
      I=LV
  64  I=I+8
      IF(I.GT.LD)GOTO 72
      IF(A(I+7).LT.999.)GOTO 64
      T=A(I+6)+A(58)
      S=AMAX1(T,A(I+6)+ABS(A(59)))
      MA=N+8
      A(MA)=-9.E9
      J=LV
  65  J=J+8
      IF(J.GT.N)GOTO 68
      IF(A(J+7).LT.999.)GOTO 65
      IF(J.EQ.I)GOTO 65
      X=A(I+2)-A(J+2)
      Y=A(I+3)-A(J+3)
      Z=A(I+4)-A(J+4)
      R=X*X*A(8)+Y*Y*A(9)+Z*Z*A(10)+Y*Z*A(11)+X*Z*A(12)+X*Y*A(13)
      IF(R.GT.(S+A(J+6))**2)GOTO 65
      R=SQRT(R)
      IF(R.LT.T+A(J+6))GOTO 67
      IF(AMAX1(A(I+1),A(J+1)).LT.SIGN(2500.,A(59)))GOTO 65
      L=LV
  66  L=L+8
      IF(L.GT.LX)GOTO 67
      IF(A(L+7).LT.999.)GOTO 66
      U=A(L+2)-A(I+2)
      V=A(L+3)-A(I+3)
      W=A(L+4)-A(I+4)
      IF(U*U*A(8)+V*V*A(9)+W*W*A(10)+V*W*A(11)+U*W*A(12)+
     +U*V*A(13).GT.(T+A(L+6))**2)GOTO 66
      U=A(L+2)-A(J+2)
      V=A(L+3)-A(J+3)
      W=A(L+4)-A(J+4)
      IF(U*U*A(8)+V*V*A(9)+W*W*A(10)+V*W*A(11)+U*W*A(12)+
     +U*V*A(13)-(A(J+6)+A(L+6)+A(58))**2)65,65,66
  67  A(MA+2)=R
      A(MA+3)=FLOAT(J)
      MA=MIN0(MA+2,N+56)
      GOTO 65
  68  J=N+8
      X=1./AINT(A(I+5))
      Y=A(44)*AMOD(A(I+5),1.)
      Z=AMOD(A(I+7),1000.)
      CALL SXUW(A(I),IC)
      IF(MA.GT.N+8)GOTO 69
      WRITE(LI,18)IC,Y,A(I+2),A(I+3),A(I+4),X
      GOTO 64
  69  J=J+2
      IF(J.GT.MA)GOTO 64
      NB=N+8
      M=1
      F(1)=A(J)
      NC=INT(A(J+1))
      K=MAX0(0,(NC-LD)/8)
      MB=NC
      IF(MB.GT.LD)MB=INT(A(NC))
      CALL SXUW(A(MB),IB)
      IF(J.GT.N+10)GOTO 70
      WRITE(LI,18)IC,Y,A(I+2),A(I+3),A(I+4),X,Z,K,IB,F(1)
      GOTO 69
  70  NB=NB+2
      IF(NB.EQ.J)GOTO 71
      L=INT(A(NB+1))
      U=A(NC+2)-A(L+2)
      V=A(NC+3)-A(L+3)
      W=A(NC+4)-A(L+4)
      S=(A(J)*A(J)+A(NB)*A(NB)-U*U*A(8)-V*V*A(9)-W*W*A(10)-
     +V*W*A(11)-U*W*A(12)-U*V*A(13))/(2.*A(J)*A(NB))
      M=M+1
      F(M)=ATAN2(SQRT(ABS(1.-S*S)),S)*57.29578
      IF(M.LT.12)GOTO 70
  71  WRITE(LI,19)K,IB,(F(L),L=1,M)
      GOTO 69
C
C SYMMETRY GENERATED ATOMS
C
  72  M=0
      IF(N.EQ.LD)GOTO 31
      WRITE(LI,20)
      I=LD
  73  I=I+8
      IF(I.GT.N)GOTO 31
      M=M+1
      Z=AMOD(A(I+7),1000.)
      K=INT(A(I))
      J=INT(A(I+1))
      L=INT(A(I+5))
      S=A(L)
      U=A(I+2)-S*(A(K+2)*A(J)+A(K+3)*A(J+1)+A(K+4)*A(J+2))
      V=A(I+3)-S*(A(K+2)*A(J+3)+A(K+3)*A(J+4)+A(K+4)*A(J+5))
      W=A(I+4)-S*(A(K+2)*A(J+6)+A(K+3)*A(J+7)+A(K+4)*A(J+8))
        DO 74 L=1,12
  74    IR(L)=IH(20)
        DO 75 NB=1,9,4
        L=NB
          DO 75 NC=14,16
          T=S*A(J)
          IF(ABS(T).LT..5)GOTO 75
          IR(L)=IH(13)
          IF(T.LT.0.)IR(L)=IH(12)
          IR(L+1)=IH(NC)
          L=L+2
  75      J=J+1
      CALL SXUW(A(K),IC)
      WRITE(LI,21)IC,M,A(I+2),A(I+3),A(I+4),Z,U,
     +(IR(J),J=1,4),V,(IR(J),J=5,8),W,(IR(J),J=9,12)
      GOTO 73
  76  I=0
      CALL SXTM(SL,LI)
  77  RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SX2I
C
C ANALYSE PATTERSON PEAK-LIST TO FIND HEAVY ATOMS
C
C ** NEXT STATEMENT MUST HAVE C IN COLUMN 1 IF NOT FORTRAN-77 **
      CHARACTER*1 IH,IT,IR
C
      COMMON F(126),G(126),SL,TL,LM,LR,LG,LH,LI,LP,LF,LA,LB,
     +LW,LY,LL,LQ,LE,LV,LX,LD,LZ,LJ,HA,HD,A(50000)
      COMMON/WORD/IH(50),IT(40),IR(80)
      LZ=LL
      LH=INT(ABS(A(21)))*8+LV
      IF(LH.LE.LV)GOTO 121
      IF(A(21).LT.0.)LH=LV+800
      A(27)=AMOD(100.*A(22),100.)
      A(LX+14)=9.E9
      A(LX+15)=9.E9
      QC=2.-A(23)
      QI=8.*QC*A(60)/FLOAT(LL-LY-8)
      QH=QC*FLOAT(LY-53)/12.
      QL=300.+700./SQRT(QH)
      LJ=LY+12
      IF(A(23).LT..5)LZ=(LZ+LJ-4)/2
      A(43)=A(1)*A(1)/(8.*A(64))
      A(53)=8./A(43)
        DO 1 I=61,63
   1    A(I)=.1*SQRT(A(43))/A(I-59)
      A(29)=12.*A(63)
      A(28)=AMIN1(A(2),A(3),A(4))
      I=LJ
   2  I=I+4
      IF(I.GT.LZ)GOTO 3
      X=AMOD(A(I+1),1.)-.5
      Y=AMOD(A(I+2),1.)-.5
      Z=AMOD(A(I+3),1.)-.5
      A(28)=AMIN1(A(28),SQRT(X*X*A(8)+Y*Y*A(9)+Z*Z*A(10)+
     +Y*Z*A(11)+X*Z*A(12)+X*Y*A(13)))
      GOTO 2
C
C MOVE UP AND EXPAND PEAKLIST
C
   3  N=LX+7
      L=LD+7
      LD=LH-LX+LD
      M=LD+7
   4  A(M)=A(L)
      L=L-1
      M=M-1
      IF(L.GT.N)GOTO 4
      LE=LD+3
      N=LE
      L=LH
   5  L=L+8
      IF(L.GT.LD)GOTO 11
      IF(A(L+7).LT.A(27))GOTO 5
      S=1.
   6    DO 10 K=65,LY,12
        A(N+5)=A(L+6)
        A(N+6)=S*(A(L+2)*A(K)+A(L+3)*A(K+1)+A(L+4)*A(K+2))
        A(N+7)=S*(A(L+2)*A(K+3)+A(L+3)*A(K+4)+A(L+4)*A(K+5))
        A(N+8)=S*(A(L+2)*A(K+6)+A(L+3)*A(K+7)+A(L+4)*A(K+8))
        A(N+9)=A(L+7)
        J=LE
   7    J=J+5
        IF(J.GT.N)GOTO 9
          DO 8 M=LJ,LZ,4
          Z=AMOD(A(M+3)+A(J+3)-A(N+8),1.)-.5
          IF(ABS(Z).GT.A(29))GOTO 8
          X=AMOD(A(M+1)+A(J+1)-A(N+6),1.)-.5
          Y=AMOD(A(M+2)+A(J+2)-A(N+7),1.)-.5
          IF(A(8)*X*X+A(9)*Y*Y+A(10)*Z*Z+A(11)*Y*Z+A(12)*X*Z+
     +    A(13)*X*Y.LT.A(43))GOTO 10
   8      CONTINUE
        GOTO 7
   9    N=N+5
  10    CONTINUE
      S=-S
      IF(S.LT.0.)GOTO 6
      LE=N
      IF(N.LT.LM-2000)GOTO 5
  11  N=LE-2
      LR=LE
      IF(LX.GT.LV)GOTO 121
C
C TRY HEAVY ATOM AT ORIGIN
C
      M=N+7
      J=LJ
  12  L=M+6
        DO 13 I=M,L
  13    A(I)=0.
      A(M)=.5*(A(J+1)-99.5)
      A(M+1)=.5*(A(J+2)-99.5)
      A(M+2)=.5*(A(J+3)-99.5)
      CALL SX2J(M,T,S,R)
      IF(T.LT.5.)GOTO 14
      N=M
      A(N+3)=S
      A(N+4)=T
      A(N+6)=T
      M=M+7
  14  J=J+4
      IF(A(26).LT.0.)GOTO 15
      IF(J.LE.LZ)GOTO 12
C
C IF CENTRIC, CONSIDER ALL VECTORS AS POSSIBLE 2X,2Y,2Z
C
  15  NG=1
        DO 16 K=1,12
        G(K)=0.
  16    G(K+12)=.5
        DO 17 K=4,12,4
        G(K)=.5
  17    G(K+12)=0.
        DO 18 K=65,LY,12
        IF(ABS(A(K)).LT..5)NG=MAX0(NG,4)
        IF(A(K+1)-A(K).GT.1.9)NG=23
        IF(ABS(A(K+8)).LT..5)NG=22
  18    CONTINUE
      IF(LZ.EQ.LJ+4)NG=MIN0(NG,10)
      IF(LZ.EQ.LJ+12)NG=MIN0(NG,4)
      IF(NG.LT.23)GOTO 19
      NG=10
      G(10)=.5
      G(11)=.5
      G(12)=0.
  19  IF(A(23).GT..5)GOTO 22
      I=LH+8
  20  I=I+8
      IF(I.GT.LD)GOTO 47
        DO 21 L=LJ,LZ,4
          DO 21 J=1,NG,3
          A(N+7)=.5*(A(I+2)+A(L+1)-99.5)+G(J)
          A(N+8)=.5*(A(I+3)+A(L+2)-99.5)+G(J+1)
          A(N+9)=.5*(A(I+4)+A(L+3)-99.5)+G(J+2)
          K=N+7
          CALL SX2J(K,T,S,R)
          IF(T.LT..1)GOTO 21
          N=K
          A(N+3)=S
          A(N+4)=T
          A(N+6)=T
  21      CONTINUE
      IF(N+1000-LM)20,47,47
  22  IF(LY.EQ.65)GOTO 47
C
C SPACE GROUPS PM, PC, CM AND CC (EITHER SETTING)
C
      NI=-4
      IF(LY.GT.77)GOTO 27
      IF(A(77).LT..9)GOTO 27
      IF(A(81)*A(85).GT.-.9)GOTO 27
      I=LH+8
  23  I=I+8
      IF(I.GT.LD)GOTO 47
        DO 26 L=LJ,LZ,4
        X=AMOD(A(L+1)+A(I+2)+A(86),1.)-.5
        Y=AMOD(A(L+2)+A(I+3)+A(87),1.)-.5
        Z=AMOD(A(L+3)+A(I+4)+A(88),1.)-.5
        IF(ABS(X).GT.5.*A(61))GOTO 26
        IF(A(81).GT..9)GOTO 24
        IF(ABS(Z).GT.5.*A(63))GOTO 26
        A(N+8)=.5*Y
        A(N+9)=0.
        GOTO 25
  24    IF(ABS(Y).GT.5.*A(62))GOTO 26
        A(N+8)=0.
        A(N+9)=.5*Z
  25    N=N+7
        A(N)=0.
        A(N+3)=1.
        A(N+4)=A(I+6)
        A(N+6)=A(I+7)
  26    CONTINUE
      GOTO 23
C
C CHECK POLAR AXIS, ENANT. FIXED BY S.G., ALLOWED ORIGINS
C
  27  NI=NI+4
      IF(NI.GT.16)GOTO 47
      NN=MIN0(NI/4,2)
      NQ=1
      PE=-.0001
        DO 28 NJ=77,LY,12
        IF(ABS(A(NJ+1))+ABS(A(NJ+2))+ABS(A(NJ+5)).GT..9)NQ=2
        IF(AMOD(99.51+A(NJ+11),.5).GT..02)PE=-1.
  28    CONTINUE
      IF(LZ.GT.LJ)PE=-.0001
      IF(NI.EQ.16)NQ=1
      IF(NI.EQ.12)NQ=3
      NX=0
        DO 29 NJ=77,LY,12
        NK=NJ+4*NN
        IF(A(NK).GT.-.9)GOTO 29
        IF(NI.EQ.0)P=ABS(A(NJ+1))+ABS(A(NJ+2))
        IF(NI.EQ.4)P=ABS(A(NJ+3))+ABS(A(NJ+5))
        IF(NI.GT.4)P=ABS(A(NJ+6))+ABS(A(NJ+7))
        IF(P.LT..1)NX=NJ
  29    CONTINUE
C
C LOCATE REAL OR SCREW 2, 3, 4 OR 6-FOLD AXIS
C
      K=65
  30  K=K+12
      IF(K.GT.LY)GOTO 27
      IF(NI.LT.12)GOTO 32
      IF(ABS(A(K))+ABS(A(K+2))+ABS(A(K+7))+ABS(A(K+9))+
     +ABS(A(K+10)).GT..1)GOTO 30
      IF(NI.EQ.16)GOTO 31
      IF(ABS(A(K+5))+ABS(A(K+6)).GT..1)GOTO 30
      IF(A(K+3)+A(K+8)-A(K+1)-A(K+4)-3.9)30,33,33
  31  IF(ABS(A(K+3))+ABS(A(K+4))+ABS(A(K+8))+
     +ABS(A(K+11)).GT..1)GOTO 30
      IF(A(K+1)+A(K+5)+A(K+6)-2.9)30,33,33
  32  IF(ABS(A(K+1))+ABS(A(K+2))+ABS(A(K+3))+ABS(A(K+5))+
     +ABS(A(K+6))+ABS(A(K+7)).GT..1)GOTO 30
      I=K+NI
      IF(A(I).LT..9)GOTO 30
      IF(A(K)+A(K+4)+A(K+8).GT.-.9)GOTO 30
C
C FIND TWO COORDINATES FROM FIRST HARKER VECTOR
C
  33  I=LH+8
      NJ=LE
  34  I=I+8
      IF(I.GT.LD)GOTO 27
      S=1.
  35    DO 46 L=LJ,LZ,4
          DO 46 NK=1,NQ
          J=NN+N+7
          A(N+7)=.5*(AMOD(A(L+1)+S*A(I+2)+A(K+9),1.)-.5)
          A(N+8)=.5*(AMOD(A(L+2)+S*A(I+3)+A(K+10),1.)-.5)
          A(N+9)=.5*(AMOD(A(L+3)+S*A(I+4)+A(K+11),1.)-.5)
          IF(NI.NE.16)GOTO 36
          IF(ABS(AMOD(99.5+2.*(A(N+7)+A(N+8)+A(N+9)),1.)-.5)
     +    .GT..2*A(29))GOTO 46
          A(N+7)=-2.*A(N+9)
          A(N+8)=2.*A(N+8)
          GOTO 37
  36      IF(ABS(A(J)).GT.2.*A(NN+61))GOTO 46
          IF(NI.NE.12)GOTO 37
          A(N+8)=.666667*(A(N+7)+A(N+8)-FLOAT(NK))
          A(N+7)=2.*A(N+7)-A(N+8)
  37      A(J)=0.
          IF(NX.EQ.0)GOTO 41
C
C SET UP SECOND HARKER VECTOR, FIND THIRD COORDINATE
C
          X=A(N+7)*(A(NX)-1.)+A(N+8)*A(NX+1)+A(N+9)*A(NX+2)+A(NX+9)
          Y=A(N+7)*A(NX+3)+A(N+8)*(A(NX+4)-1.)+A(N+9)*A(NX+5)+A(NX+10)
          Z=A(N+7)*A(NX+6)+A(N+8)*A(NX+7)+A(N+9)*(A(NX+8)-1.)+A(NX+11)
          NJ=LD+8
  38        DO 39 M=LJ,LZ,4
            F(1)=AMOD(A(M+1)+A(NJ+1)-X,1.)-.5
            F(2)=AMOD(A(M+2)+A(NJ+2)-Y,1.)-.5
            F(3)=AMOD(A(M+3)+A(NJ+3)-Z,1.)-.5
            IF(F(NN+1).LT.PE)GOTO 39
            R=ABS(F(1)*A(2))+ABS(F(2)*A(3))+ABS(F(3)*A(4))-
     +      ABS(F(NN+1)*A(NN+2))
            IF(R*R.LT..3*A(43))GOTO 40
  39        CONTINUE
          GOTO 45
  40      J=NN+N+7
          A(J)=-.5*F(NN+1)
          IF(NI.EQ.8)J=J-1
          IF(NI.NE.12)A(J)=A(J)+.5*FLOAT(NK-1)
  41      M=N+7
          CALL SX2J(M,T,P,R)
          IF(T.LT..1)GOTO 45
          MI=LE-2
  42      MI=MI+7
          IF(MI.GT.N)GOTO 43
          V=1.
          CALL SX2K(MI,M,U,V)
          IF(V-.01)45,45,42
  43        DO 44 M=1,7
            A(N+14)=A(N+7)
  44        N=N+1
          A(N+3)=P
          A(N+4)=T
          A(N+6)=T
          IF(N.GT.LM-1000)GOTO 47
  45      NJ=NJ+5
          IF(NJ.LE.LE)GOTO 38
  46      CONTINUE
      IF(NI.NE.12)GOTO 34
      S=-S
      IF(S)35,34,34
C
C READ IN LARGEST E-VALUES
C
  47  NE=N-1
  48  READ(LF)F
        DO 49 I=1,124,3
        IF(F(I).LT..5)GOTO 50
        P=AMOD(F(I+1),10.)
        IF(MOD(I,15).NE.1)GOTO 49
        CALL SXH2(F(I),X,Y,Z)
        NE=NE+8
        A(NE)=6.2831853*X
        A(NE+1)=6.2831853*Y
        A(NE+2)=6.2831853*Z
        A(NE+3)=10.*F(I+2)+P
        IF(NE.GT.LM-15)GOTO 50
  49    CONTINUE
      GOTO 48
  50  REWIND LF
C
C SET UP ATOMIC NUMBER LIST
C
      NQ=1
      F(1)=9.E9
      I=LL-1
  51  I=I+5
      IF(I.GT.LQ)GOTO 55
      J=NQ+1
      K=J
  52  J=J-1
      IF(A(I).GT.F(J))GOTO 52
      GOTO 54
  53  F(K+1)=F(K)
  54  K=K-1
      IF(K.GT.J)GOTO 53
      F(J+1)=A(I)
      IF(NQ.LT.125)NQ=NQ+1
      GOTO 51
  55  I=1
      J=1
      F(NQ+1)=-99.
  56  I=I+1
      IF(I.GT.NQ)GOTO 57
      IF(ABS(F(I)-F(I+1)).LT.2.5)I=I+1
      J=J+1
      F(J)=F(I)
      GOTO 56
  57  I=2
  58  I=I+1
      IF(I.GT.J)GOTO 59
      IF(F(I).GT.10.)GOTO 58
  59  NQ=MIN0(J+1,I)
      F(NQ)=0.
C
C ANALYSE POTENTIAL STARTING ATOMS
C
  60  FORMAT(///' LOCATION OF HEAVY ATOMS FROM PATTERSON'//
     +I6,' E-VALUES GREATER THAN',F7.3,' SELECTED FOR RE')
  61  FORMAT(/' TRIAL ATOMS FOR OPTIMISED RE =',F7.3,'   R(PAT) =',
     +F7.3/' AT.NO.    X      Y      Z    S.O.F.  SELFMF SUPMF',
     +'  MINIMUM DISTANCES (SELF FIRST)')
  62  FORMAT(F5.0,A1,F8.3,2F7.3,F8.4,2F7.1,10F7.2)
      J=(NE-N+1)/8
      O=ABS(A(26))
      WRITE(LI,60)J,O
      LR=N-1
      O=9.E8
      L=LE-2
  63  L=L+7
      IF(L.GT.N)GOTO 113
      A(L+5)=9.E9
C
C IDEALISE SPECIAL POSITION
C
      H=0.
      U=A(L)
      V=A(L+1)
      W=A(L+2)
      X=0.
      Y=0.
      Z=0.
        DO 64 K=65,LY,12
        XX=U*A(K)+V*A(K+1)+W*A(K+2)+A(K+9)
        YY=U*A(K+3)+V*A(K+4)+W*A(K+5)+A(K+10)
        ZZ=U*A(K+6)+V*A(K+7)+W*A(K+8)+A(K+11)
          DO 64 M=LJ,LL,4
          P=AMOD(A(M+1)+A(M)*XX-U,1.)-.5
          Q=AMOD(A(M+2)+A(M)*YY-V,1.)-.5
          R=AMOD(A(M+3)+A(M)*ZZ-W,1.)-.5
          IF(A(8)*P*P+A(9)*Q*Q+A(10)*R*R+A(11)*Q*R+A(12)*P*R+
     +    A(13)*P*Q.GT.A(43))GOTO 64
          H=H+1.
          X=X+P
          Y=Y+Q
          Z=Z+R
  64      CONTINUE
      H=1./H
      A(L)=AMOD(U+X*H+99.,1.)
      A(L+1)=AMOD(V+Y*H+99.,1.)
      A(L+2)=AMOD(W+Z*H+99.,1.)
      A(L+3)=H
      CALL SX2J(L,U,V,W)
      A(L+4)=U
      A(L+6)=U
      NA=NE+8
      A(NA)=A(L)
      A(NA+1)=A(L+1)
      A(NA+2)=A(L+2)
      A(NA+3)=A(L+3)
      A(NA+4)=2.
      A(NA+5)=U
      A(NA+6)=999.
      A(NA+7)=W
C
C SUPERPOSITION M.F. PEAKLIST BASED ON H.A. AND EQUIVALENTS
C
      NH=NA
      Q=.1
      M=LD+8
  65  M=M+5
      IF(M.GT.LE)GOTO 71
      U=A(M+1)+A(L)
      V=A(M+2)+A(L+1)
      W=A(M+3)+A(L+2)
      J=NA+8
      A(J)=U
      A(J+1)=V
      A(J+2)=W
      R=-1.
      CALL SX2K(J,L,T,R)
      IF(T.LT..1)GOTO 65
      IF(R.LT..1)GOTO 65
      CALL SX2J(J,H,X,Y)
      IF(H.LT..1)GOTO 65
      XS=0.
      YS=0.
      ZS=0.
      H=0.
      NJ=J
      I=NH
  66  I=I+8
      IF(I.GT.J)GOTO 70
      S=1.
  67    DO 69 K=65,LY,12
        XX=S*(U*A(K)+V*A(K+1)+W*A(K+2)+A(K+9))-A(I)
        YY=S*(U*A(K+3)+V*A(K+4)+W*A(K+5)+A(K+10))-A(I+1)
        ZZ=S*(U*A(K+6)+V*A(K+7)+W*A(K+8)+A(K+11))-A(I+2)
          DO 69 MI=LJ,LZ,4
          X=AMOD(XX+A(MI+1),1.)-.5
          Y=AMOD(YY+A(MI+2),1.)-.5
          Z=AMOD(ZZ+A(MI+3),1.)-.5
          IF(A(8)*X*X+A(9)*Y*Y+A(10)*Z*Z+A(11)*Y*Z+
     +    A(12)*X*Z+A(13)*X*Y.GT.A(43))GOTO 69
          IF(I.EQ.J)GOTO 68
          IF(T.LT.A(I+6))GOTO 65
          NJ=I
          GOTO 69
  68      XS=XS+X
          YS=YS+Y
          ZS=ZS+Z
          H=H+1.
  69      CONTINUE
      S=-S
      IF(S.LT.-.5-A(23))GOTO 67
      GOTO 66
  70  A(NJ)=AMOD(U+XS/H+99.,1.)
      A(NJ+1)=AMOD(V+YS/H+99.,1.)
      A(NJ+2)=AMOD(W+ZS/H+99.,1.)
      A(NJ+4)=FLOAT(NQ-1)
      R=-1.
      IF(H.GT.1.1)CALL SX2K(NJ,L,T,R)
      A(NJ+6)=T
      NA=MAX0(NA,NJ)
      LR=MAX0(LR,NA)
      CALL SX2J(NJ,U,V,W)
      A(NJ+3)=V
      A(NJ+5)=U
      A(NJ+7)=W
      IF(NA.LT.LM-294)GOTO 65
C
C IMPOSE FURTHER HEAVY ATOMS TO PRUNE SUPERPOSITION LIST
C
  71  Q=Q+1.
      IF(Q.GT.ABS(A(21)))GOTO 78
      K=0
      P=-1.
      M=NH
  72  M=M+8
      IF(M.GT.NA)GOTO 73
      R=(10.+A(M+5))*A(M+6)
      IF(P.GT.R)GOTO 72
      P=R
      K=M
      GOTO 72
  73  IF(K.EQ.0)GOTO 78
      J=K+7
        DO 74 I=K,J
        R=A(I)
        A(I)=A(NH+8)
        A(NH+8)=R
  74    NH=NH+1
      A(NH+4)=2.
      MI=NH
  75  MI=MI+8
  76  IF(MI.GT.NA)GOTO 71
      R=-1.
      CALL SX2K(NH,MI,T,R)
      A(MI+6)=AMIN1(A(MI+6),T)
      IF(T.GT..1)GOTO 75
      M=MI+7
        DO 77 I=MI,M
        A(I)=A(NA)
  77    NA=NA+1
      NA=NA-16
      GOTO 76
  78  IF(A(21).GT.0.)NA=NH
C
C ADJUST SFAC TYPES TO OPTIMISE RE
C
      M=NE+8
      NJ=N+7
        DO 79 I=NJ,NE,8
        A(I+4)=0.
  79    A(I+5)=0.
      R=1.
  80  NN=NA+5
      MQ=NQ
      IF(NH.LT.M)GOTO 81
      MQ=3
      IF(A(21).GT.0.)MQ=MIN0(MQ,NQ-1)
  81  U=A(M)
      V=A(M+1)
      W=A(M+2)
        DO 82 K=65,LY,12
        NN=NN+3
        A(NN)=A(K)*U+A(K+1)*V+A(K+2)*W+A(K+9)
        A(NN+1)=A(K+3)*U+A(K+4)*V+A(K+5)*W+A(K+10)
  82    A(NN+2)=A(K+6)*U+A(K+7)*V+A(K+8)*W+A(K+11)
      LR=MAX0(LR,NN-4)
      MI=NA+8
        DO 84 I=NJ,NE,8
        U=0.
        V=0.
          DO 83 K=MI,NN,3
          P=A(I)*A(K)+A(I+1)*A(K+1)+A(I+2)*A(K+2)
          IF(A(23).GT..5)V=V+SIN(P)
  83      U=U+COS(P)
        A(I+6)=U
  84    A(I+7)=V
      J=INT(A(M+4))
      IF(J.EQ.NQ)GOTO 86
      P=R*A(M+3)*F(J)
        DO 85 I=NJ,NE,8
        A(I+4)=A(I+4)+A(I+6)*P
  85    A(I+5)=A(I+5)+A(I+7)*P
  86  IF(R.LT.0.)GOTO 89
      M=M+8
      IF(M.LE.NA)GOTO 80
      U=0.
      V=0.
      H=0.
        DO 87 I=NJ,NE,8
        E=AINT(.1*A(I+3))
        W=AMOD(A(I+3),10.)
        H=H+W**2*E
        T=QC*(A(I+4)**2+A(I+5)**2)+QI*E
        U=U+T
  87    V=V+W*SQRT(T*E)
      RE=1.-V**2/AMAX1(H*U,.0001)
      IF(NA.EQ.NE+8)GOTO 98
      NT=NE+16
      R=-1.
  88  M=NE+16
      GOTO 80
  89  NX=0
      NK=1
      IF(J.EQ.MQ)GOTO 97
  90  J=J+NK
      A(M+4)=FLOAT(J)
      P=A(M+3)*F(J)
      U=0.
      V=0.
        DO 91 I=NJ,NE,8
        E=AINT(.1*A(I+3))
        X=A(I+4)+P*A(I+6)
        Y=A(I+5)+P*A(I+7)
        Z=QC*(X**2+Y**2)+QI*E
        U=U+Z
  91    V=V+AMOD(A(I+3),10.)*SQRT(Z*E)
      W=1.-V**2/AMAX1(H*U,.0001)
      IF(W.GE.RE)GOTO 95
      RE=W
      NT=M
      NX=1
      IF(J.EQ.MQ)GOTO 92
      IF(J.NE.2)GOTO 90
  92  IF(P.LT..001)GOTO 94
        DO 93 I=NJ,NE,8
        A(I+4)=A(I+4)+P*A(I+6)
  93    A(I+5)=A(I+5)+P*A(I+7)
  94  M=M+8
      IF(M.EQ.NT)GOTO 98
      IF(M.LE.NA)GOTO 80
      IF(NT.EQ.NE+16)GOTO 98
      GOTO 88
  95  J=J-NK
      IF(NX.NE.0)GOTO 96
      IF(NK.EQ.1)GOTO 97
  96  P=A(M+3)*F(J)
      A(M+4)=FLOAT(J)
      GOTO 92
  97  NK=-1
      IF(J.EQ.2)GOTO 96
      GOTO 90
C
C FIND R(PAT)
C
  98  Z=QI
      MQ=NA+7
      I=NE
  99  I=I+8
      IF(I.GT.NA)GOTO 100
      M=INT(A(I+4))
      Z=Z+QH*A(I+3)*F(M)**2
      GOTO 99
 100  Z=QL/Z
      V=.0001
      I=NE
 101  I=I+8
      IF(I.GT.NA)GOTO 103
      M=INT(A(I+4))
      IF(M.EQ.NQ)GOTO 101
      T=Z*F(M)**2
      IF(I.EQ.NE+8)U=.2*QC*T
      U=U+QC*AMAX1(T-A(I+5),0.)
      V=V+QC*T
      IF(MQ.LT.LM)MQ=MQ+1
      A(MQ)=A(I+7)
      J=NE
 102  J=J+8
      IF(J.GE.I)GOTO 101
      K=INT(A(J+4))
      IF(K.EQ.NQ)GOTO 102
      R=1.
      CALL SX2K(I,J,W,R)
      T=QC*Z*F(M)*F(K)
      IF(MQ.LT.LM)MQ=MQ+1
      A(MQ)=R
      U=U+AMAX1(T-W,0.)
      V=V+T
      GOTO 102
 103  A(L+4)=AMIN1(U/V,1.)
      RE=SQRT(RE)
      LR=MAX0(LR,MQ-6)
C
C PRINT, R(PAT) RE AND (IF BOTH LESS THAN .5) ATOMS
C
      A(L+5)=RE
      IF(A(L+4).GT..5)GOTO 106
      IF(RE.GT..5)GOTO 106
      WRITE(LI,61)RE,A(L+4)
      K=0
      J=NA+8
      I=NE
 104  I=I+8
      IF(I.GT.NA)GOTO 106
      M=INT(A(I+4))
      IF(F(M).LT..5)GOTO 104
      NX=MIN0(J+K,J+9,MQ,LM-1)
      K=K+1
      MI=21
      IF(I.GT.NH)MI=20
      IF(J.LE.NX)GOTO 105
      WRITE(LI,62)F(M),IH(MI),A(I),A(I+1),A(I+2),A(I+3),A(I+5),A(I+6)
      GOTO 104
 105  WRITE(LI,62)F(M),IH(MI),A(I),A(I+1),A(I+2),A(I+3),A(I+5),A(I+6),
     +(A(NT),NT=J,NX)
      J=J+K
      GOTO 104
C
C SAVE BEST SOLUTION
C
 106  T=A(L+5)+2.*AMAX1(.15,A(L+4))
      IF(T.GT.O)GOTO 63
      O=T
      LX=LV
 107  Q=8.E9
      J=0
      K=NE
 108  K=K+8
      IF(K.GT.NA)GOTO 109
      IF(A(K+4).GE.Q)GOTO 108
      Q=A(K+4)
      J=K
      GOTO 108
 109  IF(J.EQ.0)GOTO 63
      M=INT(Q)
      IF(A(21).GT.0.)GOTO 110
      IF(M.EQ.NQ)GOTO 63
 110  LX=LX+8
      A(LX+1)=F(M)
      A(LX+2)=A(J)
      A(LX+3)=A(J+1)
      A(LX+4)=A(J+2)
      A(LX+5)=A(J+3)
      A(LX+6)=A(J+5)
      A(LX+7)=A(J+6)
      A(J+4)=9.E9
      IF(LX-LH)107,63,63
C
C PRINT POSSIBLE STARTING ATOMS
C
 111  FORMAT(///' POSSIBLE HEAVY ATOM POSITIONS FOR ',40A1//6X,'X',
     +7X,'Y       Z     1/MULT     R(PAT)    RE(HA)   SELFMF')
 112  FORMAT(/3F8.3,F10.4,2F10.3,F9.1)
 113  WRITE(LI,111)IT
 114  S=8.E9
      I=LE-2
 115  I=I+7
      IF(I.GT.N)GOTO 116
      T=A(I+5)+2.*AMAX1(.15,A(I+4))
      IF(T.GT.S)GOTO 115
      S=T
      L=I
      GOTO 115
 116  IF(S.GT.2.*O)GOTO 118
      K=L+6
      WRITE(LI,112)(A(J),J=L,K)
      A(L+5)=9.E9
      GOTO 114
 117  FORMAT(/' ** NO HEAVY ATOMS FOUND')
 118  IF(LX.GT.LV)GOTO 120
      WRITE(LI,117)
      CALL SXIT
 119  FORMAT(/' ** IT WILL NOW BE ASSUMED THAT THE FIRST ATOM IN',
     +' THE ABOVE LIST IS A CORRECT'/' HEAVY ATOM.  IF THIS IS ',
     +'NOT TRUE, THE REST OF THE OUTPUT WILL BE WORTHLESS.'/' IN',
     +' SUCH A CASE, RERUN THE JOB WITH A DIFFERENT ATOM FROM THE',
     +' ABOVE LIST'/' INSERTED BETWEEN THE PATT AND HKLF INSTRUCTIONS')
 120  WRITE(LI,119)
      CALL SXTM(SL,LI)
 121  RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SX2J(I,T,P,R)
C
C USES EXPANDED PEAKLIST TO FIND HARKER MINIMUM FUNCTION T FOR
C ATOM AT A(I),A(I+1),A(I+2).  P IS SET TO 1/MULT AND R TO
C MINIMUM HARKER VECTOR LENGTH.
C
      COMMON F(126),G(126),SL,TL,LM,LR,LG,LH,LI,LP,LF,LA,LB,
     +LW,LY,LL,LQ,LE,LV,LX,LD,LZ,LJ,HA,HD,A(50000)
      N=LE
      R=A(28)**2
      P=0.
      T=999.
      U=A(I)
      V=A(I+1)
      W=A(I+2)
      S=1.
      K=65
   1  P=P+1.
   2  K=K+12
      IF(K.GT.LY)GOTO 6
      XX=S*(U*A(K)+V*A(K+1)+W*A(K+2)+A(K+9))-U
      YY=S*(U*A(K+3)+V*A(K+4)+W*A(K+5)+A(K+10))-V
      ZZ=S*(U*A(K+6)+V*A(K+7)+W*A(K+8)+A(K+11))-W
      Q=0.
        DO 4 L=LJ,LZ,4
        UU=XX+A(L+1)
        VV=YY+A(L+2)
        WW=ZZ+A(L+3)
        X=AMOD(UU,1.)-.5
        Y=AMOD(VV,1.)-.5
        Z=AMOD(WW,1.)-.5
        H=A(8)*X*X+A(9)*Y*Y+A(10)*Z*Z+A(11)*Y*Z+A(12)*X*Z+A(13)*X*Y
        IF(H.LT.A(43))GOTO 1
        IF(R.GT.H)R=H
        M=LD+8
   3    M=M+5
        IF(M.GT.N)GOTO 4
        Z=AMOD(WW-A(M+3),1.)-.5
        IF(ABS(Z).GT.A(29))GOTO 3
        X=AMOD(UU-A(M+1),1.)-.5
        Y=AMOD(VV-A(M+2),1.)-.5
        Z=A(8)*X*X+A(9)*Y*Y+A(10)*Z*Z+A(11)*Y*Z+A(12)*X*Z+
     +  A(13)*X*Y
        IF(Z.GT.A(43))GOTO 3
        Y=A(M)/(1.+A(53)*Z)
        IF(Y.LE.Q)GOTO 3
        Q=Y
        IF(Z-.25*A(43))5,3,3
   4    CONTINUE
   5  IF(T.GT.Q)T=Q
      IF(T.LT..01)N=LD+8
      GOTO 2
   6  S=-S
      K=53
      IF(S.LT.-.5-A(23))GOTO 2
      P=1./P
      R=SQRT(R)
      RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SX2K(I,J,T,R)
C
C USES EXPANDED PEAKLIST TO FIND CROSS MINIMUM FUNCTION T FOR
C TWO ATOMS AT A(I),A(I+1),A(I+2) AND A(J),A(J+1),A(J+2).  R IS
C THE MINIMUM DISTANCE BETWEEN ONE ATOM AND ALL SYMMETRY
C EQUIVALENTS OF THE OTHER.  IF THE ATOMS ARE FOUND TO BE
C EQUIVALENT TO EACH OTHER, SETS R=0, T=999.  IF R IS NEGATIVE
C ON CALLING, THE SUBROUTINE EXITS IMMEDIATELY WITH T=0 IF IT
C IS CLEAR THAT AT LEAST ONE PEAK IS MISSING.  THIS OPTION WILL
C GIVE AN INCORRECT VALUE OF R IN SUCH A CASE.  TO ENSURE THAT
C R IS CORRECT EVEN WHEN T IS 0, SET R ZERO OR POSITIVE ON INPUT.
C
      COMMON F(126),G(126),SL,TL,LM,LR,LG,LH,LI,LP,LF,LA,LB,
     +LW,LY,LL,LQ,LE,LV,LX,LD,LZ,LJ,HA,HD,A(50000)
      N=LE
      QE=-999.
      IF(R.LT.0.)QE=.1
      T=9.E9
      R=9.E9
      U=A(I)
      V=A(I+1)
      W=A(I+2)
      S=1.
   1    DO 5 K=65,LY,12
        XX=S*(U*A(K)+V*A(K+1)+W*A(K+2)+A(K+9))-A(J)
        YY=S*(U*A(K+3)+V*A(K+4)+W*A(K+5)+A(K+10))-A(J+1)
        ZZ=S*(U*A(K+6)+V*A(K+7)+W*A(K+8)+A(K+11))-A(J+2)
        Q=0.
          DO 3 L=LJ,LZ,4
          UU=XX+A(L+1)
          VV=YY+A(L+2)
          WW=ZZ+A(L+3)
          X=AMOD(UU,1.)-.5
          Y=AMOD(VV,1.)-.5
          Z=AMOD(WW,1.)-.5
          R=AMIN1(R,A(8)*X*X+A(9)*Y*Y+A(10)*Z*Z+A(11)*Y*Z+
     +    A(12)*X*Z+A(13)*X*Y)
          M=LD+8
   2      M=M+5
          IF(M.GT.N)GOTO 3
          Z=AMOD(WW-A(M+3),1.)-.5
          IF(ABS(Z).GT.A(29))GOTO 2
          X=AMOD(UU-A(M+1),1.)-.5
          Y=AMOD(VV-A(M+2),1.)-.5
          Z=A(8)*X*X+A(9)*Y*Y+A(10)*Z*Z+A(11)*Y*Z+A(12)*X*Z+
     +    A(13)*X*Y
          IF(Z.GT.A(43))GOTO 2
          Y=A(M)/(1.+Z*A(53))
          IF(Y.LE.Q)GOTO 2
          Q=Y
          IF(Z-.25*A(43))4,2,2
   3      CONTINUE
   4    IF(T.GT.Q)T=Q
        IF(T.LT..01)N=LD+8
        IF(T.LT.QE)GOTO 7
   5    CONTINUE
      S=-S
      IF(S.LT.-.5-A(23))GOTO 1
      IF(R.GT.A(43))GOTO 6
      T=999.
      R=0.
   6  R=SQRT(R)
   7  RETURN
      END
C
C ------------------------------------------------------------
C
      SUBROUTINE SX2L
C
C PATTERSON CROSS-VECTOR ANALYSIS
C
C ** NEXT STATEMENT MUST HAVE C IN COLUMN 1 IF NOT FORTRAN-77 **
      CHARACTER*1 IH,IT,IR,ID
C
      COMMON F(126),G(126),SL,TL,LM,LR,LG,LH,LI,LP,LF,LA,LB,
     +LW,LY,LL,LQ,LE,LV,LX,LD,LZ,LJ,HA,HD,A(50000)
      COMMON/WORD/IH(50),IT(40),IR(80)
      DIMENSION ID(4)
      IF(LH.LE.LV)GOTO 54
      KZ=INT(A(54))
C
C FIND SYMMETRY EQUIVALENTS OF STARTING ATOMS
C
      LR=LR+7
      IF(A(21).LT.0.)GOTO 59
      IF(A(22).LT..999)GOTO 54
      NK=MAX0(LD+5,LV+144+(LX-LV)/2)
      NN=NK
      I=LV
   1  I=I+8
      M=NN-3
      IF(I.GT.LX)GOTO 7
      S=1.
   2    DO 6 K=65,LY,12
        X=S*(A(I+2)*A(K)+A(I+3)*A(K+1)+A(I+4)*A(K+2)+A(K+9))
        Y=S*(A(I+2)*A(K+3)+A(I+3)*A(K+4)+A(I+4)*A(K+5)+A(K+10))
        Z=S*(A(I+2)*A(K+6)+A(I+3)*A(K+7)+A(I+4)*A(K+8)+A(K+11))
        J=M
   3    J=J+3
        IF(J.GT.NN)GOTO 5
          DO 4 L=LJ,LZ,4
          U=AMOD(A(L+1)+X-A(J),1.)-.5
          V=AMOD(A(L+2)+Y-A(J+1),1.)-.5
          W=AMOD(A(L+3)+Z-A(J+2),1.)-.5
          IF(A(8)*U*U+A(9)*V*V+A(10)*W*W+A(11)*V*W+
     +    A(12)*U*W+A(13)*U*V.LT.A(43))GOTO 6
   4      CONTINUE
        GOTO 3
   5    NN=NN+3
        A(NN)=X
        A(NN+1)=Y
        A(NN+2)=Z
   6    CONTINUE
      S=-S
      IF(S+A(23)+.5)2,1,1
C
C CONSIDER ALL VECTORS AS POSSIBLE CROSS VECTORS
C
   7  NP=NN-1
      NI=NP
      I=LH+8
   8  I=I+8
      IF(I.GT.LD)GOTO 19
      O=SQRT(A(I+6))
      J=NK
   9  J=J+3
      IF(J.GT.NN)GOTO 8
      S=1.
  10  X=A(J)+S*A(I+2)
      Y=A(J+1)+S*A(I+3)
      Z=A(J+2)+S*A(I+4)
C
C TEST AGAINST ATOM LISTS
C
        DO 14 L=LJ,LL,4
          DO 14 M=65,LY,12
          U=A(L+1)+A(L)*(X*A(M)+Y*A(M+1)+Z*A(M+2)+A(M+9))
          V=A(L+2)+A(L)*(X*A(M+3)+Y*A(M+4)+Z*A(M+5)+A(M+10))
          W=A(L+3)+A(L)*(X*A(M+6)+Y*A(M+7)+Z*A(M+8)+A(M+11))
          N=LV
  11      N=N+8
          IF(N.GT.LX)GOTO 12
          R=AMOD(W-A(N+4),1.)-.5
          IF(ABS(R).GT.A(29))GOTO 11
          P=AMOD(U-A(N+2),1.)-.5
          Q=AMOD(V-A(N+3),1.)-.5
          IF(A(8)*P*P+A(9)*Q*Q+A(10)*R*R+A(11)*Q*R+
     +    A(12)*P*R+A(13)*P*Q-A(43))16,11,11
  12      N=NP
  13      N=N+4
          IF(N.GT.NI)GOTO 14
          R=AMOD(W-A(N+2),1.)-.5
          IF(ABS(R).GT.A(29))GOTO 13
          P=AMOD(U-A(N),1.)-.5
          Q=AMOD(V-A(N+1),1.)-.5
          IF(A(8)*P*P+A(9)*Q*Q+A(10)*R*R+A(11)*Q*R+
     +    A(12)*P*R+A(13)*P*Q.GT.A(43))GOTO 13
          A(N+3)=A(N+3)+O
          H=O/A(N+3)
          A(N)=A(N)+P*H
          A(N+1)=A(N+1)+Q*H
          A(N+2)=A(N+2)+R*H
          GOTO 16
  14      CONTINUE
      IF(NI.GT.LM-7)GOTO 17
      NI=NI+4
      IF(LR.LT.NI+3)LR=NI+3
      M=NI
  15  A(M)=AMOD(X+99.,1.)
      A(M+1)=AMOD(Y+99.,1.)
      A(M+2)=AMOD(Z+99.,1.)
      A(M+3)=O
  16  S=-S
      IF(A(23)-S-1.5)9,10,10
  17  Q=9.E9
      N=NP
  18  N=N+4
      IF(N.GT.NI)GOTO 15
      IF(Q.LT.A(N+3))GOTO 18
      M=N
      Q=A(N+3)
      GOTO 18
C
C SORT POTENTIAL ATOMS
C
  19  L=(NN-NK)/3
      IF(L.EQ.0)GOTO 54
      NM=NN+MIN0((LM-NN-194)/(L+1),4*(INT(A(22))/L))
  20  Q=0.
      N=0
      I=NP
  21  I=I+4
      IF(I.GT.NI)GOTO 22
      IF(Q.GT.A(I+3))GOTO 21
      Q=A(I+3)
      N=I
      GOTO 21
  22  IF(Q.LT..01)GOTO 24
      NP=NP+4
      U=A(N)
      V=A(N+1)
      W=A(N+2)
      T=A(N+3)
      A(N)=A(NP)
      A(N+1)=A(NP+1)
      A(N+2)=A(NP+2)
      A(N+3)=A(NP+3)
C
C SPECIAL POSITIONS
C
      H=0.
      X=0.
      Y=0.
      Z=0.
        DO 23 K=65,LY,12
        XX=U*A(K)+V*A(K+1)+W*A(K+2)+A(K+9)
        YY=U*A(K+3)+V*A(K+4)+W*A(K+5)+A(K+10)
        ZZ=U*A(K+6)+V*A(K+7)+W*A(K+8)+A(K+11)
          DO 23 M=LJ,LL,4
          P=AMOD(A(M+1)+A(M)*XX-U,1.)-.5
          Q=AMOD(A(M+2)+A(M)*YY-V,1.)-.5
          R=AMOD(A(M+3)+A(M)*ZZ-W,1.)-.5
          IF(A(8)*P*P+A(9)*Q*Q+A(10)*R*R+A(11)*Q*R+A(12)*P*R+
     +    A(13)*P*Q.GT.A(43))GOTO 23
          H=H+1.
          X=X+P
          Y=Y+Q
          Z=Z+R
  23      CONTINUE
      H=1./H
      A(NP)=AMOD(U+X*H+99.,1.)
      A(NP+1)=AMOD(V+Y*H+99.,1.)
      A(NP+2)=AMOD(W+Z*H+99.,1.)
      A(NP+3)=T
      IF(NP.LT.NM)GOTO 20
C
C GENERATE INITIAL-POTENTIAL VECTOR TABLE
C
  24  NI=NP
      IF(LD.EQ.0)GOTO 32
      I=NN-1
  25  I=I+4
      IF(I.GT.NP)GOTO 38
      J=NK
  26  J=J+3
      IF(J.GT.NN)GOTO 25
      NI=NI+4
      A(NI)=0.
      A(NI+1)=A(I)-A(J)
      A(NI+2)=A(I+1)-A(J+1)
      A(NI+3)=A(I+2)-A(J+2)
      GOTO 26
C
C SELECT BEST POTENTIAL ATOMS
C
  27  NK=INT(2.1-A(23))*((LY-53)/12)
      NM=LV+144+4*MIN0(16,INT(.5*(SQRT(1.+
     +AMIN1(FLOAT(LM-LV-260)*2.,8.*AINT(A(22)))/FLOAT(NK))-3.)))
      NI=LV+140
      J=LV
  28  J=J+8
      IF(J.GT.LX)GOTO 29
      NI=NI+4
      A(NI)=A(J+2)
      A(NI+1)=A(J+3)
      A(NI+2)=A(J+4)
      A(NI+3)=0.
      GOTO 28
  29  LD=0
      K=NP
      N=NP
      NP=NI
      J=NN-1
  30  J=J+4
      IF(J.GT.N)GOTO 20
      NI=NI+4
      A(NI)=A(J)
      A(NI+1)=A(J+1)
      A(NI+2)=A(J+2)
      A(NI+3)=9.E9
        DO 31 I=1,L
        K=K+4
  31    A(NI+3)=AMIN1(A(NI+3),A(K)*H)
      GOTO 30
C
C GENERATE TRIANGULAR VECTOR TABLE
C
  32  I=LV+140
  33  I=I+4
      IF(I.GT.NP)GOTO 38
      X=A(I)
      Y=A(I+1)
      Z=A(I+2)
      P=9.E9
      J=LV+140
  34  J=J+4
      IF(J.GT.I)GOTO 33
      S=1.
  35    DO 37 K=65,LY,12
        NI=NI+4
        A(NI)=0.
        A(NI+1)=AMOD(99.5+S*(X*A(K)+Y*A(K+1)+Z*A(K+2)+
     +  A(K+9))-A(J),1.)-.5
        A(NI+2)=AMOD(99.5+S*(X*A(K+3)+Y*A(K+4)+Z*A(K+5)+
     +  A(K+10))-A(J+1),1.)-.5
        A(NI+3)=AMOD(99.5+S*(X*A(K+6)+Y*A(K+7)+Z*A(K+8)+
     +  A(K+11))-A(J+2),1.)-.5
        IF(J.EQ.I)GOTO 37
          DO 36 L=LJ,LZ,4
          U=AMOD(A(NI+1)+A(L+1),1.)-.5
          V=AMOD(A(NI+2)+A(L+2),1.)-.5
          W=AMOD(A(NI+3)+A(L+3),1.)-.5
          R=A(8)*U*U+A(9)*V*V+A(10)*W*W+A(11)*V*W+
     +    A(12)*U*W+A(13)*U*V
          IF(R.GT.P)GOTO 36
          P=R
          A(I)=U+A(J)
          A(I+1)=V+A(J+1)
          A(I+2)=W+A(J+2)
  36      CONTINUE
  37    CONTINUE
      S=-S
      IF(S+A(23)+.5)35,34,34
C
C EVALUATE E*F PATTERSON AT SPECIFIC POINTS
C
  38  IF(NI.LE.NP)GOTO 54
      JZ=NI+4
      NJ=NP+4
      H=0.
  39  READ(LA)F
        DO 45 I=1,124,3
        IF(.5.GT.F(I))GOTO 49
        IF(F(I+1).LT.0.)GOTO 45
        CALL SXH2(F(I),X,Y,Z)
        S=0.
        N=NI
          DO 43 K=65,LY,12
          P=AINT(1.001*(X*A(K)+Y*A(K+3)+Z*A(K+6)))
          Q=AINT(1.001*(X*A(K+1)+Y*A(K+4)+Z*A(K+7)))
          R=AINT(1.001*(X*A(K+2)+Y*A(K+5)+Z*A(K+8)))
          W=P+200.*(Q+200.*R)
          T=ABS(W)
          J=NI
  40      J=J+4
          IF(J.GT.N)GOTO 41
          IF(.5-ABS(A(J+3)-T))40,40,42
  41      N=N+4
          A(N)=P
          A(N+1)=Q
          A(N+2)=R
          A(N+3)=T
  42      IF(A(23).LT..5)W=T
  43      IF(ABS(F(I)-W).LT..5)S=S+1.
        P=F(I+1)
        IF(KZ.EQ.1)P=P*P
        IF(KZ.EQ.2)P=P*F(I+2)*SQRT(S)
          DO 44 J=JZ,N,4
          X=A(J)*6.283185
          Y=A(J+1)*6.283185
          Z=A(J+2)*6.283185
          H=H+P
C
C ** VERY CRITICAL LOOP **
C
            DO 44 K=NJ,NI,4
  44        A(K)=A(K)+P*COS(X*A(K+1)+Y*A(K+2)+Z*A(K+3))
  45    CONTINUE
      GOTO 39
C
C OUTPUT CROSSWORD
C
  46  FORMAT(/F7.3,2F6.3,F8.4,17F6.1)
  47  FORMAT(27X,17F6.2)
  48  FORMAT(///' PATTERSON MINIMUM FUNCTIONS AND DISTANCES FOR ',
     +40A1///'     X     Y     Z   1/MULT  SELF  CROSS-VECTORS')
  49  H=999./H
      REWIND LA
      IF(LD.GT.0)GOTO 27
      WRITE(LI,48)IT
      NN=(NP-LV-140)/4
      N=NP
      NJ=LV+144
        DO 53 I=NJ,NP,4
        M=13
          DO 51 J=NJ,I,4
          Q=0.
          M=M+1
          F(M)=9.E9
          S=.25*A(43)
          T=A(28)*A(28)
            DO 50 K=1,NK
            N=N+4
            F(M)=AMAX1(0.,AMIN1(F(M),A(N)*H))
              DO 50 L=LJ,LZ,4
              U=AMOD(A(N+1)+A(L+1),1.)-.5
              V=AMOD(A(N+2)+A(L+2),1.)-.5
              W=AMOD(A(N+3)+A(L+3),1.)-.5
              R=A(8)*U*U+A(9)*V*V+A(10)*W*W+A(11)*V*W+
     +        A(12)*U*W+A(13)*U*V
              IF(R.LT.S)GOTO 50
              T=AMIN1(T,R)
              Q=Q-1.
  50          Q=Q+1.
          LD=(I-NJ)/4
          L=(J-NJ)/4
          LH=JZ+LD+L*NN
          A(LH)=F(M)
          IF(T.LT.1.)A(LH)=0.
          K=JZ+L+LD*NN
          A(K)=A(LH)
          IF(A(23).GT..5)GOTO 51
          IF(LD.EQ.L)A(K)=2.*A(K)
  51      F(M+60)=T
        F(13)=F(M)
        A(I+3)=1./Q
        M=M-1
        WRITE(LI,46)A(I),A(I+1),A(I+2),A(I+3),(F(L),L=13,M)
        M=M+61
          DO 52 L=74,M
  52      F(L)=SQRT(F(L))
        F(73)=F(M)
        IF(F(73).GT.1.E4)F(73)=0.
        M=M-1
  53    WRITE(LI,47)(F(L),L=73,M)
      IF(LR.LT.LH)LR=LH
      WRITE(LI,55)LR
      CALL SXTM(SL,LI)
  54  CALL SXIT
  55  FORMAT(//' MAXIMUM MEMORY FOR PATTERSON ',
     +'INTERPRETATION =',I6)
C
C GENERATE ATOM NAMES AND LIST ATOMS
C
  56  FORMAT(///' TENTATIVE HEAVY ATOM ASSIGNMENT FOR ',40A1/)
  57  FORMAT(' NAME SFAC   X      Y      Z     S.O.F.',
     +'    SELFMF   SUPMF'/)
  58  FORMAT(1X,4A1,I4,3F7.3,F9.4,2F9.1)
  59  WRITE(LI,56)IT
      WRITE(LI,57)
      A(31)=FLOAT((LX-LV)/8)+.1
      IF(A(LV+9).LT.9.5)GOTO 60
      IF(LX.LT.LV+9)GOTO 60
      IF(A(LX+1).LT.9.5)A(31)=A(31)-1.
  60  N=LV
  61  N=N+8
      NN=INT(A(N+1)*1.0001)
      J=LL-1
  62  J=J+5
      IF(INT(A(J)+.1).NE.NN)GOTO 62
      Y=A(J)
      A(N+1)=200.*FLOAT(J-LL+1)+.1
      NN=INT(.001*A(N+1))
      CALL SXUW(A(J+2),ID)
      J=1
      K=2
      IF(ID(2).NE.IH(20))K=3
      NJ=K
      ID(3)=IH(20)
      ID(4)=IH(20)
      L=2
  63  ID(K)=IH(L)
      CALL SXPW(X,ID)
      M=LV
  64  M=M+8
      IF(M.GE.N)GOTO 65
      IF(ABS(A(M)-X).GT..5)GOTO 64
      L=L+1
      IF(L.LT.11)GOTO 63
      J=J+1
      ID(NJ)=IH(J)
      K=NJ+1
      L=1
      GOTO 63
  65  A(N)=X
      J=N+2
      K=N+7
      CALL SXUW(X,ID)
      WRITE(LI,58)ID,NN,(A(I),I=J,K)
      IF(N.LT.LX)GOTO 61
      A(54)=99.
      WRITE(LI,55)LR
      RETURN
      END
