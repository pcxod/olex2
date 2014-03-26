#!/usr/bin/python2
version = "110310"
"""
=====================================================================
           Submit charge flipping phasing procedure
           -----------------------------------------

 Charge Flipping for ab initio small-molecule structure determination
          A van der Lee, C.Dumas & L. Palatinus   (python version february 08th,  2010)
          corrected bug in "forcesymmetry=yes" processing ins symcards.

 prepares input file for SUPERFLIP and EDMA program  (L. Palatinus)


--------------------------------------------------------------------

 Input files:
-------------
       Shelx type INS file with cell info and scattering factor types
            (space group symmetry is NOT read)
 IMPORTANT: the cell parameters should be constrained to their lattice symmetry
            with only a very small tolerance
       Shelx type HKL file with the same generic name as the INS file

 output file:
--------------
       Shelx-type RES-file with atomic coordinates and symmetry information

==================================================================
"""

import fileinput, string, sys, os, re
from os.path import join


def process_command_line_arg(s,flip_keywords,files):
# help
    if ( len(s) == 1 ) or ( s[-1] == '-h' ) or ( s[-1] == '--help' ):
       sys.stdout.write("""
###############
Some guidelines
###############

use: %s data.ins
 or: %s data.ins  maxcycles=30000

where:
data.ins is input shelx-file with only cell info and scattering type info, optionally space group info

optional:
trial=5          ...... number of repeated trials (default 1)
normalize=no     ...... locally normalize the data (default yes)
maxcycl=30000    ...... maximum number of cycles per trial (default 10000)
forcesymmetry=yes...... use symmetry from Shelx-file, that should obviously be present (default no)
weak=0.1         ...... fraction of reflections considered to be weak (default 0.2)
ked=1.3          ...... user-defined flip-threshold delta=ked*sigma(map) (default: delta=auto)
superposition=yes...... start with minimal superposition map (default no)
edmacontinue=no  ...... continue if convergence is not detected (default no)
comments=yes     ...... have intermediate stops to check results (default=yes)
cleanup=yes      ...... remove all intermediate files, keep only .res (default=yes)

""" % (s[0],s[0]))
       print 'Start again'
       return False

# Process command line arguments
    insfile_defined = False
    for i in range(1,len(s)):
       if  ".ins" in s[i]:
          if  os.path.isfile(s[i]):
            files['insin']=s[i]
            files['base']=os.path.splitext(files['insin'])[0]
            files['hklin']=files['base']+".hkl"
            if  not os.path.isfile(files['hklin']):
                print "%s does not exist" % files['hklin']
                sys.exit(1)
            files['inflip']=files['base']+".inflip"
            files['m81']=files['base']+".m81"
            files['fliplog']=files['base']+".sflog"
            insfile_defined = True
          else:
            print "%s does not exist" % s[i]
            return False

       for k, v in flip_keywords.items():
          if  k+"=" in s[i]:
            flip_keywords[k]=string.split(s[i],"=")[1]

    if not insfile_defined:
       if olexrun:
          print "There should be at least the name of an existing ins-file as command line argument.\nTry again or do 'spy.flipsmall(--help)' to get help."
       else:
          print "There should be at least the name of an existing ins-file as command line argument.\nTry again or do 'flipsmall.py --help' to get help."
       return False

    if ( flip_keywords['normalize'] == "yes" ):
       flip_keywords['biso']=0.0
       flip_keywords['normalize']="local"

    if ( flip_keywords['forcesymmetry'] == "no" ):
       flip_keywords['derivesymmetry'] = "use"
    else:
       flip_keywords['derivesymmetry'] = "yes"

    if olexrun:
       flip_keywords['logging'] = "no"
       flip_keywords['comments'] = "no"

    exe['SF'] = flip_keywords['sflipversion']

    if  ( flip_keywords['logging'] == "yes" ): log=open('/home/vdlee/unix/python/log.txt', "a")

    return True





def process_insfile(flip_keywords,files,derived_info):
    nsymm = 0
    nsfac = 0
    derived_info['latt'] = 0
    for line in fileinput.input(files['insin']):
       if "CELL" in line:
          derived_info['cell'] = string.join(string.split(line)[2:])
          derived_info['wavelength'] = string.split(line)[1]
       if "LATT" in line: derived_info['latt'] = int(string.split(line)[1])
       if "SYMM" in line: nsymm = nsymm + 1
       if "SFAC" in line: nsfac = nsfac + 1
       if "ZERR" in line:
          derived_info['zerr'] = string.split(line)[1]
          derived_info['cellesd'] = string.join(string.split(line)[2:])

