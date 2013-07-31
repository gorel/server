I am creating a server in C to play around with the CJSON and OpenSSL libraries.

(7/28/13) Base functionality now complete.  This is a simple console-based chat server that supports multiple users with multiplexing.

(7/31/13) A few new changes have been made to make the chat experience better for the user.  Private messaging is now available, blank messages won't be relayed to users, and usernames are now checked to ensure they are unique.  The code in server_setup and client_setup was also separated into *_utils and *_messaging to make relevant functions easier to find.

The next big step for this code will probably be either a strong encryption scheme or making the jump from console to a GUI interface.  User authentication (password login) will probably wait until after encryption has been implemented.
