#!/bin/bash
# Script to instantiate n blue nodes.
# $1 = num blue nodes, $2 = Server IP, $3 = Server Port
echo "Creating $1 blue nodes with orange server ip $2 and port $3."
echo "Nodes ip: $4"
for (( i=0; i<$1; i++ ))
do
  ./blue-node $2 $3 &
  sleep 1
done

sleep 60
echo All done.
exit
