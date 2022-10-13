#!/bin/bash

cleanup () {
    rm -f ../test/jubilee/conveyor/server_* client_fix_* client_drop_*
}

init_server () {
    servertmpfile=$(mktemp)
    ( $PYTHON -c "import baml_fake_venue as fake_venue; fake_venue.run()" > $servertmpfile 2>&1 )&
    SERVER=$!
}

kill_server () {
    kill -SIGINT $SERVER;
    wait $SERVER;
}

init_client () {
    tmpfile=$(mktemp)
    ( ( sleep 3 && echo "exit" ) | $PYTHON $CMD_UTIL_PATH --symbology $SYMBOLOGY_PATH --session $CFG_PATH --license $FM_LICENSE_PATH > $tmpfile 2>&1 ) &
    CLIENT=$!
}

init_dropcopy () {
    dropcopytmpfile=$(mktemp)
    ( $PYTHON -c "import baml_drop_copy as drop_copy; drop_copy.run()" > $dropcopytmpfile 2>&1 )&
    DROPCOPY=$!
}

kill_dropcopy () {
    kill -SIGINT $DROPCOPY;
    wait $DROPCOPY;
}

validate_if () {
    if [[ "$(cat $1)" =~ "$2" ]]
        then
            echo $3
            kill -SIGINT $DROPCOPY;
            wait $DROPCOPY;
            kill -SIGINT $SERVER;
            wait $SERVER;
            exit 1;
    fi
}

validate_unless () {
    if [[ !( "$(cat $1)" =~ "$2" ) ]]
        then
            echo $3
            kill -SIGINT $DROPCOPY;
            wait $DROPCOPY;
            kill -SIGINT $SERVER;
            wait $SERVER;
            exit 1;
    fi
}

run () {
    init_server

    sleep 1

    init_client

    init_dropcopy

    wait $CLIENT
}

stop () {
    kill_dropcopy
    kill_server
}

base_run () {
    run

    stop
}

#1 both keep logs

cleanup

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "1.- Failed to validate recovery logon"

echo "1.- Successfully validated client recovery logon"

#2 client lost input logs

rm -f  client_fix_in.test.log

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "2.- Failed to validate logon with low input seqnum in client"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "2.- Failed to validate logon with low input seqnum in client"

echo "2.- Successfully validated client logon with low input seqnum in client"

#3 client lost input logs

rm -f client_fix_out.test.log

base_run

validate_unless $servertmpfile "logon has lower sequence number than expected" "3.- Failed to validate client with low incoming sequence number"

echo "3.- Successfully validated client client with low incoming sequence number from server"

#4 client lost both logs

cleanup

base_run

rm -f client_drop_*

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "4.- Failed to validate logon with low input seqnum in client"

validate_unless $servertmpfile "logon has lower sequence number than expected" "4.- Failed to validate client with low incoming sequence number"

echo "4.- Successfully validated client lost both logs"

#5 both keep logs

cleanup

base_run

validate_unless $dropcopytmpfile "LOGGED IN SUCCESSFULLY" "5.- Failed to validate recovery logon"

echo "5.- Successfully validated dropcopy recovery logon"

#6 dropcopy lost input logs

rm -f  client_drop_in

base_run

validate_unless $dropcopytmpfile "LOGGED IN SUCCESSFULLY" "6.- Failed to validate logon with low input seqnum in client"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "6.- Failed to validate logon with low input seqnum in client"

echo "6.- Successfully validated dropcopy logon with low input seqnum in client"

#7 dropcopy lost output logs

rm -f client_drop_out

base_run

validate_unless $servertmpfile "logon has lower sequence number than expected" "7.- Failed to validate dropcopy with low incoming sequence number"

echo "7.- Successfully validated dropcopy with low outgoing sequence number"

#8 dropcopy lost both logs

cleanup

base_run

rm -f client_drop_*

base_run

