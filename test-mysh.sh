#!/bin/bash
cd "$(dirname "$0")"

if ! [[ -x mysh ]]; then
    echo "mysh executable does not exist"
    exit 1
fi

./tests/run-tests.sh $*
