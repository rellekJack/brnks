#define TIMEOUT_INT		300					// in Millisekunden
#define TIMEOUT			3					// must be a multiple of TIMEOUT_INT
#define MAX_WINDOW		10
#define DEFAULT_WINDOW	"2"
#define MAX_SEQNR		2*MAX_WINOOW-1
#define MAX_BUFFER		2*MAX_WINOOW
#define MAX_MC_RECEIVER 3

extern char *errorTable[];

struct request
{
	unsigned char	ReqType;
#define ReqHello	'H'						// ReqHello
#define ReqData		'D'						// ReqData 'D'
#define ReqClose	'C'						// ReqClose
	long			FlNr;					/* length of data in Byte ; if it is a Hello packet we transmit the window here */
	long			SeNr;					/* Sequence Number (== 0) begin of file */
#define PufferSize	256
	char			name[PufferSize];
	char			fname[50];				/* if it is a Hello packet we can transmit filename here */
};

struct answer
{
	unsigned char	AnswType;
#define AnswHello	'H'
#define AnswOk		'O'
#define AnswNACK	'N'
#define AnswWarn	'W'
#define AnswClose	'C'
#define AnswErr		0xFF
	long			FlNr;
	long			SeNo;
#define ErrNo		SeNo					/* Are identical */
};