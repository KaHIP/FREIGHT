# HEADERS
header_gen=headers/headerGP.txt
header_con=headers/header_conn.txt
header_tim=headers/header_time.txt

# RELATIVE ADDRESS TO LOAD FILES results.txt AND times.txt
collect="data_results/freight_con"
collect="${collect} data_results/hashing"
collect="${collect} data_results/REF_RLX_SV_0.15"
collect="${collect} data_results/minmax_n2p"
collect="${collect} data_results/minmax_l5"
collect="${collect} data_results/patoh_con"
collect="${collect} data_results/hype"
# collect="${collect} data_results/<further_group>/<further_set_of_tests>"

# THEME OF PLOTS
plottheme=results

# CODE OF BASELINE ALGORITHM FOR PLOTS
refalg=intnetlhashing

# MAP <CODE_OF_ALGORITHM,KEY>
caption="intnetlhashing:Hashing"
caption="${caption},intnetl_SSArrayfennelapprosqrt_con:FREIGHT-con"
caption="${caption},intnetlnkx0y5:MM-L5"
caption="${caption},intnetlmmgood:MM-N2P"
caption="${caption},intnetlmmrefinethirdorgy0b0.15:RRS(0.15)"
caption="${caption},intnetlhype:HYPE"
caption="${caption},intnetlpatohx1:PaToH-con"
# caption="${caption},<code_of_further_set_of_tests>:<key_for_further_set_of_tests>"


# DEFINE RELATIVE POSITION OF CAPTIONS AND THE X-AXIS RANGE FOR PERFORMANCE PROFILES
poscap_double="70,0.85"	     # downright corner of caption (motif conductance X cluster size)
poscap_res="0.9:0.80"        # upright corner of caption (Map improvement plot)
conceptial_poscap_res="right top"        
conceptial_poscap_tim="right top"        
conceptial_poscap_boxplot="right bottom"        
poscap_res_pp="1.00:0.05"    # downright corner of caption (Map performance profile)
xrange_res_pp="1:8"	     # range on x axis (Map performance profile)
xrange_con_pp="1:16"	     # range on x axis (Map performance profile)
poscap_tim="0.90:0.3"        # upright corner of caption (running time plot)
poscap_tim_pp="0.102:0.66"   # downright corner of caption (running time performance profile)
xrange_tim_pp="1:12000"      # range on x axis (running time performance profile)
graph_check="soc-flixster"

#################### OPTIONAL PLOTTING OPTIONS ####################
simple128=0		     # scale on x-axis
simple2048=0		     # scale on x-axis
simple8192=1		     # scale on x-axis
simple16k=0		     # scale on x-axis
exps_2_16=0		     # scale on x-axis
exps_3_16=0		     # scale on x-axis
exps_4_16=0		     # scale on x-axis
maps64_32=0		     # scale on x-axis
maps64_128=0		     # scale on x-axis
percentageGP=1		     # plot average edge-cut percentage
resultIsGP=0		     # Improvement result is based on ede-cut
speedup=1		     # plot speedup instead of time ratio
no_perf_prof=0		     # skip performance plots
large_key=1		     # large key font size and spacing
log2_res_pp=0		     # x-axis of results performance profile in log2 scale
log2_con_pp=1		     # x-axis of connectivity performance profile in log2 scale
log2_tim_pp=1		     # x-axis of times performance profile in log2 scale
log2_tim_speedup=1	     # y-axis of speedup plot in log2 scale
color_jump=1		     # number of color to be jumped for better formatting
color_jumpb=7		     # number of color to be jumped for better formatting

#################### NO NEED TO CHANGE BELOW HERE ####################
source ./meta_scripts/setup_connectivity.sh