#Determine crystal system from lattice metrics
    c=string.split(derived_info['cell'])
    delta=0.001
    derived_info["crsyst"]="tric"
    c=map(float,c)
    if ( abs(c[3] - 90.00)/90.00 < delta ) and ( abs(c[4]-90.00) / 90.00 < delta ) and ( abs(c[5]-90.00) / 90.00 < delta ) \
    and ( abs((c[0]-c[1]) / c[0]) < delta ) and ( abs((c[0]-c[2]) / c[0]) < delta ) \
    and ( abs((c[1]-c[2]) / c[0]) < delta ):
        derived_info['crsyst']='cubi'
        derived_info['flipcell'] = str(c[0])+" "+str(c[0])+" "+str(c[0])+" 90.00 90.00 90.00"
    if ( abs(c[3] - 90.00)/90.00 < delta ) and ( abs(c[4]-90.00) / 90.00 < delta ) and ( abs(c[5]-90.00) / 90.00 < delta ) \
    and ( abs((c[0]-c[1]) / c[0]) < delta ) and ( abs((c[0]-c[2]) / c[0]) > delta ) \
    and ( abs((c[1]-c[2]) / c[0]) > delta ):
        derived_info['crsyst']='tetr'
        derived_info['flipcell'] = str(c[0])+" "+str(c[0])+" "+str(c[2])+" 90.00 90.00 90.00"
    if ( abs(c[3] - 90.00)/90.00 < delta ) and ( abs(c[4]-90.00) / 90.00 < delta ) and ( abs(c[5]-90.00) / 90.00 < delta ) \
    and ( abs((c[0]-c[1]) / c[0]) > delta ) and ( abs((c[0]-c[2]) / c[0]) > delta ) \
    and ( abs((c[1]-c[2]) / c[0]) > delta ):
        derived_info['crsyst']='orth'
        derived_info['flipcell'] = str(c[0])+" "+str(c[1])+" "+str(c[2])+" 90.00 90.00 90.00"
    if ( abs(c[3] - 90.00)/90.00 < delta ) and ( abs(c[4]-90.00) / 90.00 < delta ) and ( abs(c[5]-120.00) / 120.00 < delta ) \
    and ( abs((c[0]-c[1]) / c[0]) < delta ):
        derived_info['crsyst']='trig'
        derived_info['flipcell'] = str(c[0])+" "+str(c[0])+" "+str(c[2])+" 90.00 90.00 120.00"
    if ( abs(c[3] - 90.00)/90.00 < delta ) and ( abs(c[4]-90.00) / 90.00 > delta ) and ( abs(c[5]-90.00) / 90.00 < delta ):
        derived_info['crsyst']='mono'
        derived_info['flipcell'] = str(c[0])+" "+str(c[1])+" "+str(c[2])+" 90.00 "+str(c[4])+" 90.00"
    if ( derived_info["crsyst"] == "tric"):
        derived_info['flipcell'] = str(c[0])+" "+str(c[1])+" "+str(c[2])+" "+str(c[3])+" "+str(c[4])+" "+str(c[5])

