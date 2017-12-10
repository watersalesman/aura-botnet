#include "sysinfo.hh"

#include <memory>
#include <regex>
#include <string>
#include <vector>

#include "constants.hh"
#include "request.hh"
#include "util.hh"

namespace sysinfo {

std::pair<std::string, std::string> DataModule::Get() {
    if (data_.empty()) Collect();
    return std::make_pair(field_name_, data_);
}

void VersionInfo::Collect() { data_ = AURA_VERSION; }

void HashTypeInfo::Collect() { data_ = HASH_TYPE; }

void OSInfo::Collect() {
#ifdef __linux__

    data_ = util::PopenSubprocess("uname | tr -d '\n'");

#endif

#ifdef WIN32

    std::string win_version =
        util::PopenSubprocess("systeminfo | findstr /B /C:\"OS Name\"");
    std::regex pattern("[\\n\\r\\s]*.*?(Windows\\s*\\S+).*[\\n\\r\\s]*");
    std::smatch match;
    std::regex_match(win_version, match, pattern);

    data_ = match[1];

#endif
}

void UserInfo::Collect() {
#ifdef __linux__

    data_ = util::IsSuperuser() ? "root" : std::getenv("USER");

#endif

#ifdef WIN32

    data_ = util::IsSuperuser()
                ? ADMIN_INSTALL_DIR + "\\"
                : std::getenv("USERPROFILE") + ("\\" + INSTALL_DIR + "\\");

#endif
}

DataList::DataList(std::string auth_hash) {
    auth_hash_ = auth_hash;
    list_.emplace_back(std::move(std::make_unique<VersionInfo>()));
    list_.emplace_back(std::move(std::make_unique<HashTypeInfo>()));
    list_.emplace_back(std::move(std::make_unique<OSInfo>()));
    list_.emplace_back(std::move(std::make_unique<UserInfo>()));
}

// Use request::PostForm to convert DataModules to POST data
std::string DataList::GetPostData() {
    request::PostForm post_form;
    std::string field, value;

    for (auto& module : list_) {
        std::tie(field, value) = module->Get();
        post_form.AddField(field, value);
    }

    post_form.AddField("hash_sum", auth_hash_);

    return post_form.ToString();
}

}  // namespace sysinfo