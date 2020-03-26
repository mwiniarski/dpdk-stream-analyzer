import subprocess as sp
import time
import os
import signal

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

    if 'Error' not in out:
        out = '\n'.join([line for line in out.splitlines() if 'EAL' not in line])

    return out

def run():

    # server  = always on core 0
    # numbers = cores to run things on
    setup = [[1, 2, 3, 4, 5, 6],
             [1, 2, 3, 4],
             [1, 2]             ]

    # prepare arguments
    server_args = ' '.join([str(len(x)) for x in setup])

    # start server
    server = start_proc("./build/server -l 0 -n 1 -- {}".format(server_args))

    time.sleep(1)

    # run apps
    apps = []
    for i, chain in enumerate(setup):
        apps.append([])
        for j, core in enumerate(chain):
            apps[-1].append(
                start_proc("./build/app -n 1 -l {} --proc-type=auto {} {}"
                    .format(core, i, j))
            )


    # === run for some time
    time.sleep(5)
    # ===

    # stop apps
    for i, chain in enumerate(apps):
        for j, app in enumerate(chain):
            print("APP [{}, {}]".format(i, j))
            print(stop_proc(app) + "\n")

    # stop server
    print("SERVER")
    print(stop_proc(server) + "\n")


if __name__ == '__main__':
    run()