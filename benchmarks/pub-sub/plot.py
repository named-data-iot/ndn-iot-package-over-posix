import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
from matplotlib.pyplot import figure
import re

catagories = ['core i7 2.2GHz', 'core i7 2.2GHz std', 'cortex 1.4GHz', 'cortex 1.4GHz std']
overall = dict()
overall['pub-enc'] = []
overall['pub-sign'] = []
overall['pub-encoding'] = []
overall['sub-decoding'] = []
overall['sub-verify'] = []
overall['sub-schema'] = []
overall['sub-dec'] = []

def parse_results(dir: str):
    for i in ['pub', 'sub']:
        v1 = []
        v2 = []
        v3 = []
        v4 = []
        with open(F'{dir}/{i}.dat') as file:
            for line in file:
                if 'PUB-CONTENT-DATA-AES-ENC' in line or 'SUB-NEW-DATA-AES-DEC' in line:
                    v1.append(int(line.strip().split(':')[-1][:-2]))
                elif 'DATA-PKT-ECDSA-SIGN' in line or 'DATA-PKT-ECDSA-VERIFY' in line:
                    v2.append(int(line.strip().split(':')[-1][:-2]))
                elif 'DATA-PKT-ENCODING' in line or 'DATA-PKT-DECODING' in line:
                    v3.append(int(line.strip().split(':')[-1][:-2]))
                elif 'SUB-NEW-DATA-SCHEMA-VERIFY' in line:
                    v4.append(int(line.strip().split(':')[-1][:-2]))
                else:
                    pass
        if i is 'pub':
            overall['pub-enc'].append(np.mean(v1))
            overall['pub-enc'].append(np.std(v1))
            overall['pub-sign'].append(np.mean(v2))
            overall['pub-sign'].append(np.std(v2))
            overall['pub-encoding'].append(np.mean(v3))
            overall['pub-encoding'].append(np.std(v3))
        else:
            overall['sub-decoding'].append(np.mean(v3))
            overall['sub-decoding'].append(np.std(v3))
            overall['sub-verify'].append(np.mean(v2))
            overall['sub-verify'].append(np.std(v2))
            overall['sub-schema'].append(np.mean(v4))
            overall['sub-schema'].append(np.std(v4))
            overall['sub-dec'].append(np.mean(v1))
            overall['sub-dec'].append(np.std(v1))

if __name__ == "__main__":
    parse_results('corei7-4770HQ-2.2GHz')
    parse_results('cortex-A53-ARMv8-1.4GHz')

    df = pd.DataFrame(overall, index=catagories)
    print(df)

    fig = plt.figure(figsize=(6, 4))
    ax = fig.add_subplot(111)
    y_pos = np.arange(len(overall.keys()))
    patterns = [ "///" , "..", "xx", "ooo", "\\\\"]
    height = 0.4

    p1 = ax.barh(y_pos - height/2, df.loc[catagories[0]].tolist(), xerr=df.loc[catagories[1]].tolist(), height=height, hatch=patterns[0], edgecolor='gray')
    p2 = ax.barh(y_pos + height/2, df.loc[catagories[2]].tolist(), xerr=df.loc[catagories[3]].tolist(), height=height, hatch=patterns[1], edgecolor='gray')

    def autolabel_h(rects):
        """Attach a text label above each bar in *rects*, displaying its height."""
        for rect in rects:
            width = rect.get_width()
            ax.annotate('{:0.2f}'.format(width),
                        xy=(width, rect.get_y() + rect.get_height()/2),
                        xytext=(23, 0),  # 3 points vertical offset
                        textcoords="offset points",
                        ha='center', va='bottom')

    autolabel_h(p1)
    autolabel_h(p2)

    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)
    ax.set_xlim(-300, ax.get_xlim()[1])
    ax.set_yticks(y_pos)
    ax.set_yticklabels(overall.keys())
    ax.invert_yaxis()  # labels read top-to-bottom
    ax.set_xlabel('Time Elapse (us)')
    y_labels = ax.get_yticklabels()
    plt.setp(y_labels, rotation=45, horizontalalignment='right')
    # plt.rcParams.update({'font.size': myFontSize})
    ax.legend((p1[0], p2[0]), ('corei7 4770HQ 2.2GHz', 'cortex A53 ARMv8 1.4GHz'))
    fig.tight_layout()
    fig.savefig('performance.pdf', format='pdf', dpi=1000)
    plt.show()
