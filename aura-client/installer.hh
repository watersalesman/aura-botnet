#ifndef INSTALLER_HH
#define INSTALLER_HH

#include <string>

#include "constants.hh"
#include "util.hh"

/* Given an install directory, it will install necessary components. It also
 * initializes a recurring job to run this binary regularly either through
 * systemd or by scheduling a task */
class Installer {
   public:
    Installer(std::string path) { install_dir_ = path; }
    void InstallFiles();
    void InitRecurringJob();

   private:
    std::string install_dir_;
};

#ifdef WIN32

void Installer::InstallFiles() {
    std::system(("mkdir " + install_dir_).c_str());
    util::CopyFile(BIN, install_dir_ + BIN_NEW);
}

void Installer::InitRecurringJob() {
    // Schedule task for Windows
    std::string task_command = "schtasks.exe /create /F /tn " + TASK_NAME +
                               " /sc " + TASK_FREQ + " /mo " + TASK_FREQ_VALUE +
                               " /tr " + install_dir_ + BIN_NEW;
    if (util::IsSuperuser()) {
        task_command += " /rl highest";
    }

    std::system(task_command.c_str());
}

#endif  // WIN32

#ifdef __linux__

#include <sys/stat.h>

void Installer::InstallFiles() {
    std::string mkdir_command, timer_path, bin_path;

    bin_path = install_dir_ + BIN_NEW;
    mkdir_command = "mkdir -p " + install_dir_ + " ";

    if (util::IsSuperuser()) {
        mkdir_command += SYS_SERVICE_DEST;
        timer_path = SYS_SERVICE_DEST + "/" + TIMER;

        // Change service name back to normal after moving if root
        util::CopyFile(SYS_SERVICE, SYS_SERVICE_DEST + "/" + SERVICE);
    } else {
        std::string home_dir = std::getenv("HOME");
        std::string user_service_dir = home_dir + "/" + SERVICE_DEST;

        mkdir_command += user_service_dir;
        timer_path = user_service_dir + "/" + TIMER;

        util::CopyFile(SERVICE, user_service_dir + "/" + SERVICE);
    }

    std::system(mkdir_command.c_str());
    util::CopyFile(BIN, bin_path);
    util::CopyFile(TIMER, timer_path);

    // Ensure that binary is executable for owner
    chmod(bin_path.c_str(), S_IRWXU);
}

void Installer::InitRecurringJob() {
    std::string systemd_command;

    if (util::IsSuperuser()) {
        systemd_command = "systemctl enable --now " + TIMER;
    } else {
        systemd_command = "systemctl enable --now --user " + TIMER;
    }

    std::system(systemd_command.c_str());
}

#endif  // __linux__

#endif  // INSTALLER_HH