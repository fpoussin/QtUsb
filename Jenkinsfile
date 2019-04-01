pipeline {
    agent { docker { image 'fpoussin/jenkins:ubuntu-18.04-qt5' } }

    stages {
        stage("Build and Test") {
            stages {
               stage("Build") {
                   steps {
                       sh '''
                       mkdir build
                       cd build
                       qmake ..
                       nice make -j $(nproc) all
                       mkdir -p /tmp/qtusb
                       make INSTALL_ROOT=/tmp/qtusb install
                       cd ..
                       '''
                   }
               }
               stage("Test") {
                   steps {
                       sh '''
                       cd build/tests
                       make -i check TESTARGS="-o result.xml,xunitxml"
                       cd $WORKSPACE
                       ls -l build/tests/*/*/result.xml
                       '''
                   }
               }
            }
        }
    }
    post {
        always {
            junit 'build/tests/*/*/result.xml'
        }
    }
}
