if [[ -n "$1" ]]
    then
        ./$1 -C 5439| play -t raw -c 2 -r 44100 -b 16 -e signed-integer --buffer 32768 -

    else
		./sikradio-receiver -C 5439 | play -t raw -c 2 -r 44100 -b 16 -e signed-integer --buffer 32768 -
fi