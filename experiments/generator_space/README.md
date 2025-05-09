# Generate images of generator outputs

## Prerequisites

`bfg`, `proto2gds` (from Layout21), `python3` and `gdspy` are all needed. Check `sweep_generator.py` for assumptions about where they are and how that works.

## Running

```
python3 sweep_generator.py
```

## Cleanup

```
rm *.gds *.pb.txt *.svg
```
