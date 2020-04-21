import subprocess as sp
import time
import os
import signal
import sys

def start_proc(cmd):

    print(cmd)
    return sp.Popen(cmd, stdout=sp.PIPE, stderr=sp.PIPE, stdin=sp.PIPE,
                        shell=True, preexec_fn=os.setsid)

def stop_proc(proc):

    # kill proc
    os.killpg(os.getpgid(proc.pid), signal.SIGTERM)

    # get output
    out, err = proc.communicate()
    out = out.decode('utf-8')
    err = err.decode('utf-8')

    if 'Error' not in out:
        out = '\n'.join([line for line in out.splitlines() if 'EAL' not in line])
        err = '\n'.join([line for line in err.splitlines() if 'EAL' not in line])

    return out, err

def run():

    # server  = always on core 0
    # numbers = cores to run things on
    setup = [[1, 2],
             [1, 2],
             [1, 2],
             [1, 2],
             [1, 2],
             [1, 2]]

    # prepare arguments
    server_args = ' '.join([str(len(x)) for x in setup])

    # start server
    server = start_proc("./build/server -l 0 -n 1 -- {}".format(server_args))

    time.sleep(1)

    # EAL lcore number, every program must have different. 0 - server, 1 - logger, >=2 apps
    unique_id = 2

    # run apps
    apps = []
    for i, chain in enumerate(setup):
        apps.append([])
        for j, core in enumerate(chain):
            apps[-1].append(
                start_proc("./build/app -n 1 --lcores={}@{} --proc-type=auto {} {}"
                    .format(unique_id, core, i, j))
            )
            unique_id += 1

    input("Press Enter to stop...")

    # stop apps
    for i, chain in enumerate(apps):
        for j, app in enumerate(chain):
            print("APP [{}, {}]".format(i, j))
            out, err = stop_proc(app)
            print(out + "\n")
            print(err + "\n")


    # stop server
    print("SERVER")
    out, err = stop_proc(server)
    print(out + "\n")
    print(err + "\n")


if __name__ == '__main__':
    run()