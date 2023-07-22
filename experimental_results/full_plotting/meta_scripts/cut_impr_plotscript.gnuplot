set terminal postscript enhanced color
set output '| ps2pdf - impr_chart.pdf'
set style histogram columnstacked    
set boxwidth 100                     
set style histogram clustered gap 100
set style fill solid                 

# set xrange [generic_xrange]
set xrange [400:2700]
# set yrange [0:1]
set datafile separator "&"

# set ylabel "% improvement in edge-cut"  font "Times,35" offset -4,0
set ylabel "% improvement in cut"  font "Times,35" offset -4,0
set xlabel "k" font "Times,35" offset 0,-2.5

# set format x '2^{%L}'  #<- enhanced text.

set xtics font "Times,35" offset 0,-1
set ytics font "Times,35" offset -.5,0
# set xtics ("2^{10}" 1024, "2^{11}" 2048, "2^{12}" 4096, "2^{13}" 8192)
set xtics ("512" 512, "1024" 1024, "1536" 1536, "2048" 2048, "2560" 2560)
set grid y

set key right top 
set key font "Times,14"
set key samplen 4 spacing 3.2 font "Times,30" #at 1.5,0.05
set key maxrows 7 width 2



#unset logscale
#unset key
plot \
'impr_intnetlpatohx2.csv' using 'ind':'PaToH-cut' title 'PaToH-cut' 		with boxes linecolor rgb 'gray', \
'impr_intnetl_SSArrayfennelapprosqrt_cut.csv' using 'ind':'FREIGHT-cut' title 'FREIGHT-cut' 		with boxes linecolor rgb 'red', \
'impr_intnetlhype.csv' using 'ind':'HYPE' title 'HYPE' 		with boxes linecolor rgb 'purple', \
'impr_intnetlmmrefinethirdorgy0b0.15.csv' using 'ind':'RRS(0.15)' title 'RRS(0.15)' 		with boxes linecolor rgb 'green', \
'impr_intnetlmmgood.csv' using 'ind':'MM-N2P' title 'MM-N2P' 		with boxes linecolor rgb 'blue', \
'impr_intnetlnkx0y5.csv' using 'ind':'MM-L5' title 'MM-L5' 		with boxes linecolor rgb 'orange', \
# 'impr_intnetlhashing.csv' using 'ind':'Hashing' title 'Hashing' 		with boxes linecolor rgb 'black', \
