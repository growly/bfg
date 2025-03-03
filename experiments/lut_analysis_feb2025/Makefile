
lut_test.sp.csv lut_test.sp.mt0:	lut_test.sp package.sp
	~/XyceInstall/Serial/bin/Xyce lut_test.sp

sim:	lut_test.sp.csv lut_test.sp.mt0

plot.png:	plot.py
	python3 plot.py

plot:	plot.png
