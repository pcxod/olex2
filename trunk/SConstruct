import sys
import os
import string
import platform

AddOption('--olx_debug',
          dest='olx_debug',
          type='string',
          nargs=1,
          action='store',
          metavar='OLX_DEBUG',
          default='false',
          help='Builds debug version')
debug = (GetOption('olx_debug').lower() == 'true')

AddOption('--olx_sse',
          dest='olx_sse',
          type='string',
          nargs=1,
          action='store',
          metavar='OLX_SSE',
          default='SSE2',
          help='Only for optimised version, inserts given SSE, SSE2 or none instructions')

architecture = platform.architecture()[0]
if not architecture:
  architecture = 'unknown'
if architecture == '64bit':
  sse = None
print 'Build architecture: ' + architecture

if debug or sys.platform[:3] != 'win' or architecture == '64bit':
  sse = None
else:
  sse = GetOption('olx_sse').upper()
  if sse == 'NONE' or not sse:
    sse = None
  elif sse not in ['SSE', 'SSE2']:
    print 'Invalid SSE instruction: \'' + sse + '\', aborting...'
    sys.exit(1)
  else:
    print 'Using ' + sse

AddOption('--olx_profile',
          dest='olx_profile',
          type='string',
          nargs=1,
          action='store',
          metavar='OLX_PROFILE',
          default='false',
          help='Build for profiling')
profiling = (GetOption('olx_profile').lower() == 'true')

if sys.platform[:3] == 'win':
  def_tool = 'vc9'
else:
  def_tool = 'gnu'
variables = Variables()
variables.AddVariables(
    EnumVariable('TOOL', 'The tool to use', def_tool, allowed_values=('vc8', 'vc9', 'gnu', 'intel'))
    )
env = Environment(ENV = os.environ, variables = variables)
Help(variables.GenerateHelpText(env))

if env['TOOL'] == 'vc8':
  env["MSVS"] = {'VERSION': '8.0'}
  env['MSVS_VERSION'] = '8.0'
  Tool('msvc')(env)
elif env['TOOL'] == 'vc9':
  env['MSVS'] = {'VERSION': "9.0"} 
  env['MSVS_VERSION'] = '9.0'
  Tool('msvc')(env)
elif env['TOOL'] == 'gnu':
  Tool('g++')(env)
elif env['TOOL'] == 'intel':
  Tool('intelc')(env)
env.Append(CCFLAGS = ['-D__WXWIDGETS__', '-D_UNICODE', '-DUNICODE'])
env.Append(CPPPATH = ['alglib', 'sdl', 'glib', 'gxlib', 
                      'repository', 'xlib'])
out_dir = 'build/scons/' 
if sys.platform[:3] == 'win':
  out_dir += 'msvc-' + env['MSVS_VERSION'] + '/'
else:
  out_dir += env['TOOL'] + '/'
if profiling:
  out_dir += 'profiling'
  
if debug:
  out_dir += 'debug'
else:
  out_dir += 'release'
out_dir += '-' + architecture
out_dir += '/py' + sys.version[:3]
if sse:
  out_dir += '-' + sse
out_dir += '/'
print 'Building location: ' + out_dir
 
#get file lists
alglib = Glob('./alglib/*.cpp')
sdl = Glob('./sdl/*.cpp')
sdl_exp = Glob('./sdl/exparse/*.cpp')
sdl_smart = Glob('./sdl/smart/*.cpp')
xlib = Glob('./xlib/*.cpp')
xlib_macro = Glob('./xlib/macro/*.cpp')
glib = Glob('./glib/*.cpp')
gxlib = Glob('./gxlib/*.cpp')
olex2 = Glob('./olex/*.cpp')
olex2_html = Glob('./olex/html/*.cpp')
olex2_ctrls = Glob('./olex/ctrls/*.cpp')
unirun = Glob('./unirun/*.cpp')
olex2c = Split("""./olex2c/olex2c.cpp""")

np_repository = Split("""./repository/filesystem.cpp   ./repository/shellutil.cpp 
                         ./repository/httpex.cpp       ./repository/url.cpp 
                         ./repository/wxzipfs.cpp      ./repository/httpfs.cpp 
                         ./repository/IsoSurface.cpp   ./repository/fsext.cpp
                         ./repository/eprocess.cpp     ./repository/olxvar.cpp 
                         """)
repository = np_repository + Split("""./repository/pyext.cpp 
                                     ./repository/py_core.cpp ./repository/updateapi.cpp 
                                     ./repository/patchapi.cpp ./repository/hkl_py.cpp""")
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

