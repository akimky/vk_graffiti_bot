#ifndef VK_GRAFFITI_BOT_UTILS_HPP
#define VK_GRAFFITI_BOT_UTILS_HPP

#include <iostream>
#include <string>

#define VK_GRAFFITI_BOT_BEGIN namespace vk_graffiti_bot {
#define VK_GRAFFITI_BOT_END   }
#define VK_GRAFFITI_BOT       ::vk_graffiti_bot::

VK_GRAFFITI_BOT_BEGIN
namespace details {
inline void current_function_helper() noexcept {
#if defined(VK_GRAFFITI_BOT_DISABLE_CURRENT_FUNCTION)
# define VK_GRAFFITI_BOT_CURRENT_FUNCTION "(unknown)"
#elif defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) ||\
 defined(__ghs__) || defined(__clang__)
# define VK_GRAFFITI_BOT_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
# define VK_GRAFFITI_BOT_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
# define VK_GRAFFITI_BOT_CURRENT_FUNCTION __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
# define VK_GRAFFITI_BOT_CURRENT_FUNCTION __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
# define VK_GRAFFITI_BOT_CURRENT_FUNCTION __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
# define VK_GRAFFITI_BOT_CURRENT_FUNCTION __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
# define VK_GRAFFITI_BOT_CURRENT_FUNCTION __func__
#else
# define VK_GRAFFITI_BOT_CURRENT_FUNCTION "(unknown)"
#endif
}

[[nodiscard]] inline std::string dynamic_func_msg(const std::string& msg, const std::string& func) {
    constexpr const char func_prefix[] = "Function: ";
    constexpr const char msg_prefix[]  = "Msg: ";
    constexpr const char separator[]   = " | ";
    std::string result;
    result += func_prefix;
    result += func;
    result += separator;
    result += msg_prefix;
    result += msg;
    return result;
}

inline void log_base(const std::string& name, const std::string& msg) {
    std::cerr << name + ": \"" << msg << "\"" << std::endl; 
}
} // details

inline void log_warning(const std::string& msg) {
    details::log_base("Warning", msg);
}

inline void log_error(const std::string& msg) {
    details::log_base("Error", msg);
}
VK_GRAFFITI_BOT_END

#define VK_GRAFFITI_BOT_FUNC_MSG(message)\
 (VK_GRAFFITI_BOT details::dynamic_func_msg(message, VK_GRAFFITI_BOT_CURRENT_FUNCTION))

#endif // VK_GRAFFITI_BOT_UTILS_HPP