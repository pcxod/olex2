"""
POST method
server = "http://vm02.iucr.org/cgi-bin/checkcif.pl"
"file" is the openned buffered file ideally with enctype="multipart/form-data"
"runtype" is "symmonly"
"referer" is "checkcif_server"
outputtype can be HTML or PDF
"""
import os
import sys
import shutil
import re
import olex
import olx
import urllib2_file
import urllib2
from olexFunctions import OlexFunctions
OV = OlexFunctions()

'''
To run this script, type spy.OlexCheckCIF() in Olex2
'''

def OlexCheckCIF():
  
  """if result_filetype >= 1:
    result_filetype = "pdf"
  else:
    result_filetype = "html"
  """
  # Only getting html return from IUCr may need to check on this?
  try:
    filename = open('%s/%s.cif' %(OV.FilePath(), OV.FileName()))
  except IOError:
    print "The file does not exist, creating CIF file"
    OV.cmd("addins acta")
    OV.cmd("refine 4")
    return OlexCheckCIF()
  result_filetype = "html"
  params = {
                             "runtype": "symmonly",
                             "referer": "checkcif_server",
                             "outputtype": result_filetype,
                             "file": filename
  }
  #print params #Parameter check
  f = urllib2.urlopen("http://vm02.iucr.org/cgi-bin/checkcif.pl", params)
  cifcheck_res = open('%s/%s_checkcif_report.%s' %(OV.FilePath(), OV.FileName(), result_filetype), 'w')
  cifcheck_res.write("%s"%f.read())
  cifcheck_res.close()
  # Now outputs into a html file then opens browser.
  olx.Shell('%s/%s_checkcif_report.%s' %(OV.FilePath(), OV.FileName(), result_filetype)) # Thanks to Richard G. for this
  
  # Need to build a PDF version as well
  print "Completed Check your webrowser"
  
OV.registerFunction(OlexCheckCIF)