#Process symmetry information from .ins-file if this is to be used and put this is temporary symcards.tmp file
    if ( flip_keywords['forcesymmetry'] == "yes" ):
        if ( nsymm == 0 ) and not (derived_info["crsyst"]=="tric"):
           print " No symmetry information available "
           print " Calculation continues, but forcesymmetry is set to no"
           flip_keywords['forcesymmetry'] = "no"
           if ( flip_keywords['comments'] == 'yes' ): raw_input(" Press <RETURN> to continue")
        else:
           symcards = open("symcards.tmp", "w")
           print >> symcards, "symmetry"
           print >> symcards, "X Y Z"
           if ( derived_info['latt'] > 0 ): print >> symcards, " -X -Y -Z"
           for line in fileinput.input(files['insin']):
               if "SYMM" in line:
                  line = string.upper(string.join(string.split(line)[1:]))
                  for k in ['X', 'Y', 'Z']:
                      line = string.replace(line,"+ "+k,"+"+k)
                      line = string.replace(line,"- "+k,"-"+k)
                  line = string.replace(line,"+"," +")
                  line = string.replace(line,"-"," -")
                  line = string.replace(line,","," , ")
                  print >> symcards, line
                  if ( derived_info['latt'] > 0 ):
                     splitline=string.split(line)
                     invline = ""
                     for k in splitline:
                         if ( k[0] == "+" ): l = string.replace(k,'+','-')
                         if ( k[0] == "-" ): l = string.replace(k,'-','+')
                         if ( k[0] != "-" ) and ( k[0] != "+" ) and ( k[0] != ","): l = "-"+k
                         if ( k == "," ): l = k
                         invline = invline+l
                     print >> symcards, "%s" % invline
           print  >> symcards, "endsymmetry"
           symcards.close()

# Build symcards.tmp from crystal system info
    if ( flip_keywords['forcesymmetry'] == "no" ):
      symcards = open("symcards.tmp", "w")
      symcards.write("symmetry\nx y z\n")
      if ( flip_keywords['merge'] == "yes" ):
        if ( derived_info['crsyst'] == 'tric' ):
           symcards.write("-x -y -z\n")
           flip_keywords['SG'] = '2'
        if ( derived_info['crsyst'] == 'mono' ):
           symcards.write("-x y -z\n")
           flip_keywords['SG'] = '3'
        if ( derived_info['crsyst'] == 'orth' ):
           symcards.write("-x -y z\n-x y -z\nx -y -z\n")
           flip_keywords['SG'] = '16'
        if ( derived_info['crsyst'] == 'tetr' ):
           symcards.write("-x -y z\n-y x z\ny -x z\n")
           flip_keywords['SG'] = '75'
        if ( derived_info['crsyst'] == 'trig' ):
           symcards.write("-y x-y z\n-x+y -x z\n")
           flip_keywords['SG'] = '143'
        if ( derived_info['crsyst'] == 'cubi' ):
           symcards.write("-x -y z\n-x y -z\nx -y -z\nz x y\nz -x -y\n-z -x y\n-z x -y\ny z x\n-y z -x\ny -z -x\n-y -z x\n")
           flip_keywords['SG'] = '195'
      else:
        symcards.write("-x -y -z\n")
        SG = '2'
      symcards.write("endsymmetry\n")
      symcards.close()
    else:
      SG="as in import file"

#Write lattice cards
    symcards = open("symcards.tmp", "a")
    symcards.write("centers\n0.0000 0.0000 0.0000\n")
    if ( abs(derived_info['latt']) == 2 ): symcards.write("0.5000 0.5000 0.5000\n")
    if ( abs(derived_info['latt']) == 3 ): symcards.write("0.666666667 0.33333333 0.333333333\n0.33333333 0.666666667 0.666666667\n")
    if ( abs(derived_info['latt']) == 4 ): symcards.write("0.5000 0.5000 0.0000\n0.5000 0.0000 0.5000\n0.0000 0.5000 0.5000\n")
    if ( abs(derived_info['latt']) == 5 ): symcards.write("0.0000 0.5000 0.5000\n")
    if ( abs(derived_info['latt']) == 6 ): symcards.write("0.5000 0.0000 0.5000\n")
    if ( abs(derived_info['latt']) == 7 ): symcards.write("0.5000 0.5000 0.0000\n")
    symcards.write("endcenters\n")
    symcards.close()

#sometimes there is lattice info, in that case 'missing' should be zero, the default, otherwise set to other defaults
    if ( derived_info['latt'] != 0 ):
       if ( flip_keywords['normalize'] == "no" ):
          flip_keywords['missing'] = "float 0.4"
       else:
          flip_keywords['missing'] = "bound 0.4 4"


