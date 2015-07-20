#!/usr/bin/python

import sys, os, re, argparse, iso8601, shutil
from subprocess import call, check_output, Popen, CalledProcessError, STDOUT, PIPE
from xml.etree import ElementTree as ET
from traceback import print_exc

releases = ["trusty", "vivid", "utopic"]

parser = argparse.ArgumentParser()
parser.add_argument('-r', '--release', help='The Ubuntu release')
parser.add_argument('-s', '--source', help='Build signed source package', action="store_true")
parser.add_argument('-lb', '--bin', help='Build local binary package', action="store_true")
parser.add_argument('-p', '--ppa', help='Send source package to PPA', action="store_true")
parser.add_argument('-b', '--sbuild', help='Build binary package with sbuild', action="store_true")


def run_cmd(cmd, do_print=True, **kwargs):
    p = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE, **kwargs)
    if do_print:
        lines_iterator = iter(p.stdout.readline, b"")
        for line in lines_iterator:
            print(line.strip())  # yield line
    else:
        p.wait()

def makeChangelog(release, dest, rev):
    # ~ f = open(dest+"/debian/changelog", "r")
    # ~ tmp = f.read()
    # ~ f.close()

    pass


def copySrc(dest, rev):
    check_output(["rm -rf " + dest], shell=True)
    print check_output(["mkdir -v " + dest], shell=True)

    # ~ print check_output(["shopt -s extglob; cp -r ../!(package) "+dest+"/"], shell=True)
    cmd = "find .. -maxdepth 1 \! \( -name package -o -name *.pro.user* -o -name .. \) -exec cp -vr '{}' " + dest + "/ ';'"
    print check_output([cmd], shell=True)

    check_output(["cd {0}; echo '' | dh_make -n --single -e fabien.poussin@gmail.com -c gpl3".format(dest)],
                       shell=True)

    print check_output(["cp -v debian/* {0}/debian/".format(dest)], shell=True)
    check_output(["rm -f {0}/debian/*.ex {0}/debian/*.EX {0}/debian/ex.*".format(dest)], shell=True)

    f = open(dest + "/debian/changelog", "r")
    tmp = f.read().replace("unstable; urgency=low", args.release + "; urgency=medium")
    f.close()
    f = open(dest + "/debian/changelog", "w")
    f.write(str(tmp))
    f.close()

    f = open(dest + "/version", "w")
    f.write(str(rev))
    f.close()


def makeSrc(dest):
    print "Building Source package"
    run_cmd(["cd {0}; debuild -j4 -S -sa".format(dest)])

def makeSBuild(dest):
    print "Building binary package (sbuild)"
    run_cmd(["sbuild -vd {0} {1}".format(args.release, dest)])

def makeBin(dest):
    print "Building binary package"
    run_cmd(["cd {0}; debuild -j4 -b -uc -us".format(dest)])

def sendSrc(ver):
    run_cmd(["dput ppa:fpoussin/ppa {0}_source.changes".format(ver)])


if __name__ == "__main__":

    args = parser.parse_args()

    if not args.release:
        parser.print_help()

    if args.release not in releases:
        print "Invalid Ubuntu release, please chose among:", releases
        exit(1)

    ver = check_output(["grep \"VERSION =\" ../QtUsb.pro | awk '{ print $3 }' "], shell=True).replace('\n', '')
    if not ver:
        print "Could not fetch last version"
        exit(1)

    print "Last version:", ver

    folder_name = 'libqt5usb5-{0}~{1}'.format(ver, args.release)

    print "Package version:", folder_name

    copySrc(folder_name, ver)
    makeChangelog(args.release, folder_name, ver)

    try:
        if args.source:
            makeSrc(folder_name)
            if args.ppa:
                sendSrc(folder_name)
        if args.sbuild:
            if not args.source:  # Need to make sources first
                makeSrc(folder_name)
            makeSBuild(folder_name)
        if args.bin:
            makeBin(folder_name)

        print check_output(["rm -rf " + folder_name], shell=True)
    except CalledProcessError as e:
        print e.output
    except OSError as e:
        print e.filename, e
        print print_exc()
