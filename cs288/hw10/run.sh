#!/bin/bash

TYPES=(p2p collective)

function run {
  echo "mpirun -np $1 $2 $3"
  mpirun -np $1 $2 $3
}

make clean; make debug
echo "Running builds with debug printing enabled (n=100):"
for type in ${TYPES[@]}; do
  file=dot_${type}
  echo "Executing debug tests using ${type} method..."
  echo
  echo "Small number of processes (divides n):"
  run 4 ${file} 100
  echo
  echo "Small number of processes (does not divide n):"
  run 3 ${file} 100
  echo
  echo "Large number of processes (divides n):"
  run 25 ${file} 100
  echo
  echo "Large number of processes (does not divide n):"
  run 29 ${file} 100
done
make clean; make
echo "Running builds with debug printing disaled (n=1000):"
for type in ${TYPES[@]}; do
  file=dot_${type}
  echo "Executing tests using ${type} method..."
  echo
  echo "Small number of processes (divides n):"
  run 4 ${file} 1000
  echo
  echo "Small number of processes (does not divide n):"
  run 3 ${file} 1000
  echo
  echo "Large number of processes (divides n):"
  run 100 ${file} 1000
  echo
  echo "Large number of processes (does not divide n):"
  run 97 ${file} 1000
done
