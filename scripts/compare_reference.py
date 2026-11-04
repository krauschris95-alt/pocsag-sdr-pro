"""Independent validation harness. Private captures/results must stay outside git.

Requires numpy, scipy and soundfile; multimon-ng is only a test oracle, never a runtime dependency.
"""
import argparse, collections, hashlib, json, os, pathlib, re, subprocess
import numpy as np
from scipy import signal
import soundfile as sf

def run(command):
    env = os.environ.copy()
    return subprocess.run([str(x) for x in command], capture_output=True, check=True, env=env).stdout.decode('utf-8', 'replace')

def oracle(path, binary, mode):
    output = run([binary, '-q', '-t', 'raw', '-a', 'POCSAG512', '-a', 'POCSAG1200', '-a', 'POCSAG2400', '-f', mode.lower(), '-e', path])
    rows = []
    for baud, address, function, kind, text in re.findall(r'POCSAG(512|1200|2400): Address:\s*(\d+)\s+Function:\s*(\d+)\s+(Alpha|Numeric):\s{1,3}(.*)', output):
        # The oracle renders ASCII terminators; compare application payload up to terminator.
        text = re.split(r'<(?:EOT|ETX|NUL)>', text)[0].rstrip()
        rows.append((int(address), int(function), int(baud), kind.upper(), text))
    return rows

def ours(path, binary, mode):
    rows=[]
    for line in run([binary, path, mode]).splitlines():
        a,f,b,t,text=line.split('\t')
        rows.append((int(a), int(f), int(b), t, bytes.fromhex(text).decode('ascii').rstrip()))
    return rows

def compare(expected, actual):
    e,a=collections.Counter(expected),collections.Counter(actual)
    exact=sum((e&a).values())
    keys=lambda rows:collections.Counter(row[:3] for row in rows)
    matched=sum((keys(expected)&keys(actual)).values())
    return dict(reference=len(expected), application=len(actual), capcode_function_baud_matches=matched,
                exact_type_text_matches=exact, missed=sum((e-a).values()), extra=sum((a-e).values()))

def control_diagnostics(expected, actual, iq, binary):
    missing=list((collections.Counter(expected)-collections.Counter(actual)).elements())
    rows=[]
    for line in run([binary,iq,'ALPHA','ALL']).splitlines():
        a,f,b,kind,text=line.split('\t');rows.append((int(a),int(f),int(b),kind,bytes.fromhex(text).decode('ascii')))
    names=['NUL','SOH','STX','ETX','EOT','ENQ','ACK','BEL','BS','HT','LF','VT','FF','CR','SO','SI','DLE','DC1','DC2','DC3','DC4','NAK','SYN','ETB','CAN','EM','SUB','ESC','FS','GS','RS','US']
    def normalize(text):
        for value,name in enumerate(names):text=text.replace('<'+name+'>',chr(value))
        text=re.sub(r'<0x([0-9A-F]{2})>',lambda m:chr(int(m[1],16)),text)
        return text.rstrip()
    return sum(any(row[:3]==m[:3] and row[3]=='DIAGNOSTIC' and normalize(row[4])==normalize(m[4]) for row in rows) for m in missing)

def audio_to_iq(audio):
    # Independently reconstruct complex FM from the external recorded discriminator audio.
    audio=signal.resample_poly(audio,1600,147)
    peak=np.percentile(np.abs(audio),80)
    phase=np.cumsum(audio/max(peak,1e-9)*2*np.pi*4500/240000)
    iq=np.empty(phase.size*2,np.uint8)
    iq[0::2]=np.clip(127.5+100*np.cos(phase),0,255).astype(np.uint8)
    iq[1::2]=np.clip(127.5+100*np.sin(phase),0,255).astype(np.uint8)
    return iq

def main():
    p=argparse.ArgumentParser();p.add_argument('--reference-root',type=pathlib.Path,required=True);p.add_argument('--multimon',type=pathlib.Path,required=True);p.add_argument('--decoder',type=pathlib.Path,required=True);p.add_argument('--work',type=pathlib.Path,required=True);p.add_argument('--capture',type=pathlib.Path);p.add_argument('--report',type=pathlib.Path,required=True);args=p.parse_args();args.work.mkdir(parents=True,exist_ok=True)
    report={}
    for baud in (512,1200,2400):
        source=args.reference_root/'test'/'samples'/f'POCSAG_sample_-_{baud}_bps.flac'
        audio,rate=sf.read(source);assert rate==22050 and audio.ndim==1
        raw=args.work/f'ref{baud}.raw';(np.clip(audio,-1,1)*32767).astype('<i2').tofile(raw)
        iq=args.work/f'ref{baud}.cu8';audio_to_iq(audio).tofile(iq)
        expected=oracle(raw,args.multimon,'ALPHA');actual=ours(iq,args.decoder,'ALPHA')
        report[str(baud)]={'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),**compare(expected,actual)}
    if args.capture:
        iq=np.fromfile(args.capture,np.uint8).astype(np.float32);z=(iq[0::2]-127.5)+1j*(iq[1::2]-127.5)
        # Separate discriminator/resampler for the external reference on the exact same RF capture.
        z=signal.resample_poly(z,1,5);fm=np.angle(z[1:]*np.conj(z[:-1]));fm-=np.mean(fm)
        audio=signal.resample_poly(fm,147,320);audio=audio/max(np.max(np.abs(audio)),1e-9)*26000
        raw=args.work/'capture-reference.raw';audio.astype('<i2').tofile(raw)
        expected=oracle(raw,args.multimon,'ALPHA');actual=ours(args.capture,args.decoder,'ALPHA')
        # Deliberately report aggregate counts only. Text/address details never enter this report.
        report['private_rf_capture']={'sha256':hashlib.sha256(args.capture.read_bytes()).hexdigest(),**compare(expected,actual),
            'non_text_control_payload_matches':control_diagnostics(expected,actual,args.capture,args.decoder)}
    args.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
