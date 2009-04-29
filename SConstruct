import sys
import os
import string

debug = ARGUMENTS.get('debug', 0)
debug = int(debug)

out_dir = 'build/scons/' 
if debug:
  out_dir += 'debug/'
else:
  out_dir += 'release/'

#get file lists
alglib = Glob('./alglib/*.cpp')
sdl = Glob('./sdl/*.cpp')
sdl_smart = Glob('./sdl/smart/*.cpp')
xlib = Glob('./xlib/*.cpp')
xlib_macro = Glob('./xlib/macro/*.cpp')
glib = Glob('./glib/*.cpp')
gxlib = Glob('./gxlib/*.cpp')
olex2 = Glob('./olex/*.cpp')
unirun = Glob('./unirun/*.cpp')
np_repository = Split("""./repository/filesystem.cpp   ./repository/shellutil.cpp 
                         ./repository/httpex.cpp       ./repository/url.cpp 
                         ./repository/wxhttpfs.cpp     ./repository/wxzipfs.cpp 
                         ./repository/fsext.cpp         
                         ./repository/integration.cpp  ./repository/IsoSurface.cpp 
                         ./repository/eprocess.cpp     ./repository/olxvar.cpp 
                         """)
repository = np_repository + Split("""./repository/pyext.cpp 
                                     ./repository/py_core.cpp""")
#function to process file name list
def processFileNameList(file_list, envi, dest, suffix=''):
  obj_list = []
  for file in file_list:
    fn = os.path.splitext( os.path.split(file)[1])[0]
    obj_list.append( envi.Object(dest+'/'+fn+suffix, file) )
  return obj_list

def fileListToStringList(src_dir, file_list):
  str_list = []
  for file in file_list:
    str_list.append(src_dir + '/' + file.name)
  return str_list
    
item_index=0
for file in gxlib:
  if string.find(file.name, 'wglscene.cpp') != -1:
    gxlib.pop(item_index)
    break
  item_index += 1

env = Environment(CCFLAGS = ['-D__WXWIDGETS__', '-D_UNICODE', '-DUNICODE'],
                  ENV = os.environ )
env.Append(CPPPATH = ['alglib', 'sdl', 'glib', 'gxlib', 
                      'repository', 'xlib'])

if sys.platform[:3] == 'win':
  AddOption('--wxdir',
            dest='wxFolder',
            type='string',
            nargs=1,
            action='store',
            metavar='WXDIR',
            default='e:/wxWidgets-2.8.9/',
            help='Locaton of the wxWidgets library')
  wxFolder = GetOption('wxFolder')
  if wxFolder[-1] != '/' or wxFolder[-1] != '\\':
    wxFolder += '/'
  pyFolder = os.path.split(sys.executable)[0] + '\\'
  if not debug:
    env.Append(CCFLAGS = ['/EHsc', '/O2', '/Ob2', '/Oi', '/GL', '/MD', 
                          '/bigobj', '/fp:fast', '/GF', '/arch:SSE2']) 
    env.Append(LIBS = Split("""wxbase28u         wxbase28u_net  wxmsw28u_gl   
                               wxmsw28u_richtext wxmsw28u_html wxmsw28u_core 
                               wxmsw28u_adv wxregexu"""))
    env.Append(LINKFLAGS=['/LTCG'])
    env.Append(ARFFLAGS=['/LTCG'])
  else:
    env.Append(CCFLAGS = ['/EHsc', '/RTC1', '/ZI', '-D_DUBUG', '/Od', '/MDd', 
                          '/bigobj', '/fp:fast']) 
    env.Append(LIBS = Split("""wxbase28ud         wxbase28ud_net  wxmsw28ud_gl   
                               wxmsw28ud_richtext wxmsw28ud_html wxmsw28ud_core 
                               wxmsw28ud_adv wxregexud"""))
  env.Append(CPPPATH=[wxFolder+'include', 
                      wxFolder+'lib/vc_lib/mswud',
                      pyFolder+'include'])
  env.Append(LIBPATH = [pyFolder+'libs', wxFolder+'lib/vc_lib'])
  env.Append(LIBS = Split("""wxexpat wxjpeg wxpng wxtiff
                             wxzlib mapi32 glu32 user32 opengl32 gdi32 ole32 
                             advapi32 comdlg32 comctl32 shell32 rpcrt4 oleaut32
                             kernel32 wsock32"""))
  env.Append(LINKFLAGS=['/MANIFEST'])
else:
  try: 
    if sys.platform[:6] == 'darwin':
      env.Append(CCFLAGS = '-D__MAC__')
      env.ParseConfig("wx-config --cxxflags --unicode --libs gl,core,html,net,aui")
      env.Append(FRAMEWORKS=['OpenGL', 'AGL'])
    else:
      env.ParseConfig("wx-config --cxxflags --unicode --toolkit=gtk2 --libs gl,core,html,net,aui")
      env.ParseConfig("python-config --includes")
    env.ParseConfig("python-config --libs --ldflags")
  except:
    print 'Please make sure that wxWidgets and Python config scripts are available'
    Exit(1)
  if not debug:
    env.Append(CCFLAGS = ['-O3', '-exceptions']) 
  else:
    env.Append(CCFLAGS = ['-g', '-exceptions']) 
  
sdl_files = fileListToStringList('sdl', sdl) + fileListToStringList('sdl/smart', sdl_smart)
sdl_files = processFileNameList(sdl_files, env, out_dir + 'sdl')
env.StaticLibrary(out_dir + 'lib/sdl', sdl_files)

env.Append(LIBPATH=[out_dir+'lib'])
env.Append(LIBS = ['sdl'])
np_env = env.Clone()
olex2_files = fileListToStringList('alglib', alglib) + \
              fileListToStringList('xlib', xlib) + \
              fileListToStringList('xlib/macro', xlib_macro) + \
              fileListToStringList('glib', glib) + \
              fileListToStringList('gxlib', gxlib) + \
              repository + \
              fileListToStringList('olex', olex2)
olex2_files = processFileNameList(olex2_files, env, out_dir+'olex')  
#link in the res file...
if sys.platform[:3] == 'win':
  res_file = out_dir + 'olex/app.res'
  env.Append(LINKFLAGS=['/MANIFEST', res_file])
  env.Append(RCFLAGS=['/l0x809'])
  env.RES(res_file, 'olex/app.rc')
env.Program(out_dir+'exe/olex2', olex2_files)

unirun_files = np_repository + fileListToStringList('unirun', unirun)
unirun_files = processFileNameList(unirun_files, np_env, out_dir+'unirun')

np_env.Append(CCFLAGS = ['-D_NO_PYTHON'])
np_env.Program(out_dir+'exe/unirun', unirun_files)

try:
  import _imaging
except:
  print '!! You will need to install PIL' 