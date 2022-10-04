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
@package pg_pub.py
@author Andres Rangel
@date 13 Mar 2019
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import pandas as pd
import numpy as np
import psycopg2
import time
import random
import sys
import argparse


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

    cur = conn.cursor()

    cur.execute("CREATE TABLE IF NOT EXISTS fmtable(ts timestamp, px real, qty integer);")

    while(1):
        wait_period = np.random.poisson(1)
        currts = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        currpx = round(random.uniform(100, 200), 2)
        currqty = random.randint(100000, 200000)
        cur.execute("INSERT INTO fmtable(ts,px,qty) VALUES('{0}', {1}, {2});".format(currts, currpx, currqty))
        conn.commit()
        time.sleep(wait_period)


if __name__ == "__main__":
    main(sys.argv)
