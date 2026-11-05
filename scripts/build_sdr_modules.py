"""Build pinned official SoapySDR and RTL module for the Windows package (build tools only)."""
import argparse,pathlib,urllib.request,hashlib,zipfile,subprocess,shutil,io,tarfile
try:
    from compression import zstd  # Python 3.14+ standard library
except ImportError:
    from backports import zstd  # Compatibility with older local build runtimes
parser=argparse.ArgumentParser();parser.add_argument('--cache',default='build-sdr-deps');parser.add_argument('--destination',default='dist/POCSAG-SDR-Pro');parser.add_argument('--cmake',default='cmake');args=parser.parse_args()
root=pathlib.Path(args.cache).resolve();root.mkdir(parents=True,exist_ok=True);destination=pathlib.Path(args.destination).resolve();destination.mkdir(parents=True,exist_ok=True)
packages=[
 ('SoapySDR','https://github.com/pothosware/SoapySDR/archive/refs/tags/soapy-sdr-0.8.1.zip','2d535ef1ebaeeb49214874303cf7e341bfe0b8bf27fd48220c9a1ed8f5f50353'),
 ('SoapyRTLSDR','https://github.com/pothosware/SoapyRTLSDR/archive/refs/tags/soapy-rtl-sdr-0.3.3.zip','268245ff1df5a7a446b99684ecd89a68abdab1d0f45b3327236cd38d2c2c91a8'),
 ('rtl-binary','https://github.com/rtlsdrblog/rtl-sdr-blog/releases/download/V1.4.0/Release.zip','7ef33f1304647f65e5e0fde43637a73d54f076e91e651a3cecc4f55a17fd9815'),
 ('rtl-source','https://github.com/rtlsdrblog/rtl-sdr-blog/archive/refs/tags/V1.4.0.zip','efaac50d53ea2f12d9853167ac9470bf4f04de944f17c5d52d2fd73a9ec24c18'),
 ('nsis','https://downloads.sourceforge.net/project/nsis/NSIS%203/3.12/nsis-3.12.zip','56581f90db321581c5381193d796fffcf2d24b2f8fed2160a6c6a3baa67f2c4f'),
]
for name,url,digest in packages:
    archive=root/(name+'.zip')
    if not archive.exists():urllib.request.urlretrieve(url,archive)
    if hashlib.sha256(archive.read_bytes()).hexdigest()!=digest:raise RuntimeError('Checksum mismatch '+name)
    unpack=root/(name+'-source')
    if not unpack.exists():
        with zipfile.ZipFile(archive) as z:
            for member in z.infolist():
                target=(unpack/member.filename).resolve()
                if not target.is_relative_to(unpack):raise RuntimeError('Invalid archive path')
            z.extractall(unpack)
prefix=root/'prefix'
def run(*command):subprocess.run([str(x) for x in command],check=True)
# Rebuild the same RTL-SDR Blog sources with MinGW: no VCRUNTIME140/MSVCR100 runtime.
usb_archive=root/'libusb.pkg.tar.zst'
if not usb_archive.exists():urllib.request.urlretrieve('https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libusb-1.0.30-1-any.pkg.tar.zst',usb_archive)
if hashlib.sha256(usb_archive.read_bytes()).hexdigest()!='124edc26b009678a4c5e759481a364abf63ed7f2f49a53092e8bd0a47c0c0740':raise RuntimeError('libusb checksum mismatch')
sdk=root/'libusb-sdk';sdk.mkdir(exist_ok=True)
with tarfile.open(fileobj=io.BytesIO(zstd.decompress(usb_archive.read_bytes()))) as tar:
    for member in tar.getmembers():
        if member.isfile() and (member.name.endswith('libusb.h') or member.name.endswith('.dll.a') or member.name.endswith('libusb-1.0.dll')):
            (sdk/pathlib.Path(member.name).name).write_bytes(tar.extractfile(member).read())
run(args.cmake,'-S',root/'rtl-source-source/rtl-sdr-blog-1.4.0','-B',root/'rtl-native-build','-G','Ninja','-DCMAKE_POLICY_VERSION_MINIMUM=3.5','-DCMAKE_C_COMPILER=gcc','-DCMAKE_BUILD_TYPE=Release',f'-DLIBUSB_INCLUDE_DIRS={sdk}',f'-DLIBUSB_LIBRARIES={sdk}/libusb-1.0.dll.a;winpthread')
run(args.cmake,'--build',root/'rtl-native-build','--target','rtlsdr','--parallel','2')
shutil.copy2(root/'rtl-native-build/src/librtlsdr.dll',destination/'rtlsdr.dll');shutil.copy2(sdk/'libusb-1.0.dll',destination)
old_thread=destination/'pthreadVC2.dll'
if old_thread.exists():old_thread.unlink()
common=['-G','Ninja','-DCMAKE_POLICY_VERSION_MINIMUM=3.5','-DCMAKE_CXX_COMPILER=g++','-DCMAKE_BUILD_TYPE=Release',f'-DCMAKE_INSTALL_PREFIX={prefix}','-DENABLE_PYTHON=OFF','-DENABLE_PYTHON3=OFF','-DENABLE_DOCS=OFF','-DENABLE_TESTS=OFF']
run(args.cmake,'-S',root/'SoapySDR-source/SoapySDR-soapy-sdr-0.8.1','-B',root/'core-build',*common)
run(args.cmake,'--build',root/'core-build','--parallel','2');run(args.cmake,'--install',root/'core-build')
run(args.cmake,'-S',root/'SoapyRTLSDR-source/SoapyRTLSDR-soapy-rtl-sdr-0.3.3','-B',root/'rtl-build',*common,f'-DCMAKE_PREFIX_PATH={prefix}',f'-DRTLSDR_INCLUDE_DIR={root}/rtl-source-source/rtl-sdr-blog-1.4.0/include',f'-DRTLSDR_LIBRARY={root}/rtl-binary-source/x64/rtlsdr.lib')
run(args.cmake,'--build',root/'rtl-build','--parallel','2');run(args.cmake,'--install',root/'rtl-build')
shutil.copy2(prefix/'bin/libSoapySDR.dll',destination)
modules=destination/'sdr-modules';modules.mkdir(exist_ok=True);shutil.copy2(prefix/'lib/SoapySDR/modules0.8/librtlsdrSupport.dll',modules)
for name in ['SoapySDR','SoapyRTLSDR']:
    source=destination/'sources';source.mkdir(exist_ok=True);shutil.copy2(root/(name+'.zip'),source)
license_dir=destination/'licenses/SoapySDR';license_dir.mkdir(parents=True,exist_ok=True);shutil.copy2(root/'SoapySDR-source/SoapySDR-soapy-sdr-0.8.1/LICENSE_1_0.txt',license_dir)
license_dir=destination/'licenses/SoapyRTLSDR';license_dir.mkdir(parents=True,exist_ok=True);shutil.copy2(root/'SoapyRTLSDR-source/SoapyRTLSDR-soapy-rtl-sdr-0.3.3/LICENSE.txt',license_dir)
print('NSIS compiler:',root/'nsis-source/nsis-3.12/makensis.exe')
