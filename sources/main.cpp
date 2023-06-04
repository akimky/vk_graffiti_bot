#include <iostream>
#include <fstream>
#include "graffiti_bot.hpp"

using namespace vk_graffiti_bot;

int main() {
    try {
        nlohmann::json group_data;
        std::ifstream file("../group_data/group_data.json");
        if (!file.is_open()) {
            throw std::runtime_error(VK_GRAFFITI_BOT_FUNC_MSG("load group data error"));
        }
        file >> group_data;
        file.close();

        const std::string access_token = group_data["access_token"];
        const int group_id             = group_data["group_id"];

        curl_wrapper curl;
        vk_api api(curl, access_token);
        graffiti_bot bot(api, group_id);
        sf::Font font;
        if (!font.loadFromFile("../fonts/ImpactRegular.ttf")) {
            throw std::runtime_error(VK_GRAFFITI_BOT_FUNC_MSG("load font error"));
        }
        bot.set_font(font);
        bot.set_img_cach_path("img.bmp");

        std::cout << "Bot started." << std::endl;
        bot.start();
    } catch (const std::exception& ex) {
        log_error(ex.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}