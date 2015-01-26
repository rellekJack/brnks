struct timeouts
{
	unsigned long seq_nr;
	unsigned long timer;	// number of TIMEOUT_INT for retransmitting
	struct timeouts* next;	// pointer to next timeout
};


struct timeouts* add_timer(struct timeouts *list, int timer_val, unsigned long seq_nr);
struct timeouts* del_timer(struct timeouts *list, unsigned long seq_nr);
//int decrement_timer(struct timeouts *list, long seq_nr);
int decrement_timer(struct timeouts *list);