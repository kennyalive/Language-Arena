import subprocess

class CommandSession:
    def __init__(self):
        self.commands = ''

    def add_command(self, *args):
        if self.commands:
            self.commands += '&'
        self.commands += ' '.join(args)

    def run(self):
        if self.commands:
            subprocess.call(self.commands)
