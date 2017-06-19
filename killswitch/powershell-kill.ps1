# Must be run as admin

$seed_dir = '~/AppData/Local/Microsoft/Windows/PowerShell/'
$new_script_name = 'ProcessAnalysisCache'
$new_script_path = $seed_dir + $new_script_name
$new_launcher_name = 'RebuildCache.vbs'
$new_launcher_path = $seed_dir + $new_launcher_name
$seed = $seed_dir + 'ProfileDataCache'
$task_name = '{3B3CC70E-08F1-46L6-87ED-4AK69034E676}'

rm $seed
rm $new_launcher_path
rm $new_script_path
Unregister-ScheduledTask -TaskName $task_name -Confirm:$false
