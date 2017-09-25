#!/bin/bash

scenario=$1
test_num=$2

hosts_path=scenarios/$scenario/$test_num/logs/hosts
logs_path=scenarios/$scenario/$test_num/logs/log

# calculate number of stations in scenario and the sum of all the names each station can know 
n_stations=$(ls $logs_path | wc -l)

knowable_names_total=0
for file in $hosts_path/hosts-sta*; do
	knowable_names=$(cat $file | tail -n +2 | wc -l)
	knowable_names_total=$((knowable_names_total + $knowable_names))
done

# calculate the time it took for every station to know all the names they could know
start_time=$(cat $logs_path/10.0.0.1.log | tail -n +2 | head -n1 | cut -d' ' -f2 | cut -d']' -f1)
start_timestamp=$(date -d "$start_time" +"%s")
end_timestamp=0
for file in $logs_path/*; do
	time_last_nrep=$(cat $file | grep "\[<-NREP\]" | tail -n1 | cut -d' ' -f2 | cut -d']' -f1)
	time_last_nrep_timestamp=$(date -d "$time_last_nrep" +"%s")
	if [ $time_last_nrep_timestamp -gt $end_timestamp ]; then
		end_timestamp=$time_last_nrep_timestamp
		end_time=$time_last_nrep
	fi
done

elapsed_time=$((end_timestamp - start_timestamp))

# print stuff
echo
echo "n_stations: $n_stations"
echo "knowable_names_total: $knowable_names_total"
echo "start_time: $start_time"
echo "end_time: $end_time"
echo "start_timestamp: $start_timestamp"
echo "end_timestamp: $end_timestamp"
echo "elapsed_time: $elapsed_time"
echo

# count how many names are known at each second on the scenario
i=$start_timestamp
j=1
while [ $i -le $end_timestamp ]; do
	hour_min_sec=$(date -d @$i | awk '{print $4}')
	#nreps_in_second=0
	for file in $logs_path/*; do
		nreps_in_second_for_sta=$(cat $file | grep $hour_min_sec | grep "\[<-NREP\]" | wc -l)
		nreps_in_second=$((nreps_in_second + nreps_in_second_for_sta))
	done
	
	echo "$nreps_in_second"
	((i++))
	((j++))
done
