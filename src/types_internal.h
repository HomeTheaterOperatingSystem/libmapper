
#ifndef __MAPPER_TYPES_H__
#define __MAPPER_TYPES_H__

#include <lo/lo_lowlevel.h>

#include "config.h"

#ifdef HAVE_ARPA_INET_H
 #include <arpa/inet.h>
#else
 #ifdef HAVE_WINSOCK2_H
  #include <winsock2.h>
 #endif
#endif

#include <mapper/mapper_db.h>

/**** Defined in mapper.h ****/

/* Types defined here replace opaque prototypes in mapper.h, thus we
 * cannot include it here.  Instead we include some prototypes here.
 * Typedefs cannot be repeated, therefore they are refered to by
 * struct name. */

struct _mapper_signal;
struct _mapper_admin;
typedef struct _mapper_expr *mapper_expr;

/* Forward declarations for this file. */

struct _mapper_admin_allocated_t;
struct _mapper_device;
struct _mapper_id_map;

/**** String tables ****/

/*! A pair representing an arbitrary parameter value and its
 *  type. (Re-using liblo's OSC-oriented lo_arg data structure.) If
 *  type is a string, the allocated size may be longer than
 *  sizeof(mapper_osc_arg_t). */
typedef struct _mapper_prop_value {
    char type;
    int length;
    void *value;
} mapper_prop_value_t;

/*! Used to hold string look-up table nodes. */
typedef struct {
    const char *key;
    void *value;
    int is_prop;
} string_table_node_t;

/*! Used to hold string look-up tables. */
typedef struct _mapper_string_table {
    string_table_node_t *store;
    int len;
    int alloced;
} mapper_string_table_t, *table;

/**** Admin bus ****/

/*! Some useful strings for sending admin messages. */
/*! Symbolic representation of recognized @-parameters. */
typedef enum {
    ADM_MAP,
    ADM_MAP_TO,
    ADM_MAPPED,
    ADM_MODIFY_MAP,
    ADM_DEVICE,
    ADM_UNMAP,
    ADM_UNMAPPED,
    ADM_PING,
    ADM_LOGOUT,
    ADM_NAME_PROBE,
    ADM_NAME_REG,
    ADM_SIGNAL,
    ADM_SIGNAL_REMOVED,
    ADM_SUBSCRIBE,
    ADM_UNSUBSCRIBE,
    ADM_SYNC,
    ADM_WHO,
    N_ADM_STRINGS
} admin_msg_t;

/*! Function to call when an allocated resource is locked. */
typedef void mapper_admin_resource_on_lock(struct _mapper_device *md,
                                           struct _mapper_admin_allocated_t
                                           *resource);

/*! Function to call when an allocated resource encounters a collision. */
typedef void mapper_admin_resource_on_collision(struct _mapper_admin
                                                *admin);

/*! Allocated resources */
typedef struct _mapper_admin_allocated_t {
    unsigned int value;           //!< The resource to be allocated.
    int collision_count;          /*!< The number of collisions
                                   *   detected for this resource. */
    double count_time;            /*!< The last time at which the
                                   * collision count was updated. */
    int locked;                   /*!< Whether or not the value has
                                   *   been locked in (allocated). */
    double suggestion[8];         /*!< Availability of a range
                                       of resource values. */

    //!< Function to call when resource becomes locked.
    mapper_admin_resource_on_lock *on_lock;

   //! Function to call when resource collision occurs.
    mapper_admin_resource_on_collision *on_collision;
} mapper_admin_allocated_t;

/*! Clock and timing information. */
typedef struct _mapper_sync_timetag_t {
    int message_id;
    lo_timetag timetag;
} mapper_sync_timetag_t;

typedef struct _mapper_clock_t {
    mapper_timetag_t now;
    uint32_t next_ping;
} mapper_clock_t, *mapper_clock;

