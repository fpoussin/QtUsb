pipeline {
  agent any
  stages {
    stage('Build') {
      agent {
        docker {
          image 'fpoussin/jenkins:ubuntu-16.10-qt5'
        }

      }
      steps {
        sh '''mkdir build
cd build
qmake ..
nice make -j $(nproc)'''
      }
    }
  }
}