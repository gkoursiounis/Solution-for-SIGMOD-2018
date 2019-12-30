#!/bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
	echo "Usage: $0 <path/to/executable> <data_size (small/medium)>";
	exit 1;
fi

exe=$1
size=$2

if grep -q "Done$" "$size.init"; then
	init_file="$size.init";
else
	init_file="${size}_fixed.init";
	cp "$size.init" "$init_file";
	echo -e "Done\n" >> "$init_file";
fi

start=$(date +%s.%N);
cat "$init_file" "$size.work" | "$exe" > /dev/null 2> colored_result;
end=$(date +%s.%N);

dur=$(echo $end - $start | bc);
echo "Execution time: $dur seconds";

#sed -r "s/\x1B\[(1;33|0)m//g" colored_result > result;
sed -r "s/\x1B\[([0-9]{1,2}(;[0-9]{1,2})?)?[m|K]//g" colored_result > result;

diff -Z -s result "$size.result";


rm colored_result;

exit 0;