colors="black"
colors="${colors}:red"
colors="${colors}:brown"
colors="${colors}:orange"
colors="${colors}:blue"
colors="${colors}:green"
colors="${colors}:purple"
colors="${colors}:gray"
colors="${colors}:light-blue"
colors="${colors}:pink"
colors="${colors}:magenta"
colors="${colors}:yellow"
colors="${colors}:red-magenta"

colors=(${colors//:/ })

string_log2_res_pp=''
if [ "$log2_res_pp" = 1 ]; then
	string_log2_res_pp='set logscale x 2'
fi
string_log2_con_pp=''
if [ "$log2_con_pp" = 1 ]; then
	string_log2_con_pp='set logscale x 2'
fi
string_log2_tim_pp=''
if [ "$log2_tim_pp" = 1 ]; then
	string_log2_tim_pp='set logscale x 2'
fi
string_log2_tim_speedup=''
if [ "$log2_tim_speedup" = 1 ]; then
	string_log2_tim_speedup='set logscale y 2'
fi

cvs_con="final_con_${plottheme}.txt"
cvs_tim="final_tim_${plottheme}.txt"

# cat ${header_con} > ${cvs_con}
for i in ${collect}; do cat ${i}/results_connectivity.txt; done >> ${cvs_con}
# cat ${header_tim} > ${cvs_tim}
for i in ${collect}; do cat ${i}/times.txt; done >> ${cvs_tim}

sgdb=`cat meta_scripts/sgdb.txt`
base=`cat meta_scripts/base.txt`

# Create tables
${sgdb} ${base} < sqlite/create_tables.sql ; 
${sgdb} ${base} -separator ' ' ".import ${cvs_con} connectivities "
${sgdb} ${base} -separator ' ' ".import ${cvs_tim} times "

# Generate Profile Plots
counter=0
# Create folders
mkdir -p plots/${plottheme}/tim
# create gnuplot scripts
cat meta_scripts/pp_plotscript_tim.gnuplot \
	| sed "s/generic_xrange/${xrange_tim_pp}/g" \
	| sed "s/generic_xscale/${string_log2_tim_pp}/g" \
	> plots/${plottheme}/tim/pp_plotscript.gnuplot
cat meta_scripts/boxplot_plotscript.gnuplot \
	| sed "s/generic_yscale/set logscale y 10/g" \
	| sed 's/generic_yformat/set format y "10^{%L}"/g' \
	| sed 's/generic_xformat/set format x " "/g' \
	| sed "s/poscap_generic/${conceptial_poscap_boxplot}/g" \
	| sed "s/LabelY/time per pin (ns)/g" \
	| sed "s/LabelX/algorithm/g" \
	> plots/${plottheme}/tim/boxplot_plotscript.gnuplot
# create create csv files and increment gnuplot scripts
for item in $(echo ${caption} | tr "," " "); 
do 
	# jump undesired color (nly one jump allowed)
	if [ "$counter" = $color_jump ]; then
		let counter=counter+1
	fi
	if [ "$counter" = $color_jumpb ]; then
		let counter=counter+1
	fi
	alg_code=(${item//:/ })
	cat sqlite/perf_prof.sql \
		| sed "s/generic_table/times/g" \
		| sed "s/generic_objective/time_gp/g" \
		| sed "s/generic_algorithm/${alg_code[0]}/g" \
		| sed "s/generic_label/${alg_code[1]}/g" \
		| ${sgdb} ${base} -csv -separator '&' \
		> plots/${plottheme}/tim/pp_${alg_code[0]}.csv
	echo "'pp_${alg_code[0]}.csv' using '${alg_code[1]}':'ind' title '${alg_code[1]}' \
		with lines lw 9 linecolor rgb '${colors[$counter]}', \\" \
		>> plots/${plottheme}/tim/pp_plotscript.gnuplot
	cat sqlite/ratio_of_two_objectives_all.sql \
		| sed "s/generic_table1/times/g" \
		| sed "s/generic_table2/connectivities/g" \
		| sed "s/generic_objective1/time_gp/g" \
		| sed "s/generic_objective2/pins/g" \
		| sed "s/generic_algorithm/${alg_code[0]}/g" \
		| sed "s/generic_label/${alg_code[1]}/g" \
		| ${sgdb} ${base} -csv -separator '&' \
		> plots/${plottheme}/tim/boxplot_${alg_code[0]}.csv
	echo "'boxplot_${alg_code[0]}.csv' using ($((counter+1))):1 title '${alg_code[1]}' \
		linecolor rgb '${colors[$counter]}', \\" \
		>> plots/${plottheme}/tim/boxplot_plotscript.gnuplot
	let counter=counter+1
done

# print statistics
cat sqlite/mean_stddev.sql \
	| sed "s/generic_table/times/g" \
	| sed "s/generic_objective/time_gp/g" \
	| ${sgdb} ${base} -table \
	| sed "s/intnetl_SSArrayfennelapprosqrt_con/FREIGHT-con  /g" \
	| sed "s/intnetl_SSArrayfennelapprosqrt_cut/FREIGHT-cut  /g" \
	| sed "s/intnetlhashing                    /Hashing      /g" \
	| sed "s/intnetlhype                       /HYPE         /g" \
	| sed "s/intnetlmmgood                     /MM-N2P       /g" \
	| sed "s/intnetlmmrefinethirdorgy0b0.15    /RRS(0.15)    /g" \
	| sed "s/intnetlnkx0y5                     /MM-L5        /g" \
	| sed "s/intnetlpatohx1                    /PaToH-con    /g" \
	| sed "s/intnetlpatohx2                    /PaToH-cut    /g" \
	| sed "s/            algorithm             /  algorithm  /g" \
	| sed "s/---------------------------------+/------------+/g" \
	| sed "s/time_gp/runtime/g" \
	> plots/${plottheme}/tim/pp_averages.csv
cat plots/${plottheme}/tim/pp_averages.csv;

# run gnuplot scripts
cd plots/${plottheme}/tim/
gnuplot pp_plotscript.gnuplot;
gnuplot boxplot_plotscript.gnuplot;
cd -;

# Drop tables
${sgdb} ${base} < sqlite/drop_tables.sql  

for i in $cvs_con $cvs_tim; do rm $i; done

