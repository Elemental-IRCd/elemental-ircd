begin {Check chghost}

set test_channel #chghost

client myoper -oper god
client target -caps chghost
client observer
client observerv3 -caps chghost

myoper :
 >> MODE #chghost +v [target nick]
 >> CHGHOST [target nick] chghost.test

set old_mask [target hostmask]

target :
if {[have chghost]} {
 << :$old_mask CHGHOST * chghost.test
} else {
 << RPL_HOSTHIDDEN [nick] chghost.test
}

set new_mask [target hostmask]

myoper :
 << :$old_mask QUIT :Changing host
 << :$new_mask JOIN $test_channel
 << MODE #chghost +v [target nick]

observer :
 << :$old_mask QUIT :Changing host
 << :$new_mask JOIN $test_channel
 << MODE #chghost +v [target nick]

observerv3 :
if {[have chghost]} {
 << :$old_mask CHGHOST * chghost.test
}

