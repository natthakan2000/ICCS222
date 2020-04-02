#!/bin/bash

touch page_fault.txt

algor=('rand' 'fifo' 'lru');

typee=('sort.trace' 'scan.trace' 'focus.trace');

size=('256' '1024');

for ((m = 0; m < 3; m++ ));do
	for (( i = 0; i < 3; i++ )); do
		for (( t = 0; t < 3; t++ )); do
			echo "${algor[i]} ${typee[t]}">> page_fault.txt
			for (( j = 3; j <= 20; j++ )); do
				./virtmem ${typee[t]} ${size[m]} ${j} ${algor[i]} >> page_fault.txt
	      echo "Frame ${j}"
			done
	    echo "Trace Done"
		done
	  echo "Algo Done"
	done
	echo "Done for ${size[m]} =================" >> page_fault.txt
done
