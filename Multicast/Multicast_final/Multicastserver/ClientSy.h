extern void initClient(char *name); 
extern struct timeouts *sendData(struct request *req, struct timeouts *listptr, int *sqnr_counter, int *fail, struct request *temp, int *ende);
extern struct answer *recvanswer();
extern void exitClient();