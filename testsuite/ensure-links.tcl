begin {ensure all servers are linked before tests work}

client god
    oper god
    >> CONNECT leaf1.local 6698
    << NOTICE * *Link with*
    >> MAP
    << 015 * *leaf1.local*

    >> CONNECT leaf2.local 6699
    << NOTICE * *Link with*
    >> MAP
    << 015 * *leaf2.local*
