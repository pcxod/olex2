import sys
import os
import smtplib
import mimetypes
from email.Encoders import encode_base64
from email.MIMEAudio import MIMEAudio
from email.MIMEBase import MIMEBase
from email.MIMEImage import MIMEImage
from email.MIMEMultipart import MIMEMultipart
from email.MIMEText import MIMEText
import olex
import olx
from olexFunctions import OlexFunctions
OV = OlexFunctions()


def OlexMail(SendorNot=0):
  if SendorNot <= 0:
    print "This will attempt to email your PythonError.log to the Olex2 boys"
    print "If this is what you want please check you have setup your:"
    print "email address as email_address=; and"
    print "mailserver information as mailserver="
    print "In the usettings.dat file"
    return
  # These are just some variables for emailing
  recipients_address = "olex2pythonerrorlog@googlemail.com"
  Olex2Path = olex.f("BaseDir()")
  ErrorLogPath = "%s/" %OV.DataDir()
  ErrorLogPath = ErrorLogPath.replace("\\\\", "\\")
  usettings = open("%s/usettings.dat"%(Olex2Path), 'r')
  email_address = ""
  mailserver = ""

  for usettings_line in usettings:
    if not email_address or not mailserver:
      if "email_address" in usettings_line:
        email_address = usettings_line.split("=")[-1].strip()
        print "email_address = ", email_address
      elif "mailserver" in usettings_line:
        mailserver = usettings_line.split("=")[-1].strip()
        print "mailserver = ", mailserver
    else:
      print "Mail and server", email_address, mailserver
      break
  usettings.close()

  From = email_address
  To = recipients_address
  msg = MIMEMultipart()
  msg['From'] = From
  msg['To'] = To
  msg['Subject'] = 'Olex2 PythonError Log'
  body = MIMEText('Please find attached my python log') #Here is the body
  filename = 'PythonError.log'
  path = ErrorLogPath + filename

# Mailing stuff
  ctype, encoding = mimetypes.guess_type(path)
  if ctype is None or encoding is not None:
    ctype = 'application/octet-stream'
    maintype, subtype = ctype.split('/', 1)
    fp = open(path, 'rb')
    print maintype, subtype
    attach = MIMEBase(maintype, subtype)
    attach.set_payload(fp.read())
    encode_base64(attach)
    fp.close
    attach.add_header('Content-Disposition', 'attachment', filename=filename)

  msg.attach(attach) #We create our message both attachment and the body
  msg.attach(body)
  #print msg.as_string()
  server = smtplib.SMTP(mailserver)
  server.sendmail(From, To, msg.as_string()) #Send away
  server.quit()
  print "Your log file has been emailed to the Olex2 Team, thank you for your support"

OV.registerFunction(OlexMail)
