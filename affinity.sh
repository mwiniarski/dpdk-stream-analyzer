for i in $(ps -e -T -o pid | awk '{print $1}')
do
	taskset -a -p -c 4-7 $i
done
