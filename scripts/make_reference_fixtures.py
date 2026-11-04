"""Generate benign IQ fixtures with the independent, pinned multimon-ng gen-ng.
The application encoder is deliberately never imported or called.
"""
import argparse,json,pathlib,struct,zlib
import numpy as np
from compare_reference import audio_to_iq,oracle,ours,run
p=argparse.ArgumentParser();p.add_argument('--gen',required=True);p.add_argument('--multimon',required=True);p.add_argument('--decoder',required=True);p.add_argument('--work',type=pathlib.Path,required=True);args=p.parse_args();args.work.mkdir(parents=True,exist_ok=True)
destination=pathlib.Path('tests/fixtures');destination.mkdir(exist_ok=True)
manifest=[]
for baud in (512,1200,2400):
    for mode,text,address in [('ALPHA','REFERENCE 012 FUNCTION TEST',1234567),('NUMERIC','123-456.789U][',765432)]:
        name=f'{baud}-{mode.lower()}';raw=args.work/(name+'.raw')
        command=[args.gen,'-t','raw','-P',text,'-A',str(address),'-B',str(baud)]
        if mode=='NUMERIC':command+=['-N']
        command+=[raw];run(command)
        audio=np.fromfile(raw,dtype='<i2').astype(np.float64)/32768
        iq=args.work/(name+'.cu8');audio_to_iq(audio).tofile(iq)
        reference=oracle(raw,args.multimon,mode);actual=ours(iq,args.decoder,mode)
        assert len(reference)==1 and actual==reference,(name,reference,actual)
        data=iq.read_bytes();filename=name+'.iqz';(destination/filename).write_bytes(struct.pack('>I',len(data))+zlib.compress(data,9))
        row=reference[0];manifest.append(dict(file=filename,mode=mode,ric=row[0],function=row[1],baud=row[2],type=row[3],text=row[4]))
(destination/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(f'{len(manifest)} independently generated IQ fixtures verified against multimon-ng')
