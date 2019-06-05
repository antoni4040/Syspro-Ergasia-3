import subprocess
import os
import sys
import shutil
import random
import time
import signal
import socket

processes = []


def signal_handler(sig, frame):
    try:
        for proc in processes:
            proc.kill()
    except:
        pass
    sys.exit(0)

def find_port():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(("",0))
    s.listen(1)
    port = s.getsockname()[1]
    s.close()
    return port



# Deletions
# try:
#     shutil.rmtree("common")
# except:
#     pass
# try:
#     shutil.rmtree("logs")
# except:
#     pass
# for i in os.listdir("."):
#     if os.path.isdir(os.path.join(".", i)):
#         if i.endswith("mirror") or i.endswith("input"):
#             shutil.rmtree(i)


# Run with a single delete parameter to simply delete testing folders:
if sys.argv[1] == "delete":
    sys.exit()

# Get server and client executable names from the command line:
server_executable = sys.argv[1]
client_executable = sys.argv[2]

# Get number of clients to create from the command line:
numOfClients = int(sys.argv[3])

signal.signal(signal.SIGTSTP, signal_handler)

# Get free port:
port = find_port()

# Create server:
processes.append(subprocess.Popen(
    [
        "./" +
        server_executable, "-p", str(port)
    ])
)

ID = 1
for i in range(numOfClients):
    # inputFileName = str(ID) + "_input"
    # num_of_files = random.randint(10, 30)
    # num_of_dirs = random.randint(5, 10)
    # levels = random.randint(2, 4)
    # print("./create_infiles.sh {} {} {} {}".format(
    #     inputFileName, num_of_files, num_of_dirs, levels))
    # subprocess.check_call(
        # [
        #     './create_infiles.sh', inputFileName, str(num_of_files),
        #     str(num_of_dirs), str(levels)])
    newPort = find_port()
    processes.append(subprocess.Popen(
        [
            "./" +
            client_executable, "-d", "./dir" + str(ID), "-w", "3", "-b", "100", "-p", str(newPort), "-sp", str(port), "-sip",
            "172.16.121.1"
        ])
    )
    time.sleep(4)
    ID += 1

for i in processes:
    i.wait()

