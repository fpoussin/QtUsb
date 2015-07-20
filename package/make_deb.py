#!/usr/bin/python

import sys, os, re, argparse, iso8601, shutil
from subprocess import call, check_output, Popen, CalledProcessError
from xml.etree import ElementTree as ET

releases = ["trusty", "vivid", "utopic"]

parser = argparse.ArgumentParser()
parser.add_argument('-r', '--release', help='The Ubuntu release')
parser.add_argument('-s', '--source', help='Build signed source package', action="store_true")
parser.add_argument('-b', '--bin', help='Build local binary package', action="store_true")
parser.add_argument('-p', '--ppa', help='Send source package to PPA', action="store_true")


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

    check_output(["cd " + dest + "; echo "" | dh_make -n --single -e fabien.poussin@gmail.com -c gpl3"],
                       shell=True)

    print check_output(["cp -v debian/* " + dest + "/debian/"], shell=True)
    check_output(["rm -f " + dest + "/debian/*.ex " + dest + "/debian/*.EX " + dest + "/debian/ex.*"], shell=True)

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
    print check_output(["cd " + dest + "; debuild -j4 -S -sa"], shell=True)


def makeBin(dest):
    print "Building binary package"
    print check_output(["cd " + dest + "; debuild -j4 -b -uc -us"], shell=True)


def sendSrc(ver):
    print check_output(["dput ppa:fpoussin/ppa " + ver + "_source.changes"], shell=True)


if __name__ == "__main__":

    args = parser.parse_args()

    if not args.release:
        parser.print_help()

    if args.release not in releases:
        print "Invalid Ubuntu release, please chose among:", releases
        sys.exit(1)

    ver = check_output(["grep \"VERSION =\" ../QtUsb.pro | awk '{ print $3 }' "], shell=True).replace('\n', '')
    if not ver:
        print "Could not fetch last version"
        sys.exit(1)

    print "Last version:", ver

folder_name = 'libqt5usb5-{0}~{1}'.format(ver, args.release)

print "Package version:", folder_name

copySrc(folder_name, ver)
makeChangelog(args.release, folder_name, ver)

try:
    if args.source:
        makeSrc(folder_name)
        if args.ppa:
            sendSrc('libqt5usb5_' + str(ver) + '~' + args.release)
    if args.bin:
        makeBin(folder_name)

    print check_output(["rm -rf " + folder_name], shell=True)
except CalledProcessError as e:
    print e.output
