import matplotlib.pyplot as plt
import csv
from labellines import labelLine, labelLines
import sys

file_name = sys.argv[1]

vals = {}

with open(file_name, newline='') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', skipinitialspace=True)
    for row in spamreader:
        name = row[0] + ('_with_chroma' if row[1] == 'true' else "")
        vals.setdefault(name, []).append((float(row[2]), float(row[3])))

for i in vals:
    vals[i].sort(key=lambda tup: tup[1])
    print(i)
    plt.plot([x[1] for x in vals[i]], [x[0] for x in vals[i]], label=i, zorder=1)
    plt.scatter([x[1] for x in vals[i]], [x[0] for x in vals[i]], zorder=2, s=5, color="#000000")

labelLines(plt.gca().get_lines(), fontsize=5)
plt.gca().set_xlabel('Rate (bits/pixel)')
plt.gca().set_ylabel('PSNR (dB)')
plt.legend()
plt.grid()
plt.show()
