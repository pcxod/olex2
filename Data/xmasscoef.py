import sys
import os

if __name__ == '__main__':
  if len(sys.argv) == 1:
    sys.argv.append(r"C:\Documents and Settings\oleg\My Documents\My Dropbox\share\x-ray mass attenuation coefficients")
    #print 'Please provide the source folder'
    #exit
  shelx_data = {}
  #s_file = open(r"C:\Documents and Settings\oleg\My Documents\shelx-data.txt", "r")
  #line_cnt  = 0
  #elm = None
  #values = None
  #elm_mass = None
  #for line in s_file:
    #line = line.strip('\n')
    #if line[-1] == ',':
      #line = line[:-1]
    #toks = line.split(',')
    #if len(toks) <= 1:
      #line_cnt = 0
      #continue
    #line_cnt += 1
    #if line_cnt >= 5 and values != None:
      #val = toks[-1]
      #si = val.find('/')
      #val = float(val[si+1:][:-1])
      #val = val/elm_mass*0.6022142
      #values.append(val)
    #qi = toks[0].find('\'')
    #if qi == -1:  continue
    #if elm != None and values != None:
      #shelx_data[elm] = values
    #elm = toks[0][qi+1:qi+3].strip()
    #if len(elm) == 2:
      #elm = elm[0] + elm[1].lower()
    #values = []
    #elm_mass = toks[-1];
    #si = elm_mass.find('/')
    #elm_mass = float(elm_mass[si+1:][:-1])
    
  #if elm != None and values != None:
    #shelx_data[elm] = values
  #s_file.close()
  #print shelx_data

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
    tab_data = None
    if shelx_data.has_key(el_name):
      tab_data = shelx_data[el_name]
    added = [False,False,False]
    added_en = (8.041e-3,1.744e-2,2.21e-2)
    for line in fc:
      toks = line.split()
      if len(toks) > 3:
        toks = toks[-3:]
      en_val = float(toks[0])
      if tab_data != None:
        for x in xrange(0,3):
          if not added[x] and en_val >= added_en[x]:
            added[x] = True
            print >> data_file, "  {" + ("%.5e" % added_en[x]) + ", " + ("%.3e" % tab_data[x]) + ", 0},"
          
      print >> data_file, "  {" + toks[0] + ", " + toks[1] + ", " + toks[2] + "},"
    fc.close()
    print >> data_file, "  {0, 0, 0}"
    print >> data_file, "};"
    data_file.close()
  for s in dict_def:
    print >> h_file, s
  h_file.close()
  