begin test chghost {Check chghost}

client op   hub

oper

client hub  hub
client leaf leaf1

op   join_channel #chghost
hub  join_channel #chghost
leaf join_channel #chghost

# wait for joins to propagate
sleep 3

op send_cmd CHGHOST [hub nick] chghost.test

op   expect "QUIT :Changing host"
leaf expect "QUIT :Changing host"

op   expect "@chghost.test JOIN #chghost"
leaf expect "@chghost.test JOIN #chghost"

hub  expect "chghost.test :is now your hidden host"
