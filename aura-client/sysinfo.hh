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
    virtual ~DataModule() = default;
    std::pair<std::string, std::string> Get();

   protected:
    std::string field_name_;
    std::string data_;

    virtual void Collect() = 0;
};

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

// Holds pointers to DataModule objects and creates POST form from them
class DataList {
   public:
    DataList(const std::string& auth_hash);
    std::string GetPostData();

   private:
    std::string auth_hash_;
    std::vector<std::unique_ptr<DataModule>> list_;
};

}  // namespace sysinfo

#endif  // SYSINFO_HH
