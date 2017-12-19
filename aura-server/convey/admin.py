from django.contrib import admin

from .models import Bot, Command, File

class FileInline(admin.StackedInline):
    model = File
    extra = 1

class CommandAdmin(admin.ModelAdmin):
    inlines = [
        FileInline,
    ]

# Register your models here.
admin.site.register(Bot)
admin.site.register(Command, CommandAdmin)
