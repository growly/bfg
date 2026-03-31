set datafile separator ",";
set xlabel 'x';
set ylabel 'y';
set yrange [-0.1:1.9];
set grid;
set term png;
set output 'plot.png';
set key autotitle columnhead;
#plot for [col=2:29] 'tran.sp.csv' using 1:col with lines
plot 'tran.sp.csv' using 1:3 with lines, \
     'tran.sp.csv' using 1:2 with lines, \
     'tran.sp.csv' using 1:5 with lines
