pipeline {
    agent any

    environment {
        R_LIBS_USER = "${WORKSPACE}/Rlibs"
    }

    stages {
        stage('Setup Environment') {
            steps {
                script {
                    sh 'mkdir -p ${R_LIBS_USER}'
                    sh 'docker pull r-base'
                }
            }
        }

        stage('Install Dependencies') {
            steps {
                script {
                    sh '''
                    docker run --rm -v "$PWD":/workspace -w /workspace r-base R -e "
                    install.packages(c('devtools', 'testthat'), repos='http://cran.rstudio.com')"
                    '''
                }
            }
        }

        stage('Build Package') {
            steps {
                script {
                    sh '''
                    docker run --rm -v "$PWD":/workspace -w /workspace r-base R CMD build .
                    '''
                }
            }
        }

        stage('Run Tests') {
            steps {
                script {
                    sh '''
                    docker run --rm -v "$PWD":/workspace -w /workspace r-base R -e "devtools::test()"
                    '''
                }
            }
        }
    }

    post {
        success {
            echo '✅ R Package Build & Tests Successful!'
        }
        failure {
            echo '❌ Build or Tests Failed!'
        }
    }
}
