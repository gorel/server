What is This?
=============

I am creating a server in C to play around with the CJSON and OpenSSL libraries.

Usage
-----
Running make will create everything needed for both the server and client applications.  If you just want to compile the server, use "make server."  Likewise, if you just want to compile the client, use "make client."

### Server
./server <port number>

### Client
./client <host> <port number>

Updates
-------

### (7/28/13)
Base functionality now complete.  This is a simple console-based chat server that supports multiple users with multiplexing.

### (7/31/13)
A few new changes have been made to make the chat experience better for the user.  Private messaging is now available, blank messages won't be relayed to users, and usernames are now checked to ensure they are unique.  The code in server_setup and client_setup was also separated into *_utils and *_messaging to make relevant functions easier to find.

### (8/2/2013)
Minor improvements have been made to the program.  There are now admin functions available for muting and kicking users (as well as promoting other users to admin level).  The programs were tested extensively with Valgrind to test for any possible memory leaks.  Valgrind now shows a perfect output with all heap blocks freed and no leaks possible.  An ignore feature was added to allow users to block all private messages from specified users.  The system allows each user to ignore up to a maximum of 20 concurrent users.

What's Next
-----------

I'm still trying to decide how I want to proceed with encryption.  I want something with little overhead that's simple to implement within the system.  I may use openSSL's Blowfish encryption.  I want a strong block cipher that way the server doesn't have to bother with encrypting a single message N different ways (where N is the number of connected clients).

The next big step for this code will probably be either a strong encryption scheme or making the jump from console to a GUI interface.  User authentication (password login) will probably wait until after encryption has been implemented.
