
enum {
    CLUSTER_DISABLED,
    CLUSTER_DISCONNECTED,
    CLUSTER_SLEEP,
    CLUSTER_CONNECTING,
    CLUSTER_CONNECTED,
};

extern pthread_mutex_t spot_ptr_mutex;

extern int cluster_state;   // FIXME use getter function

void cluster_init(void);
void *cluster_process(void *);
void terminate_cluster_process();
void cluster_disable();

void receive_packet(void);
