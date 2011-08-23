#!/bin/bash

if [ -z "$1" ]; then
  echo "Output directory missing"
  exit 1
fi

if [ -z "$2" ]; then
  echo "Message size required."
  exit 2
fi

if [ -d "$1" ]; then 
  echo "Output directory already exists"
  exit 3
fi

mkdir -p $1

echo "Test settings:"
echo "  Output directory: $1"
echo "  Message size: $2"

for i in {1..30}; do
  echo "Starting consumer $i"
  timeout -s INT 120 python Consumer.py -g localhost -p 32869 > "$1/$i.csv" &
  sleep 0.3
done

echo "Starting producer"
timeout -s INT 100 python Producer.py -g localhost -p 32869 -r 0.2 -m "$2" > "$1/producer.log.txt" &

echo "Running for 120 seconds"
sleep 120
echo "Finished"