typedef struct _mapper_sync_clock_t {
    double rate;
    double offset;
    double latency;
    double jitter;
    mapper_sync_timetag_t sent;
    mapper_sync_timetag_t response;
    int new;
} mapper_sync_clock_t, *mapper_sync_clock;

typedef struct _mapper_admin_subscriber {
    lo_address                      address;
    uint32_t                        lease_expiration_sec;
    int                             flags;
    struct _mapper_admin_subscriber *next;
} *mapper_admin_subscriber;

/*! A structure that keeps information about a device. */
typedef struct _mapper_admin {
    int random_id;                    /*!< Random ID for allocation speedup. */
    lo_server_thread bus_server;      /*!< LibLo server thread for the
                                       *   admin bus. */
    int msgs_recvd;                   /*!< Number of messages received on the
                                           admin bus. */
    lo_address bus_addr;              /*!< LibLo address for the admin bus. */
    lo_server_thread mesh_server;     /*!< LibLo server thread for the
                                       *   admin mesh. */
    char *interface_name;             /*!< The name of the network interface
                                       *   for receiving messages. */
    struct in_addr interface_ip;      /*!< The IP address of interface. */
    struct _mapper_device *device;    /*!< Device that this admin is
                                       *   in charge of. */
    struct _mapper_monitor *monitor;  /*!< Monitor that this admin is
                                       *   in charge of. */
    mapper_clock_t clock;             /*!< Clock for processing timed events. */
    lo_bundle bundle;                 /*!< Bundle pointer for sending
                                       *   messages on the admin bus. */
    lo_address bundle_dest;
    int message_type;
    mapper_admin_subscriber subscribers; /*!< Linked-list of subscribed peers. */
} mapper_admin_t;

/*! The handle to this device is a pointer. */
typedef mapper_admin_t *mapper_admin;

#define ADMIN_TIMEOUT_SEC 10        // timeout after 10 seconds without ping

/**** Signal ****/

/*! A structure that stores the current and historical values and timetags
 *  of a signal. The size of the history arrays is determined by the needs
 *  of mapping expressions.
 *  @ingroup signals */

typedef struct _mapper_history
{
    char type;                      /*!< The type of this signal, specified as
                                     *   an OSC type character. */
    int position;                   /*!< Current position in the circular buffer. */
    int size;                       /*!< History size of the buffer. */
    int length;                     /*!< Vector length. */
    void *value;                    /*!< Value of the signal for each sample of
                                     *   stored history. */
    mapper_timetag_t *timetag;      /*!< Timetag for each sample of stored history. */
} mapper_history_t, *mapper_history;

/**** Router ****/

typedef struct _mapper_queue {
    mapper_timetag_t tt;
    lo_bundle bundle;
    struct _mapper_queue *next;
} *mapper_queue;

/*! The link structure is a linked list of links each associated
 *  with a destination address that belong to a controller device. */
typedef struct _mapper_link {
    mapper_db_device_t props;
    int self_link;                      //!< 1 if this is a router to self.
    lo_address admin_addr;              //!< Network address of remote endpoint
    lo_address data_addr;               //!< Network address of remote endpoint
    struct _mapper_device *device;      /*!< The device associated with
                                         *   this link */
    mapper_queue queues;                /*!< Linked-list of message queues
                                         *   waiting to be sent. */
    mapper_sync_clock_t clock;
    struct _mapper_link *next;          //!< Next link in the list.
} *mapper_link;

#define MAPPER_TYPE_KNOWN   0x01
#define MAPPER_LENGTH_KNOWN 0x02
#define MAPPER_LINK_KNOWN   0x04
#define MAPPER_READY        0x07
#define MAPPER_ACTIVE       0x17

#define MAPPER_SOURCE       0x01
#define MAPPER_DESTINATION  0x02

#define MAX_NUM_MAP_SOURCES 8    // arbitrary

