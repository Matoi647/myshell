#!/bin/bash
cd "$(dirname "$0")"

show() {
    if [[ -f "$1" ]]; then
        tail -v -n +1 "$1"
        echo ""
    fi
}

num=1
while true; do
    if [[ ! -f "$num.run" ]]; then
        exit
    fi
    show $num.desc
    show $num.run
    show $num.in
    show $num.out
    show $num.err
    ((num++))
    printf "%$(tput cols)s\n" |tr " " "="
done

