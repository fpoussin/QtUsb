pipeline {
  agent any
  stages {
    stage('') {
      agent {
        docker {
          image 'fpoussin/jenkins:ubuntu-16.10-qt5'
        }

      }
      steps {
        sh '''qmake
make'''
      }
    }
  }
}