#include "TdParser.hpp"

// Simple single-threaded example of TDLib usage.
// Real world programs should use separate thread for the user input.
// Example includes user authentication, receiving updates, getting chat list and sending text messages.

namespace td_api = td::td_api;

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


  std::uint64_t TdParse::next_query_id() {
    return ++current_query_id_;
  }

int main() {
  TdParse parse;
  parse.loop();
}