typedef struct _mapper_map_slot {
    // each slot can point to local signal or a remote link structure
    struct _mapper_map *map;                //!< Parent mapping.
    struct _mapper_router_signal *local;    //!< Parent signal if local
    mapper_link link;                       //!< Remote device if not local

    mapper_history history;                 /*!< Array of value histories
                                             *   for each signal instance. */
    mapper_db_map_slot props;
    int history_size;                       //!< History size.
    char status;
    char calibrating;
} mapper_map_slot_t, *mapper_map_slot;

/*! The mapper_map structure is a linked list of mappings for a given signal.
 *  Each signal can be associated with multiple inputs or outputs. This
 *  structure only contains state information used for performing mapping, the
 *  properties are publically defined in mapper_db.h. */
typedef struct _mapper_map {
    struct _mapper_router *router;
    int is_admin;
    int is_local;
    mapper_db_map_t props;

    mapper_map_slot sources;
    mapper_map_slot_t destination;

    // TODO: move expr_vars into expr structure?
    mapper_expr expr;                       //!< The mapping expression.
    mapper_history *expr_vars;              //!< User variables values.
    int num_expr_vars;                      //!< Number of user variables.
    int num_var_instances;

    int status;
    int one_source;
    char *expression;
    mapper_mode_type mode;                  /*!< Raw, linear, or expression. */
    
    struct _mapper_map *complement;         /*!< Pointer to complement in case
                                             *   of self-connection. */
} mapper_map_t, *mapper_map;

/*! The router_signal is a linked list containing a signal and a list of
 *  mappings.  TODO: This should be replaced with a more efficient approach
 *  such as a hash table or search tree. */
typedef struct _mapper_router_signal {
    struct _mapper_router *link;        //!< The parent link.
    struct _mapper_signal *signal;      //!< The associated signal.

    mapper_map_slot *slots;
    int num_slots;
    int id_counter;

    struct _mapper_router_signal *next; //!< The next router_signal in the list.
} *mapper_router_signal;

/*! The router structure. */
typedef struct _mapper_router {
    struct _mapper_device *device;  //!< The device associated with this link.
    mapper_router_signal signals;   //!< The list of mappings for each signal.
    mapper_link links;              //!< The list of links to other devices.
} *mapper_router;

/*! The instance ID map is a linked list of int32 instance ids for coordinating
 *  remote and local instances. */
typedef struct _mapper_id_map {
    uint64_t global;                        //!< Hash for originating device.
    int refcount_local;
    int local;                              //!< Local instance id to map.
    int refcount_global;
    struct _mapper_id_map *next;            //!< The next id map in the list.
} *mapper_id_map;

/**** Device ****/
struct _mapper_device;
typedef struct _mapper_device *mapper_device;

/*! Bit flags indicating if information has already been
 *  sent in a given polling step. */
#define FLAGS_SENT_DEVICE_INFO              0x01
#define FLAGS_SENT_DEVICE_INPUTS            0x02
#define FLAGS_SENT_DEVICE_OUTPUTS           0x04
#define FLAGS_SENT_DEVICE_INCOMING_MAPS     0x08
#define FLAGS_SENT_DEVICE_OUTGOING_MAPS     0x10
#define FLAGS_SENT_ALL_DEVICE_MESSAGES      0x1F
#define FLAGS_DEVICE_ATTRIBS_CHANGED        0x20

/**** Monitor ****/

/*! A list of function and context pointers. */
typedef struct _fptr_list {
    void *f;
    void *context;
    struct _fptr_list *next;
} *fptr_list;

typedef struct _mapper_db {
    mapper_db_device devices;       //<! List of devices.
    mapper_db_signal signals;       //<! List of signals.
    mapper_db_map maps;             //<! List of mappings.
    fptr_list device_callbacks;     //<! List of device record callbacks.
    fptr_list signal_callbacks;     //<! List of signal record callbacks.
    fptr_list map_callbacks;        //<! List of mapping record callbacks.
} mapper_db_t, *mapper_db;

