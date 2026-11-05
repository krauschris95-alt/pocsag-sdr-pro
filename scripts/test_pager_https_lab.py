"""Isolated loopback TLS LAB; never connects to PrivateRadio."""
import base64,datetime,http.server,ipaddress,json,pathlib,ssl,subprocess,sys,tempfile,threading
from cryptography import x509
from cryptography.hazmat.primitives import hashes,serialization
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.x509.oid import NameOID
def main():
    executable=pathlib.Path(sys.argv[1]).resolve()
    report=pathlib.Path(sys.argv[2]).resolve()
    with tempfile.TemporaryDirectory(prefix="pocsag-https-lab-") as folder:
        root=pathlib.Path(folder)
        key=rsa.generate_private_key(public_exponent=65537,key_size=2048)
        name=x509.Name([x509.NameAttribute(NameOID.COMMON_NAME,"POCSAG isolated LAB")])
        now=datetime.datetime.now(datetime.timezone.utc)
        cert=(x509.CertificateBuilder().subject_name(name).issuer_name(name).public_key(key.public_key()).serial_number(x509.random_serial_number()).not_valid_before(now-datetime.timedelta(minutes=5)).not_valid_after(now+datetime.timedelta(days=1)).add_extension(x509.SubjectAlternativeName([x509.IPAddress(ipaddress.ip_address("127.0.0.1"))]),critical=False).add_extension(x509.BasicConstraints(ca=True,path_length=None),critical=True).sign(key,hashes.SHA256()))
        (root/"cert.pem").write_bytes(cert.public_bytes(serialization.Encoding.PEM))
        (root/"key.pem").write_bytes(key.private_bytes(serialization.Encoding.PEM,serialization.PrivateFormat.PKCS8,serialization.NoEncryption()))
        seen=set();stats={"unauthorized":0,"accepted_unique":0,"duplicates":0,"private_path_rejected":0}
        class Handler(http.server.BaseHTTPRequestHandler):
            def log_message(self,*args):pass
            def do_POST(self):
                if self.headers.get("Authorization")!="Bearer lab-only-token":
                    stats["unauthorized"]+=1;self.send_response(401);self.end_headers();return
                payload=json.loads(self.rfile.read(int(self.headers["Content-Length"])))
                event=payload["event"];audio=base64.b64decode(payload["audio_base64"],validate=True)
                if "path" in event.get("audio",{}) or not audio.startswith(b"RIFF") or self.headers.get("Idempotency-Key")!=event["message_id"]:
                    stats["private_path_rejected"]+=1;self.send_response(400);self.end_headers();return
                identity=event["message_id"]
                if identity in seen:stats["duplicates"]+=1
                else:seen.add(identity);stats["accepted_unique"]+=1
                data=json.dumps({"accepted":True,"message_id":identity}).encode()
                self.send_response(200);self.send_header("Content-Type","application/json");self.send_header("Content-Length",str(len(data)));self.end_headers();self.wfile.write(data)
        server=http.server.ThreadingHTTPServer(("127.0.0.1",0),Handler)
        ctx=ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER);ctx.minimum_version=ssl.TLSVersion.TLSv1_2;ctx.load_cert_chain(root/"cert.pem",root/"key.pem")
        server.socket=ctx.wrap_socket(server.socket,server_side=True)
        thread=threading.Thread(target=server.serve_forever,daemon=True);thread.start()
        try:result=subprocess.run([str(executable),"--https",f"https://127.0.0.1:{server.server_port}/isolated-lab-events",str(root/"cert.pem"),"-platform","offscreen"],capture_output=True,text=True,timeout=90)
        finally:server.shutdown();server.server_close();thread.join()
        stats.update({"test_exit":result.returncode,"loopback_only":True,"official_server_contacted":False,"private_audio_recorded":False})
        report.write_text(json.dumps(stats,indent=2),encoding="utf-8")
        print(result.stdout);print(result.stderr,file=sys.stderr)
        assert result.returncode==0 and stats["accepted_unique"]==1 and stats["duplicates"]==1 and stats["unauthorized"]==1,stats
if __name__=="__main__":main()
