begin {Client and channel metadata}

set test_channel #metadata

client :
    if {![supports METADATA]} {
        skip {Metadata not available}
    }

    # Channel metadata
    >> METADATA $test_channel SET Key Value
    << RPL_KEYVALUE $test_channel Key * Value

    >> METADATA $test_channel GET Key
    << RPL_KEYVALUE $test_channel Key * Value

    >> METADATA $test_channel LIST
    << RPL_KEYVALUE [meta nick] Key * Value
    << RPL_METADATAEND

    # User metadata
    >> METADATA * SET UserKey UserValue
    << RPL_KEYVALUE * UserKey * UserValue

    >> METADATA * GET UserKey
    << RPL_KEYVALUE * UserKey * UserValue

    >> METADATA * LIST
    << RPL_KEYVALUE * UserKey * UserValue
    << RPL_METADATAEND

    # Check that we can fetch the same by nick instead of *
    >> METADATA [nick] GET UserKey
    << RPL_KEYVALUE [nick] UserKey * UserValue

    >> METADATA [nick] LIST
    << RPL_KEYVALUE [nick] UserKey * UserValue
    << RPL_METADATAEND
