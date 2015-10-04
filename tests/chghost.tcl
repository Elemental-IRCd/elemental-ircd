begin {Check chghost}

set test_channel #chghost

client myoper
    oper god

client target -caps chghost
client observer
client observerv3 -caps chghost

set old_mask [from target]

myoper :
 >> MODE +p [nick]
 >> MODE #chghost +v [target nick]
 >> CHGHOST [target nick] chghost.test

target :
if {[have chghost]} {
 << :$old_mask CHGHOST * chghost.test
} else {
 << RPL_HOSTHIDDEN [target nick] chghost.test
}

set new_mask [from target]

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

