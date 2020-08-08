for map in gusher\ graphs/*.txt; do python stratfmt.py "${map}" map.bin && ./strats map.bin | sort -k3,3n -k4,4g -s > out/"$(basename "${map}")"; done
