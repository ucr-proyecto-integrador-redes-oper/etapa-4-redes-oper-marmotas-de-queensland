#!/bin/bash
# $1 = num blue nodes, $2 = Server IP, $3 = Server Port
echo "Creating $1 blue nodes with orange server ip $2 and port $3."
for (( i=0; i<$1; i++ ))
do
  ./blueNode $2 $3 & disown
done
echo All done.
exit
