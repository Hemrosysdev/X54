from sys import version_info
from eth008 import *


exit = False

class cls(object):
    def __repr__(self):
        import os
        os.system('cls' if os.name == 'nt' else 'clear')
        return ''

cls = cls()  # Now printing cls will clear the screen

def set_exit(command):
    disconnect()
    global exit
    exit = 0

def help_text(command):
    print(cls)
    print("Invalid command entered.")
    print("To change state of digital port type number of port followed by on of off,")
    print("e.g. \'3 on\' to turn output 3 on.\n")
    print("To refresh the displayed states on the screen type\'states\'.\n")
    print("To exit type \'exit\'.\n")
    ip_address = input("Press enter to continue")

def no_action(comand):
    print(cls)

function_dict = {
    "1 on": digital_command,
    "1 off": digital_command,
    "2 on": digital_command,
    "2 off": digital_command,
    "3 on": digital_command,
    "3 off": digital_command,
    "4 on": digital_command,
    "4 off": digital_command,
    "5 on": digital_command,
    "5 off": digital_command,
    "6 on": digital_command,
    "6 off": digital_command,
    "7 on": digital_command,
    "7 off": digital_command,
    "8 on": digital_command,
    "8 off": digital_command,
    "states": no_action,
    "exit": set_exit}

print(cls)
print("Enter board details to connect.\nPassword can be left blank if no password is implemented\n")

ip_address = input("enter ip.. ")

port = input("enter port.. ")

password = input("enter password.. ")

exit = connect(ip_address, int(port), password)

while exit:
    print(cls)
    get_states()
    response = input(">>")

    function_dict.get(response, help_text)(response)

