//#include <nlohmann/json.hpp>
#include "nlohmann/json.hpp"
#include "overload.hpp"

#include <td/telegram/Client.h>

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <thread>

namespace td_api = td::td_api;


class TdParse {
public:
    TdParse() {
        td::ClientManager::execute(td_api::make_object<td_api::setLogVerbosityLevel>(1));
        client_manager_ = std::make_unique<td::ClientManager>();
        client_id_ = client_manager_->create_client_id();
        send_query(td_api::make_object<td_api::getOption>("version"), {});
    }
    void loop();
private:
    std::int64_t admin_log_chat_id = -1;
    std::int32_t admin_log_admin_action_id = 0;
    std::int32_t bread_log_entries_id = 0;
    std::size_t dumped_count = 0;
    std::int64_t last_event_id = 0;
    std::size_t processed_entries_count = 0;
    std::string json_filename = "recent_actions_dump.json";
    bool stop_dump = false;
    nlohmann::json entries;
    std::uint64_t handled_query_id = 0;

    std::vector<int> chats_to_parse;

    using Object = td_api::object_ptr<td_api::Object>;
    std::unique_ptr<td::ClientManager> client_manager_;
    std::int32_t client_id_{0};

    td_api::object_ptr<td_api::AuthorizationState> authorization_state_;
    bool are_authorized_{false};
    bool need_restart_{false};
    std::uint64_t current_query_id_{0};
    std::uint64_t authentication_query_id_{0};

    std::map<std::uint64_t, std::function<void(Object)>> handlers_;

    std::map<std::int32_t, td_api::object_ptr<td_api::user>> users_;

    std::map<std::int64_t, std::string> chat_title_;

    void restart();

    void send_query(td_api::object_ptr<td_api::Function> f, std::function<void(Object)> handler);

    void process_response(td::ClientManager::Response response);

    std::string get_user_name(std::int32_t user_id) const;

    std::string get_chat_title(std::int64_t chat_id) const;

    void process_update(td_api::object_ptr<td_api::Object> update);
    
    auto  create_authentication_query_handler();

    void  on_authorization_state_update();

    void  check_authentication_error(Object object);

    std::uint64_t next_query_id();

};