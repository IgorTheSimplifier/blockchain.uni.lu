# daemon
With specified z_addr, periodically checks if there any new transaction 

In order to run this app, you have to: 
1. have already zcashd up and running
2. accessible zcash-cli from any place
3. build with g++ -sdt=11 option
4. specify your z_addr in main

# client
Sends specified amount of money to specified addr and returns transaction id.