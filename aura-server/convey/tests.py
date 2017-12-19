import datetime
from django.test import TestCase
from django.utils import timezone
from convey.models import Bot, Command, File
import json


def create_bot(
        version='0.1.2',
        hash_type='sha256sum',
        hash_sum='testsum123',
        operating_sys='Test',
        ip_addr='8.8.8.8',
        user='tester',
        group=-5
):
    return Bot.objects.create(
        version=version,
        hash_type=hash_type,
        hash_sum=hash_sum,
        operating_sys=operating_sys,
        ip_addr=ip_addr,
        user=user,
        group=group
    )

def create_command(
        start_days=-5,
        end_days=5,
        shell="default",
        cmd_txt='Test',
        group_assigned=-2,
        hash_assigned=None
):
    start_time = timezone.now() + datetime.timedelta(days=start_days)
    end_time = timezone.now() + datetime.timedelta(days=end_days)
    return Command.objects.create(
        start_time=start_time,
        end_time=end_time,
        shell=shell,
        cmd_txt=cmd_txt,
        group_assigned=group_assigned,
        hash_assigned=hash_assigned
    )

class RegisterViewTest(TestCase):
    def test_linux_standard_registration(self):
        user = 'user'
        params = {
            'version': '0.1.2',
            'hash_type':'sha256sum',
            'hash_sum':'alskdjf;lji2laskdjfi',
            'operating_sys':'Linux',
            'user': user
        }
        response = self.client.post('/convey/register/', params)
        self.assertEqual(response.status_code, 200)
        bot = Bot.objects.filter(user=user)[0]
        self.assertNotEqual(bot.group, -5)

    def test_linux_root_registration(self):
        user = 'root'
        params = {
            'version': '0.1.2',
            'hash_type':'sha256sum',
            'hash_sum':'alskdjf;lji2laskdjfi',
            'operating_sys':'Linux',
            'user':user
        }
        response = self.client.post('/convey/register/', params)
        self.assertEqual(response.status_code, 200)
        bot = Bot.objects.filter(user=user)[0]
        self.assertNotEqual(bot.group, -5)

    def test_windows10_standard_registration(self):
        user = 'user'
        params = {
            'version': '0.1.2',
            'hash_type':'sha256sum',
            'hash_sum':'alskdjf;lji2laskdjfi',
            'operating_sys':'windows 10',
            'user':user
        }
        response = self.client.post('/convey/register/', params)
        self.assertEqual(response.status_code, 200)
        bot = Bot.objects.filter(user=user)[0]
        self.assertNotEqual(bot.group, -5)

    def test_windows10_admin_registration(self):
        user = 'user(admin)'
        params = {
            'version': '0.1.2',
            'hash_type':'sha256sum',
            'hash_sum':'alskdjf;lji2laskdjfi',
            'operating_sys':'windows 10',
            'user': user
        }
        response = self.client.post('/convey/register/', params)
        self.assertEqual(response.status_code, 200)
        bot = Bot.objects.filter(user=user)[0]
        self.assertNotEqual(bot.group, -5)

    def test_no_version_registration(self):
        user = 'user(admin)'
        params = {
            'hash_type':'sha256sum',
            'hash_sum':'alskdjf;lji2laskdjfi',
            'operating_sys':'windows 10',
            'user': user
        }
        response = self.client.post('/convey/register/', params)
        self.assertEqual(response.status_code, 200)
        bot = Bot.objects.filter(user=user)[0]
        self.assertEqual(bot.version, None)

