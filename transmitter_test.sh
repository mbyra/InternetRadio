    if [[ -n "$1" ]]
        then
            sox -S "Westworld.mp3" -r 44100 -b 16 -e signed-integer -c 2 -t raw - | pv -q -L $((44100*4)) | ./$1 -a 239.10.11.12 -C 5439 -n "Radio Marcina"
        else
            sox -S "Westworld.mp3" -r 44100 -b 16 -e signed-integer -c 2 -t raw - | pv -q -L $((44100*4)) | ./sikradio-sender -a 239.10.11.12 -C 5439 -n "Radio Marcina"
    fi

