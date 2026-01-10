extern bool onlinescore;
extern char *onlinescore_url;
extern char *onlinescore_user;
extern char *onlinescore_pass;

void onlinescore_init();
void send_onlinescore();
void *onlinescore_process(void *ptr);
