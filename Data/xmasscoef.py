import sys
import os

if __name__ == '__main__':
  if len(sys.argv) == 1:
    sys.argv.append(r"C:\Documents and Settings\oleg\My Documents\My Dropbox\share\x-ray mass attenuation coefficients")
    #print 'Please provide the source folder'
    #exit
  files = os.listdir(sys.argv[1])
  h_file = open("e:/tmp/xmasscoef.h", "w+b")
  dict_def = []
  base = os.path.normpath(sys.argv[1]) + '/'
  for f in files:
    el_name = os.path.split(f)[-1].split('.')[0]
    data_file = open("e:/tmp/a/absorpc_" + el_name + ".cpp", "w+b")
    print >> h_file, 'extern const cm_Absorption_Coefficient _cm_absorpc_' + el_name + '[];'
    dict_def.append('  dict.Add("' +el_name + '", ' + '_cm_absorpc_' + el_name + ');')
    print >> data_file, '#include "../absorpc.h"'
    print >> data_file, "const cm_Absorption_Coefficient XlibObject(_cm_absorpc_" + el_name + ")[] = {"
    fc = open(base + f, "rb")
    for line in fc:
      toks = line.split()
      if len(toks) > 3:
        toks = toks[-3:]
      print >> data_file, "  {" + toks[0] + ", " + toks[1] + ", " + toks[2] + "},"
    fc.close()
    print >> data_file, "  {0, 0, 0}"
    print >> data_file, "};"
    data_file.close()
  for s in dict_def:
    print >> h_file, s
  h_file.close()
  