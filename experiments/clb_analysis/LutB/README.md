# CLB experiments

This folder contains the comparison scripts and data for measuring the
performance of the BFG Sky130 LutB cell, which is a CLB based on a 4-LUT.

## Structure

Common python scripts and Spice testbenches are in `src/`. To avoid
dealing with python packaging and Spice variable scoping these are generally
symlinked directly into each run folder.

Numbered subdirectories represent individual experiments.


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
```

### To enable Innovus to synthesise a scan chain

and therefore not complain about there being an undeclared scan chain, add this
hook to insert

```
create_scan_chain -name test -start scan_in[0] -stop scan_out[0]
```

```python
def scan_chain(x: HammerTool) -> bool:
    x.append('''
# what an absolute pain in the dick
create_scan_chain -name test -start scan_in[0] -stop scan_out[0]
''')
    return True

class BFGExperimentDriver(CLIDriver):
    def get_extra_par_hooks(self) -> List[HammerToolHookAction]:
        extra_hooks = [
            HammerTool.make_removal_hook("clock_tree"),
            HammerTool.make_removal_hook("place_bumps"),
            HammerTool.make_pre_insertion_hook("place_opt_design", scan_chain)
        ]
        return extra_hooks
```
