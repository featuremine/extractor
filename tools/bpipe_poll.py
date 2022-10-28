"""
@package bpipe_poll.py
@author Andres Rangel
@date 24 Apr 2019
@brief File contains sample b-pipe polling script

This is a b-pipe polling sample.
"""

import extractor as extr
from collections import namedtuple
import blpapi
from datetime import datetime, timedelta

TOKEN_SUCCESS = blpapi.Name("TokenGenerationSuccess")
TOKEN_FAILURE = blpapi.Name("TokenGenerationFailure")
AUTHORIZATION_SUCCESS = blpapi.Name("AuthorizationSuccess")
TOKEN = blpapi.Name("token")

LastPrice = namedtuple('LastPrice', ['ticker', 'price'])


class ConfigDict(dict):
    def __init__(self, hosts=[], port=8194, service="//blp/mktdata",
                 topics=["/ticker/IBM Equity"], fields=["LAST_PRICE"], options=[], auth=None,
                 tls_client_credentials=None, tls_client_credentials_password=None,
                 tls_trust_material=None, tls_cert_files=None):
        super(ConfigDict, self).__init__()
        self.__dict__ = self

        self.hosts = hosts

        self.port = port

        self.service = service

        self.topics = topics

        self.fields = fields

        self.options = options

        self.auth = self.__proc_auth(auth)

        self.tlsOptions = self.__getTlsOptions({"credentials": tls_client_credentials,
                                                "password": tls_client_credentials_password,
                                                "trust": tls_trust_material,
                                                "cert_files": tls_cert_files})

    def __proc_auth(self, value):
        """Parse authorization options from user input"""

        if not value:
            return {'option': None}

        vals = value.split('=', 1)

        if value == "user":
            return {'option': "AuthenticationType=OS_LOGON"}
        elif value == "none":
            return {'option': None}
        elif vals[0] == "app" and len(vals) == 2:
            return {
                'option': "AuthenticationMode=APPLICATION_ONLY;"
                "ApplicationAuthenticationType=APPNAME_AND_KEY;"
                "ApplicationName=" + vals[1]}
        elif vals[0] == "userapp" and len(vals) == 2:
            return {
                'option': "AuthenticationMode=USER_AND_APPLICATION;"
                "AuthenticationType=OS_LOGON;"
                "ApplicationAuthenticationType=APPNAME_AND_KEY;"
                "ApplicationName=" + vals[1]}
        elif vals[0] == "dir" and len(vals) == 2:
            return {
                'option': "AuthenticationType=DIRECTORY_SERVICE;"
                "DirSvcPropertyName=" + vals[1]}
        elif vals[0] == "manual":
            parts = []
            if len(vals) == 2:
                parts = vals[1].split(',')

            # TODO: Add support for user+ip only
            if len(parts) != 3:
                raise OptionValueError("Invalid auth option '%s'" % value)

            option = "AuthenticationMode=USER_AND_APPLICATION;" + \
                     "AuthenticationType=MANUAL;" + \
                     "ApplicationAuthenticationType=APPNAME_AND_KEY;" + \
                     "ApplicationName=" + parts[0]

            return {'option': option,
                    'manual': {'ip': parts[1],
                               'user': parts[2]}}
        else:
            raise OptionValueError("Invalid auth option '%s'" % value)

    def __getTlsOptions(self, tls):
        """Parse TlsOptions from user input"""

        if (tls["credentials"] is None or
                tls["trust"] is None):
            return None

        print("TlsOptions enabled")
        if tls["tls_cert_files"]:
            credential_blob = None
            trust_blob = None
            with open(tls["credentials"], 'rb') as credentialfile:
                credential_blob = credentialfile.read()
            with open(tls["trust"], 'rb') as trustfile:
                trust_blob = trustfile.read()
            return blpapi.TlsOptions.createFromBlobs(
                credential_blob,
                tls["password"],
                trust_blob)

        return blpapi.TlsOptions.createFromFiles(
            tls["credentials"],
            tls["password"],
            tls["trust"])


