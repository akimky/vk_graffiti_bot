#ifndef VK_GRAFFITI_BOT_CURL_WRAPPER_HPP
#define VK_GRAFFITI_BOT_CURL_WRAPPER_HPP

#include "utils.hpp"

#include <curl/curl.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <filesystem>

#include <SFML/Graphics/Image.hpp>

VK_GRAFFITI_BOT_BEGIN
class curl_wrapper {
private:
    using _image_data_type     = std::vector<std::byte>;
    using _write_function_type = std::size_t(*)(const void*, const std::size_t, const std::size_t, void*);

    enum class _write_state {
        none,
        to_string,
        to_image
    };

    CURL* _handle = nullptr;
    _write_state _write_state_curr = _write_state::none;

    static constexpr void _check_code(const CURLcode code) {
        if (code != CURLE_OK) {
            throw std::runtime_error(VK_GRAFFITI_BOT_FUNC_MSG(curl_easy_strerror(code)));
        }
    }

    template <typename Container>
    static inline std::size_t _write_to_container(
        const void* data_src, const std::size_t size, const std::size_t count, void* data_dst) {
        if (!data_src || !data_dst) {
            throw std::invalid_argument(VK_GRAFFITI_BOT_FUNC_MSG("callback data was nullptr"));
        }
        const std::size_t bytes  = size * count;
        Container& container_dst = *static_cast<Container*>(data_dst);
        auto data_src_first = static_cast<const typename Container::value_type*>(data_src);
        auto data_src_last  = data_src_first + bytes;
        container_dst.insert(container_dst.end(), data_src_first, data_src_last);
        return bytes;
    }

    inline void _set_write_function(const _write_function_type& func) {
        const CURLcode code = curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION, func);
        _check_code(code);
    }

    inline void _set_write_data(void* data) {
        const CURLcode code = curl_easy_setopt(_handle, CURLOPT_WRITEDATA, data);
        _check_code(code);
    }

    inline void _set_write_state(const _write_state state) {
        if (_write_state_curr == state) {
            return;
        }

        switch (state) {
        case _write_state::to_string:
            _set_write_function(_write_to_container<std::string>);
        break;
        case _write_state::to_image:
            _set_write_function(_write_to_container<_image_data_type>);
        break;
        default:
            throw std::runtime_error(VK_GRAFFITI_BOT_FUNC_MSG("not correct write state"));
        }

        _write_state_curr = state;
    }

    inline void _set_http_get() {
        const CURLcode code = curl_easy_setopt(_handle, CURLOPT_HTTPGET, 1l);
        _check_code(code);
    }

    inline void _set_http_post(curl_httppost* form_post_first) {
        const CURLcode code = curl_easy_setopt(_handle, CURLOPT_HTTPPOST, form_post_first);
        _check_code(code);
    }

    inline void _perform(const std::string& url) {
        CURLcode code = curl_easy_setopt(_handle, CURLOPT_URL, url.c_str());
        _check_code(code);
        code = curl_easy_perform(_handle);
        _check_code(code);
    }

public:
    inline ~curl_wrapper() {
        if (_handle) {
            curl_easy_cleanup(_handle);
            _handle = nullptr;
        }
    }

    inline curl_wrapper() {
        _handle = curl_easy_init();
        if (!_handle) {
            throw std::runtime_error(VK_GRAFFITI_BOT_FUNC_MSG("init error"));
        }
    }

    inline void perform(const std::string& url, std::string& answer) {
        _set_write_state(_write_state::to_string);
        _set_write_data(static_cast<void*>(&answer));
        _perform(url);
    }

    inline void perform(const std::string& url, sf::Image& answer) {
        _set_write_state(_write_state::to_image);
        _image_data_type image_data;
        _set_write_data(static_cast<void*>(&image_data));
        _perform(url);
        const bool load_image_result =
            answer.loadFromMemory(static_cast<const void*>(image_data.data()), image_data.size());
        if (!load_image_result) {
            throw std::runtime_error(VK_GRAFFITI_BOT_FUNC_MSG("perform to image error"));
        }
    }

    inline void perform(const std::string& url, std::string& answer,
        const std::string& field_name, const std::filesystem::path& file_path, const bool reset_to_http_get = true) {
        if (!std::filesystem::exists(file_path)) {
            throw std::runtime_error(VK_GRAFFITI_BOT_FUNC_MSG("file does not exist"));
        }

        curl_httppost* form_post_first = nullptr;
        curl_httppost* form_post_last  = nullptr;
        curl_formadd(&form_post_first, &form_post_last, CURLFORM_COPYNAME,
            field_name.c_str(), CURLFORM_FILE, file_path.c_str(), CURLFORM_END);
        _set_http_post(form_post_first);
        perform(url, answer);
        if (reset_to_http_get) {
            _set_http_get();
        }
        curl_formfree(form_post_first);
    }

private:
    [[nodiscard]] inline std::string _coded_url_process(const char* result_c_str) {
        if (!result_c_str) {
            throw std::runtime_error(VK_GRAFFITI_BOT_FUNC_MSG("error of url code process"));
        }
        const std::string result(result_c_str);
        delete[] result_c_str;
        return result;
    }


public:
    [[nodiscard]] inline std::string encode_url(const std::string& url) {
        const char* result_c_str = curl_easy_escape(_handle, url.c_str(), url.length());
        return _coded_url_process(result_c_str);
    }

    [[nodiscard]] inline std::string decode_url(const std::string& url) {
        [[maybe_unused]] int decoded_str_length = 0;
        const auto result_c_str = curl_easy_unescape(_handle, url.c_str(), url.length(), &decoded_str_length);
        return _coded_url_process(result_c_str);
    }
};
VK_GRAFFITI_BOT_END

#endif // !VK_GRAFFITI_BOT_CURL_WRAPPER_HPP
