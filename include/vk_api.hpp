#ifndef VK_GRAFFITI_BOT_VK_API_HPP
#define VK_GRAFFITI_BOT_VK_API_HPP

#include "curl_wrapper.hpp"
#include <nlohmann/json.hpp>

#include <utility>
#include <optional>

VK_GRAFFITI_BOT_BEGIN
struct vk_api_version {
    std::size_t major = 5;
    std::size_t minor = 131;

    [[nodiscard]] inline std::string to_string() const {
        return std::to_string(major) + '.' + std::to_string(minor);
    }
};

class method {
private:
    std::string _string;

public:
    inline method() noexcept = default;

    inline method(const std::string& name) :
        _string("method/" + name + '?') {}

    inline method(const std::string& name, const std::initializer_list<std::pair<std::string, std::string>>& params) :
        method(name) {
        for (const auto& param : params) {
            add_param(param);
        }
    }

    inline void add_param(const std::string name, const std::string& value) {
        _string += name + '=' + value + '&';
    }

    inline void add_param(const std::pair<std::string, std::string>& param) {
        add_param(param.first, param.second);
    }

    [[nodiscard]] inline const std::string& to_string() const noexcept {
        return _string;
    }
};

struct long_poll_server {
    std::string server;
    std::string key;
    std::string ts;
};

class message {
public:
    std::string text;
    std::string attachment;

    inline message() noexcept = default;

    inline message(const std::string& text, const std::string& attachment = "") :
        text(text),
        attachment(attachment) {}
};

class base_vk_api {
private:
    curl_wrapper& _curl;
    std::string _token;
    vk_api_version _version;

    [[nodiscard]] inline std::string _construct_url_from_method(const method& method) const {
        return "https://api.vk.com/" + method.to_string() + "access_token=" + _token + "&v=" + _version.to_string();
    }

public:
    inline base_vk_api(curl_wrapper& curl, const std::string& token, const vk_api_version& version = vk_api_version()) :
        _curl(curl),
        _token(token),
        _version(version) {}

    [[nodiscard]] inline curl_wrapper& curl() noexcept {
        return _curl;
    }

    [[nodiscard]] inline const std::string& get_token() const noexcept {
        return _token;
    }

    [[nodiscard]] inline const vk_api_version& get_version() const noexcept {
        return _version;
    }

    inline void set_token(const std::string& token) {
        _token = token;
    }

    inline void set_version(const vk_api_version& version) {
        _version = version;
    }

    inline nlohmann::json call_method(const method& method) {
        std::string answer_str;
        _curl.perform(_construct_url_from_method(method), answer_str);
        return nlohmann::json::parse(answer_str);
    }
};

class base_sub_vk_api {
private:
    base_vk_api& _api;

protected:
    [[nodiscard]] inline base_vk_api& api() noexcept {
        return _api;
    }

public:
    inline base_sub_vk_api(base_vk_api& api) noexcept :
        _api(api) {}
};

class groups : public base_sub_vk_api {
public:
    using base_sub_vk_api::base_sub_vk_api;

    [[nodiscard]] inline long_poll_server get_long_poll_server(const int group_id) {
        long_poll_server server;
        const method method("groups.getLongPollServer", {
            { "group_id", std::to_string(group_id) }
        });
        const auto answer = api().call_method(method);
        if (answer.contains("error")) {
            throw std::runtime_error(VK_GRAFFITI_BOT_FUNC_MSG(answer["error"]["error_msg"].get<std::string>()));
        }
        const auto response = answer["response"];
        server.server       = response["server"].get<std::string>();
        server.key          = response["key"].get<std::string>();
        server.ts           = response["ts"].get<std::string>();
        return server;
    }
};

class messages : public base_sub_vk_api {
public:
    using base_sub_vk_api::base_sub_vk_api;

    inline nlohmann::json send(const int user_id, const message& message) {
        if (message.text.empty() && message.attachment.empty()) {
            throw std::runtime_error(VK_GRAFFITI_BOT_FUNC_MSG(
                "you must specify at least one of the parameters when sending a message: text, attachment"));
        }

        method method("messages.send");
        method.add_param("user_id", std::to_string(user_id));
        method.add_param("random_id", "0");
        if (!message.text.empty()) {
            method.add_param("message", message.text);
        }
        if (!message.attachment.empty()) {
            method.add_param("attachment", message.attachment);
        }

        return api().call_method(method);
    }
};

class photos : public base_sub_vk_api {
public:
    struct messages_upload_server {
        std::string upload_url;
        int album_id = 0;
        int user_id  = 0;
        int group_id = 0;
    };

    using base_sub_vk_api::base_sub_vk_api;

    [[nodiscard]] inline messages_upload_server get_messages_upload_server(const int peer_id) {
        const method method("photos.getMessagesUploadServer", {
            { "peer_id", std::to_string(peer_id) }
        });
        const auto answer = api().call_method(method);
        if (answer.contains("error")) {
            throw std::runtime_error(VK_GRAFFITI_BOT_FUNC_MSG(answer["error"]["error_msg"].get<std::string>()));
        }

        const auto response = answer["response"];
        messages_upload_server server;
        server.upload_url = response["upload_url"];
        server.album_id   = response["album_id"];
        server.user_id    = response["user_id"];
        server.group_id   = response["group_id"];
        return server;
    }

    [[nodiscard]] inline nlohmann::json save_messages_photo(
        const std::string& photo, const int server, const std::string& hash) {
        const method method("photos.saveMessagesPhoto", {
            { "photo", photo },
            { "server", std::to_string(server) },
            { "hash", hash }
        });
        return api().call_method(method);
    }
};

class vk_api : public base_vk_api {
public:
    using base_vk_api::base_vk_api;

    [[nodiscard]] inline nlohmann::json connect_to_long_poll_server(
        const long_poll_server& server, const std::size_t wait) {
        std::string answer_str;
        const std::string request =
            server.server + "?act=a_check&key=" + server.key + "&ts=" + server.ts + "&wait=" + std::to_string(wait);
        curl().perform(request, answer_str);
        return nlohmann::json::parse(answer_str);
    }

    [[nodiscard]] inline vk_graffiti_bot::groups groups() {
        return vk_graffiti_bot::groups(*this);
    }

    [[nodiscard]] inline vk_graffiti_bot::messages messages() {
        return vk_graffiti_bot::messages(*this);
    }

    [[nodiscard]] inline vk_graffiti_bot::photos photos() {
        return vk_graffiti_bot::photos(*this);
    }
};
VK_GRAFFITI_BOT_END

#endif // !VK_GRAFFITI_BOT_VK_API_HPP