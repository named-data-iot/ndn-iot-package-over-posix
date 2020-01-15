import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
from matplotlib.pyplot import figure
import re

overall = {}
overall['other-crypto'] = []
overall['other-crypto-std'] = []
overall['encode-decode'] = []
overall['encode-decode-std'] = []
overall['sign-verify'] = []
overall['sign-verify-std'] = []
overall['enc-dec'] = []
overall['enc-dec-std'] = []

data = {}
data['other-crypto'] = []
data['encode-decode'] = []
data['sign-verify'] = []
data['enc-dec'] = []

def parse_result(file_name: str):
  tmp_data = {}
  tmp_data['other-crypto'] = 0
  tmp_data['encode-decode'] = 0
  tmp_data['sign-verify'] = 0
  tmp_data['enc-dec'] = 0
  with open(file_name) as file:
    for line in file:
      if 'BOOTSTRAPPING-INT1-ECDH-KEYGEN:' in line or 'BOOTSTRAPPING-DATA1-ECDH' in line or 'BOOTSTRAPPING-DATA1-HKDF' in line:
        tmp_data['other-crypto'] += int(line.strip().split(':')[-1][:-2])
      elif 'BOOTSTRAPPING-INT1-PKT-ENCODING:' in line or 'DATA-PKT-DECODING' in line or 'BOOTSTRAPPING-INT2-PKT-ENCODING' in line:
        tmp_data['encode-decode'] += int(line.strip().split(':')[-1][:-2])
      elif 'BOOTSTRAPPING-INT1-PKT-ECDSA-SIGN' in line or 'DATA-PKT-HMAC-VERIFY' in line or 'BOOTSTRAPPING-INT2-ECDSA-SIGN' in line:
        tmp_data['sign-verify'] += int(line.strip().split(':')[-1][:-2])
      elif 'BOOTSTRAPPING-DATA2-PKT-AES-DEC' in line:
        tmp_data['enc-dec'] += int(line.strip().split(':')[-1][:-2])
      else:
        pass

  data['other-crypto'].append(tmp_data['other-crypto'])
  data['encode-decode'].append(tmp_data['encode-decode'])
  data['sign-verify'].append(tmp_data['sign-verify'])
  data['enc-dec'].append(tmp_data['enc-dec'])

def parse_result_per_type():
  dir = 'corei7-4770HQ-2.2GHz'
  for i in range(1, 6):
    parse_result(dir + '/' + str(i) + '.dat')
  df = pd.DataFrame(data)
  print(df)
  overall['other-crypto'].append(df['other-crypto'].mean())
  overall['other-crypto-std'].append(df['other-crypto'].std())
  overall['encode-decode'].append(df['encode-decode'].mean())
  overall['encode-decode-std'].append(df['encode-decode'].std())
  overall['sign-verify'].append(df['sign-verify'].mean())
  overall['sign-verify-std'].append(df['sign-verify'].std())
  overall['enc-dec'].append(df['enc-dec'].mean())
  overall['enc-dec-std'].append(df['enc-dec'].std())

if __name__ == "__main__":
  parse_result_per_type()
  labels = ['pc', 'pi']
  df = pd.DataFrame(overall, index=labels)
  print(df)

  fig, ax = plt.subplots(figsize=(14, 7))
  N = 2
  width = 0.35
  ind = np.arange(N)
  p1 = plt.bar(ind, df['sign-verify'], width, yerr=df['sign-verify-std'])
  p2 = plt.bar(ind, df['other-crypto'], width, bottom=df['sign-verify'], yerr=df['other-crypto-std'])
  p3 = plt.bar(ind, df['enc-dec'], width, bottom=df['sign-verify']+df['other-crypto'], yerr=df['enc-dec-std'])
  p4 = plt.bar(ind, df['encode-decode'], width, bottom=df['sign-verify']+df['other-crypto']+df['enc-dec'], yerr=df['encode-decode-std'])

  ax.legend((p1[0], p3[0], p4[0], p2[0]), ('Pkt Sign/Verify', 'Pkt Enc/Dec', 'Pkt Encoding/Decoding', 'Other Crypto'))
  plt.show()