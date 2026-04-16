To extract signal names post spice simulation:

```
head -n 1  lut_test.sp.csv  | tr , '\n' | sort | uniq > columns.txt
```

To visualize data according to the parameters in `plot.py` (change for the run
you're visualizing):

```
cd run_0
../plot.py ./ && feh -. *.png
```
