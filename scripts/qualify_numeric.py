"""Aggregate-only comparison. IQ/audio and message text remain in private work files.
Uses independent multimon-ng and a separate box-FIR/FM/linear resampler, standard library only.
"""
import argparse,array,collections,hashlib,json,math,os,pathlib,re,subprocess,sys
p=argparse.ArgumentParser();p.add_argument('--capture',type=pathlib.Path,required=True);p.add_argument('--report',type=pathlib.Path,required=True);a=p.parse_args()
root=pathlib.Path.cwd();env=os.environ.copy();env['PATH']=str(root/'.tools/Qt/Tools/mingw1310_64/bin')+os.pathsep+str(root/'.tools/Qt/6.8.3/mingw_64/bin')+os.pathsep+env['PATH']
def run(args):
 r=subprocess.run([str(x) for x in args],capture_output=True,env=env,check=True);return r.stdout.decode('utf-8','replace')
def ours(path,mode):
 rows=[]
 for line in run([root/'build/pocsag-iq-decode.exe',path,mode,'ALL']).splitlines():
  v=line.split('\t');rows.append(v)
 return rows
names=['NUL','SOH','STX','ETX','EOT','ENQ','ACK','BEL','BS','HT','LF','VT','FF','CR','SO','SI','DLE','DC1','DC2','DC3','DC4','NAK','SYN','ETB','CAN','EM','SUB','ESC','FS','GS','RS','US']
def normalize(text,mode):
 for i,name in enumerate(names):text=text.replace('<'+name+'>',chr(i))
 text=text.replace('<DEL>',chr(127))
 text=re.sub(r'<0x([0-9A-F]{2})>',lambda x:chr(int(x[1],16)),text)
 if mode=='ALPHA':text=re.split('[\x00\x03\x04]',text)[0]
 return text.rstrip()
def oracle(path,mode):
 output=run([root/'.tools/multimon-build/multimon-ng.exe','-q','-t','raw','-a','POCSAG512','-a','POCSAG1200','-a','POCSAG2400','-f',mode.lower(),'-e',path]);rows=[]
 for baud,ric,fn,kind,text in re.findall(r'POCSAG(512|1200|2400): Address:\s*(\d+)\s+Function:\s*(\d+)\s+(Alpha|Numeric):\s{1,3}(.*)',output):rows.append((int(ric),int(fn),int(baud),normalize(text,mode)))
 return rows
def cmp(e,o):
 ce,co=collections.Counter(e),collections.Counter(o);return {'reference':len(e),'application':len(o),'exact_matches':sum((ce&co).values()),'reference_unmatched':sum((ce-co).values()),'application_unmatched':sum((co-ce).values())}
report={'independent_tool':'multimon-ng b5f7d71c5e9b5a76093ba55c515968dc40f92804','synthetic':{}}
for baud in (512,1200,2400):
 for mode in ('ALPHA','NUMERIC'):
  stem=root/f'.tools/generated-reference/{baud}-{mode.lower()}';expected=oracle(stem.with_suffix('.raw'),mode);actual=[(int(v[0]),int(v[1]),int(v[2]),normalize(bytes.fromhex(v[4]).decode('ascii'),mode)) for v in ours(stem.with_suffix('.cu8'),mode) if v[3] not in ('TONE','INCOMPLETE')];report['synthetic'][f'{baud}-{mode}']=cmp(expected,actual)
# Separate discriminator: 5-tap rectangular low-pass, complex decimation 240k ->48k,
# then phase difference and independent linear 48k ->22050 interpolation.
data=a.capture.read_bytes();audio=array.array('h');previous=complex(1,0);fm=[]
for offset in range(0,len(data)-9,10):
 z=complex(sum(data[offset:offset+10:2])-637.5,sum(data[offset+1:offset+10:2])-637.5);v=z*previous.conjugate();previous=z;fm.append(math.atan2(v.imag,v.real))
mean=sum(fm)/max(1,len(fm));peak=max((abs(x-mean) for x in fm),default=1)
for k in range(int(len(fm)*22050/48000)-1):
 pos=k*48000/22050;i=int(pos);f=pos-i;v=fm[i]*(1-f)+fm[i+1]*f;audio.append(round((v-mean)/peak*26000))
if sys.byteorder!='little':audio.byteswap()
raw=root/'.tools/private-rf-0112-reference.raw';raw.write_bytes(audio.tobytes());del fm,audio,data
inspected=ours(a.capture,'INSPECT');private={'capture_sha256':hashlib.sha256(a.capture.read_bytes()).hexdigest(),'iq_bytes':a.capture.stat().st_size,'duration_seconds':a.capture.stat().st_size/480000,'complete':sum(v[3]=='1' for v in inspected),'incomplete':sum(v[3]!='1' for v in inspected),'messages_with_corrected_bits':sum(int(v[4])>0 for v in inspected),'alpha_control_payloads':sum(v[5]=='0' for v in inspected)}
for mode,col in [('ALPHA',6),('NUMERIC',7)]:
 actual=[(int(v[0]),int(v[1]),int(v[2]),normalize(bytes.fromhex(v[col]).decode('ascii'),mode)) for v in inspected if v[3]=='1' and v[8]];expected=oracle(raw,mode);private[mode]=cmp(expected,actual)
 private[mode]['application_unmatched_shapes']=[{'baud':r[2],'function':r[1],'length':len(r[3])} for r in (collections.Counter(actual)-collections.Counter(expected)).elements()]
 if mode=='ALPHA':
  unmatched=list((collections.Counter(expected)-collections.Counter(actual)).elements());stats=[]
  for e in unmatched:
   candidates=[o for o in actual if o[:3]==e[:3]]
   stats.append({'same_address_function_baud_candidates':len(candidates),'reference_length':len(e[3]),'application_lengths':[len(o[3]) for o in candidates],'first_difference_codepoints':[[next(([ord(x),ord(y)] for x,y in zip(e[3],o[3]) if x!=y),[])][0] for o in candidates]})
  private['alpha_difference_shapes']=stats
 matched=collections.Counter(expected)&collections.Counter(actual)
 if mode=='ALPHA':private['matched_long_compact_alpha']=sum(n for row,n in matched.items() if len(row[3])>=24 and ' ' not in row[3])
 if mode=='ALPHA':private['matched_control_or_symbol_rich_alpha']=sum(n for row,n in matched.items() if any(ord(c)<32 and c not in '\t\r\n' for c in row[3]) or len(row[3])>=20 and sum(not c.isalnum() and not c.isspace() for c in row[3])/len(row[3])>.25)
report['private_rf_capture']=private;report['limitations']=['RF sender encoding is unknown; numeric rendering alone is not evidence of a numeric transmission.','Independent discriminator differs from the application; unmatched frames are reported, not hidden.','Synthetic numeric IQ is independently generated; no identified live numeric transmitter confirmed.']
a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report,indent=2))
