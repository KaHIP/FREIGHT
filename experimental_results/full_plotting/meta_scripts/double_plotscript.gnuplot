set terminal postscript enhanced color
set output '| ps2pdf - double_chart.pdf'

# set xrange [generic_xrange]
# set yrange [0:1]
set datafile separator "&"

set ylabel "LabelY"  font "Times,35" offset -4,0
set xlabel "LabelX" font "Times,35" offset 0,-2.5

set format x '10^{%L}'  #<- enhanced text.

set xtics font "Times,35" offset 0,-1
set ytics font "Times,35" offset -.5,0
# set grid

set key right bottom 
set key font "Times,14"
set key samplen 4 spacing 3.2 font "Times,30" at poscap_double #at 1.5,0.05
set key maxrows 10 width 2

generic_xscale

set logscale x 10
#unset logscale
#unset key
plot \
