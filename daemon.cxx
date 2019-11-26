#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <set>
#include <thread>
#include <chrono>
#include "json.hpp"

using json = nlohmann::json;

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
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

class Daemon {
private:
	const int 		sleepTime = 10000; // ms
	std::set<json> 	transactions;
	int 			prevTransactionsSize = 0;

public:
	void run(std::string filename, std::string address);

private:
	// address has to be z_addr
	void retrievTransaction(std::string filename, std::string address);
	
	// filename has to have *.json extension
	// returns vector of json transactions
	void parceResultList(std::string filename);
	
	void processTransactions();
};


void Daemon::retrievTransaction(std::string filename, std::string address){
	std::string z_listreceivedbyaddress = "zcash-cli z_listreceivedbyaddress \"";
				z_listreceivedbyaddress += address;
				z_listreceivedbyaddress += "\" >> ";
				z_listreceivedbyaddress += filename;
	system(z_listreceivedbyaddress.c_str());
	std::cout << "Transaction list has been retrieved" << std::endl;
}


void Daemon::parceResultList(std::string filename) {
	// std::cout << "Start parsing result list" << std::endl;
	std::ifstream	file(filename);

	std::string			transactionJSONasString = "";
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
			transactions.insert(transactionJSON);
			transactionJSONasString = "";
		}
	}
	file.close();
}


void Daemon::processTransactions() {
	std::cout << "Total transactions: " << transactions.size() << std::endl;
	if (transactions.size() > prevTransactionsSize) {
		int diff = transactions.size() - prevTransactionsSize;
		std::cout << "New transactions found: " << diff << std::endl;
	}
	else {
		std::cout << "No new transactions;" << std::endl;
	}
	prevTransactionsSize = transactions.size();
}


void Daemon::run(std::string filename, std::string address) {
	for (;;)
	{
		cleanUp(filename);
		retrievTransaction(filename, address);
		parceResultList(filename);
		processTransactions();
		
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
	}
}


int main() {
	std::cout << "Execution starts" << std::endl;
	
	std::string z_addr 		= "ztestsapling19xjj9mdjudlr3863a0pjcf0kwhfqdpremlhcz5sy4r9sj64xmw6z89crar50tyrgf279x2l07uu";
	std::string filename 	= "z_listreceivedbyaddress.json";

	Daemon daemon;
	daemon.run(filename,z_addr);

	return 0;
}