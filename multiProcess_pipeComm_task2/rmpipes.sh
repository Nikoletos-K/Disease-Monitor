command=$(ls | grep -E NAME)
command2=$(ls | grep -E log_file)

for a in ${command[@]}; do

	rm -i $a
done  

for a in ${command2[@]}; do

	rm -i $a
done  


