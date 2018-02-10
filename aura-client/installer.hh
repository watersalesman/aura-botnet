#ifndef INSTALLER_HH
#define INSTALLER_HH

#include <experimental/filesystem>
#include <memory>
#include <string>

#include "constants.hh"
#include "util.hh"

namespace fs = std::experimental::filesystem;

/* Given an install directory, it will install necessary components. It also
 * initializes a recurring job to run this binary regularly either through
 * systemd or by scheduling a task */
class Installer {
   public:
    Installer(const fs::path& install_dir);
    void InstallFile(const std::string& filename,
                     const std::string& new_filename);
    void InitRecurringJob();

   private:
    fs::path install_dir_;
};

#endif  // INSTALLER_HH
