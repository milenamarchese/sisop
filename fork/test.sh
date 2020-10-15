#/bin/bash

make all

printf "Pingpong:\n\n" > "result.txt"
./pingpong >> "result.txt"
printf "Pingpong:\n\n" >> "result.txt"
./pingpong >> "result.txt"

printf "Primes:\n n = 35:\n" >> "result.txt"
./primes 35 >> "result.txt"
printf "n = 50\n" >> "result.txt"
./primes 50 >> "result.txt"

printf "\nFind:\n./find make\n" >> "result.txt"
./find make >> "result.txt"
printf "./find -i make\n" >> "result.txt"
./find -i make >> "result.txt"

printf "\nXargs:\n\n" >> "result.txt"
(printf "/home\n/var\n/tmp\n/" | ./xargs ls) >> "result.txt"

make clean
