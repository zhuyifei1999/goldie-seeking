for map in gusher\ graphs/*.txt; do python stratfmt.py "${map}" map.bin && ./strats map.bin | sort -k3 -n > out/"$(basename "${map}")"; done
