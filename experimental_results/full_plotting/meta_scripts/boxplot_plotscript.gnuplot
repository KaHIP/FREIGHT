set terminal postscript enhanced color               
set output '| ps2pdf - boxplot_chart.pdf'                  

set datafile separator "&"                           

set ylabel "LabelY"  font "Times,35" offset -5.5,0 
set xlabel "LabelX" font "Times,35" offset 0,0
set xtics font "Times,35" offset 0,-1
set ytics font "Times,35" offset -.5,0

set grid y
generic_yscale
generic_yformat
generic_xformat

#unset key                                    
set key right top 
set key font "Times,14"
set key samplen 4 spacing 3.2 font "Times,30" poscap_generic #at 1.5,0.05
set key maxrows 10 width 3.5

set style fill solid 0.25 border -1           
set style boxplot nooutliers pointtype 7      
set style data boxplot                        
                                              
#set title 'My Plot' font 'Arial,14';         
#set xtics ('' 1, '' 2, '' 3, '' 4, '' 5, '' 6, '' 7, '' 8, '' 9, '' 10, '' 11, '' 12)

plot \
