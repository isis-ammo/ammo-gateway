#!/bin/bash

if [ -z "$1" ]; then
  echo "Output directory missing"
  exit 1
fi

if [ -z "$2" ]; then
  echo "Message size required."
  exit 2
fi

if [ -z "$3" ]; then
  echo "Consumer count required."
  exit 2
fi

if [ -z "$4" ]; then
  echo "Producer runtime required."
  exit 2
fi

if [ -z "$5" ]; then
  echo "Gateway address required."
  exit 2
fi

if [ -d "$1" ]; then 
  echo "Output directory already exists"
  exit 3
fi

mkdir -p $1

echo "Test settings:" | tee -a "$1/setup.txt"
echo "  Output directory: $1" | tee -a "$1/setup.txt"
echo "  Message size: $2" | tee -a "$1/setup.txt"
echo "  Consumer count: $3" | tee -a "$1/setup.txt"
echo "  Producer runtime: $4" | tee -a "$1/setup.txt"
echo "  Gateway IP: $5" | tee -a "$1/setup.txt"

let consumerRuntime=$4+20
let totalRuntime=$4+50

echo "  Consumer runtime: $consumerRuntime" | tee -a "$1/setup.txt"
echo "  Total runtime: $totalRuntime" | tee -a "$1/setup.txt"

for (( i=1; i<=$3; i++ )); do
  echo "Starting consumer $i"
  timeout -s INT $consumerRuntime python Consumer.py -g $5 -p 33289 > "$1/$i.csv" &
  sleep 0.3
done

echo "Starting producer"
timeout -s INT $4 python Producer.py -g $5 -p 33289 -r 0.5 -m "$2" > "$1/producer.log.txt" &

echo "Running for $totalRuntime seconds"
sleep $totalRuntime
echo "Finished"
