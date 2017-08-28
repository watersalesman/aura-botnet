import datetime
from django.test import TestCase
from django.utils import timezone

from convey.models import Bot, Command


def create_bot(
    hash_type='sha256sum',
    hash_sum='testsum123',
    operating_sys='Test',
    ip_addr='8.8.8.8',
    user='tester',
    group=-5
):
    return Bot.objects.create(
        hash_type=hash_type,
        hash_sum=hash_sum,
        operating_sys=operating_sys,
        ip_addr=ip_addr,
        user=user,
        group=group
    )

def create_command(
    start_days,
    end_days,
    cmd_txt='Test',
    group_assigned=-2,
    hash_assigned=None
):
    start_time = timezone.now() + datetime.timedelta(days=start_days)
    end_time = timezone.now() + datetime.timedelta(days=end_days)
    return Command.objects.create(
        start_time=start_time,
        end_time=end_time,
        cmd_txt=cmd_txt,
        group_assigned=group_assigned,
        hash_assigned=hash_assigned
    )

class RegisterViewTest(TestCase):
    def test_linux_standard_registration(self):
        user = 'user'
        params = {
            'hash_type':'sha256sum',
            'hash_sum':'alskdjf;lji2laskdjfi',
            'operating_sys':'Linux',
            'ip_addr':'8.8.8.8',
            'user': user
        }
        response = self.client.post('/convey/register/', params)
        self.assertEqual(response.status_code, 200)
        bot = Bot.objects.filter(user=user)[0]
        self.assertNotEqual(bot.group, -5)

    def test_linux_root_registration(self):
        user = 'root'
        params = {
            'hash_type':'sha256sum',
            'hash_sum':'alskdjf;lji2laskdjfi',
            'operating_sys':'Linux',
            'ip_addr':'8.8.8.8',
            'user':user
        }
        response = self.client.post('/convey/register/', params)
        self.assertEqual(response.status_code, 200)
        bot = Bot.objects.filter(user=user)[0]
        self.assertNotEqual(bot.group, -5)

    def test_windows10_standard_registration(self):
        user = 'user'
        params = {
            'hash_type':'sha256sum',
            'hash_sum':'alskdjf;lji2laskdjfi',
            'operating_sys':'windows 10',
            'ip_addr':'8.8.8.8',
            'user':user
        }
        response = self.client.post('/convey/register/', params)
        self.assertEqual(response.status_code, 200)
        bot = Bot.objects.filter(user=user)[0]
        self.assertNotEqual(bot.group, -5)

    def test_windows10_admin_registration(self):
        user = 'user(admin)'
        params = {
            'hash_type':'sha256sum',
            'hash_sum':'alskdjf;lji2laskdjfi',
            'operating_sys':'windows 10',
            'ip_addr':'8.8.8.8',
            'user': user
        }
        response = self.client.post('/convey/register/', params)
        self.assertEqual(response.status_code, 200)
        bot = Bot.objects.filter(user=user)[0]
        self.assertNotEqual(bot.group, -5)

class CmdViewTests(TestCase):
    def test_unauthorized_bot_post(self):
        cmd = create_command(-5, 5)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum':'fake', 'ip_addr':'fake'}
        )
        self.assertEqual(response.status_code, 404)

    def test_authorized_bot_post(self):
        bot = create_bot()
        cmd = create_command(-5, 5)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum, 'ip_addr': bot.ip_addr}
        )
        self.assertEqual(response.status_code, 200)

    def test_authorized_root_bot_post(self): #TODO
        bot = create_bot(group=0)
        cmd = create_command(-5, 5, group_assigned=-2)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum, 'ip_addr': bot.ip_addr}
        )
        self.assertEqual(response.status_code, 200)

    def test_early_cmd_request(self):
        bot = create_bot()
        create_command(5, 10)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum, 'ip_addr': bot.ip_addr}
        )
        self.assertEqual(response.status_code, 404)

    def test_late_cmd_request(self):
        bot = create_bot()
        create_command(-10, -5)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum, 'ip_addr': bot.ip_addr}
        )
        self.assertEqual(response.status_code, 404)

    def test_command_prioritizing_all(self):
        create_command(-1, 1, group_assigned=-1, cmd_txt='ALL Command')
        create_command(-1, 1, group_assigned=-2, cmd_txt='Default')
        create_command(-1, 1, group_assigned=5, cmd_txt='Group 5')
        bot = create_bot(group=5)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum, 'ip_addr': bot.ip_addr}
        )
        self.assertContains(response, 'ALL Command')

    def test_command_prioritizing_individual(self):
        create_command(-1, 1, hash_assigned='test', cmd_txt='Individual')
        create_command(-1, 1, group_assigned=-2, cmd_txt='Default')
        create_command(-1, 1, group_assigned=5, cmd_txt='Group 5')
        bot = create_bot(hash_sum='test', group=5)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum, 'ip_addr': bot.ip_addr}
        )
        self.assertContains(response, 'Individual')

    def test_command_group_prioritizing_group_assigned(self):
        create_command(-1, 1, group_assigned=-2, cmd_txt='Default')
        create_command(-1, 1, group_assigned=5, cmd_txt='Group 5')
        bot = create_bot(group=5)
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum, 'ip_addr': bot.ip_addr}
        )
        self.assertContains(response, 'Group 5')

    def test_oneshot_command(self):
        create_command(-1, 1, group_assigned=-2, cmd_txt='Default')
        bot = create_bot(group=5)
        self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum, 'ip_addr': bot.ip_addr}
        )
        response = self.client.post(
            '/convey/cmd/',
            {'hash_sum': bot.hash_sum, 'ip_addr': bot.ip_addr}
        )
        self.assertEqual(response.status_code, 404)
