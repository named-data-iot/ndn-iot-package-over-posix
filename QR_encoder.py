import pyqrcode
import json

info = {}
info["device_identifier"] = "default"
info["public_key"] = "default"
info["symmetric_key"] = "default"

with open("build/tutorial_shared_info.txt") as fp:
  fp.readline()
  info["device_identifier"] = fp.readline()[:-1]
  info["public_key"] = fp.readline()[:-1]
  info["symmetric_key"] = fp.readline()

print(info["device_identifier"])
print(info["public_key"])
print(info["symmetric_key"])

shared_info = pyqrcode.create(json.dumps(info))
shared_info.png('shared_info.png', scale=5)
cmd_line_qr = shared_info.terminal(quiet_zone=1)
print(cmd_line_qr)