#Process SFAC info (is a bit complicated, since there are two styles in Shelx-files
# there can be two types of SFAC cards in .ins file; second character of element need to be lower-case
    if ( nsfac == 1 ):
       for line in fileinput.input(files['insin']):
          if "SFAC" in line:
             multi=re.search('=$',line)
             sfacline=string.split(line)[1:]
    else:
       multi = 'yes'
    if ( nsfac > 1 ) or ( multi == 'yes' ):
       sfacline=''
       for line in fileinput.input(files['insin']):
          if "SFAC" in line: sfacline=sfacline+' '+string.split(line)[1]
       sfacline=string.split(sfacline)
    if ( nsfac == 1 ) and ( multi ): del sfacline[1:]
    derived_info['sfac']=''
    for k in sfacline:
       if ( len(k) > 1 ):
          derived_info['sfac']=derived_info['sfac']+' '+string.replace(k,k[1],k[1].lower())
       else:
          derived_info['sfac']=derived_info['sfac']+' '+k
    derived_info['sfac']=string.lstrip(derived_info['sfac'])

def check_executables(exe):
   for k, v in exe.items():
       if ( find_executable(v) == None ):
         print "%s executable not found, check whether it is installed and in the path." % v
         return False
   return True



def generate_superflip_file(flip_keywords,files,derived_info):
#Generate Superflip .inflip file and write some info to the screen
    print """
============================================================================
!                  Ab initio Charge Flipping procedure                     !
!                      for small-molecule structures                       !
!                                                                          !
!          script written by: A. van der Lee, C. Dumas & L. Palatinus      !
!  CF and map interpretation calculations by : L. Palatinus & G. Chapuis   !
!       Palatinus, L. & Chapuis, G.(2007): J. Appl. Cryst. 40, 786-790     !
!              http://superspace.fzu.cz/superflip                         !
!                                                                          !
!                          python-script-version %s                    !
============================================================================
    """ % version

    print """
-------------------  Crystal data --------------------
import file              .......  %s
hkl-file                 .......  %s
unit cell parameters     .......  %s
crystal system           .......  %s
merging space group      .......  %s

----------------  CF parameters used  -----------------
flip threshold            ......  %s
weak threshold            ......  %s
Biso                      ......  %s
maximum cycles / trial    ......  %s
normalize                 ......  %s
number of trials          ......  %s
superposition             ......  %s
----------------------------------------------------

    """ % (files['insin'], files['hklin'], derived_info['flipcell'], derived_info['crsyst'], flip_keywords['SG'], flip_keywords['ked'], flip_keywords['weak'], flip_keywords['biso'],
       flip_keywords['maxcycl'], flip_keywords['normalize'], flip_keywords['trial'], flip_keywords['superposition'])

    if ( flip_keywords['comments'] == 'yes' ): raw_input("\n Press <RETURN> to continue\n")

#Now generate inflip file
    f = open(files['inflip'], "w")
    g = open("symcards.tmp", "r")


    f.write("""
#=============================================
#   Ab initio phasing by Charge Flipping
#
#                   SUPERFLIP
#
#=============================================
title    ab initio Phasing by Charge Flipping

# Basic crystallographic information
cell             %s
""" % derived_info['flipcell'])

    f.writelines(g.readlines())

    f.write("""

voxel            AUTO
terminal         %s

# Keywords influencing the CF algorithm
weakratio        %s
biso             %s
normalize        %s
missing          %s
maxcycles        %s
repeatmode       %s

# Output density map
searchsymmetry   average
derivesymmetry   %s
outputfile       %s

# Input reflections

dataformat       %s
""" % ( flip_keywords['terminal'], flip_keywords['weak'], flip_keywords['biso'], flip_keywords['normalize'],
       flip_keywords['missing'], flip_keywords['maxcycl'], flip_keywords['trial'],
       flip_keywords['derivesymmetry'], files['m81'], flip_keywords['dataformat']  ))

    if ( flip_keywords['ked'] != "auto" ):
       f.write("""

delta %s sigma

               """ % flip_keywords['ked'])

    if ( flip_keywords['superposition'] == "yes" ):
       f.write("""

modelfile superposition 0.05

               """)

    if ( flip_keywords['dataformat'] == "shelx" ):
       f.write("""
fbegin           %s
endf""" % files['hklin'])

    f.close()
    g.close()

