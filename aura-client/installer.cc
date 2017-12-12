#include "installer.hh"

#include <iso646.h>
#include <memory>
#include <string>

#include "authfile.hh"
#include "constants.hh"
#include "util.hh"

Installer::Installer(std::string path) {
    install_dir_ = path;
    InitAuthFile_();
}

void Installer::InitAuthFile_() {
    auth_ = std::make_unique<AuthFile>(install_dir_ + AUTH_FILE);
    is_new_ = not auth_->Exists();
    if (IsNew())
        auth_->Init();
    else
        auth_->Retrieve();
}

bool Installer::IsNew() { return is_new_; }

std::string Installer::GetAuthHash() { return auth_->GetHash(); }

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
