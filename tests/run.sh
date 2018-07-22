#!/bin/sh

set -e
set -u

start_mongodb() {
    mkdir -p $1

    if [ -z "$(pgrep mongod)" ]
    then
        echo "Starting MongoDB"
        /usr/bin/mongod --quiet --dbpath $1 --smallfiles > /dev/null 2>&1 &
        MONGO_PID=$!
        sleep 10
    fi

    if [ -z "$(pgrep mongod)" ]
    then
        echo "Could not start MongoDB"
        return 1
    fi
}

stop_mongodb() {
  if [ -n "${MONGO_PID:-}" ]
  then
      kill ${MONGO_PID}
  fi

  # Remove all temporary files
  rm -rf ${DIRECTORY}
}

# Set-up
export DIRECTORY=`mktemp -d`
start_mongodb ${DIRECTORY}/mongodb
sleep 1

# Execute unit tests
ctest --no-compress-output --schedule-random -T Test $@ || true

# Clean-up
sleep 1
stop_mongodb