def analyze_superflip_logfile(flip_keywords,files,derived_info):
#
# Just a small note: in order to get a specific line number fileinpu.filelineno is used, then
# the the first line starts with number 1; however with readlines it starts with zero.
# In the next block the line before 'Last run from" is needed, thus the filelineno has to be
# decremented with 2 instead of 1 (however: it depends a bit on how the lines are processed)
#
# Analyse the log-file
    derived_info['itlino']=[]
    for line in fileinput.input(files['fliplog']):
        if "Last run from" in line: cev=fileinput.filelineno()-2
        if "Number of successes" in line: nsuccess=int(string.split(line)[4])
        if "# Iteration #" in line: derived_info['itlino'].append(fileinput.filelineno()-1)
    try:
        cev
        nsuccess
    except NameError:
# x doesn't exist, do something
        print "\n Note from Flipsmall:\n Not all required information could be found in Superflip's logfile.\n Did Superflip report a problem?"
        return False
    derived_info['itlino'].append(fileinput.filelineno()-1)
    bestresults = open(files['fliplog'],'r').readlines()[cev].strip()
    if ( bestresults[0:3] == "" ): bestresults = open(files['fliplog'],'r').readlines()[cev-2].strip()
    derived_info['bestrun'] = int(string.split(bestresults)[0])
    derived_info['spgr']=string.split(bestresults)[4]
    phisym=float(string.split(bestresults)[3])

    if  ( flip_keywords['logging'] == "yes" ):
        log.write("\
%12s: %12s %12s\n" % (files['base'], derived_info['spgr'], phisym))


    print """


            =====================================================
                         ANALYSIS OF THE RESULTS
            =====================================================


    """

# Interpret the results from the log-file
    if ( phisym > 25.0 ) and ( nsuccess == 0 ):
       print ""
       print "            ***  No convergence detected  ***"
       print ""
       if ( flip_keywords['edmacontinue'] == "no"):
          print "              * you can try another time"
          print ""
          print " normal exit"
          return False
       else:
          print "              * continue anyhow"
          print ""
    if ( phisym < 25.0  )  and ( nsuccess > 0 ):
       print ""
       print "            ***     Convergence detected  and low PhiSym ***"
       print "            ***         Structure is probably solved     ***"
       print ""
       print "                     PhiSym = %s" % abs(phisym/100.0)
       print ""
       print "            ***      Spacegroup proposed: %s ****" % derived_info['spgr']
    if ( phisym < 25.0  )  and ( nsuccess == 0 ):
       print ""
       print "            ***  No Convergence detected  but low PhiSym ***"
       print "            ***      Structure is probably solved        ***"
       print ""
       print "                     PhiSym = %s" % abs(phisym/100.0)
       print ""
       print "            ***      Spacegroup proposed: %s ****" % derived_info['spgr']
    if ( phisym > 25.0  )  and ( nsuccess > 0 ):
       print ""
       print "            ***    Convergence detected  but high PhiSym ***"
       print "            ***      Structure may be solved (P1?)       ***"
       print ""
       print "                     PhiSym = %s" % abs(phisym/100.0)
       print ""
       print "            ***      Spacegroup proposed: %s ****" % derived_info['spgr']
    if ( flip_keywords['forcesymmetry'] == 'yes' ):
       print ""
       print "               Check whether Superflip's spacegroup is the same as  "
       print "               yours in the input-file. If not,"
       print "               you may consider to use forcesymmetry=no or          "
       print "              to use in your input (ins)file the space group       "
       print "                          proposed by Superflip                     "
       print "             Note that the resulting structure is possibly wrong "
       print ""
       derived_info['spgr']="spacegroup from ins-file"

    if ( flip_keywords['comments'] == "yes" ): raw_input(" Press <RETURN> to continue")
    return True

def generate_edma_file(flip_keywords,files,derived_info):
#Create EDMA file:
    files['edmain']=files['base']+'_edma.inflip'
    files['edmaout']=files['base']+'_structure.ins'

    h = open(files['edmain'], "w")
    g = open("symcards.tmp", "r")
    h.write("""
#=============================================
#              Map interpretation
#
#                     EDMA
#
#=============================================
title    CF solution in %s

# Basic crystallographic information
cell             %s

""" % (derived_info['spgr'],derived_info['flipcell']))

