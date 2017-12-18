#include "installer.hh"

#include <iso646.h>
#include <experimental/filesystem>
#include <memory>
#include <string>

#include "authfile.hh"
#include "constants.hh"
#include "util.hh"

namespace fs = std::experimental::filesystem;

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
    fs::create_directories(install_dir_);
    fs::copy_file(BIN, install_dir_ + BIN_NEW,
                  fs::copy_options::overwrite_existing);
}

void Installer::InitRecurringJob() {
    // Schedule task for Windows
    std::string task_command = "schtasks.exe /create /F /tn " + TASK_NAME +
                               " /sc " + TASK_FREQ + " /mo " + TASK_FREQ_VALUE +
                               " /tr " + install_dir_ + BIN_NEW;
    task_command += util::IsSuperuser() ? " /rl highest" : "";

    std::system(task_command.c_str());
}

#endif  // WIN32

#ifdef __linux__

void Installer::InstallFiles() {
    if (not install_dir_.empty()) fs::create_directories(install_dir_);

    fs::path timer_path = TIMER;
    fs::path service_path = SERVICE;
    std::string original_service = SERVICE;
    if (util::IsSuperuser()) {
        // Create directory for system systemd services
        fs::create_directories(SYS_SERVICE_DEST);

        // If root, change service name back to normal after copying
        original_service = SYS_SERVICE;
        service_path = SYS_SERVICE_DEST / service_path;
        timer_path = SYS_SERVICE_DEST / timer_path;
    } else {
        // Create directory for storing user systemd services
        fs::path user_service_dir = std::getenv("HOME");
        user_service_dir /= SERVICE_DEST;
        fs::create_directories(user_service_dir);

        service_path = user_service_dir / service_path;
        timer_path = user_service_dir / timer_path;
    }

    fs::copy_file(BIN, install_dir_ + BIN_NEW,
                  fs::copy_options::overwrite_existing);
    fs::copy_file(original_service, service_path,
                  fs::copy_options::overwrite_existing);
    fs::copy_file(TIMER, timer_path, fs::copy_options::overwrite_existing);
}

void Installer::InitRecurringJob() {
    std::string systemd_command =
        util::IsSuperuser()
            ? "systemctl enable --now " + TIMER
            : "systemctl enable --now --user " + TIMER;

    std::system(systemd_command.c_str());
}

#endif  // __linux__
