import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
from matplotlib.pyplot import figure

catagories = ['core i7 2.2GHz', 'core i7 2.2GHz std', 'cortex 1.4GHz', 'cortex 1.4GHz std', 'cortex 64MHz', 'cortex 64MHz std']
overall = dict()
overall['encryption&\ndecryption'] = []
overall['pkt sign&\nsig verify'] = []
overall['encoding&\ndecoding'] = []
overall['other crypto\noperations'] = []

def parse_results(dir: str):
    result_l1 = []
    result_l2 = []
    result_l3 = []
    result_l4 = []
    for i in range(1, 6):
        v1 = 0
        v2 = 0
        v3 = 0
        v4 = 0
        with open(F'{dir}/{i}.dat') as file:
            for line in file:
                if 'BOOTSTRAPPING-INT1-ECDH-KEYGEN:' in line or 'BOOTSTRAPPING-DATA1-ECDH' in line or 'BOOTSTRAPPING-DATA1-HKDF' in line:
                    v1 += int(line.strip().split(':')[-1][:-2])/1000
                elif 'BOOTSTRAPPING-INT1-PKT-ENCODING:' in line or 'DATA-PKT-DECODING' in line or 'BOOTSTRAPPING-INT2-PKT-ENCODING' in line:
                    v2 += int(line.strip().split(':')[-1][:-2])/1000
                elif 'BOOTSTRAPPING-INT1-PKT-ECDSA-SIGN' in line or 'DATA-PKT-HMAC-VERIFY' in line or 'BOOTSTRAPPING-INT2-ECDSA-SIGN' in line:
                    v3 += int(line.strip().split(':')[-1][:-2])/1000
                elif 'BOOTSTRAPPING-DATA2-PKT-AES-DEC' in line:
                    v4 += int(line.strip().split(':')[-1][:-2])/1000
                else:
                    pass
        result_l1.append(v1) # other crypto
        result_l2.append(v2) # encoding
        result_l3.append(v3) # sign/verify
        result_l4.append(v4) # enc/dec

    overall['other crypto\noperations'].append(np.mean(result_l1))
    overall['other crypto\noperations'].append(np.std(result_l1))
    overall['encoding&\ndecoding'].append(np.mean(result_l2))
    overall['encoding&\ndecoding'].append(np.std(result_l2))
    overall['pkt sign&\nsig verify'].append(np.mean(result_l3))
    overall['pkt sign&\nsig verify'].append(np.std(result_l3))
    overall['encryption&\ndecryption'].append(np.mean(result_l4))
    overall['encryption&\ndecryption'].append(np.std(result_l4))

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
    height = 0.25

    p1 = ax.barh(y_pos - height, df.loc[catagories[0]].tolist(), xerr=df.loc[catagories[1]].tolist(), height=height, hatch=patterns[0], edgecolor='gray')
    p2 = ax.barh(y_pos, df.loc[catagories[2]].tolist(), xerr=df.loc[catagories[3]].tolist(), height=height, hatch=patterns[1], edgecolor='gray')
    p3 = ax.barh(y_pos + height, df.loc[catagories[4]].tolist(), xerr=df.loc[catagories[5]].tolist(), height=height, hatch=patterns[2], edgecolor='gray')

    def autolabel_v(rects):
        """Attach a text label above each bar in *rects*, displaying its height."""
        for rect in rects:
            height = rect.get_height()
            ax.annotate('{}'.format(height),
                        xy=(rect.get_x() + rect.get_width() / 2, height),
                        xytext=(0, 4),  # 3 points vertical offset
                        textcoords="offset points",
                        ha='center', va='bottom')

    def autolabel_h(rects):
        """Attach a text label above each bar in *rects*, displaying its height."""
        for rect in rects:
            width = rect.get_width()
            ax.annotate('{:0.2f}'.format(width),
                            xy=(width, rect.get_y() + rect.get_height()/2),
                            xytext=(18, -5),
                            textcoords="offset points",
                            ha='center', va='bottom')

    autolabel_h(p1)
    autolabel_h(p2)
    autolabel_h(p3)

    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)
    ax.set_yticks(y_pos)
    ax.set_yticklabels(overall.keys())
    ax.invert_yaxis()  # labels read top-to-bottom
    ax.set_xscale('log')
    ax.set_xticks([0.1, 1, 20, 100, 300])
    ax.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
    ax.set_xlabel('Time Elapse (ms)')
    # y_labels = ax.get_yticklabels()
    # plt.setp(y_labels, rotation=30, horizontalalignment='right')
    # plt.rcParams.update({'font.size': myFontSize})
    ax.legend((p1[0], p2[0], p3[0]), ('core i7 2.2GHz', 'cortex A53 1.4GHz', 'cortex M4 64MHz'),
              ncol=3,
              bbox_to_anchor=(0, 1),
              loc='lower left', fontsize='small')
    fig.tight_layout()
    fig.savefig('micro-bench-bootstrapping.pdf', format='pdf', dpi=1000)
    plt.show()
