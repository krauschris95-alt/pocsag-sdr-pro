"""Compare release-tag decoders on identical private IQ; reports contain no radio text."""
import pathlib,subprocess,os,json,collections,base64,hashlib
root=pathlib.Path.cwd();work=root/'.tools/compare-0113';env=os.environ.copy();env['PATH']=str(root/'.tools/Qt/Tools/mingw1310_64/bin')+os.pathsep+env['PATH']
# Rebuild each tagged decoder directly from its recorded source, never from current code.
compiler=root/'.tools/Qt/Tools/mingw1310_64/bin/g++.exe'
for version in ['0.1.10','0.1.11','0.1.12']:
 d=work/version;d.mkdir(parents=True,exist_ok=True)
 for name in ['src/decoder.cpp','src/decoder.h','src/dsp.h','tests/iq_decode.cpp']:(d/pathlib.Path(name).name).write_bytes(subprocess.check_output(['git','show',f'v{version}:{name}']))
 subprocess.run([str(compiler),'-std=c++20','-O2','-I',str(d),str(d/'decoder.cpp'),str(d/'iq_decode.cpp'),'-o',str(d/'decode.exe')],check=True,env=env)
def rows(exe,capture,mode):
 result=subprocess.run([str(exe),str(capture),mode,'ALL'],capture_output=True,check=True,env=env)
 return [line.split('\t') for line in result.stdout.decode().splitlines()]
def key(v):return tuple(v[:3])+tuple(v[4:5])
report={}
for name in ['private-rf-012.cu8','private-rf-012b.cu8','private-rf-0112.cu8']:
 capture=root/'.tools'/name;entry={'sha256':hashlib.sha256(capture.read_bytes()).hexdigest(),'iq_bytes':capture.stat().st_size,'versions':{}};baseline={}
 for version in ['0.1.10','0.1.11','0.1.12','working-0.1.13']:
  data={}
  for mode in ['ALPHA','NUMERIC','AUTO']:
   values=rows(root/'build/pocsag-iq-decode.exe' if version.startswith('working-') else work/version/'decode.exe',capture,mode);complete=[v for v in values if v[3]!='INCOMPLETE'];counter=collections.Counter(key(v) for v in complete)
   if version=='0.1.10':baseline[mode]=counter
   data[mode]={'types':dict(collections.Counter(v[3] for v in values)),'complete_payloads_equal_0110':counter==baseline[mode]}
   if mode=='ALPHA':
    b64=[]
    for v in complete:
     text=bytes.fromhex(v[4])
     if len(text)<24 or len(text)%4:continue
     try:b=base64.b64decode(text,validate=True)
     except ValueError:continue
     if base64.b64encode(b)!=text:continue
     try:decoded=b.decode('utf8');binary=any(ord(c)<32 and c not in '\t\r\n' or ord(c)==127 for c in decoded)
     except UnicodeDecodeError:binary=True
     b64.append(binary)
    data['canonical_base64']={'count':len(b64),'non_textual_bytes':sum(b64)}
  entry['versions'][version]=data
 report[name]=entry
out=root/'docs/qualification-0113/version-comparison.json';out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(report,indent=2))
