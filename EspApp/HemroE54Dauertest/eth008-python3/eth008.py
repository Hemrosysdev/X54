from network_socket import NetworkSocket

eth008 = NetworkSocket()

#These will be the relays

def connect(ip, port, password):
    return eth008.connect_socket(ip, port, password)

def disconnect():
    eth008.close_socket()

def digital_command(command):

    parts = command.split(" ")     # Seperate the parts of the command
    message = None

    if parts[1] == "on":           # Are we turning the io on or off
        message = '\x20'
    else:
        message = '\x21'

    message += chr(int(parts[0]))  # Which io is it

    message += '\x00'              # 0 pulse time

    eth008.write(message)          # send command and read back responce byte
    eth008.read(1)

def get_states():
    command = '\x24'
    eth008.write(command)          # send command and read back responce byte
    states = eth008.read(1)
    print(states)
    print('Relay states 8->1 : ' + ''.join('{0:08b}'.format(x, 'b') for x in states))
