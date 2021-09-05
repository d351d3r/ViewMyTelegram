#include "TdParser.hpp"
#include "monitoring_chats.hpp"

namespace td_api = td::td_api;

void TdParse::loop() {
    while (true) {
      if (need_restart_) {
        restart();
      } else if (!are_authorized_) {
        process_response(client_manager_->receive(10));
      } else {
        std::cout << "Enter action [q] quit | [u] check for updates and request results | [c] show chats [m <chat_id> "
                     "<text>] | [add <chat_id>] to add in parse list | send message [me] | show self | [l] logout: "
                  << std::endl;
        std::string line;
        std::getline(std::cin, line);
        std::istringstream ss(line);
        std::string action;
        if (!(ss >> action)) {
          continue;
        }
        if (action == "q") {
          std::cout << "Exit.." << std::endl;
          return;
        }
        else if (action == "l") {
          std::cout << "Logging out..." << std::endl;
          send_query(td_api::make_object<td_api::logOut>(), {});
        }
        if (action == "u") {
          std::cout << "Checking for updates..." << std::endl;
          while (true) {
            auto response = client_manager_->receive(0);
            if (response.object) {
              process_response(std::move(response));
            } else {
              break;
            }
          }
        } else if (action == "close") {
          std::cout << "Closing..." << std::endl;
          send_query(td_api::make_object<td_api::close>(), {});
        } else if (action == "me") {
          send_query(td_api::make_object<td_api::getMe>(),
                     [this](Object object) { std::cout << to_string(object) << std::endl; });
        } else if (action == "m") {
          std::int64_t chat_id;
          ss >> chat_id;
          ss.get();
          std::string text;
          std::getline(ss, text);
        } else if (action == "add") {
          // std::string line;
          // std::getline(std::cin, line);
          // std::int64_t chat_id = 0;
          // chat_id = std::stoi(line);
          std::int64_t chat_id = 0;
          ss >> chat_id;
          ss.get();
          if(chat_id !=0) { 
            add_chat_to_monitoring(chat_id);
            std::cout << "Chat " << chat_id << " added for monitoring." << std::endl;
          }
          else std::cout << "Enter the corrected chat ID!" << std::endl;
        } else if (action == "c") {
          std::cout << "Loading chat list..." << std::endl;
          send_query(td_api::make_object<td_api::getChats>(nullptr, 20), [this](Object object) {
            if (object->get_id() == td_api::error::ID) {
              return;
            }
            auto chats = td::move_tl_object_as<td_api::chats>(object);
            for (auto chat_id : chats->chat_ids_) {
              std::cout << "[chat_id:" << chat_id << "] [title:" << chat_title_[chat_id] << "]" << std::endl;
            }
          });
        }
      }
    }
}