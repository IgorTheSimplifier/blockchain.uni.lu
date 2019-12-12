#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <set>
#include <thread>
#include <chrono>
#include <SDL2/SDL.h>
#include "json.hpp"

using json = nlohmann::json;

#define LIBRARY_FILENAME "using_bulletproofs/target/release/libusing_bulletproofs.dylib"

typedef bool (*verify_bullet)(char*); 

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// remove file
void cleanUp(std::string filename) {
	if (std::remove(filename.c_str())!=0){
		// std::cout << "Nothing to delete" << std::endl;
	}
	else {
		// std::cout << filename << " has been successfully deleted" << std::endl;
	}
}

class Server {
private:
	const int 		sleepTime = 1000; // ms
	std::set<json> 	transactions;
	std::set<json>	newTransactions;
	verify_bullet 	verify;
public:
	Server(std::string filename, std::string address);
private:
	void run(std::string filename, std::string address);
	// address has to be z_addr
	void retrievTransaction(std::string filename, std::string address);
	
	// filename has to have *.json extension
	// returns vector of json transactions
	void parceResultList(std::string filename);
	
	void processNewTransactions();
};


Server::Server(std::string filename, std::string address) {
	void *library_handle = SDL_LoadObject(LIBRARY_FILENAME);
	if (library_handle == NULL) {
		std::cout << "COULD NOT LOAD LIB" << std::endl;
		exit(-1);
	}

	verify = (verify_bullet) SDL_LoadFunction(library_handle, "verify_encoded_age_bulletproof");
	if (verify == NULL) {
		std::cout << "COULD NOT FIND FN verify_encoded_age_bulletproof" << std::endl;
		exit(-1);
	}
	
	run(filename, address);
}

void Server::retrievTransaction(std::string filename, std::string address){
	std::string z_listreceivedbyaddress = "zcash-cli z_listreceivedbyaddress \"";
				z_listreceivedbyaddress += address;
				z_listreceivedbyaddress += "\" >> ";
				z_listreceivedbyaddress += filename;
	system(z_listreceivedbyaddress.c_str());
	// std::cout << "Transaction list has been retrieved" << std::endl;
}


void Server::parceResultList(std::string filename) {
	newTransactions.clear();
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
			if (transactions.find(transactionJSON) == transactions.end()) {
				transactions.insert(transactionJSON);
				newTransactions.insert(transactionJSON);
			}
			transactionJSONasString = "";
		}
	}
	file.close();
}


void Server::processNewTransactions() {
	if (newTransactions.size() > 0) {
		std::cout << std::endl;
		for(auto transaction : newTransactions) {
			std::string txid = transaction["txid"].dump();
			txid = txid.substr(1, txid.size() - 2);
			std::string proof = transaction["memo"].dump();
			proof = proof.substr(1, proof.size() - 2);
			std::cout << txid << " ";
			//std::cout << proof << std::endl;

			std::vector<char> v(proof.begin(), proof.end());
			char* proofCh = &v[0]; 
			if (verify(proofCh)) {
				std::cout << "[+]" << std::endl;
			}
			else {
				std::cout << "[-]" << std::endl;
			}
		}
	}
	else {
		std::cout << "." << std::flush;
	}
}


void Server::run(std::string filename, std::string address) {
	for (;;)
	{
		cleanUp(filename);
		retrievTransaction(filename, address);
		parceResultList(filename);
		processNewTransactions();
		
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
	}
}


int main() {
	std::cout << "Execution starts" << std::endl;
	
	std::string z_addr 		= "ztestsapling1mrtff36e7as3k8wxvmzuawjck3p8je0krwy9x7krh6gstgncrx70h9qw4hv7et4v2e8q5rf2n70";
	std::string filename 	= "z_listreceivedbyaddress.json";

	Server server(filename,z_addr);

	return 0;
}