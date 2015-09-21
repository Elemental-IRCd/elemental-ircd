begin {Topic setting}

set test_channel #topic

client oper
       >> OPER god testsuite

client chanop
client victim

chanop >> JOIN $test_channel

oper :
  >> MODE [oper nick] +p
  >> MODE $test_channel +yo [chanop nick] [chanop nick]
  << WALLOPS

chanop :
  << MODE $test_channel
  >> TOPIC $test_channel :This is a test topic
  << TOPIC $test_channel {This is a test topic}

victim :
  << TOPIC
  >> TOPIC $test_channel :No I think this should be the topic
  << $ERR_CHANOPRIVSNEEDED

chanop :
  >> MODE $test_channel +v [victim nick]
  << MODE $test_channel +v

victim :
  << MODE $test_channel +v
  >> TOPIC $test_channel :No I think this should be the topic
  << $ERR_CHANOPRIVSNEEDED

chanop :
  >> MODE $test_channel +h [victim nick]
  << MODE $test_channel +h

victim :
  >> TOPIC $test_channel :No I think this should be the topic
  << TOPIC $test_channel {No I think this should be the topic}

chanop :
  >> MODE $test_channel +o [victim nick]
  << MODE $test_channel +o

victim :
  >> TOPIC $test_channel :No I think this should be the topic
  << TOPIC $test_channel {No I think this should be the topic}

chanop :
  >> MODE $test_channel +a [victim nick]
  << MODE $test_channel +a

victim :
  >> TOPIC $test_channel :No I think this should be the topic
  << TOPIC $test_channel {No I think this should be the topic}

chanop :
  >> MODE $test_channel +y [victim nick]
  << MODE $test_channel +y

victim :
  >> TOPIC $test_channel :No I think this should be the topic
  << TOPIC $test_channel {No I think this should be the topic}
