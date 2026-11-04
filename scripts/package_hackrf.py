"""Bundle the pinned MSYS2 MinGW libraries and their corresponding upstream sources."""
import hashlib,io,pathlib,tarfile,urllib.request,sys
from backports import zstd
destination=pathlib.Path(sys.argv[1]);cache=pathlib.Path(sys.argv[2]);cache.mkdir(parents=True,exist_ok=True)
packages=[('hackrf','mingw-w64-x86_64-hackrf-2026.01.2-1-any.pkg.tar.zst','53980aeb68b15e979d12685b831eacc440e5f8c15f55d9e883f9cef9de5593a7'),('libusb','mingw-w64-x86_64-libusb-1.0.30-1-any.pkg.tar.zst','124edc26b009678a4c5e759481a364abf63ed7f2f49a53092e8bd0a47c0c0740')]
def download(url,path):
    if not path.exists():urllib.request.urlretrieve(url,path)
for package,name,digest in packages:
    archive=cache/name;download('https://mirror.msys2.org/mingw/mingw64/'+name,archive)
    assert hashlib.sha256(archive.read_bytes()).hexdigest()==digest,'Package checksum mismatch'
    with tarfile.open(fileobj=io.BytesIO(zstd.decompress(archive.read_bytes()))) as tar:
        for member in tar.getmembers():
            if not member.isfile():continue
            if member.name.endswith('.dll'):target=destination/pathlib.Path(member.name).name
            elif '/licenses/' in member.name:target=destination/'licenses'/package/pathlib.Path(member.name).name
            elif member.name in ('.PKGINFO','.BUILDINFO'):target=destination/'licenses'/package/member.name[1:]
            else:continue
            target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(tar.extractfile(member).read())
for name,url,digest in [('hackrf-2026.01.2.zip','https://github.com/greatscottgadgets/hackrf/releases/download/v2026.01.2/hackrf-2026.01.2.zip','0135adfecf3a4d9ac47d07897fbcf712c729aa8e9d57de75bc3d65874f604a10'),('libusb-1.0.30.zip','https://github.com/libusb/libusb/archive/refs/tags/v1.0.30.zip','543a61dbc8878435c096aec6543d51ff73f6a80743621c66ecab2ba1eb66a974')]:
    source=cache/name;download(url,source)
    assert hashlib.sha256(source.read_bytes()).hexdigest()==digest,'Source checksum mismatch'
    target=destination/'sources'/name;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(source.read_bytes())
print('HackRF/libusb binaries, licenses, build metadata and sources packaged')
