#!/usr/bin/python3

import sys, os, re, argparse, iso8601, shutil, time
from subprocess import call, check_output, Popen, CalledProcessError, STDOUT, PIPE
from xml.etree import ElementTree as ET
from traceback import print_exc
from glob import glob

releases = ['xenial', 'bionic', 'cosmic', 'disco']

parser = argparse.ArgumentParser()
parser.add_argument('-r', '--release', help='The Ubuntu release')
parser.add_argument('-s', '--source', help='Build signed source package', action="store_true")
parser.add_argument('-lb', '--bin', help='Build local binary package', action="store_true")
parser.add_argument('-u', '--upload', help='Send source package to PPA', action="store_true")
parser.add_argument('-b', '--sbuild', help='Build binary package with sbuild', action="store_true")
parser.add_argument('-p', '--prefix', help='Location of the build')


def run_cmd(cmd, do_print=True, **kwargs):
    p = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE, **kwargs)
    if do_print:
        lines_iterator = iter(p.stdout.readline, b"")
        for line in lines_iterator:
            print(line.strip())  # yield line
    else:
        p.wait()


def make_changelog(release, dest, rev):
    # ~ f = open(dest+"/debian/changelog", "r")
    # ~ tmp = f.read()
    # ~ f.close()

    pass


def copy_src(dest, rev, release):

    shutil.rmtree(dest, ignore_errors=True)
    shutil.copytree("..", "{0}".format(dest))
    shutil.rmtree('{}/examples'.format(dest))
    shutil.rmtree('{}/tests'.format(dest))

    r = check_output(["cd {0}; dh_make -yn --single -e fabien.poussin@gmail.com -c gpl3".format(dest)],
                 shell=True).decode("utf-8")
    print(r)

    shutil.copy("debian/control_{0}".format(release), "{0}/debian/control".format(dest))

    for i in ['copyright', 'README', 'rules']:
        shutil.copy("debian/{0}".format(i), "{0}/debian/{1}".format(dest, i))

    for f in glob("{0}/debian/*.ex".format(dest)):
        os.remove(f)

    with open(dest + "/debian/changelog", "r") as f:
        tmp = f.read().replace("unstable", args.release).replace("urgency=low", "urgency=medium")

    with open(dest + "/debian/changelog", "w") as f:
        f.write(str(tmp))

    with open(dest + "/version", "w") as f:
        f.write(str(rev))


def make_src(dest):
    print("Building Source package")
    run_cmd(["cd {0}; debuild -j4 -S -sa".format(dest)])


def make_s_build(dest):
    print("Building binary package (sbuild)")
    run_cmd(["sbuild -vd {0} -c {0}-amd64 -j4 {1}".format(args.release, dest)])


def make_bin(dest):
    print("Building binary package")
    run_cmd(["cd {0}; debuild -j4 -b -uc -us".format(dest)])


def send_src(ver):
    run_cmd(["dput ppa:fpoussin/ppa {0}_source.changes".format(ver)])


if __name__ == "__main__":

    args = parser.parse_args()
    now = time.strftime("%Y%m%d%H%M%S", time.gmtime())

    if not args.release:
        parser.print_help()

    if args.release not in releases:
        print("Invalid Ubuntu release, please chose among:", releases)
        exit(1)

    if not args.prefix:
        args.prefix = '/tmp'

    with open('../.qmake.conf') as f:
        for l in f.readlines():
            r = re.search(r'^MODULE_VERSION = (.+)', l)
            if r:
                ver = r.group(1)
                break

    if not ver:
        print("Could not read version from .qmake.conf")
        exit(1)

    print("Source Version:", ver)

    folder_name = '{0}/libqt5usb5-{1}~{2}~{3}'.format(args.prefix, ver, now, args.release)
    dsc_name = '{0}/libqt5usb5_{1}~{2}~{3}'.format(args.prefix, ver, now, args.release)

    print("Package location:", folder_name)

    copy_src(folder_name, ver, args.release)
    make_changelog(args.release, folder_name, ver)

    try:
        if args.source:
            make_src(folder_name)
            if args.ppa:
                send_src(dsc_name)
        if args.sbuild:
            if not args.source:  # Need to make sources first
                make_src(folder_name)
            make_s_build(folder_name)
        if args.bin:
            make_bin(folder_name)

    except CalledProcessError as e:
        print(e.output)
    except OSError as e:
        print(e.filename, e)
        print(print_exc())
    finally:
        pass
        #shutil.rmtree(folder_name)
        #check_output(["schroot -e --all-sessions"], shell=True)
