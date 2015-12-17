#!/bin/sh

set -e
set -u

start_mongodb() {
    mkdir -p $1

    if [ -z "$(pgrep mongod)" ]
    then
        /usr/bin/mongod --quiet --dbpath $1 --smallfiles > /dev/null 2>&1 &
        sleep 1
    fi 

    if [ -z "$(pgrep mongod)" ]
    then
        echo "Could not start MongoDB"
        return 1
    fi
}

setup_dopamine_test_environment() {
    export DOPAMINE_TEST_DATABASE=${DOPAMINE_TEST_DATABASE:-dopamine_test}
    export DOPAMINE_TEST_LISTENINGPORT=${DOPAMINE_TEST_LISTENINGPORT:-11112}
    export DOPAMINE_TEST_WRITINGPORT=${DOPAMINE_TEST_WRITINGPORT:-11113}
    export DOPAMINE_TEST_CONFIG=${DIRECTORY}/config
    export DOPAMINE_TEST_BADCONFIG=${DIRECTORY}/badconfig
    export DOPAMINE_TEST_ADD_JANE="mongo --quiet ${DIRECTORY}/insert_jane_doe.js"
    export DOPAMINE_TEST_DEL_JANE="mongo --quiet ${DIRECTORY}/remove_jane_doe.js"
    export DOPAMINE_TEST_ADD_AUTH="mongo --quiet ${DIRECTORY}/create_authorization.js"
    export DOPAMINE_TEST_SPE_AUTH="mongo --quiet ${DIRECTORY}/create_specific_authorization.js"
    export DOPAMINE_TEST_DEL_AUTH="mongo --quiet ${DIRECTORY}/remove_authorization.js"
}

setup_ldap_environment() {
    export TEST_LDAP_SERVER=${TEST_LDAP_SERVER:-ldap://localhost}
    export TEST_LDAP_BASE=${TEST_LDAP_BASE:-cn=Users,dc=ToBeDefined,dc=local}
    export TEST_LDAP_BIND=${TEST_LDAP_BIND:-%user@ToBeDefined.local}
    export TEST_LDAP_USER=${TEST_LDAP_USER:-TOBEDEFINED}
    export TEST_LDAP_PASSWORD=${TEST_LDAP_PASSWORD:-ToB3D3fin3d}
}

setup_mongodb() {
    # MongoDB scripts to initialize database
    scripts=$(cat << EOF
        create_authorization create_db create_specific_authorization delete_db
        insert_jane_doe remove_authorization remove_jane_doe
EOF
)
    for script in ${scripts}
    do
        envsubst < ${TEST_ROOT}/data/${script}.js > ${DIRECTORY}/${script}.js
    done
    
    mongo --quiet ${DIRECTORY}/delete_db.js
    mongo --quiet ${DIRECTORY}/create_db.js
    mongo --quiet ${DIRECTORY}/create_authorization.js
}

configure_files() {
    # Dopamine configuration files
    envsubst < ${TEST_ROOT}/data/config > ${DOPAMINE_TEST_CONFIG}
    envsubst < ${TEST_ROOT}/data/badconfig > ${DOPAMINE_TEST_BADCONFIG}
}

create_data_sets() {
    export DOPAMINE_TEST_DATA=${DIRECTORY}/data
    mkdir -p ${DOPAMINE_TEST_DATA}

    export DOPAMINE_TEST_DICOMFILE_01=${DOPAMINE_TEST_DATA}/Patient01_Study01_Series01_Instance01
    export DOPAMINE_TEST_DICOMFILE_02=${DOPAMINE_TEST_DATA}/Patient02_Study01_Series01_Instance01
    export DOPAMINE_TEST_DICOMFILE_03=${DOPAMINE_TEST_DATA}/Patient02_Study01_Series01_Instance02
    export DOPAMINE_TEST_DICOMFILE_04=${DOPAMINE_TEST_DATA}/Patient02_Study01_Series01_Instance03
    export DOPAMINE_TEST_DICOMFILE_05=${DOPAMINE_TEST_DATA}/Patient03_Study01_Series01_Instance01
    export DOPAMINE_TEST_DICOMFILE_06=${DOPAMINE_TEST_DATA}/Patient03_Study02_Series01_Instance01
    export DOPAMINE_TEST_DICOMFILE_07=${DOPAMINE_TEST_DATA}/Patient04_Study01_Series01_Instance01
    export DOPAMINE_TEST_DICOMFILE_08=${DOPAMINE_TEST_DATA}/Patient04_Study01_Series01_Instance02
    export DOPAMINE_TEST_DICOMFILE_09=${DOPAMINE_TEST_DATA}/Patient04_Study01_Series01_Instance03

    files=$(cat << EOF
        ${DOPAMINE_TEST_DICOMFILE_01} ${DOPAMINE_TEST_DICOMFILE_02} 
        ${DOPAMINE_TEST_DICOMFILE_03} ${DOPAMINE_TEST_DICOMFILE_04}
        ${DOPAMINE_TEST_DICOMFILE_05} ${DOPAMINE_TEST_DICOMFILE_06}
        ${DOPAMINE_TEST_DICOMFILE_07} ${DOPAMINE_TEST_DICOMFILE_08}
        ${DOPAMINE_TEST_DICOMFILE_09}
EOF
    )
    # dump2dcm ${DIRECTORY}/dopamine_test_file_01 "${DOPAMINE_TEST_DICOMFILE_01}"
    for file in $files
    do
        dump2dcm ${TEST_ROOT}/data/$(basename ${file}).txt ${file}
    done

    # Get data tests
    wget -P ${DOPAMINE_TEST_DATA} http://www.dclunie.com/images/charset/charsettests.20070405.tar.bz2
    tar -C ${DOPAMINE_TEST_DATA} -xf ${DOPAMINE_TEST_DATA}/charsettests.20070405.tar.bz2
}

TEST_ROOT=$(dirname $0)
export DIRECTORY=`mktemp -d`

start_mongodb ${DIRECTORY}/mongodb
setup_dopamine_test_environment
setup_ldap_environment
setup_mongodb
configure_files
create_data_sets

#Output Directory
export DOPAMINE_TEST_OUTPUTDIR=${DIRECTORY}/output
mkdir ${DOPAMINE_TEST_OUTPUTDIR}

export DOPAMINE_BUILD_DIR=${PWD}

./src/appli/dopamine &

sleep 1

# Execute unit tests
ctest --no-compress-output --schedule-random -T Test $@ || true

termscu localhost ${DOPAMINE_TEST_LISTENINGPORT}

sleep 1

# FIXME: no xunit for older nosetests
nosetests --with-xunit --xunit-file=${DOPAMINE_BUILD_DIR}/../generatedJUnitFiles/nosetests.xml -w ${DOPAMINE_BUILD_DIR}/../tests/code

# Remove Database
mongo --quiet ${DIRECTORY}/delete_db.js

# FIXME: kill mongo if necessary

# Remove all temporary files
rm -rf ${DIRECTORY}
