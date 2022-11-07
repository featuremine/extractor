"""
        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

        """

"""
@package pg_play.py
@author Andres Rangel
@date 13 Mar 2019
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import extractor as extr
import pandas as pd
import psycopg2
import sys
import argparse


def data_feed(conn):
    prev = datetime.now()
    timeout = timedelta(seconds=5)
    while(1):
        now = datetime.now()
        if (now - prev > timeout):
            print("Connection attempt timed out")
            return
        state = conn.poll()
        if state == psycopg2.extensions.POLL_OK:
            break
        else:
            continue
    cur = conn.cursor()
    curridx = 0
    while(1):
        cur.execute("SELECT * from fmtable OFFSET {0};".format(curridx))
        while(1):
            state = conn.poll()
            if state == psycopg2.extensions.POLL_OK:
                data = cur.fetchall()
                if data:
                    df = pd.DataFrame(data)
                    df = df.set_index(df.columns[0])
                    df.columns = ['px', 'qty']
                    yield df
                    curridx = curridx + len(data)
                yield None
                break
            elif state == psycopg2.extensions.POLL_ERROR:
                break
            else:
                continue


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("dbname", help="the database name")
    parser.add_argument("--user", help="user name used to authenticate")
    parser.add_argument("--password", help="password used to authenticate")
    parser.add_argument("--host", help="database host address (defaults to UNIX socket if not provided)")
    parser.add_argument("--port", help="connection port number (defaults to 5432 if not provided)")
    args = parser.parse_args()

    if args.user:
        if args.password:
            if args.host and args.port:
                conn = psycopg2.connect(
                    dbname=args.dbname,
                    user=args.user,
                    password=args.password,
                    host=args.host,
                    port=args.port)
            elif args.host:
                conn = psycopg2.connect(dbname=args.dbname, user=args.user, password=args.password, host=args.host)
            else:
                conn = psycopg2.connect(dbname=args.dbname, user=args.user, password=args.password)
        else:
            if args.host and args.port:
                conn = psycopg2.connect(dbname=args.dbname, user=args.user, host=args.host, port=args.port)
            elif args.host:
                conn = psycopg2.connect(dbname=args.dbname, user=args.user, host=args.host)
            else:
                conn = psycopg2.connect(dbname=args.dbname, user=args.user)
    elif args.host:
        if args.port:
            conn = psycopg2.connect(dbname=args.dbname, host=args.host, port=args.port)
        else:
            conn = psycopg2.connect(dbname=args.dbname, host=args.host)
    else:
        conn = psycopg2.connect(dbname=args.dbname)

    graph = extr.system.comp_graph()
    op = graph.features

    data_in_one = op.immediate_play(
        data_feed(conn),
        (("px", extr.Rprice),
         ("qty", extr.Int64)),
        timedelta(milliseconds=10))

    op.csv_record(data_in_one, '/dev/stdout')

    graph.stream_ctx().run()

    extr.flush()


if __name__ == "__main__":
    main(sys.argv)