unirun_env = None
if sys.platform[:3] == 'win':
  #http://www.scons.org/wiki/EmbedManifestIntoTarget
  #embedd manifest...
  env['LINKCOM'] = [env['LINKCOM'], 'mt.exe -nologo -manifest ${TARGET}.manifest -outputresource:$TARGET;1']
  env['SHLINKCOM'] = [env['SHLINKCOM'], 'mt.exe -nologo -manifest ${TARGET}.manifest -outputresource:$TARGET;2']
  AddOption('--wxdir',
            dest='wxFolder',
            type='string',
            nargs=1,
            action='store',
            metavar='WXDIR',
            default='',
            help='Locaton of the wxWidgets library')
  wxFolder = GetOption('wxFolder')
  if not wxFolder:
    wxFolder = os.environ.get('WX_DIR')
  if not wxFolder:
    print 'Please provide --wxdir=wxWidgets_root_folder or set WX_DIR system variable to point to wxWidgets root folder'
    Exit(0)
  if wxFolder[-1] != '/' or wxFolder[-1] != '\\':
    wxFolder += '/'
  pyFolder = os.path.split(sys.executable)[0] + '\\'
  # generic libs
  env.Append(LIBS = Split("""wxexpat wxjpeg wxpng wxtiff
                             wxzlib mapi32 glu32 user32 opengl32 gdi32 ole32 
                             advapi32 comdlg32 comctl32 shell32 rpcrt4 oleaut32
                             kernel32 wsock32 Iphlpapi.lib"""))
  if not debug:
#    cc_flags = ['/EHsc', '/O2', '/Ob2', '/Oi', '/GL', '/MD', 
#                          '/bigobj', '/fp:fast', '/GF']
    cc_flags = ['/EHsc', '/O2', '/Ob2', '/Oi', '/MD', 
                          '/bigobj', '/fp:fast', '/GF']
    if sse:
      cc_flags.append( '/arch:'+sse)
    env.Append(CCFLAGS = cc_flags) 
    env.Append(LIBS = Split("""wxbase28u         wxbase28u_net  wxmsw28u_gl   
                               wxmsw28u_html wxmsw28u_core 
                               wxmsw28u_adv wxregexu"""))
    env.Append(CPPPATH=[wxFolder+'include', wxFolder+'lib/vc_lib/mswu', pyFolder+'include'])
    #env.Append(LINKFLAGS=['/LTCG'])
  else:
    cc_flags = ['/EHsc', '/RTC1', '-D_DUBUG', '/Od', '/MDd', '/bigobj', '/fp:fast']
    if architecture == '64bit':  cc_flags.append('/Zi')
    else:  cc_flags.append('/ZI')
    env.Append(CCFLAGS = cc_flags) 
    env.Append(LIBS = Split("""wxbase28ud         wxbase28ud_net  wxmsw28ud_gl   
                               wxmsw28ud_richtext wxmsw28ud_html wxmsw28ud_core 
                               wxmsw28ud_adv wxregexud"""))
    env.Append(LINKFLAGS=['/DEBUG', '/ASSEMBLYDEBUG', '/NODEFAULTLIB:msvcrt'])
    env.Append(CPPPATH=[wxFolder+'include', wxFolder+'lib/vc_lib/mswud', pyFolder+'include'])
  if architecture == '64bit':
    env.Append(LINKFLAGS=['/MACHINE:X64'])
    lib_64 = [r'C:\Program Files\Microsoft SDKs\Windows\v6.0A\Lib\x64',
              r'C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\lib\amd64']
    for p in lib_64:
      if os.path.exists(p):
        env.Append(LIBPATH=[p])  
    inc_64 = [r'C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include',
              r'C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\include']
    for p in inc_64:
      if os.path.exists(p):
        env.Append(CPPPATH=[p])
    env['ENV']['PATH'] = r'C:\Program Files\Microsoft SDKs\Windows\v6.0A\bin\x64' +\
                         os.pathsep + r'C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\bin\amd64' +\
                         os.pathsep + env['ENV']['PATH']
  else:
    env.Append(LINKFLAGS=['/MACHINE:X86'])
    lib_32 = [r'C:\Program Files\Microsoft SDKs\Windows\v6.0A\Lib',
              r'C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\lib']
    for p in lib_32:
      if os.path.exists(p):
        env.Append(LIBPATH=[p])  
    env['ENV']['PATH'] = r'C:\Program Files\Microsoft SDKs\Windows\v6.0A\bin' +\
                         os.pathsep + r'C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\bin' +\
                         os.pathsep + env['ENV']['PATH']

  unirun_env = env.Clone()
  env.Append(LIBPATH = [pyFolder+'libs', wxFolder+'lib/vc_lib'])
  unirun_env.Append(CPPPATH=[wxFolder+'include', wxFolder+'lib/vc_lib/mswud'])
  unirun_env.Append(LIBPATH = [wxFolder+'lib/vc_lib'])