def prepareStandardSessionOptions(options):
    """Prepare SessionOptions for a regular session"""

    sessionOptions = blpapi.SessionOptions()
    for idx, host in enumerate(options.hosts):
        sessionOptions.setServerAddress(host, options.port, idx)

    # NOTE: If running without a backup server, make many attempts to
    # connect/reconnect to give that host a chance to come back up (the
    # larger the number, the longer it will take for SessionStartupFailure
    # to come on startup, or SessionTerminated due to inability to fail
    # over).  We don't have to do that in a redundant configuration - it's
    # expected at least one server is up and reachable at any given time,
    # so only try to connect to each server once.
    sessionOptions.setNumStartAttempts(len(options.hosts)
                                       if len(options.hosts) > 1 else 1000)

    print("Connecting to port %d on %s" % (
        options.port, ", ".join(options.hosts)))

    if options.tlsOptions:
        sessionOptions.setTlsOptions(options.tlsOptions)

    return sessionOptions


def authorize(authService, identity, session, cid, manual_options=None):
    """Authorize the identity"""

    tokenEventQueue = blpapi.EventQueue()

    if manual_options:
        session.generateToken(authId=manual_options['user'],
                              ipAddress=manual_options['ip'],
                              eventQueue=tokenEventQueue)
    else:
        session.generateToken(eventQueue=tokenEventQueue)

    # Process related response
    ev = tokenEventQueue.nextEvent()
    token = None
    if ev.eventType() == blpapi.Event.TOKEN_STATUS or \
            ev.eventType() == blpapi.Event.REQUEST_STATUS:
        for msg in ev:
            print(msg)
            if msg.messageType() == TOKEN_SUCCESS:
                token = msg.getElementAsString(TOKEN)
            elif msg.messageType() == TOKEN_FAILURE:
                break

    if not token:
        print("Failed to get token")
        return False

    # Create and fill the authorization request
    authRequest = authService.createAuthorizationRequest()
    authRequest.set(TOKEN, token)

    # Send authorization request to "fill" the Identity
    session.sendAuthorizationRequest(authRequest, identity, cid)

    # Process related responses
    startTime = datetime.today()
    WAIT_TIME_SECONDS = 10
    while True:
        event = session.nextEvent(WAIT_TIME_SECONDS * 1000)
        if event.eventType() == blpapi.Event.RESPONSE or \
            event.eventType() == blpapi.Event.REQUEST_STATUS or \
                event.eventType() == blpapi.Event.PARTIAL_RESPONSE:
            for msg in event:
                print(msg)
                if msg.messageType() == AUTHORIZATION_SUCCESS:
                    return True
                print("Authorization failed")
                return False

        endTime = datetime.today()
        if endTime - startTime > timedelta(seconds=WAIT_TIME_SECONDS):
            return False


def proc_msg(message):
    lastpx = message.getElementAsFloat("LAST_PRICE")
    tickername = message.correlationIds()[0].value()
    ticker = tickername.rpartition('/')[2]
    return LastPrice(ticker=ticker, price=lastpx)


def iter_gen(options):
    # Fill SessionOptions
    sessionOptions = prepareStandardSessionOptions(options)
    sessionOptions.setAuthenticationOptions(options.auth["option"])
    sessionOptions.setAutoRestartOnDisconnection(True)

    session = blpapi.Session(sessionOptions)

    if not session.start():
        print("Failed to start session.")
        return

    subscriptionIdentity = session.createIdentity()

    if options.auth:
        isAuthorized = False
        authServiceName = "//blp/apiauth"
        if session.openService(authServiceName):
            authService = session.getService(authServiceName)
            isAuthorized = authorize(
                authService, subscriptionIdentity, session,
                blpapi.CorrelationId("auth"),
                options.auth.get('manual'))
        if not isAuthorized:
            print("No authorization")

    subscriptions = blpapi.SubscriptionList()
    for t in options.topics:
        topic = options.service + t
        subscriptions.add(topic,
                          options.fields,
                          options.options,
                          blpapi.CorrelationId(topic))

    session.subscribe(subscriptions, subscriptionIdentity)

    try:
        while True:
            # Specify timeout to give a chance for Ctrl-C
            event = session.nextEvent(1000)
            print("Event proceessed:", event)
            msgs = []
            for msg in event:
                print("Message proceessed:", msg)
                if event.eventType() == blpapi.Event.SUBSCRIPTION_DATA:
                    msgs.append(proc_msg(msg))
            if msgs:
                yield msgs
            else:
                yield None
    finally:
        session.stop()


if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    options = ConfigDict()

    data_in = op.live_poll(iter_gen(options), timedelta(milliseconds=100))

    last_px = op.tuple_msg(data_in, "LastPrice",
                           (("ticker", extr.Array(extr.Char, 16)),
                            ("price", extr.Float64)))

    graph.callback(last_px, lambda frame: print(frame.as_pandas()))

    graph.stream_ctx().run_live()
