#!/usr/bin/python3.7
import os
import random
from subprocess import Popen, call, DEVNULL
import shutil
import string
import time

# Judge setup
print ('Judge setup')
cwd = os.getcwd()
server_dir = os.path.join(cwd, 'server_dir')
client_dir = os.path.join(cwd, 'client_dir')
fifo_dir = os.path.join(cwd, 'fifo_dir')
server_config = os.path.join(cwd, 'server.config')
client_config = os.path.join(cwd, 'client.config')
server_path = os.path.join(cwd, 'csie_box_server')
client_path = os.path.join(cwd, 'csie_box_client')

def exitWithCleanUp():
    #Judge clean up
    print('Judge clean up')
    try:
        call(['chmod', '-R', 'u+rw', server_dir], stderr = DEVNULL)
        call(['chmod', '-R', 'u+rw', client_dir], stderr = DEVNULL)
        call(['chmod', '-R', 'u+rw', fifo_dir], stderr = DEVNULL)
        shutil.rmtree(server_dir, ignore_errors = True)
        shutil.rmtree(client_dir, ignore_errors = True)
        shutil.rmtree(fifo_dir, ignore_errors = True)
        os.remove(server_config)
        os.remove(client_config)
    except:
        pass
    exit(0)

try:
    call(['chmod', '-R', 'u+rw', server_dir], stderr = DEVNULL)
    call(['chmod', '-R', 'u+rw', client_dir], stderr = DEVNULL)
    call(['chmod', '-R', 'u+rw', fifo_dir], stderr = DEVNULL)
    shutil.rmtree(server_dir, ignore_errors = True)
    shutil.rmtree(client_dir, ignore_errors = True)
    shutil.rmtree(fifo_dir, ignore_errors = True)
except:
    pass

with open(server_config, 'w') as f:
    f.write('fifo_path = '+fifo_dir+'\n')
    f.write('directory = '+server_dir+'\n')

with open(client_config, 'w') as f:
    f.write('fifo_path = '+fifo_dir+'\n')
    f.write('directory = '+client_dir+'\n')

os.mkdir(server_dir)
os.mkdir(client_dir)
os.mkdir(fifo_dir)

if os.path.exists(server_path):
    os.remove(server_path)
if os.path.exists(client_path):
    os.remove(client_path)
call(['make'])

if not os.path.exists(server_path) or not os.path.exists(client_path):
    print ('Cannot make executable files.')
    exitWithCleanUp()

grade = 0
# Judge 1
print ('Judge #1')
server = Popen([server_path, server_config])
time.sleep(1)
if not os.path.exists(os.path.join(fifo_dir, 'server_to_client.fifo')) or \
   not os.path.exists(os.path.join(fifo_dir, 'client_to_server.fifo')):
    print ('FIFO not exist')
else:
    print ('Judge #1 passed. (2pt)')
    grade += 2
server.send_signal(2)
time.sleep(1)

if server.poll() == None:
    print ('Server does not terminate after judge #1.')
    #exitWithCleanUp()

# Judge 2
print ('Judge #2')
client = Popen([client_path, client_config])
time.sleep(1)
mode = oct(os.stat(client_dir).st_mode)[-3:]
if mode == '000':
    print ('Judge #2 passed. (2pt)')
    grade += 2
else:
    print ('Premission setting of client_dir is wrong. Your mode is:', mode)

# Judge 3
print ('Judge #3')
os.makedirs(os.path.join(server_dir, 'A', 'B', 'C'))
os.makedirs(os.path.join(server_dir, 'A', 'D', 'E'))
def addFile(path, data):
    with open(path, 'w') as f:
        f.write(data)
    time.sleep(1)

def randomString(length):
    return ''.join(random.choice(string.ascii_uppercase) for _ in range(length))
addFile(os.path.join(server_dir, 'root.txt'), randomString(10))
addFile(os.path.join(server_dir, 'A', 'a.txt'), randomString(10))
addFile(os.path.join(server_dir, 'A', 'B', 'b.txt'), randomString(10))
addFile(os.path.join(server_dir, 'A', 'B', 'C', 'c.txt'), randomString(10))
addFile(os.path.join(server_dir, 'A', 'D', 'd.txt'), randomString(10))
addFile(os.path.join(server_dir, 'A', 'D', 'E', 'e.txt'), randomString(10))

