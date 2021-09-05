#include<iostream>
#include<fstream>
#include <string>

std::string monitored_chats="monitored_chats.txt";

void add_chat_to_monitoring(int64_t& chat){
    std::ofstream outdata;
    outdata.open(monitored_chats,std::ios::app);

  if (outdata.is_open())
  {
    outdata << chat << std::endl;
    outdata.close();
  }
      else std::cerr << "Unable to open file";
}

void list_monitored_chats() {
std::string line;
  std::ifstream infile (monitored_chats,std::ios::in);
  if (infile.is_open())
  {
    while ( std::getline (infile,line) )
    {
      std::cout << line << '\n';
    }
    infile.close();
  }

  else std::cerr << "Unable to open file"; 
}