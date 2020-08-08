for map in gusher\ graphs/*.txt; do python stratfmt.py "${map}" map.bin && ./strats map.bin | sort -k2,2n -k3,3g -s > out/"$(basename "${map}")"; done
