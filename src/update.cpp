#include "TdParser.hpp"

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