validate_if $dropcopytmpfile "LOGGED IN SUCCESSFULLY" "8.- Failed to validate logon with low input seqnum in client"

validate_unless $servertmpfile "logon has lower sequence number than expected" "8.- Failed to validate dropcopy with low incoming sequence number"

echo "8.- Successfully validated dropcopy lost both logs"

#9 server lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_fix_in_in.test.log

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "9.- Failed to validate low incoming seqnum in server"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "9.- Failed to validate low incoming seqnum in server"

echo "9.- Successfully validated server lost input logs for client"

#10 client and server lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_fix_in_in.test.log client_fix_in.test.log

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "10.- Failed to validate low incoming seqnum in server"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "10.- Failed to validate low incoming seqnum in server"

echo "10.- Successfully validated client and server lost input logs"

#11 client lost output logs and server lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_fix_in_in.test.log client_fix_out.test.log

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "11.- Failed to validate low incoming seqnum in server"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "11.- Failed to validate low incoming seqnum in server"

echo "11.- Successfully validated client lost output logs and server lost input logs"

#12 client lost both logs and server lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_fix_in_in.test.log client_fix_*

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "12.- Failed to validate low incoming seqnum in server"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "12.- Failed to validate low incoming seqnum in server"

echo "12.- Successfully validated client lost both logs and server lost input logs"

#13 server lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_drop_in.test.log

base_run

validate_unless $dropcopytmpfile "LOGGED IN SUCCESSFULLY" "13.- Failed to validate low incoming seqnum in server"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "13.- Failed to validate low incoming seqnum in server"

echo "13.- Successfully validated server lost input logs for dropcopy"

#14 dropcopy and server lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_drop_in.test.log client_drop_in

base_run

validate_unless $dropcopytmpfile "LOGGED IN SUCCESSFULLY" "14.- Failed to validate low incoming seqnum in server"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "14.- Failed to validate low incoming seqnum in server"

echo "14.- Successfully validated dropcopy and server lost input logs"

#15 dropcopy lost output logs and server lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_drop_in.test.log client_drop_out

base_run

validate_unless $dropcopytmpfile "LOGGED IN SUCCESSFULLY" "15.- Failed to validate low incoming seqnum in server"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "15.- Failed to validate low incoming seqnum in server"

echo "15.- Successfully validated dropcopy lost output logs and server lost input logs"

#16 dropcopy lost both logs and server lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_drop_in.test.log client_drop_*

base_run

validate_unless $dropcopytmpfile "LOGGED IN SUCCESSFULLY" "16.- Failed to validate low incoming seqnum in server"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "16.- Failed to validate low incoming seqnum in server"

echo "16.- Successfully validated dropcopy lost both logs and server lost input logs"

#17 Low seqnum received by session

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_fix_out.test.log

base_run

validate_unless $tmpfile "logon has lower sequence number than expected" "17.- Failed to validate fake venue with low outgoing seqnum"

echo "17.- Successfully validated fake venue with low outgoing seqnum"

#18 Low seqnum received by session, input logs lost in client

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_fix_out.test.log client_fix_in.test.log

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "18.- Failed to validate fake venue with low outgoing seqnum, client lost logs"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "18.- Failed to validate fake venue with low outgoing seqnum, client lost logs"

echo "18.- Successfully validated fake venue with low outgoing seqnum, client lost logs"

#19 Low seqnum received by session, output logs lost in client

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_fix_out.test.log client_fix_out.test.log

base_run

validate_unless $servertmpfile "logon has lower sequence number than expected" "19.- Failed to validate fake venue with low outgoing seqnum, client lost output log"

echo "19.- Successfully validated fake venue with low outgoing seqnum, client lost output log"

#20 Low seqnum received by session, both logs lost in client

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_fix_out.test.log client_fix_*

base_run

validate_unless $servertmpfile "logon has lower sequence number than expected" "20.- Failed to validate fake venue with low outgoing seqnum, client lost both logs"

