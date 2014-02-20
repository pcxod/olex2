import os
import sys
import shutil
import re
import olex
import olx
from olexFunctions import OlexFunctions
OV = OlexFunctions()

'''
To run this script, type spy.OlexPlaton("help") in Olex2
'''

def platon(command):
  try:
    platon_result = olx.Exec(command)
    #platon_result = os.popen("platon %s%s %s"%(tickornot, platonflag, inputfilename)).read()
  except:
    print "Platon failed to run"
    return

def OlexPlaton(platonflag="0"):
  print "Olex2 to Platon Linker"
  print "You are running flag: %s"%(platonflag)
  inputfilename = OV.FileName()
  print "Input file is: ", inputfilename

  platonflagcodes = {
  'a' : ["ORTEP/ADP [PLOT ADP]", "lis"], 
  'b' : ["CSD-Search [CALC GEOM CSD]","lis"],
  'c' : ["Calc Mode [CALC]", "lis"],
  'd' : ["DELABS [CALC DELABS]", "lis"],
  'e' : ["MULABS", ""],
  'f' : ["HFIX", "pjn"],
  'g' : ["GenRes:filter [CALC GEOM SHELX]" , "lis"],
  'h' : ["HKL-CALC [ASYM GENERATE]" , "lis"],
  'i' : ["Patterson PLOT" , "eld"],
  'j' : ["GenSPF-filter [CALC GEOM EUCLID]" , "lis"],
  'k' : ["HELENA" , ""],
  'l' : ["ASYM VIEW" , "res"],
  'm' : ["ADDSYM (MISSYM) [CALC ADDSYM]" , "lis"],
  'n' : ["ADDSYM SHELX" , "res"],
  'o' : ["Menu Off" , ""],
  'p' : ["PLUTON Mode" , "pjn"],
  'q' : ["SQUEEZE [CALC SQUEEZE]" , "lis"],
  'r' : ["RENAME (RES)" , "res"],
  's' : ["SYSTEM-S" , ""],
  't' : ["TABLE Mode [TABLE]" , "sup"],
  'u' : ["Validation Mode [VALIDATION]" , "chk"],
  'v' : ["SOLV Mode [CALC SOLV]" , "lis"],
  'w' : ["Difference Map Plot" , "eld"],
  'x' : ["Fo-Map PLOT" , "eld"],
  'y' : ["SQUEEZE-Map PLOT" , "eld"],
  'z' : ["WRITE IDENT" , ""],
  'A' : ["PLATON/ANIS" , "pjn"],
  'C' : ["GENERATE CIF for current data set (e.g. .spf or .res)" , "acc"],
  'F' : ["SILENT NQA SYSTEM-S PATH (FILTER)" , "log"],
  'I' : ["AUTOFIT 2 MOLECULES" , "lis"],
  'K' : ["CALC KPI" , "lis"],
  'L' : ["TWINROTMAT (INTERACTIVE)" , ""],
  'M' : ["TWINROTMAT (FILTER MODE)" , ""],
  'N' : ["'ADDSYM EQUAL SHELX' MODE" , "res"],
  'O' : ["PLOT ADP (PostScript)" , "lis"],
  'P' : ["Powder Pattern from Iobs" , "cpi"],
  'Q' : ["Powder Pattern from Icalc" , "cpi"],
  'R' : ["Auto Renumber and Write SHELX.res" , "res"],
  'S' : ["CIF2RES + FCF2HKL filter" , ""],
  'T' : ["TwinRotMat" , ""],
  'U' : ["CIF-VALIDATION (without VALIDATION DOC)" , "chk"],
  'V' : ["FCF-VALIDATION (LAUE)" , "ckf"],
  'W' : ["FCF-VALIDATION (BIJVOET)" , "ckf"],
  'X' : ["Stripped SHELXS86 (Direct Methods Only) Mode" , ""],
  'Y' : ["Native Structure Tidy (Parthe & Gelato) Mode" , ""]
  }
  
  # OS Checking
  if sys.platform[:3] == 'lin':
    # Risky but assuming that this is a debroglie version of platon
    tickornot = '-'
  elif sys.platform[:3] == 'win':
    # Windows
    tickornot = '-o -'
  elif sys.platform[:3] == 'dar':
    # Mac assuming like windows
    tickornot = '-o -'
    
  # Checking for help string
  if len(platonflag) > 1 or platonflag == "help":
    print "Unknown option, please check options and try again"
    #if platonflag == help: # If no options given then we print all the possible commands out LOL
    # Prefered printing out the raw text but using the dict again is logical but it does not print in order anymore :-(
    for key in platonflagcodes:
      print " '%s' - %s"%(key, platonflagcodes[key][0])
    return
  else:
    if platonflag == "0":
      # Start just platon with the INS file
      print "Calling Platon Directly"
      command = "platon %s.ins"%(inputfilename)
      platon(command)
      return
    else:
      if platonflag == 'U':
        print "This option requires a valid CIF file - checking"
        # Check for CIF
        try:
          cifornot = open("%s.%s"%(OV.FileName(), cif), 'r')
          cifornot.close()
        except:
          print "No CIF present - why not make one with ACTA?"
          print "Or run spy.OlexPlaton(C) and rename the %s.acc to %s.cif?"%(OV.FileName(), OV.FileName())
        inputfilename = OV.FileName() + '.cif'
      command = "platon %s%s %s"%(tickornot, platonflag, inputfilename)
      platon(command)
    # Old code works for Linux but not windows thanks to the stupid vritual cmdline built into Platon by LF
    #  platon_extension = platon_result.split(":")[-1].split(".")[-1].split("\n")[0]
    # To compensate now check flag against dictionary and then use that file extension, predominantly this is going to be lis
      platon_extension = platonflagcodes[platonflag][1]
      print "The file extension is: ", platon_extension, " filename is: ", "%s.%s"%(OV.FileName(), platon_extension)
      try:
        platon_result_file = open("%s.%s"%(OV.FileName(), platon_extension), 'r')
        print "Successfully opened file", platon_result_file
        for platon_line in platon_result_file:
          print platon_line
        platon_result_file.close()
      except IOError: 
        print "Failed to open file"
      print "You can read this file by typing:"
      print "edit %s"%(platon_extension)
      return

