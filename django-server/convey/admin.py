from django.contrib import admin

from .models import Bot, Command

# Register your models here.
admin.site.register(Bot)
admin.site.register(Command)
