# Prerequisites
Before running server or client you have 

In order to build client and server, you have to: 
1. to go insed of using_bulletproofs folder and build the library by typing `cargo build --release`
2. Have SDL2 installed.
3. In the root dir build client and server by typing `make` in the console (works for Mac OS as of now, but you can build it yourself).

It's time to run, but don't rush, couple more steps are needed before:
1. zcashd already up and running
2. accessible zcash-cli from any place (hint: create symbolic link)

# client
Sends specified amount of money to specified addr and returns transaction id. Inside the memo field it writes the serialized bulletproof and commited valued concatenated as one string.

# server
With specified z_addr, periodically checks if there any new transaction, and if there are such, checks zk (age >= 18) with given encoded string in memo field.