server = Popen([server_path, server_config])
time.sleep(3)
if call(['diff', '-r', server_dir, client_dir], stdout = DEVNULL):
    print ('Dir does not sync.')
else:
    print ('Judge #3 passed. (2pt)')
    grade += 2

# Judge 4
print ('Judge #4')
error = False
try:
    addFile(os.path.join(server_dir, 'A', 'B', 'server.txt'), randomString(10))
    addFile(os.path.join(server_dir, 'A', 'B', 'C', 'c.txt'), randomString(10))
    os.remove(os.path.join(server_dir, 'A', 'B', 'b.txt'))
    time.sleep(1)
    os.symlink(os.path.join(server_dir, 'A', 'B', 'server.txt'),
               os.path.join(server_dir, 'A', 'B', 'server_s.txt'))
    time.sleep(10)
    os.makedirs(os.path.join(server_dir, 'A', 'B', 'C', 'S'))
    time.sleep(1)
    addFile(os.path.join(client_dir, 'A', 'D', 'client.txt'), randomString(10))
    addFile(os.path.join(client_dir, 'A', 'D', 'E', 'e.txt'), randomString(10))
    os.symlink(os.path.join(server_dir, 'A', 'D', 'client.txt'),
               os.path.join(server_dir, 'A', 'D', 'client_s.txt'))
    time.sleep(1)
    os.remove(os.path.join(client_dir, 'A', 'D', 'd.txt'))
    time.sleep(1)
    os.makedirs(os.path.join(server_dir, 'A', 'D', 'E', 'CC'))
    time.sleep(1)
except Exception as e:
    error = True
    print (e)
time.sleep(1)

if error or call(['diff', '-r', '--no-dereference', server_dir, client_dir], stdout = DEVNULL):
    print ('Dir does not sync.')
else:
    print ('Judge #4 passed. (2pt)')
    grade += 2


# Judge 5
print ('Judge #5')
server.send_signal(2)
time.sleep(1)
if client.poll() == None:
    print ('Judge #5 passed. (1pt)')
    grade += 1
else:
    print ('Client was terminated with exit code:', client.poll())

# Judge 6
print ('Judge #6')
server = Popen([server_path, server_config])
time.sleep(1)
error = False
try:
    addFile(os.path.join(server_dir, 'A', 'B', 'server2.txt'), randomString(10))
    addFile(os.path.join(server_dir, 'A', 'B', 'C', 'c.txt'), randomString(10))
    addFile(os.path.join(client_dir, 'A', 'D', 'client2.txt'), randomString(10))
    addFile(os.path.join(client_dir, 'A', 'D', 'E', 'e.txt'), randomString(10))
except Exception as e:
    error = True
    print (e)
time.sleep(1)

if error or call(['diff', '-r', '--no-dereference', server_dir, client_dir], stdout = DEVNULL):
    print ('Dir does not sync.')
else:
    print ('Judge #6 passed. (2pt)')
    grade += 2

# Judge 7
print ('Judge #7')
error = False
client.send_signal(2)
time.sleep(1)
if os.path.exists(client_dir) and len(os.listdir(client_dir)) != 0:
    print ('Client dir does not be removed.')
    error = True
if server.poll() != None:
    print ('Server was terminated with exit code:', server.poll())
    error = True

if not error:
    print ('Judge #7 passed. (2pt)')
    grade += 2


# Judge 8
print ('Judge #8')
client = Popen([client_path, client_config])
time.sleep(1)
if call(['diff', '-r', '--no-dereference', server_dir, client_dir], stdout = DEVNULL):
    print ('Dir does not sync.')
else:
    print ('Judge #8 passed. (1pt)')
    grade += 1

# Judge 9
print ('Judge #9')
client.send_signal(2)
time.sleep(1)
server.send_signal(2)
time.sleep(1)
if len(os.listdir(fifo_dir)) != 0:
    print ('Fifo dir is not empty.')
else:
    print ('Judge #9 passed. (1pt)')
    grade += 1

print ('Grade:', grade)
exitWithCleanUp()
