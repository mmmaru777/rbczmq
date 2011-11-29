#ifndef RBCZMQ_SOCKET_H
#define RBCZMQ_SOCKET_H

#define ZMQ_SOCKET_PENDING 0
#define ZMQ_SOCKET_BOUND 2
#define ZMQ_SOCKET_CONNECTED 4

typedef struct {
    zctx_t *ctx;
    void *socket;
    Bool verbose;
    int state;
    VALUE handler;
    VALUE endpoint;
    VALUE thread;
    VALUE recv_timeout;
    VALUE send_timeout;
} zmq_sock_wrapper;

#define ZMQ_SOCKET_DEFAULT_TIMEOUT Qnil

#define ZmqAssertSocket(obj) ZmqAssertType(obj, rb_cZmqSocket, "ZMQ::Socket")
#define GetZmqSocket(obj) \
    zmq_sock_wrapper *sock = NULL; \
    ZmqAssertSocket(obj); \
    Data_Get_Struct(obj, zmq_sock_wrapper, sock); \
    if (!sock) rb_raise(rb_eTypeError, "uninitialized ZMQ socket!");

#define ZmqDumpFrame(method, frame) \
  do { \
      sprintf(print_prefix, "%sI: %s socket %p: %s", cur_time, zsocket_type_str(sock->socket), (void *)sock->socket, method); \
      zframe_print((frame), print_prefix); \
      xfree(cur_time); \
  } while(0)

#define ZmqDumpMessage(method, message) \
  do { \
      zclock_log("I: %s socket %p: %s", zsocket_type_str(sock->socket), (void *)sock->socket, method); \
      zmsg_dump((message)); \
  } while(0)

#define ZmqSockGuardCrossThread(sock) \
  if (sock->thread != rb_thread_current()) \
      rb_raise(rb_eZmqError, "Cross thread violation for %s socket %p: created in thread %p, invoked on thread %p", zsocket_type_str(sock->socket), (void *)sock, (void *)sock->thread, (void *)rb_thread_current());

#define ZmqAssertSockOptFor(sock_type) \
    if (zsockopt_type(sock->socket) != sock_type) \
        rb_raise(rb_eZmqError, "Socket option not supported on a %s socket!", zsocket_type_str(sock->socket));

#define CheckBoolean(arg) \
    if (TYPE((arg)) != T_TRUE && TYPE((arg)) != T_FALSE) \
        rb_raise(rb_eTypeError, "wrong argument %s (expected true or false)", RSTRING_PTR(rb_obj_as_string((arg))));

#define ZmqSetSockOpt(obj, opt, desc, value) \
    int val; \
    GetZmqSocket(obj); \
    ZmqSockGuardCrossThread(sock); \
    Check_Type(value, T_FIXNUM); \
    val = FIX2INT(value); \
    (opt)(sock->socket, val); \
    if (sock->verbose) \
        zclock_log ("I: %s socket %p: set option \"%s\" %d", zsocket_type_str(sock->socket), (void *)obj, (desc),  val); \
    return Qnil;

#define ZmqSetStringSockOpt(obj, opt, desc, value, assertion) \
    char *val; \
    GetZmqSocket(obj); \
    ZmqSockGuardCrossThread(sock); \
    Check_Type(value, T_STRING); \
    (assertion); \
    val = StringValueCStr(value); \
    (opt)(sock->socket, val); \
    if (sock->verbose) \
        zclock_log ("I: %s socket %p: set option \"%s\" \"%s\"", zsocket_type_str(sock->socket), obj, (desc),  val); \
    return Qnil;

#define ZmqSetBooleanSockOpt(obj, opt, desc, value) \
    int val; \
    GetZmqSocket(obj); \
    ZmqSockGuardCrossThread(sock); \
    CheckBoolean(value); \
    val = (value == Qtrue) ? 1 : 0; \
    (opt)(sock->socket, val); \
    if (sock->verbose) \
        zclock_log ("I: %s socket %p: set option \"%s\" %d", zsocket_type_str(sock->socket), (void *)obj, (desc),  val); \
    return Qnil;

void rb_czmq_free_sock(zmq_sock_wrapper *sock);

void rb_czmq_mark_sock(void *ptr);
void rb_czmq_free_sock_gc(void *ptr);

struct nogvl_send_args {
    zmq_sock_wrapper *socket;
    const char *msg;
    Bool read;
};

struct nogvl_send_frame_args {
    zmq_sock_wrapper *socket;
    zframe_t *frame;
    int flags;
    Bool read;
};

struct nogvl_send_message_args {
    zmq_sock_wrapper *socket;
    zmsg_t *message;
    Bool read;
};

struct nogvl_recv_args {
    zmq_sock_wrapper *socket;
};

struct nogvl_conn_args {
    zmq_sock_wrapper *socket;
    char *endpoint;
};

void _init_rb_czmq_socket();

#endif