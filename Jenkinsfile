pipeline {
    agent {
        docker {
            image 'r-base'
        }
    }

    environment {
        R_LIBS_USER = "${WORKSPACE}/Rlibs"
    }

    stages {
        stage('Install Dependencies') {
            steps {
                sh 'mkdir -p ${R_LIBS_USER}'
                sh 'R -e "install.packages(c(\'devtools\', \'testthat\'), repos=\'http://cran.rstudio.com\')"'
            }
        }

        stage('Build Package') {
            steps {
                sh 'R CMD build .'
            }
        }

        stage('Run Tests') {
            steps {
                sh 'R -e "devtools::test()"'
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
