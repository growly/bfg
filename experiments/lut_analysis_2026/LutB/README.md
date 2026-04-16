### To extract signal names post spice simulation:

```
head -n 1  lut_test.sp.csv  | tr , '\n' | sort | uniq > columns.txt
```

### To visualize data according to the parameters in `plot.py` (change for the run
you're visualizing):

```
cd run_0
../plot.py ./ && feh -. *.png
```
ssh aryap@eda-1.eecs.berkeley.edu

### To include instructional set up for class accounts and base conda environment
```
source ~/.bashrc
cd /scratch/aryap/chipyard
source env.sh
scripts/init-vlsi.sh
```

### Now it shoud work, and python3 should point to the one in the local conda
# environment

```
cd vlsi
./bfg-vlsi syn -e env.yml -p tools.yml -p bfg_clb/sky130-hd.yml -p bfg_clb/bfg_clb.yml --obj_dir build/bfg_clb
./bfg-vlsi syn-to-par -e env.yml -p build/bfg_clb/syn-rundir/syn-output-full.json -o build/bfg_clb/par-input.json
./bfg-vlsi par -e env.yml -p build/bfg_clb/par-input.json --obj_dir build/bfg_clb
./bfg-vlsi par-to-lvs -e env.yml -p build/bfg_clb/par-rundir/par-output-full.json -o build/bfg_clb/lvs-input.json
./bfg-vlsi lvs -e env.yml -p tools.yml -p build/bfg_clb/lvs-input.json --obj_dir build/bfg_clb

create_scan_chain -name test -start scan_in[0] -stop scan_out[0]
```