typedef struct _mapper_subscription {
    mapper_db_device device;
    int flags;
    uint32_t lease_expiration_sec;
    struct _mapper_subscription *next;
} *mapper_subscription;

typedef struct _mapper_monitor {
    mapper_admin      admin;    //<! Admin for this monitor.

    /*! Non-zero if this monitor is the sole owner of this admin, i.e.,
     *  it was created during mmon_new() and should be freed during
     *  mmon_free(). */
    int own_admin;

    /*! Flags indicating whether information on signals and mappings should
     *  be automatically subscribed to when a new device is seen.*/
    int autosubscribe;

    /*! The time after which the monitor will declare devices "unresponsive". */
    int timeout_sec;

    /*! Linked-list of autorenewing device subscriptions. */
    mapper_subscription subscriptions;

    mapper_db_t       db;       //<! Database for this monitor.
}  *mapper_monitor;

/**** Messages ****/

/*! Symbolic representation of recognized @-parameters. */
typedef enum {
    AT_CALIBRATING,
    AT_CAUSE_UPDATE,
    AT_DEST_BOUND_MAX,
    AT_DEST_BOUND_MIN,
    AT_DEST_LENGTH,
    AT_DEST_MAX,
    AT_DEST_MIN,
    AT_DEST_TYPE,
    AT_DIRECTION,
    AT_EXPRESSION,
    AT_HOST,
    AT_ID,
    AT_INSTANCES,
    AT_LENGTH,
    AT_LIB_VERSION,
    AT_MAX,
    AT_MIN,
    AT_MODE,
    AT_MUTE,
    AT_NUM_INCOMING_MAPS,
    AT_NUM_OUTGOING_MAPS,
    AT_NUM_INPUTS,
    AT_NUM_OUTPUTS,
    AT_PORT,
    AT_PROCESS,
    AT_RATE,
    AT_REV,
    AT_SCOPE,
    AT_SEND_AS_INSTANCE,
    AT_SLOT,
    AT_SRC_BOUND_MAX,
    AT_SRC_BOUND_MIN,
    AT_SRC_LENGTH,
    AT_SRC_MAX,
    AT_SRC_MIN,
    AT_SRC_TYPE,
    AT_TYPE,
    AT_UNITS,
    AT_EXTRA,
    N_AT_PARAMS
} mapper_msg_param_t;

/* Maximum number of "extra" signal parameters. */
#define N_EXTRA_PARAMS 20

/*! Strings that correspond to mapper_msg_param_t. */
extern const char* mapper_msg_param_strings[];

/*! Strings that correspond to mapper_boundary_action, defined in
 *  mapper_db.h. */
extern const char* mapper_boundary_action_strings[];

/*! Strings that correspond to mapper_mode_type, defined in
 *  mapper_db.h. */
extern const char* mapper_mode_type_strings[];

/*! Queriable representation of a parameterized message parsed from an
 *  incoming OSC message. Does not contain a copy of data, so only
 *  valid for the duration of the message handler. Also allows for a
 *  constant number of "extra" parameters; that is, unknown parameters
 *  that may be specified for a signal and used for metadata, which
 *  will be added to a general-purpose string table associated with
 *  the signal. */
typedef struct _mapper_message
{
    const char *path;               //!< OSC address.
    lo_arg **values[N_AT_PARAMS];   //!< Array of parameter values.
    const char *types[N_AT_PARAMS]; //!< Array of types for each value.
    int lengths[N_AT_PARAMS];       //!< Array of lengths for each value.
    lo_arg **extra_args[N_EXTRA_PARAMS]; //!< Pointers to extra parameters.
    char extra_types[N_EXTRA_PARAMS];    //!< Types of extra parameters.
    char extra_lengths[N_EXTRA_PARAMS];  //!< Lengths of extra parameters.
} mapper_message_t;

#endif // __MAPPER_TYPES_H__
