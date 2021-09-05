#include "TdParser.hpp"
#include "overload.hpp"

// Simple single-threaded example of TDLib usage.
// Real world programs should use separate thread for the user input.
// Example includes user authentication, receiving updates, getting chat list and sending text messages.

struct date_printer {
    std::time_t datetime_;

    date_printer(std::time_t t) : datetime_(t) {}
};

std::ostream &operator<<(std::ostream &os, const date_printer &prn) {
    auto printed_date = gmtime(&prn.datetime_);
    char buf[40];
    strftime(buf, 40, "%c", printed_date);
    os << buf;
    return os;
}


namespace td_api = td::td_api;


void TdParse::loop() {
    while (true) {
      if (need_restart_) {
        restart();
      } else if (!are_authorized_) {
        process_response(client_manager_->receive(10));
      } else {
        std::cout << "Enter action [q] quit [u] check for updates and request results [c] show chats [m <chat_id> "
                     "<text>] send message [me] show self [l] logout: "
                  << std::endl;
        std::string line;
        std::getline(std::cin, line);
        std::istringstream ss(line);
        std::string action;
        if (!(ss >> action)) {
          continue;
        }
        if (action == "q") {
          return;
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
        } else if (action == "l") {
          std::cout << "Logging out..." << std::endl;
          send_query(td_api::make_object<td_api::logOut>(), {});
        } else if (action == "m") {
          std::int64_t chat_id;
          ss >> chat_id;
          ss.get();
          std::string text;
          std::getline(ss, text);

          std::cout << "Sending message to chat " << chat_id << "..." << std::endl;
          auto send_message = td_api::make_object<td_api::sendMessage>();
          send_message->chat_id_ = chat_id;
          auto message_content = td_api::make_object<td_api::inputMessageText>();
          message_content->text_ = td_api::make_object<td_api::formattedText>();
          message_content->text_->text_ = std::move(text);
          send_message->input_message_content_ = std::move(message_content);

          send_query(std::move(send_message), {});
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
void TdParse::restart() {
    client_manager_.reset();
    *this = TdParse();
  }

  void TdParse::send_query(td_api::object_ptr<td_api::Function> f, std::function<void(Object)> handler) {
    auto query_id = next_query_id();
    if (handler) {
      handlers_.emplace(query_id, std::move(handler));
    }
    client_manager_->send(client_id_, query_id, std::move(f));
  }

  void TdParse::process_response(td::ClientManager::Response response) {
    if (!response.object) {
      return;
    }
    //std::cout << response.request_id << " " << to_string(response.object) << std::endl;
    if (response.request_id == 0) {
      return process_update(std::move(response.object));
    }
    auto it = handlers_.find(response.request_id);
    if (it != handlers_.end()) {
      it->second(std::move(response.object));
      handlers_.erase(it);
    }
  }

  std::string TdParse::get_user_name(std::int32_t user_id) const {
    auto it = users_.find(user_id);
    if (it == users_.end()) {
      return "unknown user";
    }
    return it->second->first_name_ + " " + it->second->last_name_;
  }

  std::string TdParse::get_chat_title(std::int64_t chat_id) const {
    auto it = chat_title_.find(chat_id);
    if (it == chat_title_.end()) {
      return "unknown chat";
    }
    return it->second;
  }

  void TdParse::process_update(td_api::object_ptr<td_api::Object> update) {
    td_api::downcast_call(
        *update, overloaded(
                     [this](td_api::updateAuthorizationState &update_authorization_state) {
                       authorization_state_ = std::move(update_authorization_state.authorization_state_);
                       on_authorization_state_update();
                     },
                     [this](td_api::updateNewChat &update_new_chat) {
                       chat_title_[update_new_chat.chat_->id_] = update_new_chat.chat_->title_;
                     },
                     [this](td_api::updateChatTitle &update_chat_title) {
                       chat_title_[update_chat_title.chat_id_] = update_chat_title.title_;
                     },
                     [this](td_api::updateUser &update_user) {
                       auto user_id = update_user.user_->id_;
                       users_[user_id] = std::move(update_user.user_);
                     },
                     [this](td_api::updateNewMessage &update_new_message) {
                       auto chat_id = update_new_message.message_->chat_id_;
                       std::string sender_name;
                       td_api::downcast_call(*update_new_message.message_->sender_,
                                             overloaded(
                                                 [this, &sender_name](td_api::messageSenderUser &user) {
                                                   sender_name = get_user_name(user.user_id_);
                                                 },
                                                 [this, &sender_name](td_api::messageSenderChat &chat) {
                                                   sender_name = get_chat_title(chat.chat_id_);
                                                 }));
                       std::string text;
                       if (update_new_message.message_->content_->get_id() == td_api::messageText::ID) {
                         text = static_cast<td_api::messageText &>(*update_new_message.message_->content_).text_->text_;
                       }
                       std::cout << "Got message: [chat_id:" << chat_id << "] [from:" << sender_name << "] [" << text
                                 << "]" << std::endl;
                     },
                     [](auto &update) {}));
  }

  std::uint64_t TdParse::next_query_id() {
    return ++current_query_id_;
  }

int main() {
  TdParse example;
  example.loop();
}