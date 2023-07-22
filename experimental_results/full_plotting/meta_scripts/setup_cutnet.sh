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

cvs_res="final_res_${plottheme}.txt"

# cat ${header_gen} > ${cvs_res}
for i in ${collect}; do cat ${i}/resultsGP.txt; done >> ${cvs_res}

sgdb=`cat meta_scripts/sgdb.txt`
base=`cat meta_scripts/base.txt`

# Create tables
${sgdb} ${base} < sqlite/create_tables.sql ; 
${sgdb} ${base} -separator ' ' ".import ${cvs_res} results "

# Generate Profile Plots
counter=0
# Create folders
mkdir -p plots/${plottheme}/res
# create gnuplot scripts
cat meta_scripts/pp_plotscript_res.gnuplot \
	| sed "s/generic_metric_yaxis/cut/g" \
	| sed "s/generic_xrange/${xrange_res_pp}/g" \
	| sed "s/generic_xscale/${string_log2_res_pp}/g" \
	> plots/${plottheme}/res/pp_plotscript.gnuplot
cat meta_scripts/cut_impr_plotscript.gnuplot \
	> plots/${plottheme}/res/impr_plotscript.gnuplot
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
		| sed "s/generic_table/results/g" \
		| sed "s/generic_objective/obj_func/g" \
		| sed "s/generic_algorithm/${alg_code[0]}/g" \
		| sed "s/generic_label/${alg_code[1]}/g" \
		| ${sgdb} ${base} -csv -separator '&' \
		> plots/${plottheme}/res/pp_${alg_code[0]}.csv
	echo "'pp_${alg_code[0]}.csv' using '${alg_code[1]}':'ind' title '${alg_code[1]}' \
		with lines lw 9 linecolor rgb '${colors[$counter]}', \\" \
		>> plots/${plottheme}/res/pp_plotscript.gnuplot
	cat sqlite/improvement.sql \
		| sed "s/generic_table/results/g" \
		| sed "s/generic_objective/obj_func/g" \
		| sed "s/generic_algorithm/${alg_code[0]}/g" \
		| sed "s/reference_algorithm/${refalg}/g" \
		| sed "s/generic_label/${alg_code[1]}/g" \
		| ${sgdb} ${base} -csv -separator '&' \
		> plots/${plottheme}/res/impr_${alg_code[0]}.csv
	let counter=counter+1
done

# print statistics
cat sqlite/mean_stddev.sql \
	| sed "s/generic_table/results/g" \
	| sed "s/generic_objective/obj_func/g" \
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
	| sed "s/obj_func/cut-net /g" \
	> plots/${plottheme}/res/pp_averages.csv
cat plots/${plottheme}/res/pp_averages.csv;

# run gnuplot scripts
cd plots/${plottheme}/res/
gnuplot pp_plotscript.gnuplot;
gnuplot impr_plotscript.gnuplot;
cd -;

# Drop tables
${sgdb} ${base} < sqlite/drop_tables.sql  

for i in $cvs_res; do rm $i; done

