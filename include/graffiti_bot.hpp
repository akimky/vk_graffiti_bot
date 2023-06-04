#ifndef VK_GRAFFITI_BOT_HPP
#define VK_GRAFFITI_BOT_HPP

#include "base_vk_bot.hpp"

#include <codecvt>
#include <utility>
#include <optional>
#include <cctype>

#include <SFML/Graphics.hpp>

VK_GRAFFITI_BOT_BEGIN
class graffiti_bot : public base_vk_bot {
private:
    sf::Text _text;
    sf::Font _font;
    std::filesystem::path _img_cach_path;
    float _default_character_size = 100;

    [[nodiscard]] static inline std::wstring string_to_wstring(const std::string& string) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(string);
    }

    [[nodiscard]] static inline std::string _max_size_photo_url(const nlohmann::json& sizes) {
        const auto max_it = std::max_element(sizes.begin(), sizes.end(), [](const auto& left, const auto& right) {
            const auto left_area  =
                left["width"].template get<std::size_t>() * left["height"].template get<std::size_t>();
            const auto right_area =
                right["width"].template get<std::size_t>() * right["height"].template get<std::size_t>();
            return left_area < right_area;
        });
        return (*max_it)["url"].get<std::string>();
    }

    [[nodiscard]] inline std::string _get_img_cach_attachment(const int peer_id) {
        // get server to load
        const auto upload_server = api().photos().get_messages_upload_server(peer_id);

        // load to server
        std::string answer;
        api().curl().perform(upload_server.upload_url, answer, "photo", _img_cach_path);

        // save on server
        nlohmann::json answer_json = nlohmann::json::parse(answer);
        answer_json = api().photos().save_messages_photo(answer_json["photo"].get<std::string>(),
            answer_json["server"].get<int>(), answer_json["hash"].get<std::string>());
        if (answer_json.contains("error")) {
            throw std::runtime_error(VK_GRAFFITI_BOT_FUNC_MSG(answer_json["error"]["error_msg"].get<std::string>()));
        }

        // construct attachment
        const auto answer_json_response_first = answer_json["response"][0];
        const auto owner_id = answer_json_response_first["owner_id"].get<int>();
        const auto photo_id = answer_json_response_first["id"].get<int>();
        return "photo" + std::to_string(owner_id) + '_' + std::to_string(photo_id);
    }

    struct _graffiti_info {
        std::string text;
        std::optional<float> character_size;
    };

    [[nodiscard]] static inline _graffiti_info _parse_text(const std::string& text) {
        _graffiti_info info;
        if (text.empty()) {
            return info;
        }

        std::string character_size_str;
        bool looking_for_digit = true;
        for (const auto ch : text) {
            if (looking_for_digit) {
                if (ch == ' ') {
                    continue;;
                }

                if (std::isdigit(ch)) {
                    character_size_str += ch;
                } else {
                    if (!character_size_str.empty()) {
                        info.character_size = std::make_optional(std::stoi(character_size_str));
                    }
                    info.text += ch;
                    looking_for_digit = false;
                }
            } else {
                info.text += ch;
            }
        }

        return info;
    }

    inline void _process_image(sf::Image& image, const _graffiti_info& info) {
        const sf::Vector2f image_size(image.getSize());
        sf::Texture texture;
        if (!texture.loadFromImage(image)) {
            throw std::runtime_error(VK_GRAFFITI_BOT_FUNC_MSG(
                "texture load from image error"));
        }
        sf::Sprite sprite(texture);
        sf::RenderTexture render_texture;
        render_texture.create(image_size.x, image_size.y);

        _text.setString(string_to_wstring(info.text));
        _text.setCharacterSize(*info.character_size);
        const auto text_local_bounds = _text.getLocalBounds();
        _text.setPosition(image_size.x / 2 - text_local_bounds.width / 2,
            (image_size.y - image_size.y / 3.5f) - text_local_bounds.height / 2);

        render_texture.draw(sprite);
        render_texture.draw(_text);
        render_texture.display();

        image = render_texture.getTexture().copyToImage();
    }

    inline void on_new_message(const int from_id, const message& message_recv) override {
        message message_answer;

        try {
            const auto attachment_recv = nlohmann::json::parse(message_recv.attachment);
            auto info                  = _parse_text(message_recv.text);
            if (info.text.empty() || attachment_recv.empty()) {
                message_answer.text = api().curl().encode_url("Error! No text or photo is specified.");
                api().messages().send(from_id, message_answer);
                return;
            }
            if (!info.character_size) {
                info.character_size = _default_character_size;
            }

            const std::string photo_recv_url = _max_size_photo_url(attachment_recv[0]["photo"]["sizes"]);
            sf::Image photo_recv;
            api().curl().perform(photo_recv_url, photo_recv);
            api().messages().send(from_id, api().curl().encode_url("Photo received! I'm starting work..."));
            _process_image(photo_recv, info);
            photo_recv.saveToFile(_img_cach_path);
            message_answer.attachment = _get_img_cach_attachment(from_id);
            if (!std::filesystem::remove(_img_cach_path)) {
                log_warning("img cach remove error");
            }
            message_answer.text       = api().curl().encode_url("Here is your photo!");
        } catch (const std::exception& ex) {
            message_answer.text = api().curl().encode_url(std::string("Server error: \"") + ex.what() + '\"');
            message_answer.attachment.clear();
        }

        try {
            api().messages().send(from_id, message_answer);
        } catch (const std::exception& ex) {
            log_error(ex.what());
        }
    }

public:
    inline graffiti_bot(vk_api& api, const int group_id) :
        base_vk_bot(api, group_id) {
        _text.setFont(_font);
        _text.setFillColor(sf::Color::White);
        _text.setOutlineThickness(1.5);
        _text.setOutlineColor(sf::Color::Black);
    }

    [[nodiscard]] inline const sf::Font& get_font() const noexcept {
        return _font;
    }

    [[nodiscard]] inline float get_default_charcter_size() const noexcept {
        return _default_character_size;
    }

    [[nodiscard]] inline const std::filesystem::path& get_img_cach_path() const noexcept {
        return _img_cach_path;
    }

    inline void set_font(const sf::Font& font) {
        _font = font;
    }

    inline void set_default_character_size(const float size) noexcept {
        _default_character_size = size;
    }

    inline void set_img_cach_path(const std::filesystem::path& path) {
        _img_cach_path = path;
    }
};
VK_GRAFFITI_BOT_END

#endif // !VK_GRAFFITI_BOT_HPP