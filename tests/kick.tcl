begin {kicking, etc}

set test_channel #kickland

client oper
       >> OPER god testsuite

client chanop
client victim

puts {TODO: clean this up: fix cantkickrange}

# Set up client oper as god
oper :
  >> MODE [oper nick] +p

##############

# data Client = Peon
#             | Voiced
#             | Halfop
#             | Chanop
#             | Admin
#             | Owner
#
# kick :: Client -> Client -> Bool
# kick Peon   _      = False
# kick Voiced _      = False
# kick Halfop Peon   = True
# kick Halfop Voiced = True
# kick Halfop _      = False
# kick Chanop Admin  = False
# kick Chanop Owner  = False
# kick Chanop _      = True
# kick Admin  Owner  = False
# kick Admin  _      = True
# kick Owner  _      = True

proc addrank {who rank} {
  global oper
  global test_channel

  oper >> MODE $test_channel $rank [$who nick]
  $who << MODE $test_channel $rank [$who nick]
}

proc shouldkick {src dest} {
  global test_channel

  $src >> KICK $test_channel [$dest nick] :Test kick!
  $dest :
    << KICK
    >> JOIN $test_channel
}

proc cantkick {src dest} {
  global test_channel

  $src :
    >> KICK $test_channel [$dest nick] :Test kick!
    << ERR_CHANOPRIVSNEEDED
}

proc reset client {
  global test_channel

  $client :
    >> PART $test_channel :This game of tag is boring, I'm out of here
    >> JOIN $test_channel
}

# cantkickrange :: Client -> Client -> [Mode] -> IO ()
#proc cantkickrange {src dest modes} {
#  reset $src
#  reset $dest
#
#  foreach mode {*}$modes {
#    addrank $mode $dest
#    cantkick $src $dest
#    reset $dest
#  }
#}

# Both clients start out as peons
reset chanop
reset victim

puts {Peons cannot kick anyone}
cantkick chanop victim
reset victim

addrank victim +v
cantkick chanop victim
reset victim

addrank victim +h
cantkick chanop victim
reset victim

addrank victim +o
cantkick chanop victim
reset victim

addrank victim +a
cantkick chanop victim
reset victim

addrank victim +y
cantkick chanop victim
reset victim

puts {A voiced user cannot kick anyone either}
addrank chanop +v
cantkick chanop victim
reset victim

addrank victim +v
cantkick chanop victim
reset victim

addrank victim +h
cantkick chanop victim
reset victim

addrank victim +o
cantkick chanop victim
reset victim

addrank victim +a
cantkick chanop victim
reset victim

addrank victim +y
cantkick chanop victim
reset victim

reset chanop

puts {A halfop cannot kick halfops or up}
addrank chanop +h
shouldkick chanop victim
reset victim

addrank victim +v
shouldkick chanop victim
reset victim

addrank victim +h
cantkick chanop victim
reset victim

addrank victim +o
cantkick chanop victim
reset victim

addrank victim +a
cantkick chanop victim
reset victim

addrank victim +y
cantkick chanop victim
reset victim

reset chanop

puts {A chanop can kick everything but admin and owner}
addrank chanop +o
shouldkick chanop victim
reset victim

addrank victim +v
shouldkick chanop victim
reset victim

addrank victim +h
shouldkick chanop victim
reset victim

addrank victim +o
shouldkick chanop victim
reset victim

addrank victim +a
cantkick chanop victim
reset victim

addrank victim +y
cantkick chanop victim
reset victim

reset chanop

puts {An admin can kick anyone but an owner}
addrank chanop +a
shouldkick chanop victim
reset victim

addrank victim +v
shouldkick chanop victim
reset victim

addrank victim +h
shouldkick chanop victim
reset victim

addrank victim +o
shouldkick chanop victim
reset victim

addrank victim +a
shouldkick chanop victim
reset victim

addrank victim +y
cantkick chanop victim
reset victim

reset chanop

puts {An owner can kick anyone. Period.}
addrank chanop +y
shouldkick chanop victim
reset victim

addrank victim +v
shouldkick chanop victim
reset victim

addrank victim +h
shouldkick chanop victim
reset victim

addrank victim +o
shouldkick chanop victim
reset victim

addrank victim +a
shouldkick chanop victim
reset victim

addrank victim +y
shouldkick chanop victim
reset victim
