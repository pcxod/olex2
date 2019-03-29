import os
import sys
import shutil
import re
import olex
import olx
from olexFunctions import OlexFunctions
OV = OlexFunctions()

'''
To run this example script, type spy.OlexSir() in Olex2
'''

def OlexSir(level="normal", ManAtomContents="C 20 H 20 O 10", rhomax="0.25"):
  print "This script takes an exsiting file and pushes it into SIR"
  # Experimenting with patterson function but having issues using the partial command to work?
  patterson = 0

  # We can assume that the INS name and information can come from Olex2
  # In fact probably won't need this as I only need Cell, SpaceGroup, Content (SFAC/UNIT), reflection)
  """
  SIR97 - Altomare A., Burla M.C., Camalli M., Cascarano G.L., Giacovazzo C. , Guagliardi A., Moliterni A.G.G., Polidori G.,Spagna R. (1999) J. Appl. Cryst. 32, 115-119.
  """

  SirLevel = {
  'normal' : '%invariants\n%phase\n%fourier\n',
  'hard' : '%invariants\n%phase\n\tMinfom 0.9\n\tRandom\n%fourier\n',
  'harder' : '%invariants\n\tCochran\n%phase\n\tRandom\n%fourier\n',
  'difficult' : '%invariants\n\tCochran\n%phase\n\tSeed 9437\n\tRandom 250\n%fourier\n',
  'bonkers' : '%invariants\n\tCochran\n%phase\n\tSeed 8793\n\tRandom 500\n%fourier\n'
  }
  sirversion = 97
  rhomax = 0.250 #default for sintheta/lambda 0.5
  Olex2SirIn = OV.FileName()
  SirCompatCell = ''.join(olx.xf_au_GetCell().split(','))
  brokensym = list(olx.xf_au_GetCellSymm())
  SirCompatSymmSetting = brokensym.pop(0)
  SirCompatSymmOpps = []
  while len(brokensym) > 0:
    print brokensym
    print SirCompatSymmOpps
    if brokensym[0] == '-':
      print "true for -"
      SirCompatSymmOpps.append(" "+brokensym.pop(0))
      SirCompatSymmOpps.append(brokensym.pop(0))
      continue
    elif brokensym[0] in "a b c d n m":
      print "true for letter"
      SirCompatSymmOpps.append(" "+brokensym.pop(0))
      continue
    elif brokensym[0] in "1":
      print "true for number"
      SirCompatSymmOpps.append(brokensym.pop(0))
      continue
    elif brokensym[0] in "2 3 4 6 8 9":
      print "true for number"
      SirCompatSymmOpps.append(" "+brokensym.pop(0))
      continue
    elif brokensym[0] in "/":
      print "true for number"
      SirCompatSymmOpps.append(" "+brokensym.pop(0))
      continue
    print brokensym
    print SirCompatSymmOpps
  print ''.join(SirCompatSymmOpps)
  OlexZ = int(olx.xf_au_GetZ())
  AtomPairs = olx.xf_GetFormula().split()
  print AtomPairs
  AtomGroups = []
  CorrectedAtoms = []
  for atom in AtomPairs:
    AtomGroups.append(re.split("([A-Za-z]*)",atom)[1:3])
  print AtomGroups
  for j in range(0, len(AtomGroups)):
    print AtomGroups[j][0], AtomGroups[j][1]
    CorrectedAtoms.append("%s %s"%(AtomGroups[j][0], (AtomGroups[j][1])))
  AtomContents = ' '.join(CorrectedAtoms)
  print AtomContents
  print "ETF", AtomContents
  #AtomContents = ' '.join(re.split("([A-Za-z]*)",olx.xf_GetFormula()))
  snuff = re.split("([A-Za-z]*)",olx.xf_GetFormula())

  NumPeaks = int(OlexZ) * int(olx.xf_au_GetAtomCount())
  CellV = float(olx.xf_au_GetCellVolume())
  if ManAtomContents == " " or ManAtomContents != "C 20 H 20 O 10":
# Ok allowing the manual input of a formula is great but we really need to check it and make sure it is in the correct format, and or take a couple of formats and convert them
    print "Using user input formula", ManAtomContents
    AtomContents = ManAtomContents

  print "Job name", Olex2SirIn
  print "Unit Cell", SirCompatCell
  print "Olex2 Symmetry:Sir Symmetry", olx.xf_au_GetCellSymm(), ':',   SirCompatSymmSetting, ' '.join(SirCompatSymmOpps)
  print "Olex2 Formula", olx.xf_GetFormula()
  print "Sir Friendly Formula", AtomContents
  print "New formula using Z", OlexZ,
  print "Number of atoms to look for", NumPeaks, 'or', float(CellV/18.0)
# There is an issue with ever decreasing returns from Olex2 and SIR.
# Basically Olex2 is updating the SFAC from SIR results this then is being posted back to if you try again
# NET result is you end up with NO ATOMS!
# With the idea of just taking atoms from the produced INS we can get around this

# Creating our SIR file
# This is primative will need to add features such as patterson on and off
  SIRINS = open("%s.sir"%(Olex2SirIn), 'w')
  SIRINS.write("%window\n")
  SIRINS.write("%%Structure sir%s\n"%(sirversion))
  SIRINS.write("%%job   %s in %s\n"%(Olex2SirIn, olx.xf_au_GetCellSymm()))
  SIRINS.write("%Init\n")
  SIRINS.write("%Data\n")
  SIRINS.write("\tCell %s\n"%(SirCompatCell))
  SIRINS.write("\tSpaceGroup %s %s\n"%(SirCompatSymmSetting, "".join(SirCompatSymmOpps)))
  SIRINS.write("\tContent %s\n"%(AtomContents))
  SIRINS.write("\tRhomax %s\n"%(rhomax))
  SIRINS.write("\tReflections %s.hkl\n"%(Olex2SirIn))
  SIRINS.write("\tFosquare\n")
  SIRINS.write("%normal\n")
  SIRINS.write("%s"%(SirLevel[level]))
  SIRINS.write("%export\n")
  SIRINS.write("\tshelx  sir%s.res\n"%(sirversion))
  SIRINS.write("%end\n")
  SIRINS.close()

# All this need error control
  content = os.popen("sir%s %s.sir "%(sirversion, Olex2SirIn)).read() # This pipes our new .sir file into sir using sirversion can use 92/97 etc
  print content # Output from pipe need proper error control here

# File generated from SIR - name controlled by the Structure line in .sir file - made sir to prevent overwrite of INS
  src = "sir%s.res"%sirversion

# We are going to push our sir result into our res file
  dst = Olex2SirIn + '.res'

# This needs rewriting to reform the orginal Information in the INS/RES file
# We have lost the:
# cell ESD parameters
# the HKL file used
# the orginal title
# This should all be possible to be either prestripped saved and returned
# This needs to be done before the replacement. Or we should just replace the atoms assuming SIR puts the list in the same order?
# This works fine in Linux not certain in windows?
  shutil.copy2(src, dst)

# Get Olex2 to reload the res file and update our display
  olx.Atreap(dst)

OV.registerFunction(OlexSir)