#Process symmetry information, either the input info or that from the best run in the log-file
    if ( flip_keywords['forcesymmetry'] == "yes" ):
       g.seek(0)
       h.writelines(g.readlines())
    else:
       h.write("symmetry\n")
       cevcent = 0
       cevsym = 0
       for n,line in enumerate(fileinput.FileInput(files['fliplog'])):
          if ( n > derived_info['itlino'][derived_info['bestrun']-1] ) and ( n < derived_info['itlino'][derived_info['bestrun']] ):
             if ' Centering vectors' in line: cevcent=n+1
             if ' Symmetry operations:' in line: cevsym=n+1
       for n,line in enumerate(fileinput.FileInput(files['fliplog'])):
          if ( n >= cevsym):
             if (line.strip() == ''):
                break
             else:
                h.write(string.join(string.split(line.strip())[1:])+'\n')
       h.write("\
endsymmetry\n\
centers\n")
       if (cevcent != 0 ):
          for n,line in enumerate(fileinput.FileInput(files['fliplog'])):
             if ( n >= cevcent) and (cevcent != 0 ):
                if 'Symmetry operations' in line:
                   break
                else:
                   a=string.split(line)
                   if ( len(a) > 0 ):
                      if (a[0] == '0.333') or (a[0] == '0.667'):
                         for k in a:
                            if (k == '0.333'): h.write('0.33333333 ')
                            if (k == '0.667'): h.write('0.66666667 ')
                         h.write('\n')
                      else:
                          h.write(string.join(string.split(line.strip())[0:])+'\n')
       else:
          h.write('0.000 0.000 0.000\n')
       h.write('endcenters\n')

    h.write("""

# EDMA-specific keywords
inputfile %s
outputbase %s
export %s
numberofatoms  0
composition %s
maxima all
fullcell no
scale fractional
plimit    1.5 sigma
centerofcharge yes
chlimit  0.25
""" % (files['m81'], files['base'], files['edmaout'], derived_info['sfac']) )
    h.close()
    g.close()

def cleanup(files):
# rename the EDMA ins-file to a res file

    nsymm = 0
    zerrfound = 0
    for line in fileinput.input(files['edmaout']):
       if "LATT" in line: latt = int(string.split(line)[1])
       if "SYMM" in line: nsymm = nsymm + 1
       if "ZERR" in line:
          derived_info['zerr'] = string.split(line)[1]
          zerrfound = 1

    if (zerrfound == 0):
       nsymm = nsymm + 1
       if ( latt < 0 ):
           mult=1
       else:
           mult=2

       if ( abs(latt) == 1 ): derived_info['zerr'] = str(nsymm*mult)
       if (( abs(latt) == 2 ) or ( abs(latt) == 5 ) or ( abs(latt) == 6 ) or ( abs(latt) == 7 )) : derived_info['zerr'] = str(nsymm*mult*2)
       if ( abs(latt) == 3 ): derived_info['zerr'] = str(nsymm*mult*3)
       if ( abs(latt) == 4 ): derived_info['zerr'] = str(nsymm*mult*4)


    for line in fileinput.input(files['edmaout'],inplace=1):
       if (not "END" in line) and (not "CELL" in line) : print line[:-1]
       if "FVAR" in line: print "L.S. 4\nWGHT 0.1\nFMAP 2\nPLAN 25\nBOND"
       if "END" in line: print "HKLF 4\nEND"
       if "TITL" in line:
           print "CELL " + derived_info['wavelength'] + " " + derived_info['cell']
           print "ZERR " + derived_info['zerr'] + " " + derived_info['cellesd']

    if  os.path.isfile(files['base']+'.res'):os.remove(files['base']+'.res')
    os.rename(files['edmaout'],files['base']+'.res')
    print """

          Use %s for subsequent refinement

    """ % (files['base'] + '.res')

#clean up
    if ( flip_keywords['cleanup'] == 'yes' ):
       if  os.path.isfile(files['base']+'.coo'):os.remove(files['base']+'.coo')
       if  os.path.isfile(files['m81']):os.remove(files['m81'])
       if  os.path.isfile(files['edmain']):os.remove(files['edmain'])
       if  os.path.isfile(files['edmaout']):os.remove(files['edmaout'])
       if  os.path.isfile(files['inflip']):os.remove(files['inflip'])
       if  os.path.isfile(files['fliplog']):os.remove(files['fliplog'])
       if  os.path.isfile('symcards.tmp'):os.remove('symcards.tmp')


