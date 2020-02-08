import matplotlib.pyplot as plt
import matplotlib
import numpy as np

font = {'family' : 'DejaVu Sans',
        'size'   : 10}
matplotlib.rc('font', **font)

labels = ['Bootstrapping', 'Sub:\nContent Delievery', 'Pub:\nCommand Delivery']
laptop_means = []
laptop_stds = []
pi_means = []
pi_stds = []
aws_means = []
aws_stds = []
nrf_means = []
nrf_stds = []

def parse_results():
    tmp = []
    with open('bootstrapping/latency/laptop.dat') as file:
        for line in file:
            tmp.append(int(line.strip()[:-2]))
    laptop_means.append(np.mean(tmp))
    laptop_stds.append(np.std(tmp))

    tmp = []
    with open('bootstrapping/latency/pi.dat') as file:
        for line in file:
            tmp.append(int(line.strip()[:-2]))
    pi_means.append(np.mean(tmp))
    pi_stds.append(np.std(tmp))

    tmp = []
    with open('pub-sub/latency/laptop-fetch-content.dat') as file:
        for line in file:
            tmp.append(float(line.strip()[:-1])*1000)
    laptop_means.append(np.mean(tmp))
    laptop_stds.append(np.std(tmp))

    tmp = []
    with open('pub-sub/latency/pi-fetch-content.dat') as file:
        for line in file:
            tmp.append(float(line.strip()[:-1])*1000)
    pi_means.append(np.mean(tmp))
    pi_stds.append(np.std(tmp))

    tmp = []
    with open('pub-sub/latency/laptop-issue-cmd.dat') as file:
        for line in file:
            tmp.append(int(line.strip()[:-2]))
    laptop_means.append(np.mean(tmp))
    laptop_stds.append(np.std(tmp))

    tmp = []
    with open('pub-sub/latency/pi-issue-cmd.dat') as file:
        for line in file:
            tmp.append(int(line.strip()[:-2]))
    pi_means.append(np.mean(tmp))
    pi_stds.append(np.std(tmp))

    tmp = []
    with open('bootstrapping/latency/aws-iot-tls-setup.dat') as file:
        for line in file:
            tmp.append(int(line.strip()[:-2]))
    aws_means.append(np.mean(tmp))
    aws_stds.append(np.std(tmp))

    tmp = []
    with open('pub-sub/latency/aws-iot-content-delivery.dat') as file:
        for line in file:
            tmp.append(int(line.strip()))
    aws_means.append(np.mean(tmp))
    aws_stds.append(np.std(tmp))

    tmp = []
    with open('pub-sub/latency/aws-iot-cmd-delivery.dat') as file:
        for line in file:
            tmp.append(int(line.strip()))
    aws_means.append(np.mean(tmp))
    aws_stds.append(np.std(tmp))

    tmp = []
    with open('bootstrapping/latency/nrf.dat') as file:
        for line in file:
            tmp.append(int(line.strip()[:-2]))
    nrf_means.append(np.mean(tmp))
    nrf_stds.append(np.std(tmp))

    tmp = []
    with open('pub-sub/latency/nrf-fetch-content.dat') as file:
        for line in file:
            tmp.append(int(line.strip()[:-2]))
    nrf_means.append(np.mean(tmp))
    nrf_stds.append(np.std(tmp))

    tmp = []
    with open('pub-sub/latency/nrf-issue-cmd.dat') as file:
        for line in file:
            tmp.append(int(line.strip()[:-2]))
    nrf_means.append(np.mean(tmp))
    nrf_stds.append(np.std(tmp))

    print(pi_means)
    print(pi_stds)
    print(laptop_means)
    print(laptop_stds)
    print(aws_means)
    print(aws_stds)
    print(nrf_means)
    print(nrf_stds)

parse_results()

x = np.arange(len(labels))  # the label locations
width = 0.2  # the width of the bars

fig, ax = plt.subplots(figsize=(6, 4.5))
patterns = [ "//" , "..", "xx", "oo", "\\\\"]


rects3 = ax.bar(x - width, aws_means, width, yerr=aws_stds, hatch=patterns[0], label='AWS IoT Device, WiFi TLS\nDevice:cortex A53 1.4GHz', edgecolor='black')
# rects1 = ax.bar(x - width/2, laptop_means, width, yerr=laptop_stds, hatch=patterns[0], label='Sovereign App, Unix Socket\nApp host:core i7 2.2GHz', edgecolor='black')
rects2 = ax.bar(x, pi_means, width, yerr=pi_stds, hatch=patterns[1], label='Sovereign Device, WiFi Multicast\nDevice:cortex A53 1.4GHz', edgecolor='black')
rects4 = ax.bar(x + width, nrf_means, width, yerr=nrf_stds, hatch=patterns[2], label='Sovereign Device, IEEE 802.15.4\nDevice:cortex M4 64MHz', edgecolor='black')

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('Total Latency (ms)')
ax.set_xticks(x)
ax.set_xticklabels(labels)
ax.legend()

def autolabel(rects):
    """Attach a text label above each bar in *rects*, displaying its height."""
    for rect in rects:
        height = rect.get_height()
        ax.annotate('{:0.2f}'.format(height),
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(2, 3),  # 3 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom')


# autolabel(rects1)
autolabel(rects2)
autolabel(rects3)
autolabel(rects4)

fig.tight_layout()
fig.savefig('operation-latency.pdf', format='pdf', dpi=1000)
plt.show()