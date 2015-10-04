begin {Topic setting}

set test_channel #topic

# XXX HACK work around #164
client:

client god
     oper god

client chanop
client victim

chanop >> JOIN $test_channel

god :
  >> MODE [god nick] +p
  >> MODE $test_channel +yo [chanop nick] [chanop nick]

chanop :
  << MODE $test_channel
  >> TOPIC $test_channel :This is a test topic
  << TOPIC $test_channel :This is a test topic

victim :
  << TOPIC $test_channel :This is a test topic
  >> TOPIC $test_channel :No I think this should be the topic
  << ERR_CHANOPRIVSNEEDED

chanop :
  >> MODE $test_channel +v [victim nick]

victim :
  << MODE $test_channel +v
  >> TOPIC $test_channel :No I think this should be the topic
  << ERR_CHANOPRIVSNEEDED

chanop :
  >> MODE $test_channel +h [victim nick]

victim :
  << MODE $test_channel +h
  >> TOPIC $test_channel :No I think this should be the topic
  << TOPIC $test_channel :No I think this should be the topic

chanop :
  >> MODE $test_channel +o [victim nick]

victim :
  << MODE $test_channel +o
  >> TOPIC $test_channel :No I think this should be the topic
  << TOPIC $test_channel :No I think this should be the topic

chanop :
  >> MODE $test_channel +a [victim nick]

victim :
  << MODE $test_channel +a
  >> TOPIC $test_channel :No I think this should be the topic
  << TOPIC $test_channel :No I think this should be the topic

chanop :
  >> MODE $test_channel +y [victim nick]

victim :
  << MODE $test_channel +y
  >> TOPIC $test_channel :No I think this should be the topic
  << TOPIC $test_channel :No I think this should be the topic
