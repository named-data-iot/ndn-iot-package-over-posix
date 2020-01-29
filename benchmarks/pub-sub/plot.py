import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
from matplotlib.pyplot import figure
import re

catagories = ['core i7 2.2GHz', 'core i7 2.2GHz std', 'cortex 1.4GHz', 'cortex 1.4GHz std', 'cortex 64MHz', 'cortex 64MHz std']
overall = dict()
overall['pub\nencryption'] = []
overall['pub\npkt sign'] = []
overall['pub\nencoding'] = []
overall['sub\ndecoding'] = []
overall['sub\nsig verify'] = []
overall['sub\nschema'] = []
overall['sub\ndecryption'] = []

def parse_results(dir: str):
    for i in ['pub', 'sub']:
        v1 = []
        v2 = []
        v3 = []
        v4 = []
        with open(F'{dir}/{i}.dat') as file:
            for line in file:
                if 'PUB-CONTENT-DATA-AES-ENC' in line or 'SUB-NEW-DATA-AES-DEC' in line or 'PUB-COMMAND-DATA-AES-ENC' in line :
                    v1.append(int(line.strip().split(':')[-1][:-2])/1000)
                elif 'DATA-PKT-ECDSA-SIGN' in line or 'DATA-PKT-ECDSA-VERIFY' in line:
                    v2.append(int(line.strip().split(':')[-1][:-2])/1000)
                elif 'DATA-PKT-ENCODING' in line or 'DATA-PKT-DECODING' in line:
                    v3.append(int(line.strip().split(':')[-1][:-2])/1000)
                elif 'SUB-NEW-DATA-SCHEMA-VERIFY' in line:
                    v4.append(int(line.strip().split(':')[-1][:-2])/1000)
                else:
                    pass
        if i is 'pub':
            overall['pub\nencryption'].append(np.mean(v1))
            overall['pub\nencryption'].append(np.std(v1))
            overall['pub\npkt sign'].append(np.mean(v2))
            overall['pub\npkt sign'].append(np.std(v2))
            overall['pub\nencoding'].append(np.mean(v3))
            overall['pub\nencoding'].append(np.std(v3))
        else:
            overall['sub\ndecoding'].append(np.mean(v3))
            overall['sub\ndecoding'].append(np.std(v3))
            overall['sub\nsig verify'].append(np.mean(v2))
            overall['sub\nsig verify'].append(np.std(v2))
            overall['sub\nschema'].append(np.mean(v4))
            overall['sub\nschema'].append(np.std(v4))
            overall['sub\ndecryption'].append(np.mean(v1))
            overall['sub\ndecryption'].append(np.std(v1))

if __name__ == "__main__":
    parse_results('corei7-4770HQ-2.2GHz')
    parse_results('cortex-A53-ARMv8-1.4GHz')
    parse_results('cortex-M4-64MHz')

    df = pd.DataFrame(overall, index=catagories)
    print(df)

    fig = plt.figure(figsize=(6, 4))
    ax = fig.add_subplot(111)

    y_pos = np.arange(len(overall.keys()))
    patterns = [ "///" , "..", "xx", "ooo", "\\\\"]
    height = 0.3

    p1 = ax.barh(y_pos - height, df.loc[catagories[0]].tolist(), xerr=df.loc[catagories[1]].tolist(), 
                height=height, hatch=patterns[0], edgecolor='gray')
    p2 = ax.barh(y_pos, df.loc[catagories[2]].tolist(), xerr=df.loc[catagories[3]].tolist(), 
                height=height, hatch=patterns[1], edgecolor='gray')
    p3 = ax.barh(y_pos + height, df.loc[catagories[4]].tolist(), xerr=df.loc[catagories[5]].tolist(), 
                height=height, hatch=patterns[2], edgecolor='gray')

    def autolabel_h(rects, ax):
        """Attach a text label above each bar in *rects*, displaying its height."""
        for rect in rects:
            width = rect.get_width()
            ax.annotate('{:0.2f}'.format(width),
                            xy=(width, rect.get_y() + rect.get_height()/2),
                            xytext=(18, -5),
                            textcoords="offset points",
                            ha='center', va='bottom')

    autolabel_h(p1, ax)
    autolabel_h(p2, ax)
    autolabel_h(p3, ax)

    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)
    # ax.set_xlim(-5, ax.get_xlim()[1])
    ax.set_yticks(y_pos)
    ax.set_yticklabels(overall.keys())
    ax.invert_yaxis()  # labels read top-to-bottom
    ax.set_xscale('log')
    ax.set_xticks([0.1, 1, 20, 100])
    ax.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
    ax.set_xlabel('Time Elapse (ms)')
    y_labels = ax.get_yticklabels()
    # plt.setp(y_labels, rotation=45, horizontalalignment='right')
    # plt.rcParams.update({'font.size': myFontSize})
    ax.legend((p1[0], p2[0], p3[0]), ('core i7 2.2GHz', 'cortex A53 1.4GHz', 'cortex M4 64MHz'),
              ncol=3,
              bbox_to_anchor=(0, 1),
              loc='lower left', fontsize='small')

    fig.tight_layout()
    fig.savefig('micro-bench-pub-sub.pdf', format='pdf', dpi=1000)
    plt.show()
