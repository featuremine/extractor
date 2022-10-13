#!/bin/bash

cleanup () {
    rm -f ../test/jubilee/conveyor/ep_server_* client_fix_*
}

init_server () {
    servertmpfile=$(mktemp)
    ( $PYTHON -c "import ep_fake_venue as fake_venue; fake_venue.run()" > $servertmpfile 2>&1 )&
    SERVER=$!
}

validate_if () {
    if [[ "$(cat $1)" =~ "$2" ]]
        then
            echo $3
            kill -SIGINT $SERVER;
            exit 1;
    fi
}

validate_unless () {
    if [[ !( "$(cat $1)" =~ "$2" ) ]]
        then
            echo $3
            kill -SIGINT $SERVER;
            exit 1;
    fi
}

kill_server () {
    kill -SIGINT $SERVER;
    wait $SERVER;
}

init_client () {
    tmpfile=$(mktemp)
    ( ( sleep 5 && echo "exit" ) | $PYTHON $CMD_UTIL_PATH --symbology $SYMBOLOGY_PATH --session $CFG_PATH --license $FM_LICENSE_PATH > $tmpfile 2>&1 )&
    CLIENT=$!
}

run () {
    init_client

    sleep 3

    init_server

    sleep 1

    wait $CLIENT
}

stop () {
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

validate_if $tmpfile "logon has lower sequence number than expected" "4.- Failed to validate client with low incoming sequence number"

validate_if $servertmpfile "logon has lower sequence number than expected" "4.- Failed to validate client with low incoming sequence number"

echo "4.- Successfully validated client lost both logs"

#5 server lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/ep_server_fix_in_in.test.log

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "5.- Failed to validate low incoming seqnum in server"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "5.- Failed to validate low incoming seqnum in server"

echo "5.- Successfully validated server lost input logs for client"

#6 client and server lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/ep_server_fix_in_in.test.log client_fix_in.test.log

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "6.- Failed to validate low incoming seqnum in server"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "6.- Failed to validate low incoming seqnum in server"

echo "6.- Successfully validated client and server lost input logs"

#7 client lost output logs and server lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/ep_server_fix_in_in.test.log client_fix_out.test.log

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "7.- Failed to validate low incoming seqnum in server"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "7.- Failed to validate low incoming seqnum in server"

echo "7.- Successfully validated client lost output logs and server lost input logs"

#8 client lost both logs and server lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/ep_server_fix_in_in.test.log client_fix_*

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "8.- Failed to validate low incoming seqnum in server"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "8.- Failed to validate low incoming seqnum in server"

echo "8.- Successfully validated client lost both logs and server lost input logs"

#9 Low seqnum received by session

cleanup

base_run

rm -f ../test/jubilee/conveyor/ep_server_fix_out.test.log

base_run

validate_unless $tmpfile "logon has lower sequence number than expected" "9.- Failed to validate fake venue with low outgoing seqnum"

echo "9.- Successfully validated fake venue with low outgoing seqnum"

#10 Low seqnum received by session, input logs lost in client

cleanup

base_run

rm -f ../test/jubilee/conveyor/ep_server_fix_out.test.log client_fix_in.test.log

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "10.- Failed to validate fake venue with low outgoing seqnum, client lost logs"

validate_unless $servertmpfile "LOGGED IN SUCCESSFULLY" "10.- Failed to validate fake venue with low outgoing seqnum, client lost logs"

echo "10.- Successfully validated fake venue with low outgoing seqnum, client lost logs"

#11 Low seqnum received by session, output logs lost in client

cleanup

base_run

rm -f ../test/jubilee/conveyor/ep_server_fix_out.test.log client_fix_out.test.log

base_run

validate_unless $tmpfile "logon has lower sequence number than expected" "11.- Failed to validate fake venue with low outgoing seqnum, client lost output log"

validate_if $servertmpfile "LOGGED IN SUCCESSFULLY" "11.- Failed to validate fake venue with low outgoing seqnum, client lost output log"

echo "11.- Successfully validated fake venue with low outgoing seqnum, client lost output log"

#12 Low seqnum received by session, both logs lost in client

cleanup

base_run

rm -f ../test/jubilee/conveyor/ep_server_fix_out.test.log client_fix_*

base_run

validate_unless $servertmpfile "logon has lower sequence number than expected" "12.- Failed to validate fake venue with low outgoing seqnum, client lost both logs"

echo "12.- Successfully validated fake venue with low outgoing seqnum, client lost both logs"

#13 server lost both logs, client keeps logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/ep_server_fix*

base_run

validate_unless $tmpfile "logon has lower sequence number than expected" "13.- Failed to validate fake venue with lost logs, client keeps both logs"

echo "13.- Successfully validated fake venue with lost logs, client keeps both logs"

#14 server lost both logs, client lost input logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/ep_server_fix* client_fix_in.test.logs

base_run

validate_unless $tmpfile "logon has lower sequence number than expected" "14.- Failed to validate fake venue with lost logs, client lost input logs"

echo "14.- Successfully validated fake venue with lost logs, client lost input logs"

#15 server lost both logs, client lost output logs

cleanup

base_run

rm -f ../test/jubilee/conveyor/ep_server_fix* client_fix_out.test.logs

base_run

validate_unless $tmpfile "logon has lower sequence number than expected" "15.- Failed to validate fake venue with lost logs, client lost output logs"

echo "15.- Successfully validated fake venue with lost logs, client lost output logs"

#16 both lost logs

cleanup

base_run

validate_unless $tmpfile "LOGGED IN SUCCESSFULLY" "16.- Failed to validate clean logon client"

echo "16.- Successfully validated clean logon client"

echo "Successfully validated recovery scenarios"

exit 0;