import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
from matplotlib.pyplot import figure

catagories = ['core i7 2.2GHz', 'core i7 2.2GHz std', 'cortex 1.4GHz', 'cortex 1.4GHz std', 'cortex 64MHz', 'cortex 64MHz std']
overall = dict()
overall['encrypt&\ndecrypt'] = []
overall['pkt sign&\nsig verify'] = []
overall['encoding&\ndecoding'] = []
overall['other crypto\noperations'] = []
overall['  '] = []
overall['pub:\nencrypt'] = []
overall['pub:\npkt sign'] = []
overall['pub:\nencoding'] = []
overall['sub:\ndecoding'] = []
overall['sub:\nsig verify'] = []
overall['sub:\nschema'] = []
overall['sub:\ndecrypt'] = []

font = {'family' : 'DejaVu Sans',
        'size'   : 10}
matplotlib.rc('font', **font)

def parse_results_2(dir: str):
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
            overall['pub:\nencrypt'].append(np.mean(v1))
            overall['pub:\nencrypt'].append(np.std(v1))
            overall['pub:\npkt sign'].append(np.mean(v2))
            overall['pub:\npkt sign'].append(np.std(v2))
            overall['pub:\nencoding'].append(np.mean(v3))
            overall['pub:\nencoding'].append(np.std(v3))
        else:
            overall['sub:\ndecoding'].append(np.mean(v3))
            overall['sub:\ndecoding'].append(np.std(v3))
            overall['sub:\nsig verify'].append(np.mean(v2))
            overall['sub:\nsig verify'].append(np.std(v2))
            overall['sub:\nschema'].append(np.mean(v4))
            overall['sub:\nschema'].append(np.std(v4))
            overall['sub:\ndecrypt'].append(np.mean(v1))
            overall['sub:\ndecrypt'].append(np.std(v1))

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
    overall['encrypt&\ndecrypt'].append(np.mean(result_l4))
    overall['encrypt&\ndecrypt'].append(np.std(result_l4))

if __name__ == "__main__":
    parse_results('bootstrapping/corei7-4770HQ-2.2GHz')
    parse_results('bootstrapping/cortex-A53-ARMv8-1.4GHz')
    parse_results('bootstrapping/cortex-M4-64MHz')
    overall['  '] = [0, 0, 0, 0, 0, 0]
    parse_results_2('pub-sub/corei7-4770HQ-2.2GHz')
    parse_results_2('pub-sub/cortex-A53-ARMv8-1.4GHz')
    parse_results_2('pub-sub/cortex-M4-64MHz')

    df = pd.DataFrame(overall, index=catagories)
    print(df)

    fig = plt.figure(figsize=(7.5, 4.5))
    ax = fig.add_subplot(111)
    x_pos = np.arange(len(overall.keys()))
    patterns = [ "///" , "..", "xx", "ooo", "\\\\"]
    width = 0.25

    p1 = ax.bar(x_pos - width, df.loc[catagories[0]].tolist(), yerr=df.loc[catagories[1]].tolist(), width=width, hatch=patterns[0], edgecolor='black')
    p2 = ax.bar(x_pos, df.loc[catagories[2]].tolist(), yerr=df.loc[catagories[3]].tolist(), width=width, hatch=patterns[1], edgecolor='black')
    p3 = ax.bar(x_pos + width, df.loc[catagories[4]].tolist(), yerr=df.loc[catagories[5]].tolist(), width=width, hatch=patterns[2], edgecolor='black')

    def autolabel_v(rects):
        """Attach a text label above each bar in *rects*, displaying its height."""
        for rect in rects:
            height = rect.get_height()
            if height > 0:
                ax.annotate('{:0.2f}'.format(height),
                            xy=(rect.get_x() + rect.get_width() / 2, height),
                            xytext=(-5, 2),  # 3 points vertical offset
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

    autolabel_v(p1)
    autolabel_v(p2)
    autolabel_v(p3)

    ax.axvline(x=4, color='black')
    ax.text(3.6, 10, "Bootstrapping", rotation=-90)
    ax.text(4.1, 8, "Pub/Sub", rotation=90)

    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)
    ax.set_xticks(x_pos)
    ax.set_xticklabels(overall.keys())
    # ax.invert_yaxis()  # labels read top-to-bottom
    ax.set_yscale('log')
    ax.set_yticks([0.1, 1, 20, 100, 300])
    ax.get_yaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
    # ax.set_ylabel('Time Elapse (ms)')
    ax.text(-2, 8, "Time Elapse (ms)", rotation=90)
    x_labels = ax.get_xticklabels()
    plt.setp(x_labels, rotation=45, horizontalalignment='center')
    plt.rc('ytick', labelsize=11)
    ax.legend((p1[0], p2[0], p3[0]), ('App: core i7 2.2GHz', 'Device: cortex A53 1.4GHz', 'Device: cortex M4 64MHz'),
            ncol=3,
            bbox_to_anchor=(0, 1),
            loc='lower left',
            fontsize=9)
    fig.tight_layout()
    fig.savefig('micro-bench-all.pdf', format='pdf', dpi=1000)
    plt.show()
