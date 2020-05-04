#README File for e-ADARP instances and solutions
Instance names: <a/u><numvehicles>-<numcustomers>-<minimum end battery ratio levels>

All instances have the following format:

#vehicles #users #origindepots #destinationdepots #stations #replications #time horizon
node_id lat long service time load arr dep
...
common origin depot id
common destination depot id
(artificial) origin depots id
(artificial) destination depots id
charging stations id
users maximum ride time
vehicles capacity
vehicles initial battery inventory
vehicles battery capacities
minimum end battery ratio levels
recharging rates at charging stations
vehicles discharging rate
weight factors
original travel times (for Uber instances -- need to be multiplied by a factor of 2 to reproduce results)

#solutions 
Solution for uber dataset
Instance names: <u><numvehicles>-<numcustomers>-<minimum end battery ratio levels>-<number of visits per station (if >1)>
