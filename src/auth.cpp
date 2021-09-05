#include "TdParser.hpp"

namespace td_api = td::td_api;

auto TdParse::create_authentication_query_handler() {
        return [this, id = authentication_query_id_](Object object) {
            if (id == authentication_query_id_) {
                check_authentication_error(std::move(object));
            }
        };
    }

 void TdParse::on_authorization_state_update() {
        authentication_query_id_++;
        td_api::downcast_call(
                *authorization_state_,
                overloaded(
                        [this](td_api::authorizationStateReady &) {
                            are_authorized_ = true;
                            std::cout << "Got authorization" << std::endl;
                        },
                        [this](td_api::authorizationStateLoggingOut &) {
                            are_authorized_ = false;
                            std::cout << "Logging out" << std::endl;
                        },
                        [this](td_api::authorizationStateClosing &) { std::cout << "Closing" << std::endl; },
                        [this](td_api::authorizationStateClosed &) {
                            are_authorized_ = false;
                            need_restart_ = true;
                            std::cout << "Terminated" << std::endl;
                        },
                        [this](td_api::authorizationStateWaitCode &) {
                            std::cout << "Enter authentication code: " << std::flush;
                            std::string code;
                            std::cin >> code;
                            send_query(td_api::make_object<td_api::checkAuthenticationCode>(code),
                                       create_authentication_query_handler());
                        },
                        [this](td_api::authorizationStateWaitRegistration &) {
                            std::string first_name;
                            std::string last_name;
                            std::cout << "Enter your first name: " << std::flush;
                            std::cin >> first_name;
                            std::cout << "Enter your last name: " << std::flush;
                            std::cin >> last_name;
                            send_query(td_api::make_object<td_api::registerUser>(first_name, last_name),
                                       create_authentication_query_handler());
                        },
                        [this](td_api::authorizationStateWaitPassword &) {
                            std::cout << "Enter authentication password: " << std::flush;
                            std::string password;
                            std::getline(std::cin, password);
                            send_query(td_api::make_object<td_api::checkAuthenticationPassword>(password),
                                       create_authentication_query_handler());
                        },
                        [this](td_api::authorizationStateWaitOtherDeviceConfirmation &state) {
                            std::cout << "Confirm this login link on another device: " << state.link_ << std::endl;
                        },
                        [this](td_api::authorizationStateWaitPhoneNumber &) {
                            std::cout << "Enter phone number: " << std::flush;
                            std::string phone_number;
                            std::cin >> phone_number;
                            send_query(td_api::make_object<td_api::setAuthenticationPhoneNumber>(phone_number, nullptr),
                                       create_authentication_query_handler());
                        },
                        [this](td_api::authorizationStateWaitEncryptionKey &) {
                            std::cout << "Enter encryption key or DESTROY: " << std::flush;
                            std::string key;
                            std::getline(std::cin, key);
                            if (key == "DESTROY") {
                                send_query(td_api::make_object<td_api::destroy>(),
                                           create_authentication_query_handler());
                            } else {
                                send_query(td_api::make_object<td_api::checkDatabaseEncryptionKey>(std::move(key)),
                                           create_authentication_query_handler());
                            }
                        },
                        [this](td_api::authorizationStateWaitTdlibParameters &) {
                            auto parameters = td_api::make_object<td_api::tdlibParameters>();
                            parameters->database_directory_ = "Telegram";
                            parameters->use_message_database_ = true;
                            parameters->use_secret_chats_ = true;
                            parameters->api_id_ = 94575;
                            parameters->api_hash_ = "a3406de8d171bb422bb6ddf3bbd800e2";
                            parameters->system_language_code_ = "en";
                            parameters->device_model_ = "Desktop";
                            parameters->application_version_ = "1.7";
                            parameters->enable_storage_optimizer_ = true;
                            TdParse::send_query(td_api::make_object<td_api::setTdlibParameters>(std::move(parameters)),
                                       TdParse::create_authentication_query_handler());
                        }));
    }

     void TdParse::check_authentication_error(Object object) {
        if (object->get_id() == td_api::error::ID) {
            auto error = td::move_tl_object_as<td_api::error>(object);
            std::cout << "Error: " << to_string(error) << std::flush;
            on_authorization_state_update();
        }
    }