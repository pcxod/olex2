
import sys
import olex
import MySQLdb
class Writer:
    def write(self, Str):
        olex.m("echo " + repr(Str))
    
sys.stdout = Writer()
sys.stderr = sys.stdout

print 'hello'

import olx
print 'hello 1'

#olex.f( (1,2) )

olex.m('echo 1111')

olx.Echo('echo is called!')
print 'hmm'

print ('a: ' + olex.f("cell(a)") + ' b: ' + olex.f("cell(b)") + ' c: ' + olex.f("cell(c)")),
print olex.f("filepath()"),
olex.m("sel atoms where xatom.type=='h'")

olx.Echo("curently loaded file: ", olx.FileFull())

#print "atoms selected: "
print olx.Sel()

olx.Help()

def __init__(self, host='129.234.12.100', user='DIMAS', passwd='fddd-anode', db='dimas_plone'):
	self.conn = MySQLdb.connect(host, user, passwd, db)

#execfile( fileName )


print "here we go"
olx.Direction()