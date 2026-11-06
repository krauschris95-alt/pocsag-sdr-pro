"""Build pinned official Airspy/HF+ libraries with the package's MinGW/libusb runtime."""
import argparse,hashlib,pathlib,shutil,subprocess,urllib.request,zipfile,os,ctypes
p=argparse.ArgumentParser();p.add_argument('--cache',default='build-airspy-deps');p.add_argument('--destination',default='dist/POCSAG-SDR-Pro');p.add_argument('--usb-sdk',default='build-sdr-deps/libusb-sdk');p.add_argument('--cmake',default='cmake');a=p.parse_args()
cache=pathlib.Path(a.cache).resolve();cache.mkdir(parents=True,exist_ok=True);dest=pathlib.Path(a.destination).resolve();dest.mkdir(parents=True,exist_ok=True);sdk=pathlib.Path(a.usb_sdk).resolve()
sources=[('airspy','airspyone_host','fc61ab6be57ed61f0e2bdd9c6dfae74cacef57d0','b2a842b9da3abcfc907842d3606f143f264f74cacd16879406051c311997238d'),('airspyhf','airspyhf','24fe8ffcb00b14f827268bbad89ae1392de055e5','fd3e578e6f01636a7d6602f11c6aed18efbb985b4a12d93c5bba465c2ed272f3')]
def run(*cmd):subprocess.run([str(x) for x in cmd],check=True)
def buildpath(path):
 path=str(pathlib.Path(path).resolve())
 if os.name=='nt':
  out=ctypes.create_unicode_buffer(32768)
  if ctypes.windll.kernel32.GetShortPathNameW(path,out,len(out)):path=out.value
 return path.replace('\\','/')
for name,repo,commit,digest in sources:
 archive=cache/(name+'.zip')
 if not archive.exists():urllib.request.urlretrieve(f'https://codeload.github.com/airspy/{repo}/zip/{commit}',archive)
 if hashlib.sha256(archive.read_bytes()).hexdigest()!=digest:raise RuntimeError('Official source hash mismatch: '+name)
 unpack=cache/(name+'-source')
 if not unpack.exists():
  with zipfile.ZipFile(archive) as z:
   for f in z.infolist():
    if not (unpack/f.filename).resolve().is_relative_to(unpack):raise RuntimeError('Unsafe archive path')
   z.extractall(unpack)
 source=unpack/(repo+'-'+commit);build=cache/(name+'-x64-final-build');build.mkdir(exist_ok=True)
 # The upstream resource command/windres does not preserve spaces through its
 # preprocessor. Use Windows' existing short aliases, without moving files.
 resource=source/('lib'+name)/'src/CMakeLists.txt'
 text=resource.read_text();text=text.replace('COMMAND ${CMAKE_RC_COMPILER} --preprocessor=gcc\n','COMMAND ${CMAKE_RC_COMPILER}\n');resource.write_text(text)
 run(a.cmake,'-S',buildpath(source),'-B',buildpath(build),'-G','Ninja',f'-DCMAKE_C_COMPILER={buildpath(shutil.which("gcc"))}',f'-DCMAKE_CXX_COMPILER={buildpath(shutil.which("g++"))}',f'-DCMAKE_RC_COMPILER={buildpath(shutil.which("windres"))}','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_POLICY_VERSION_MINIMUM=3.5',f'-DLIBUSB_INCLUDE_DIR={buildpath(sdk)}',f'-DLIBUSB_LIBRARIES={buildpath(sdk)}/libusb-1.0.dll.a;winpthread','-DTHREADS_PTHREADS_WIN32_LIBRARY=winpthread','-DTHREADS_PTHREADS_INCLUDE_DIR=','-DINSTALL_STATIC_LIBS=OFF')
 run(a.cmake,'--build',build,'--target',name,'--parallel','2')
 binaries=list(build.rglob('lib'+name+'.dll'))+list(build.rglob(name+'.dll'))
 if not binaries:raise RuntimeError('Built Airspy DLL not found')
 shutil.copy2(binaries[0],dest/('lib'+name+'.dll'))
 (dest/'sources').mkdir(exist_ok=True);shutil.copy2(archive,dest/'sources'/f'{name}-{commit}.zip')
 licenses=dest/'licenses'/name;licenses.mkdir(parents=True,exist_ok=True)
 license_files=[x for x in source.iterdir() if x.is_file() and x.name.lower() in ('license','license.md','copying','license.txt')]
 if not license_files:
  header=(source/('lib'+name)/'src'/f'{name}.h').read_text()
  notice=header[header.index('/*'):header.index('*/')+2]
  if 'Redistribution' not in notice or 'DISCLAIM' not in notice:raise RuntimeError('Airspy license notice not found')
  (licenses/'LICENSE-from-official-header.txt').write_text(notice+'\n')
 for f in license_files:shutil.copy2(f,licenses/f.name)
