#ifndef VK_GRAFFITI_BOT_BASE_VK_BOT_HPP
#define VK_GRAFFITI_BOT_BASE_VK_BOT_HPP

#include "vk_api.hpp"

VK_GRAFFITI_BOT_BEGIN
class base_vk_bot {
private:
    vk_api& _api;
    int _group_id;

    inline void _process_updates(const nlohmann::json& updates) {
        for (const auto& update : updates) {
            if (update["type"].get<std::string>() == "message_new") {
                const auto message_json = update["object"]["message"];
                const auto from_id      = message_json["from_id"].get<int>();
                const auto text         = message_json["text"].get<std::string>();
                const auto attachment   = message_json["attachments"].dump();
                on_new_message(from_id, { text, attachment });
            }
        }
    }

protected:
    [[nodiscard]] inline vk_api& api() noexcept {
        return _api;
    }

    virtual inline void on_new_message(const int from_id, const message& message_recv) {}

public:
    inline base_vk_bot(vk_api& api, const int group_id) noexcept :
        _api(api),
        _group_id(group_id) {}

    [[nodiscard]] inline int get_group_id() const noexcept {
        return _group_id;
    }

    inline void set_group_id(const int group_id) noexcept {
        _group_id = group_id;
    }

    inline void start(const std::size_t wait = 25) {
        auto groups = _api.groups();
        auto server = groups.get_long_poll_server(_group_id);
        while (true) {
            const auto answer = _api.connect_to_long_poll_server(server, wait);

            if (answer.contains("failed")) {
                const int code = answer["failed"];
                switch(code) {
                case 1:
                    server.ts = answer["ts"].get<std::string>();
                    continue;
                break;
                case 2:
                    server.key = groups.get_long_poll_server(_group_id).key;
                    continue;
                break;
                default:
                    server = groups.get_long_poll_server(_group_id);
                    continue;
                break;
                }
            }

            _process_updates(answer["updates"]);
            server.ts = answer["ts"].get<std::string>();
        }
    }
};
VK_GRAFFITI_BOT_END

#endif // !VK_GRAFFITI_BOT_BASE_VK_BOT_HPP