OV.registerFunction(OlexPlaton)
"""
    print " 'a' - ORTEP/ADP [PLOT ADP]"
    print " 'b' - CSD-Search [CALC GEOM CSD]"
    print " 'c' - Calc Mode [CALC]"
    print " 'd' - DELABS [CALC DELABS]"
    print " 'e' - MULABS"
    print " 'f' - HFIX"
    print " 'g' - GenRes-filter [CALC GEOM SHELX]"
    print " 'h' - HKL-CALC [ASYM GENERATE]"
    print " 'i' - Patterson PLOT"
    print " 'j' - GenSPF-filter [CALC GEOM EUCLID]"
    print " 'k' - HELENA"
    print " 'l' - ASYM VIEW"
    print " 'm' - ADDSYM (MISSYM) [CALC ADDSYM]"
    print " 'n' - ADDSYM SHELX"
    print " 'o' - Menu Off"
    print " 'p' - PLUTON Mode"
    print " 'q' - SQUEEZE [CALC SQUEEZE]"
    print " 'r' - RENAME (RES)"
    print " 's' - SYSTEM-S"
    print " 't' - TABLE Mode [TABLE]"
    print " 'u' - Validation Mode [VALIDATION]"
    print " 'v' - SOLV Mode [CALC SOLV]"
    print " 'w' - Difference Map Plot"
    print " 'x' - Fo-Map PLOT"
    print " 'y' - SQUEEZE-Map PLOT"
    print " 'z' - WRITE IDENT"
    print " 'A' - PLATON/ANIS"
    print " 'C' - GENERATE CIF for current data set (e.g. .spf or .res)"
    print " 'F' - SILENT NQA SYSTEM-S PATH (FILTER)"
    print " 'I' - AUTOFIT 2 MOLECULES"
    print " 'K' - CALC KPI"
    print " 'L' - TWINROTMAT (INTERACTIVE)"
    print " 'M' - TWINROTMAT (FILTER MODE)"
    print " 'N' - 'ADDSYM EQUAL SHELX' MODE"
    print " 'O' - PLOT ADP (PostScript)"
    print " 'P' - Powder Pattern from Iobs"
    print " 'Q' - Powder Pattern from Icalc"
    print " 'R' - Auto Renumber and Write SHELX.res"
    print " 'S' - CIF2RES + FCF2HKL filter"
    print " 'T' - TwinRotMat"
    print " 'U' - CIF-VALIDATION (without VALIDATION DOC)"
    print " 'V' - FCF-VALIDATION (LAUE)"
    print " 'W' - FCF-VALIDATION (BIJVOET)"
    print " 'X' - Stripped SHELXS86 (Direct Methods Only) Mode"
    print " 'Y' - Native Structure Tidy (Parthe & Gelato) Mode"
"""
"""
call with print platonflagcodes['letter'][0]

99999 FORMAT (/,
     1        ':: POV-Ray File on :', A, '.pov')
99998 FORMAT (':: SQUEEZE  out on :', A, '.hkp')
99997 FORMAT (':: OMEGA File   on :', A, '.ome')
99996 FORMAT (':: MOGLI Files  on :', A, '.dge')
99995 FORMAT (':: SPF File     on :', A, '.eld')
99994 FORMAT (':: CSD-QUE      on :', A, '.que')
99993 FORMAT (':: PUBL. Tables on :', A, '.pub')
99992 FORMAT (':: SUPP. Mat.   on :', A, '.sup')
99991 FORMAT (':: CIF/CSD-File on :', A, '.csd')
99990 FORMAT (':: CIF/ACC-File on :', A, '.acc')
99989 FORMAT (':: SAR-File     on :', A, '.sar')
99988 FORMAT (':: SHELXL Style Output on :', A, '.res')
99987 FORMAT (':: FCF-CIF  hkl on :', A, '.hkp')
99985 FORMAT (':: SPGR.PAR     on :', A, '.par')
99984 FORMAT (':: HKLF3.HKL    on :', A, '.hkp')
99983 FORMAT (':: ABSGAUSS hkl on :', A, '.hkp')
99982 FORMAT (':: ABSTOMPA hkl on :', A, '.hkp')
99981 FORMAT (':: ASYM     hkl on :', A, '.hkp (# refl. =', I7, ')')
99980 FORMAT (':: ABSPSI   hkl on :', A, '.hkp')
99979 FORMAT (':: ABSSPHER hkl on :', A, '.hkp')
99978 FORMAT (':: PSIDIR   hkl on :', A, '.hkp')
99977 FORMAT (':: PDB-FILE out on :', A, '.pdb')
99976 FORMAT (':: SQUEEZE  xyz on :', A, '.sqz', /,
     1        ':: SQUEEZE  CIF on :', A, '.sqf')
99975 FORMAT (/, ':: Modified SHELX-File on ', A, '.new')
99974 FORMAT (/,
     1        ':: MOGLI   File on :', A, '.dge')
99973 FORMAT (/,
     1        ':: Journal File on :', A, '.pjn', //,
     2        ':: Normal End of PLATON/PLUTON RUN.')
99972 FORMAT (':: MULABS   hkl on :', A, '.hkp')
99971 FORMAT (':: CHECK    out on :', A, '.chk')
99970 FORMAT (':: HKLTRANS hkl on :', A, '.hkp')
99969 FORMAT (':: RASMOL(pdb)  on :', A, '.ras')
99967 FORMAT (':: HKLF4.HKL    on :', A, '.hkp')
99966 FORMAT (':: ASYM    -hkl on :', A, '.hks (# refl. =', I7, ')')
99965 FORMAT (':: POWDER   cpi on :', A, '.cpi')
99964 FORMAT (':: HKLF5.HKL    on :', A, '.hkp')
99963 FORMAT (':: SHXABS   hkl on :', A, '.hkp')
## 99962 FORMAT (':: FCF-CHK  out on :', A, '.ckf')
99961 FORMAT (A)
99960 FORMAT (':: Expanded coordinate set (shelx-style) on :', A)
99959 FORMAT (':: Fourier3D    on :', A, '.fou')
99958 FORMAT (':: Solv3D       on :', A, '.slv')
99957 FORMAT (':: Flip Results on: ', A, '_flp.res',
     1        ' - Concatenation of Flip-maps', /, 20X,
     2        A, '_sol.res - Concatenation of Solutions', /, 20X,
     3        A, '_res.res - Best Solution')
99956 FORMAT (':: DifFourPeaks on :', A, '.dif')
99955 FORMAT (20X, A, '_res.new - Updated version of ', A, '_res.res')
99954 FORMAT (A,'_res.new')
"""