else:
  if env['TOOL'] != 'intel':
    env.Append(CCFLAGS = ['-exceptions']) 
  if debug:
    env.Append(CCFLAGS = ['-g']) 
  else:
    env.Append(CCFLAGS = ['-O3']) 
  if profiling:
    env.Append(CCFLAGS = ['-pg']) 
    env.Append(LINKFLAGS=['-pg'])
  try: 
    if sys.platform[:6] == 'darwin':
      env.Append(CCFLAGS = '-D__MAC__')
      env.ParseConfig("wx-config --cxxflags --unicode --libs gl,core,html,net,aui")
      env.Append(FRAMEWORKS=['OpenGL', 'AGL', 'Python'])
    else:
      env.ParseConfig("wx-config --cxxflags --unicode --toolkit=gtk2 --libs gl,core,html,net,aui")
#!!!
    unirun_env = env.Clone()
    env.ParseConfig("python-config --includes")
    env.ParseConfig("python-config --ldflags")
  except:
    print 'Please make sure that wxWidgets and Python config scripts are available'
    Exit(1)
    
#sdl
sdl_files = fileListToStringList('sdl', sdl) + fileListToStringList('sdl/smart', sdl_smart) +\
  fileListToStringList('sdl/exparse', sdl_exp)
sdl_files = processFileNameList(sdl_files, env, out_dir + 'sdl')
env.StaticLibrary(out_dir + 'lib/sdl', sdl_files)

env.Append(LIBPATH=[out_dir+'lib'])
env.Append(LIBS = ['sdl'])

unirun_env.Append(LIBPATH=[out_dir+'lib'])
unirun_env.Append(LIBS=['sdl'])

generic_files = fileListToStringList('alglib', alglib) + \
                fileListToStringList('xlib', xlib) + \
                fileListToStringList('xlib/macro', xlib_macro) + \
                repository
generic_files = processFileNameList(generic_files, env, out_dir+'generic')

olex2c_env = env.Clone()

olex2_files = fileListToStringList('glib', glib) + \
              fileListToStringList('gxlib', gxlib) + \
              fileListToStringList('olex', olex2) + \
              fileListToStringList('olex/html', olex2_html) + \
              fileListToStringList('olex/ctrls', olex2_ctrls)
olex2_files = processFileNameList(olex2_files, env, out_dir+'olex')  
#link in the res file...
if sys.platform[:3] == 'win':
  res_file = out_dir + 'olex/app.res'
  olex2_files = olex2_files + env.RES(res_file, 'olex/app.rc')
  env.Append(RCFLAGS=['/l 0x809'])
  env.Append(LINKFLAGS=['/PDB:' + out_dir + 'exe/olex2.pdb'])
env.Program(out_dir+'exe/olex2', generic_files + olex2_files)

unirun_files = np_repository + fileListToStringList('unirun', unirun)
unirun_files.append('./repository/patchapi.cpp')
unirun_files.append('./repository/updateapi.cpp')
unirun_files = processFileNameList(unirun_files, unirun_env, out_dir+'unirun')

unirun_env.Append(CCFLAGS = ['-D_NO_PYTHON'])
if sys.platform[:3] == 'win':
  unirun_env.Append(LINKFLAGS=['/PDB:' + out_dir + 'exe/unirun.pdb'])
unirun_env.Program(out_dir+'exe/unirun', unirun_files)

# make olex2c?

olex2c_files = processFileNameList(olex2c, olex2c_env, out_dir+'olex2c')
if sys.platform[:3] == 'win':
  res_file = out_dir + 'olex2c/app.res'
  olex2c_files = olex2c_files + olex2c_env.RES(res_file, 'olex2c/app.rc')
  olex2c_env.Append(RCFLAGS=['/l 0x809'])
  olex2c_env.Append(LINKFLAGS=['/PDB:' + out_dir + 'exe/olex2c.pdb'])
else:
  olex2c_env.Append(LIBS=['readline'])

olex2c_env.Program(out_dir+'exe/olex2c', generic_files + olex2c_files)

try:
  import _imaging
except:
  print '!! You will need to install PIL' 