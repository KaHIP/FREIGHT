set terminal postscript enhanced color
set output '| ps2pdf - speedup_chart.pdf'

# set xrange [generic_xrange]
# set yrange [0:1]
set datafile separator "&"

set ylabel "% improvement in edge-cut"  font "Times,35" offset -4,0
set xlabel "k" font "Times,35" offset 0,-2.5

set format x '2^{%L}'  #<- enhanced text.

set xtics font "Times,35" offset 0,-1
set ytics font "Times,35" offset -.5,0
set xtics ("2^{10}" 1024, "2^{11}" 2048, "2^{12}" 4096, "2^{13}" 8192)
set grid y

set key generic_conceptual_pos_cap 
set key font "Times,14"
set key samplen 4 spacing 3.2 font "Times,30" #at 1.5,0.05
set key maxrows 5 width 2

generic_yscale

#unset logscale
#unset key
plot \
