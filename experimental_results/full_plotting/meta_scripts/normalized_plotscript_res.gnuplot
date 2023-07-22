set terminal postscript enhanced color
set output '| ps2pdf - chart.pdf'

# set xrange [generic_xrange]
# set yrange [0:1]
set datafile separator "&"

# set ylabel "Normalized {/Symbol f}_{/Symbol m}"  font "Times,35" offset -4,0
set ylabel "generic_yaxis"  font "Times,35" offset -4,0
set xlabel "instance" font "Times,35" offset 0,-2.5



set xtics font "Times,35" offset 0,-1
set ytics font "Times,35" offset -.5,0
# set grid

set key right bottom 
set key font "Times,14"
set key samplen 4 spacing 3.2 font "Times,30" #at 1.5,0.05
set key maxrows 10 width 2

generic_xscale

#unset logscale
#unset key
plot \
