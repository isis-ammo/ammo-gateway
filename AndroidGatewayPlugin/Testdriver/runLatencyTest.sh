#!/bin/bash

if [ -d /tmp/results ]; then 
  rm -rf /tmp/results
fi

mkdir /tmp/results

for i in {1..30}; do
  echo "Starting consumer $i"
  timeout -s INT 120 python Consumer.py localhost 33289 > /tmp/results/$i.csv &
  sleep 0.3
done

echo "Starting producer"
timeout -s INT 100 python Producer.py localhost 33289 0.2 > /tmp/results/producer.log.txt &

echo "Running for 120 seconds"
sleep 120
echo "Finished"
