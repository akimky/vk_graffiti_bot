# vk_graffiti_bot
<p align="center">
    <img src="git_rep/preview.gif" alt="preview" />
</p>

# What is it?
vk_graffiti_bot is a vk bot for rendering text on a photo, written in c++17 without using specialized libraries for working with the vk api.

# How to use?
Send the bot a message with the text you want to draw on the photo and the photo itself. In response, you will receive a processed photo.
If the size of the text does not suit you, you can explicitly specify it before the text. For example: "150 my text", where 150 is the size of the text.

# How to launch a bot for my group?
To launch a bot in your group, you need to follow a few steps.
- Сlone the repository to your computer.
If you use https.
```sh
git clone https://github.com/Akimpaus/vk_graffiti_bot.git
```
If you use ssh.
```sh
git clone git@github.com:Akimpaus/vk_graffiti_bot.git
```
- Install the necessary libraries.
It is necessary for CMake to be able to find libcurl and SFML graphics module.
The following is an example using a package manager.
on Linux:
```sh
sudo apt-get install libcurl4-openssl-dev
sudo apt-get install libsfml-dev
```
on Windows(via vcpkg).
```sh
vcpkg install curl
vcpkg install sfml
```
- Next, create a build folder and build the project. From the root directory.
```sh
mkdir build && cd build
cmake ..
cmake --build .
```
- Turn on the messages of the group.
- Enable the Long Poll API by specifying event types such as: incoming and outgoing messages.
- Create an access_token and grant it access to group management, group photos and messages.
- In the file located on the path vk_graffiti_bot/group_data/group_data.json specify your access_token and group_id.
- Now run your program. The bot is ready!
