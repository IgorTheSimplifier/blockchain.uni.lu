#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>

namespace utils {
	// remove file
	void cleanUp(std::string filename) {
		if (std::remove(filename.c_str())!=0){
			std::cout << "Nothing to delete" << std::endl;
		}
		else {
			std::cout << filename << " has been successfully deleted" << std::endl;
		}
	}
}// namespace utils


class Client {
private:
	uint32_t age = 0;
	std::string address;
public:
	Client(	uint32_t age, std::string address);
	std::string sendMoney(double amount, std::string recvAddress);
private:
	void generateProof();
	std::string z_sendmany(double amount, std::string recvAddress);
	std::string getTransactionId(std::string opid);
};

Client::Client(	uint32_t _age, std::string _address)
: age(_age)
, address(_address) {
	std::cout << "Age " << age << std::endl;
	std::cout << "Address " << address << std::endl;
	generateProof();
}

void Client::generateProof() {
	std::cout << "Generating proof..." << std::endl;
	// get proof
	std::cout << "Proof generation is done." << std::endl;
}

// zcash-cli z_sendmany "$ZADDR" "[{\"amount\": 0.01, \"address\": \"$FRIEND_1\"}, {\"amount\": 0.01, \"address\": \"$FRIEND_2\"}]"
std::string Client::z_sendmany(double amount, std::string recvAddress){
	utils::cleanUp("opid.txt");
	std::string command = "zcash-cli z_sendmany \"";
				command += address;
				command += "\" \'[";
				// 1 block
				command += "{\"amount\": ";
				command += std::to_string(amount);
				command += ", \"address\": \"";
				command += recvAddress;
				command += "\"}";

				// 2 block
				command += ", {\"amount\": 0";
				command += ", \"address\": \"";
				command += recvAddress;
				command += "\"}";
				
				// 3 block
				command += ", {\"amount\": 0";
				command += ", \"address\": \"";
				command += recvAddress;
				command += "\"}";

				command += "]\' >> opid.txt";
	std::cout << command << std::endl;
	system(command.c_str());
	
	std::ifstream file("opid.txt");
	std::string opid;
	getline(file, opid);
	file.close();

	std::cout << "Money has been sent. OPID " << opid << std::endl;

	return opid;
}

static inline void trimNonAlphabetical(std::string& s) {
	s.erase(std::remove_if(s.begin(), s.end(), 
		[](int c) -> bool { return !std::isalnum(c); } ), s.end());
}

// zcash-cli z_getoperationstatus '["opid-e22f8794-d01d-4e23-b4d2-dbb24cf2cd28"]'
std::string Client::getTransactionId(std::string opid) {
	std::string command = "zcash-cli z_getoperationstatus \'[\"";
				command += opid;
				command += "\"]\'";
				command += " >> operationstatus.txt";
	std::cout << command << std::endl;

	std::string status;
	do {
		std::cout << "Let's check...." << std::endl;
		const int sleepTime = 5000; // ms
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
		utils::cleanUp("operationstatus.txt");
		system(command.c_str());
		
		// getting status
		std::ifstream file("operationstatus.txt");
		std::string word;
		bool toBreak = false;
		while(file >> word) { //take word and print
			if(toBreak) {
				status = word;
				trimNonAlphabetical(status);
				break;
			}
			if (word == "\"status\":")
				toBreak = true;
		}		
		file.close();

	} while(status == "executing");

	std::string txid = "";

	if (status == "success") {
		// getting txid
		std::ifstream file("operationstatus.txt");
		std::string word;
		bool toBreak = false;
		while(file >> word) { //take word and print
			if(toBreak) {
				txid = word;
				trimNonAlphabetical(txid);
				break;
			}
			if (word == "\"txid\":")
				toBreak = true;
		}		
		file.close();
	}
	else {
		std::cout << "Transaction has failed" << std::endl;
	}

	return txid;
}

std::string Client::sendMoney(double amount, std::string recvAddress) {
	std::cout << "Sending " << amount << " to " << recvAddress << std::endl;

	// TODO: encode proof in memo
	std::string opid = z_sendmany(amount, recvAddress);
	std::string txid = getTransactionId(opid);

	return txid;
}


int main() {
	std::string from = "ztestsapling19xjj9mdjudlr3863a0pjcf0kwhfqdpremlhcz5sy4r9sj64xmw6z89crar50tyrgf279x2l07uu";
	std::string to = "ztestsapling1dcmz9s58kx85atjmw89c0nh05dqe5qyakpth8szw8c7hxymgkdsastksrptdfnmc6390qh8gl8r";
	uint32_t age = 18;
	double amount = 0.01;

	Client client {age, from};	
	std::cout << client.sendMoney(amount, to) << std::endl;
	return 0;
}	