
# Table of Contents

1.  [Contributors](#orgede61b9)
2.  [Using select()](#org0b6ed1d)
3.  [Using select() with INET Server-Client](#org35bb7db)
4.  [Using poll() with INET Server-Client](#orgea411ec)
5.  [Using epoll with Client-Server](#org7218c22)
    1.  [The EPOLLET flag](#org5b681fd)



<a id="orgede61b9"></a>

# Contributors

-   Jonathan Rodriguez Gomez


<a id="org0b6ed1d"></a>

# Using select()

Program Output:

    sammy@mac files % ./ex2
    test
    Input available in STDIN
    Input received: test
    
    12
    Input available in STDIN
    Input received: 12
    
    quit
    Input available in STDIN
    Input received: quit
    Quit command received


<a id="org35bb7db"></a>

# Using select() with INET Server-Client

Client Output:

    sammy@mac files % ./select_client
    Connection established
    Server FQDN: mac.dhcp.wustl.edu
    Server time of day: 1776704232.617814
    
    sammy@mac files % ./select_client
    Connection established
    Server FQDN: mac.dhcp.wustl.edu
    Server time of day: 1776704237.598486

Server Output:

    sammy@mac files % ./select_server
    Client connection #0 accepted
    Client connection #1 accepted
    test
    Input available in STDIN
    Input received: test
    
    quit
    Input available in STDIN
    Input received: quit
    Quit command received


<a id="orgea411ec"></a>

# Using poll() with INET Server-Client

Client Output:

    sammy@mac files % ./poll_client
    Connection established
    sammy@mac files % ./poll_client
    Connection established
    sammy@mac files % ./poll_client quit
    Connection established
    Issuing quit command

Server Output:

    sammy@mac files % ./poll_server
    Client 1: Message 1
    Client 1: Message 2
    test
    Input received from STDIN: test
    
    Client 2: Message 1
    Client 2: Message 2
    Client 3: Message 1
    Client 3: Message 2
    Client 3: quit
    Quit command received
    sammy@mac files % ./poll_server
    quit
    Input received from STDIN: quit
    Quit command received


<a id="org7218c22"></a>

# Using epoll with Client-Server

Client Output:

    sammy@ramanujan:~/Nextcloud/college/Classes/2026/Spring/CSE4202/studios/cse4202-studio19/files$ ./epoll_client
    Connection established
    sammy@ramanujan:~/nextcloud/college/classes/2026/spring/cse4202/studios/cse4202-studio19/files$ ./epoll_client
    Connection established

Server Output (before STDIN input):

    sammy@ramanujan:~/nextcloud/college/classes/2026/spring/cse4202/studios/cse4202-studio19/files$ ./epoll_server
    Input available from Client 1
    Client 1 has closed the connection
    Input available from Client 2
    Client 2 has closed the connection

Server Output (after STDIN input):

    studio19/files$ ./epoll_server
    Input available from Client 1
    Client 1 has closed the connection
    Input available from Client 2
    Client 2 has closed the connection
    
    [...]
    
    Input available in STDIN
    Input available in STDIN
    Input available in STDIN
    Input available in STDIN

This happens because unlike the client socket STDIN is not closed after it obtains data. Since the default mode of epoll is level-triggered and the data is not cleaned up epoll keeps creating an EPOLLIN event for STDIN.


<a id="org5b681fd"></a>

## The EPOLLET flag

Client Output:

    sammy@ramanujan:~/Nextcloud/college/Classes/2026/Spring/CSE4202/studios/cse4202-studio19/files$ ./epoll_client
    Connection established
    sammy@ramanujan:~/Nextcloud/college/Classes/2026/Spring/CSE4202/studios/cse4202-studio19/files$ ./epoll_client
    Connection established

Server Output:

    sammy@ramanujan:~/Nextcloud/college/Classes/2026/Spring/CSE4202/studios/cse4202-studio19/files$ ./epoll_server
    Input available from Client 1
    Client 1 has closed the connection
    Input available from Client 2
    Client 2 has closed the connection
    test
    Input available in STDIN

Since the change to a POLLIN state only happens once, we don&rsquo;t see the same behavior with STDIN in this case. As we can see the client socket behaves pretty much the same since we are not reading the data that becomes available.