def find_executable(executable, path=None):
    """Try to find 'executable' in the directories listed in 'path' (a
    string listing directories separated by 'os.pathsep'; defaults to
    os.environ['PATH']).  Returns the complete filename or None if not
    found
    """
    if path is None:
        path = os.environ['PATH']
    paths = path.split(os.pathsep)
    extlist = ['']
    if os.name == 'os2':
        (base, ext) = os.path.splitext(executable)
        # executable files on OS/2 can have an arbitrary extension, but
        # .exe is automatically appended if no dot is present in the name
        if not ext:
            executable = executable + ".exe"
    elif sys.platform == 'win32':
        pathext = os.environ['PATHEXT'].lower().split(os.pathsep)
        (base, ext) = os.path.splitext(executable)
        if ext.lower() not in pathext:
            extlist = pathext
    for ext in extlist:
        execname = executable + ext
        if os.path.isfile(execname):
            return execname
        else:
            for p in paths:
                f = os.path.join(p, execname)
                if os.path.isfile(f):
                    return f
    else:
        return None




def flipsmall (*args):

    if ((len(args) == 1) and (not olexrun)) or ((len(args) == 0) and (olexrun)):
       if olexrun:
          print "There should be at least the name of an existing ins-file as command line argument.\nTry again or do 'spy.flipsmall(--help)' to get help."
       else:
          print "There should be at least the name of an existing ins-file as command line argument.\nTry again or do 'flipsmall.py --help' to get help."
       return False
    args=list(args)
    if olexrun: args.insert(0,'flipsmall')
    print 'Starting flipsmall procedure for ' + args[1] + '.'
    q=process_command_line_arg(args,flip_keywords,files)
    if not q: return False
    q=check_executables(exe)
    if not q: return False

    process_insfile(flip_keywords,files,derived_info)
    generate_superflip_file(flip_keywords,files,derived_info)

    ################## RUN SUPERFLIP #########################################
    if not olexrun:
       status = os.system(exe['SF']+' '+files['inflip'])
    else:
       olex.m('exec ' + exe['SF']+' '+ files['inflip'])
       olex.m('waitfor process')
    ##########################################################################

    q=analyze_superflip_logfile(flip_keywords,files,derived_info)
    if not q: return False
    generate_edma_file(flip_keywords,files,derived_info)

    ###################### RUN EDMA #######################################
    if not olexrun:
       status = os.system(exe['EDMA']+' '+files['edmain'])
    else:
       olex.m('exec -q ' + exe['EDMA']+' '+ files['edmain'])
       olex.m('waitfor process')
       olex.m('reap ' + args[1].split('.')[0] + '.res')
       olex.m('compaq -a')
    #######################################################################

    cleanup(files)
    print 'Finishing flipsmall procedure'
    print """\n
 ========================
 Flipsmall version %s""" % version

    return

# the main keywords
flip_keywords=dict(weak=0.20, biso=2.5, maxcycl=10000, comments="yes", edmacontinue="no",
                   normalize="yes", merge="yes", forcesymmetry="no", trial="1", SG="1", missing="zero",
                   derivesymmetry="use",dataformat="shelx",cleanup="yes",terminal='yes', logging='no', superposition='no',
                   sflipversion='superflip',ked='auto')
# calculated and extracted info, from ins-file and logfile
derived_info=dict(crsyst='tric', cell="9.0 9.0 9.0 90.0 90.0 90.0", flipcell="9.0 9.0 9.0 90.0 90.0 90.0",
                  cellesd = "0.0 0.0 0.0 0.0 0.0 0.0", wavelength ="0.71073", sfac="C H O N",
                  latt=1, spgr='P1', zerr = '1', itlino=[], bestrun=1)
# file names in use
files=dict(base='name',insin='name.ins',hklin='name.hkl',m81='name.m81',inflip='name.inflip',
           fliplog='name.sflog',edmain='name_edma.inflip',edmaout='name_structure.ins')
# the external executables
exe=dict(SF="superflip",EDMA="EDMA")


olexrun = False
try:
  import olex
  import olx
  from olexFunctions import OlexFunctions
  OV = OlexFunctions()
  olexrun = True
  OV.registerFunction(flipsmall)
except:
  comlineargs = tuple(sys.argv)
  flipsmall(*comlineargs)
sys.exit(0)
