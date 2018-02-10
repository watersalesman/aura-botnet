#include "installer.hh"

#include <experimental/filesystem>
#include <memory>
#include <string>

#include "constants.hh"
#include "util.hh"

namespace fs = std::experimental::filesystem;

Installer::Installer(const fs::path& install_dir) {
    install_dir_ = install_dir;
}

void Installer::InstallFile(const std::string& filename,
                            const std::string& new_filename) {
    if (!install_dir_.empty()) fs::create_directories(install_dir_);
    fs::copy_file(filename, install_dir_ / new_filename,
                  fs::copy_options::overwrite_existing);
}

#ifdef WIN32

void Installer::InitRecurringJob() {
    // Schedule task for Windows
    std::string task_command = "schtasks.exe /create /F /tn " + TASK_NAME +
                               " /sc " + TASK_FREQ + " /mo " + TASK_FREQ_VALUE +
                               " /tr " + (install_dir_ / BIN_NEW).string();
    task_command += util::IsSuperuser() ? " /rl highest" : "";

    util::PopenSubprocess(task_command);
}

#endif  // WIN32

#ifdef __linux__

void Installer::InitRecurringJob() {
    if (!install_dir_.empty()) fs::create_directories(install_dir_);

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

    // Copy files then enable and start systemd service
    fs::copy_file(original_service, service_path,
                  fs::copy_options::overwrite_existing);
    fs::copy_file(TIMER, timer_path, fs::copy_options::overwrite_existing);

    std::string systemd_command =
        util::IsSuperuser() ? "systemctl enable --now " + TIMER
                            : "systemctl enable --now --user " + TIMER;

    util::PopenSubprocess(systemd_command);
}

#endif  // __linux__