class CmdViewTests(TestCase):
    def test_unauthorized_bot_post(self):
        cmd = create_command()
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum':'fake'}
        )
        self.assertEqual(response.status_code, 404)

    def test_authorized_bot_post(self):
        bot = create_bot()
        cmd = create_command()
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum}
        )
        self.assertEqual(response.status_code, 200)

    def test_authorized_root_bot_post(self):
        bot = create_bot(group=0)
        cmd = create_command(group_assigned=-2)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum}
        )
        self.assertEqual(response.status_code, 200)

    def test_early_cmd_request(self):
        bot = create_bot()
        create_command(5, 10)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum}
        )
        self.assertEqual(response.status_code, 404)

    def test_late_cmd_request(self):
        bot = create_bot()
        create_command(-10, -5)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum}
        )
        self.assertEqual(response.status_code, 404)

    def test_command_prioritizing_all(self):
        create_command(-1, 1, group_assigned=-1, cmd_txt='ALL Command')
        create_command(-1, 1, group_assigned=-2, cmd_txt='Default')
        create_command(-1, 1, group_assigned=5, cmd_txt='Group 5')
        bot = create_bot(group=5)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum}
        )
        self.assertContains(response, 'ALL Command')

    def test_command_prioritizing_individual(self):
        create_command(-1, 1, hash_assigned='test', cmd_txt='Individual')
        create_command(-1, 1, group_assigned=-2, cmd_txt='Default')
        create_command(-1, 1, group_assigned=5, cmd_txt='Group 5')
        bot = create_bot(hash_sum='test', group=5)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum}
        )
        self.assertContains(response, 'Individual')

    def test_command_group_prioritizing_group_assigned(self):
        create_command(-1, 1, group_assigned=-2, cmd_txt='Default')
        create_command(-1, 1, group_assigned=5, cmd_txt='Group 5')
        bot = create_bot(group=5)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum}
        )
        self.assertContains(response, 'Group 5')

    def test_oneshot_command(self):
        create_command(group_assigned=-2, cmd_txt='Default')
        bot = create_bot(group=5)
        self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum}
        )
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum}
        )
        self.assertEqual(response.status_code, 404)

    def test_version_is_updated(self):
        user = "test";
        bot = create_bot()
        cmd = create_command()
        new_version = "UpdatedVersion"
        response = self.client.post(
            '/convey/cmd/',
            {
                'version': new_version,
                'hash_sum': bot.hash_sum,
            }
        )
        self.assertEqual(response.status_code, 200)
        bot = Bot.objects.all()[0]
        self.assertEqual(bot.version, new_version)

    def test_json_if_version_not_none(self):
        bot = create_bot()
        cmd = create_command()
        response = self.client.post(
            '/convey/cmd/',
            {
                'version': bot.version,
                'hash_sum': bot.hash_sum,
            }
        )
        command_json = json.loads(response.content.decode('utf-8'))
        self.assertEqual(command_json['command_text'], cmd.cmd_txt)
        self.assertEqual(command_json['shell'], cmd.shell)


    def test_json_if_version_is_none(self):
        bot = create_bot()
        cmd = create_command()
        response = self.client.post(
            '/convey/cmd/',
            {
                'hash_sum': bot.hash_sum,
            }
        )
        self.assertEqual(response.content.decode('utf-8'), cmd.cmd_txt)

    def test_json_has_empty_file_deps(self):
        bot = create_bot()
        cmd = create_command()
        response = self.client.post(
            '/convey/cmd/',
            {
                'version': bot.version,
                'hash_sum': bot.hash_sum,
            }
        )
        command_json = json.loads(response.content.decode('utf-8'))
        self.assertEqual(command_json['files'], [])

    def test_json_has_file_deps(self):
        bot = create_bot()
        cmd = create_command()
        cmd.file_set.bulk_create([
            File(name="file1",file_type="local",path="/etc/test",command_id=cmd.id),
            File(name="file2",file_type="network", path="https://google.com",command_id=cmd.id),
            File(name="file3",file_type="local", path="/etc/test2",command_id=cmd.id),
        ])
        response = self.client.post(
            '/convey/cmd/',
            {
                'version': bot.version,
                'hash_sum': bot.hash_sum,
            }
        )
        command_json = json.loads(response.content.decode('utf-8'))
        self.assertNotEqual(command_json['files'], [])
        self.assertEqual(len(command_json['files']), 3)
        dep_data = [
                {'name': 'file1', 'type': 'local', 'path': '/etc/test'},
                {'name': 'file2', 'type': 'network', 'path': 'https://google.com'},
                {'name': 'file3', 'type': 'local', 'path': '/etc/test2'},
        ]
        self.assertEqual(dep_data, command_json['files'])
