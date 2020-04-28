#!/usr/bin/python3

import os, re, argparse, shutil
from subprocess import Popen, CalledProcessError, STDOUT, PIPE
from traceback import print_exc
from glob import glob
import requests

# qtsync.pl version for each distro release
distros = {'bionic': '5.9',
           'focal': '5.12'}

build_dir = os.getcwd() + '/../qtusb-build'

parser = argparse.ArgumentParser()
parser.add_argument('-d', '--distro', help='The Ubuntu release')
parser.add_argument('-r', '--release', help='Package release number', type=int, default=1)
parser.add_argument('-S', '--suffix', help='Package version suffix', type=str, default='')
parser.add_argument('-s', '--source', help='Build signed source package', action='store_true')
parser.add_argument('-u', '--upload', help='Send source package to PPA', action='store_true')
parser.add_argument('-b', '--build', help='Build binary package with sbuild', action='store_true')
parser.add_argument('-k', '--keep', help='Keep build folder', action='store_true', default=False)


def run_cmd(cmd, do_print=True, **kwargs):
    p = Popen(cmd, shell=True, stdout=PIPE, stderr=STDOUT, **kwargs)
    if do_print:
        lines_iterator = iter(p.stdout.readline, b'')
        for line in lines_iterator:
            print(line.strip().decode('utf-8'))  # yield line
    else:
        p.wait()


def clean_src():
    # Clean local build folders
    for i in ('bin', 'lib', 'include', 'mkspecs'):
        shutil.rmtree(i, ignore_errors=True)

    # Clean files from local builds
    types = ('*.deb', '*.ddeb', 'qtusb_*', 'libqt5usb5*')
    files_grabbed = []
    for files in types:
        files_grabbed.extend(glob(files))
    for i in files_grabbed:
        try:
            os.remove(i)
        except OSError:
            pass


def get_syncqt(qt_ver):
    # Check is file have already been downloaded
    if os.path.isfile('debian/syncqt_{}.pl'.format(qt_ver)):
        return

    url = 'https://raw.githubusercontent.com/qt/qtbase/{}/bin/syncqt.pl'
    r = requests.get(url.format(qt_ver), allow_redirects=True)
    open('debian/syncqt_{}.pl'.format(qt_ver), 'wb').write(r.content)


def copy_src(dest, ver, suffix, release, distro):

    ver_suffix = ver
    if suffix:
        ver_suffix += '~' + suffix
    # Add distro / release
    with open('debian/changelog_template', 'r') as f:
        tmp = f.read().replace('distro', distro)\
            .replace('(0.0.0)', '({0}-{1}{2})'.format(ver_suffix, distro, release))
        print(tmp)

    with open('debian/changelog', 'w') as f:
        f.write(str(tmp))

    with open('version', 'w') as f:
        f.write(str(ver_suffix))

    clean_src()

    qt_ver = distros[distro]
    get_syncqt(qt_ver)

    # Generate headers manually since we are exporting to an archive
    run_cmd('perl debian/syncqt_{0}.pl -version {1} .'.format(qt_ver, ver))

    # Create external build dir
    run_cmd('mkdir -p {}'.format(build_dir))

    # Copy debian files
    run_cmd('cp -r debian {}'.format(build_dir))

    # Exclude all the files we don't need to build the package
    run_cmd('tar cvf {0}/../qtusb_{1}.orig.tar.gz '
            '--exclude=./debian* '
            '--exclude=./.git* '
            '--exclude=./libusb* '
            '--exclude=./build* '
            '--exclude=./*.clang* '
            '--exclude=./*.user '
            '--exclude=./.travis* '
            '--exclude=./appveyor.yml '
            '--exclude=./Jenkinsfile '
            '--exclude=./*.py '
            '--exclude=./*.bat '
            '--exclude=./Doxyfile '
            '.'.format(build_dir, ver_suffix))

    # Extract our cleaned code in build dir
    run_cmd(['tar xf {0}/../qtusb_{1}.orig.tar.gz'.format(build_dir, ver_suffix)], cwd=build_dir)


def make_src(dest):
    print('Building Signed Source package')
    run_cmd(['debuild -S -sa'], cwd=build_dir)


def make_local_src(dest):
    print('Building Unsigned Source package')
    run_cmd(['debuild -S -us -uc'], cwd=build_dir)


def make_s_build(dest):
    print('Building binary package (sbuild)')
    cmd = 'sbuild -vd {0} -c {0}-amd64 -j8 {1}'.format(args.distro, dest)
    print(cmd)
    run_cmd([cmd], cwd=build_dir)


def upload(ver):
    run_cmd(['dput ppa:fpoussin/ppa {0}_source.changes'.format(ver)])


if __name__ == '__main__':

    args = parser.parse_args()

    if not args.distro:
        parser.print_help()

    if args.distro not in distros:
        print('Invalid Ubuntu release, please chose among:', ", ".join(distros.keys()))
        exit(1)

    # Extract version from .qmake.conf
    ver = ''
    with open('.qmake.conf') as f:
        for l in f.readlines():
            r = re.search(r'^MODULE_VERSION = (.+)', l)
            if r:
                ver = r.group(1)
                break

    if not ver:
        print('Could not read version from .qmake.conf')
        exit(1)

    ver_suffix = ver
    suffix = args.suffix
    if suffix:
        ver_suffix = ver + '~' + suffix

    build_dir += '/' + args.distro

    print('Source Version:', ver)
    print('Suffix:', suffix)
    print('Build dir:', build_dir)

    folder_name = '{0}/qtusb-{1}.orig.tar.gz'.format(build_dir, ver_suffix)
    dsc_name = '{0}/../qtusb_{1}-{2}{3}'.format(build_dir, ver_suffix, args.distro, args.release)

    print('Package location:', folder_name)

    copy_src(folder_name, ver, suffix, args.release, args.distro)

    try:
        if args.source:
            make_src(folder_name)
        if args.upload:
            upload(dsc_name)
        if args.build:
            make_local_src(folder_name)
            make_s_build(dsc_name + '.dsc')

    except CalledProcessError as e:
        print(e.output)
    except OSError as e:
        print(e.filename, e)
        print_exc()
    finally:
        if not args.keep:
            run_cmd('schroot -e --all-sessions')
            clean_src()
