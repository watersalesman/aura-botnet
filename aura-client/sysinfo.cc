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

#ifdef __linux__

void OSInfo::Collect() { data_ = util::PopenSubprocess("uname | tr -d '\n'"); }

void UserInfo::Collect() {
    data_ = util::IsSuperuser() ? "root" : std::getenv("USER");
}

#endif

#ifdef WIN32

void OSInfo::Collect() {
    std::string win_version =
        util::PopenSubprocess("systeminfo | findstr /B /C:\"OS Name\"");
    std::regex pattern("[\\n\\r\\s]*.*?(Windows\\s*\\S+).*[\\n\\r\\s]*");
    std::smatch match;
    std::regex_match(win_version, match, pattern);

    data_ = match[1];
}

void UserInfo::Collect() {
    data_ =
        std::getenv("USERNAME") + (std::string) "." + std::getenv("USERDOMAIN");
    if (util::IsSuperuser()) data_ += "(admin)";
}

#endif

DataList::DataList(const std::string& auth_hash) {
    auth_hash_ = auth_hash;
    list_.push_back(std::make_unique<VersionInfo>());
    list_.push_back(std::make_unique<HashTypeInfo>());
    list_.push_back(std::make_unique<OSInfo>());
    list_.push_back(std::make_unique<UserInfo>());
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
