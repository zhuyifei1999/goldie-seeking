import ast
import sys
import struct

with open(sys.argv[1], 'r') as f:
    f.readline()
    r_penalties = ast.literal_eval(f.readline().lstrip('$ '))
    r_connects = [line.split() for line in f]

all_gushers = list(sorted(set(sum(r_connects, []))))
num_gushers = len(all_gushers)
gushers_rev = {g: i for i, g in enumerate(all_gushers)}

penalties = [r_penalties['.'] for k in range(num_gushers)]
for rk, v in r_penalties.items():
    if rk == '.':
        continue

    for k in rk:
        penalties[gushers_rev[k]] = v

connects = [[False for i in range(num_gushers)] for j in range(num_gushers)]
for connect in r_connects:
    src, *dsts = connect
    src = gushers_rev[src]
    for dst in dsts:
        connects[src][gushers_rev[dst]] = True
        connects[gushers_rev[dst]][src] = True

with open(sys.argv[2], 'wb') as f:
    f.write(struct.pack('@I', num_gushers))
    for g in all_gushers:
        f.write(struct.pack('@b', ord(g)))
    for p in penalties:
        f.write(struct.pack('@I', p))
    for cl in connects:
        for c in cl:
            f.write(struct.pack('@?', c))
