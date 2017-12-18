#ifndef INSTALLER_HH
#define INSTALLER_HH

#include <iso646.h>
#include <experimental/filesystem>
#include <memory>
#include <string>

#include "authfile.hh"
#include "constants.hh"
#include "util.hh"

namespace fs = std::experimental::filesystem;

/* Given an install directory, it will install necessary components. It also
 * initializes a recurring job to run this binary regularly either through
 * systemd or by scheduling a task */
class Installer {
   public:
    Installer(const fs::path& path);
    bool IsNew();
    std::string GetAuthHash();
    void InstallFiles();
    void InitRecurringJob();

   private:
    bool is_new_;
    fs::path install_dir_;
    std::unique_ptr<AuthFile> auth_;

    void InitAuthFile_();
};

#endif  // INSTALLER_HH
