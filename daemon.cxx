#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <vector>
#include <thread>
#include <chrono>
#include "json.hpp"

using json = nlohmann::json;

const int sleepTime = 10000; // ms

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// address has to be z_addr
void retrievTransaction(std::string filename, std::string address){
	std::string z_listreceivedbyaddress = "zcash-cli z_listreceivedbyaddress \"";
				z_listreceivedbyaddress += address;
				z_listreceivedbyaddress += "\" >> ";
				z_listreceivedbyaddress += filename;
	system(z_listreceivedbyaddress.c_str());
	std::cout << "Transaction list has been retrieved" << std::endl;
}

// filename has to have *.json extension
// returns vector of json transactions
std::vector<json> parceResultList(std::string filename) {
	std::cout << "Start parsing result list" << std::endl;
	
	std::ifstream	file(filename);

	std::string			transactionJSONasString = "";
	std::vector<json> 	transactions;
	for(std::string line; getline(file, line ); )
	{
		if (line == "["
			|| line == "]") 
			continue;

		ltrim(line);
		
		if (line == "},"){
			line = "}";
		}

		transactionJSONasString += line + "\n";

		if (line == "}")
		{
			json			transactionJSON;
			transactionJSON = json::parse(transactionJSONasString);
			transactions.push_back(transactionJSON);
			transactionJSONasString = "";
		}
	}

	file.close();

	std::cout << "Finished parsing result list" << std::endl;
	return transactions;
}

// remove file
void cleanUp(std::string filename) {
	if (std::remove(filename.c_str())!=0){
		std::cout << "Nothing to delete" << std::endl;
	}
	else {
		std::cout << filename << " has been successfully deleted" << std::endl;
	}
}

int processTransactions(std::vector<json> transactions, int prevTransactionsSize) {
	std::cout << "Total transactions: " << transactions.size() << std::endl;
	if (transactions.size() > prevTransactionsSize) {
		int diff = transactions.size() - prevTransactionsSize;
		std::cout << "New transactions found: " << diff << std::endl;
	}
	else {
		std::cout << "No new transactions" << std::endl;
	}
	return transactions.size();
}

// filename has to have *.json extension
// address has to be z_addr
void daemonTransactionRetriever(std::string filename, std::string address) {
	std::vector<json> transactions;
	int prevTransactionsSize = 0;
	for (;;)
	{
		std::cout << "New iteration" << std::endl;
		cleanUp(filename);
		retrievTransaction(filename, address);
		transactions = parceResultList(filename);
		prevTransactionsSize =  processTransactions(transactions, prevTransactionsSize);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
		std::cout << "New attempt" << std::endl;
	}
}

int main() {
	std::cout << "Execution starts" << std::endl;
	
	std::string z_addr 		= "ztestsapling19xjj9mdjudlr3863a0pjcf0kwhfqdpremlhcz5sy4r9sj64xmw6z89crar50tyrgf279x2l07uu";
	std::string filename 	= "z_listreceivedbyaddress.json";

	daemonTransactionRetriever(filename, z_addr);
	
	return 0;
}