echo "20.- Successfully validated fake venue with low outgoing seqnum, client lost both logs"

#21 Low seqnum received by dropcopy

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_drop_out.test.log

base_run

validate_unless $dropcopytmpfile "logon has lower sequence number than expected" "21.- Failed to validate fake venue with low outgoing seqnum"

echo "21.- Successfully validated fake venue with low outgoing seqnum"

#22 Low seqnum received by session, input logs lost in client

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_drop_out.test.log client_drop_in

base_run

validate_unless $dropcopytmpfile "LOGGED IN SUCCESSFULLY" "22.- Failed to validate fake venue with low outgoing seqnum, client lost logs"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "22.- Failed to validate fake venue with low outgoing seqnum, client lost logs"

echo "22.- Successfully validated fake venue with low outgoing seqnum, client lost logs"

#23 Low seqnum received by session, output logs lost in client

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_drop_out.test.log client_drop_out

base_run

validate_unless $servertmpfile "logon has lower sequence number than expected" "23.- Failed to validate fake venue with low outgoing seqnum, client lost output log"

echo "23.- Successfully validated fake venue with low outgoing seqnum, client lost output log"

#24 Low seqnum received by session, both logs lost in client

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_drop_out.test.log client_drop_*

base_run

validate_unless $servertmpfile "logon has lower sequence number than expected" "24.- Failed to validate fake venue with low outgoing seqnum, client lost both logs"

echo "24.- Successfully validated fake venue with low outgoing seqnum, client lost both logs"

#25 server lost both logs, client keeps logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_fix*

base_run

validate_unless $tmpfile "logon has lower sequence number than expected" "25.- Failed to validate fake venue with lost logs, client keeps both logs"

echo "25.- Successfully validated fake venue with lost logs, client keeps both logs"

#26 server lost both logs, client lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_fix* client_fix_in.test.log

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "26.- Failed to validate fake venue with lost logs, client lost input logs"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "26.- Failed to validate fake venue with lost logs, client lost input logs"

echo "26.- Successfully validated fake venue with lost logs, client lost input logs"

#27 server lost both logs, client lost output logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_fix* client_fix_out.test.log

base_run

validate_unless $tmpfile "logon has lower sequence number than expected" "27.- Failed to validate fake venue with lost logs, client lost output logs"

echo "27.- Successfully validated fake venue with lost logs, client lost output logs"

#28 both lost logs

cleanup

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "28.- Failed to validate clean logon client"

echo "28.- Successfully validated clean logon client"

#29 server lost both logs, client keeps logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_drop*

base_run

validate_unless $dropcopytmpfile "logon has lower sequence number than expected" "29.- Failed to validate fake venue with lost logs, dropcopy keeps both logs"

echo "29.- Successfully validated fake venue with lost logs, dropcopy keeps both logs"

#30 server lost both logs, client lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_drop* client_drop_in

base_run

validate_unless $dropcopytmpfile "LOGGED IN SUCCESSFULLY" "30.- Failed to validate fake venue with lost logs, dropcopy lost input logs"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "30.- Failed to validate fake venue with lost logs, dropcopy lost input logs"

echo "30.- Successfully validated fake venue with lost logs, dropcopy lost input logs"

#31 server lost both logs, client lost output logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/server_drop* client_drop_out

base_run

validate_unless $dropcopytmpfile "logon has lower sequence number than expected" "31.- Failed to validate fake venue with lost logs, dropcopy lost output logs"

echo "31.- Successfully validated fake venue with lost logs, dropcopy lost output logs"

#32 both lost logs

cleanup

base_run

validate_unless $dropcopytmpfile "LOGGED IN SUCCESSFULLY" "32.- Failed to validate clean logon dropcopy"

echo "32.- Successfully validated clean logon dropcopy"

echo "Successfully validated recovery scenarios"

exit 0;