#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
  char data[20];
  };

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
   int seqnum;
   int acknum;
   int checksum;
   char payload[20];
    };



/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* Statistics
 * Do NOT change the name/declaration of these variables
 * You need to set the value of these variables appropriately within your code.
 * */
int A_application = 0;
int A_transport = 0;
int B_application = 0;
int B_transport = 0;

/* Globals
 * Do NOT change the name/declaration of these variables
 * They are set to zero here. You will need to set them (except WINSIZE) to some proper values.
 * */
float TIMEOUT = 20.0;
//int WINSIZE;         //Not applicable to ABT
//int SND_BUFSIZE = 0; //Not applicable to ABT
//int RCV_BUFSIZE = 0; //Not applicable to ABT

/* Function declarations */

void tolayer3(int,struct pkt);
void tolayer5(int,char *);
int checksum(char * data);
void starttimer(int, float);
void stoptimer(int);
/* Helper functions */
int checksum(char * data){
  int checksum=0;
  /*

   uint32_t sum;
   uint16_t seg1 = ((uint16_t)data & 0x0000000000000000ffff);
   uint16_t seg2 = ((uint16_t)(data >> 16 ) & 0x0000000000000000ffff);
   uint16_t seg3 = ((uint16_t)(data >> 32 ) & 0x0000000000000000ffff);
   uint16_t seg4 = ((uint16_t)(data >> 48 ) & 0x0000000000000000ffff);
   uint16_t seg5 = ((uint16_t)(data >> 64 ) & 0x0000000000000000ffff);
   sum = seg1 + seg2;
   if(sum>65535){
     sum = sum & 0x000000ff;
     sum += 0x00000001;
   }
   sum = sum + seg3;
   if(sum>65535){
     sum = sum & 0x000000ff;
     sum += 0x00000001;
   }
   sum = sum + seg4;
   if(sum>65535){
     sum = sum & 0x000000ff;
     sum += 0x00000001;
   }
   sum = sum + seg5;
   if(sum>65535){
     sum = sum & 0x000000ff;
     sum += 0x00000001;
   }
   */
   for(int i = 0; i < 20; i++){
     checksum += (int)data[i];
   }
   return checksum;
}

/* called from layer 5, passed the data to be sent to other side */
#define CALL0 0
#define CALL1 1
#define ACK0 2
#define ACK1 3
#define WAIT0 4
#define WAIT1 5

int seqnoA = 0;
int acknoA = 0;
int seqnoB = 0;
int acknoB = 0;
int expacknoA = 0;
int expacknoB = 0;
int expseqnoA = 0;
int expseqnoB = 0;
int waitingforackA = 0;
int waitingforcallA = 0;
int waitingforackB = 0;
int waitingforcallB = 0;
float timerA = TIMEOUT;
int A = 0;
int B = 1;
struct pkt packetA = {0};
struct pkt packetB = {0};
struct pkt ackpacketA = {0};
struct pkt ackpacketB0 = {0};
struct pkt ackpacketB1 = {0};
char ackdata[25]="0000000000000000000100";
int LOG = 1;
int timerstate = 0; // 0 off, 1 on
int state = CALL0; // waiting for call 0 or call 1 or (2 and 3) for ack 0 and 1
int stateB = WAIT0;
void checkandstarttimer(int AorB, float increment){
  if(!timerstate){
    starttimer(AorB, increment);
    timerstate = 1;
  }

}
void checkandstoptimer(int AorB){
  if(timerstate){
    stoptimer(AorB);
    timerstate = 0;
  }

}
void A_output(message)
  struct msg message;
{
  A_application++;
  // Encapsulation
  fprintf(stderr, ">>> A_output: state = %d\n\n", state);
  if(state==CALL0){
	   packetA.seqnum = 0;
     expacknoA = 0;
  }else if(state == CALL1){
    packetA.seqnum = 1;
    expacknoA = 1;
  }else{
    fprintf(stderr,"A_output: Illegal state, state=%d \n", state);
    return;
  }
	packetA.acknum = acknoA;
	packetA.checksum = checksum(message.data);
	strncpy(packetA.payload, message.data, 20);
	tolayer3(A,packetA);
  A_transport++;
  if(LOG)
    fprintf(stderr,"\nSending: data = %s, seq=%d, ack=%d, checksum=%d \n\n",packetA.payload,packetA.seqnum, packetA.acknum, packetA.checksum);
  checkandstarttimer(A,timerA);
  if(state == CALL0){
    state=ACK0;
  }else if(state == CALL1){
    state=ACK1;
  }else{
    fprintf(stderr,"A_output: Illegal state, state=%d \n", state);
    return;
  }

}

void B_output(message)  /* need be completed only for extra credit */
  struct msg message;
{

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  if(LOG){
    fprintf(stderr,"A_input: Packet received\n");
    fprintf(stderr,"Ack data: data = %s, seq=%d, ack=%d, checksum=%d \n",packet.payload,packet.seqnum, packet.acknum, packet.checksum);
  }
  if(!(state == ACK0 || state == ACK1)){
    fprintf(stderr,"A_input: Illegal state, state=%d \n", state);
    return;
  }
  if(strncmp(packet.payload,ackdata,20)!=0){
    perror("Error: Not an ACK packet");
    return;
  }
  if(checksum(packet.payload)!=packet.checksum){
    perror("Error: checksum does not match");
    return;
  }
  if(packet.acknum != expacknoA){
    perror("Error: not the expected sequence number");
    return;
  }
  if(LOG){
    fprintf(stderr,"Stopping timer\n");
  }

  checkandstoptimer(A);
  if(state == ACK0){
    state = CALL1;
  }else if(state == ACK1){
    state = CALL0;
  }


}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  if(!(state != ACK0 || state != ACK1)){
    fprintf(stderr,"A_output: Illegal state, state=%d \n", state);
    return;
  }
  checkandstoptimer(A);
  if(LOG){
    fprintf(stderr,"Timer fired\n");
    fprintf(stderr,"Sending due to TO: data = %s, seq=%d, ack=%d, checksum=%d \n",packetA.payload,packetA.seqnum, packetA.acknum, packetA.checksum);
  }
  tolayer3(A,packetA);
  A_transport++;
  checkandstarttimer(A,timerA);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    waitingforcallA = 1;
    waitingforackA = 0;
    state = CALL0;
}


/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  if(LOG){
    fprintf(stderr,"AT B\n");
    fprintf(stderr,"Data received at B: data = %s, seq=%d, ack=%d, checksum=%d \n",packet.payload,packet.seqnum, packet.acknum, packet.checksum);
  }
  B_transport++;
  int recvseq = packet.seqnum;
  int recvchecksum = packet.checksum;
  char recvdata[20];
  strncpy(recvdata, packet.payload, 20);
  if(LOG)
  fprintf(stderr,"At B, expseqnoB= %d , expchecksum = %d , recvseq = %d, recvchecksum = %d \n",expseqnoB, checksum(recvdata), recvseq, recvchecksum);
  if(recvseq != expseqnoB || recvchecksum != checksum(recvdata) ){
    if(stateB == WAIT0){
      tolayer3(B,ackpacketB1);
      if(LOG)
        fprintf(stderr,"At B, error: Sending: data = %s, seq=%d, ack=%d, checksum=%d \n\n",ackpacketB1.payload,ackpacketB1.seqnum, ackpacketB1.acknum, ackpacketB1.checksum);
      return;
    }else{
      tolayer3(B,ackpacketB0);
      if(LOG)
        fprintf(stderr,"At B, error: Sending: data = %s, seq=%d, ack=%d, checksum=%d \n\n",ackpacketB0.payload,ackpacketB0.seqnum, ackpacketB0.acknum, ackpacketB0.checksum);

      return;
    }
  }
    tolayer5(B,recvdata);
    B_application++;
    // create an ACK packet. Define ACK as a packet with unit(LSB) bit set to 1
    // to  1




    if(stateB == WAIT0){
      ackpacketB0.seqnum = seqnoB;
      tolayer3(B,ackpacketB0);
      if(LOG)
        fprintf(stderr,"At B, success: Sending B0: data = %s, seq=%d, ack=%d, checksum=%d \n\n",ackpacketB0.payload,ackpacketB0.seqnum, ackpacketB0.acknum, ackpacketB0.checksum);
      stateB = WAIT1;
    }else{
      ackpacketB1.seqnum = seqnoB;
      tolayer3(B,ackpacketB1);
      if(LOG)
        fprintf(stderr,"At B, success: Sending B1: data = %s, seq=%d, ack=%d, checksum=%d \n\n",ackpacketB1.payload,ackpacketB1.seqnum, ackpacketB1.acknum, ackpacketB1.checksum);
      stateB = WAIT0;
    }
    expseqnoB = (expseqnoB+1)%2;
    seqnoB = (seqnoB+1)%2;

}

/* called when B's timer goes off */
void B_timerinterrupt()
{

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  waitingforcallB = 1;
  waitingforackB = -1;
  expseqnoB=0;
  stateB = WAIT0;
  ackpacketB0.acknum = 0;
  ackpacketB1.acknum = 1;
  ackpacketB0.checksum = checksum(ackdata);
  ackpacketB1.checksum = checksum(ackdata);
  strncpy(ackpacketB0.payload,ackdata,20);
  strncpy(ackpacketB1.payload,ackdata,20);
}

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event {
   float evtime;           /* event time */
   int evtype;             /* event type code */
   int eventity;           /* entity where event occurs */
   struct pkt *pktptr;     /* ptr to packet (if any) assoc w/ this event */
   struct event *prev;
   struct event *next;
 };
struct event *evlist = NULL;   /* the event list */

//forward declarations
void init();
void generate_next_arrival();
void insertevent(struct event*);

/* possible events: */
#define  TIMER_INTERRUPT 0
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1



int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */
int nsimmax = 0;           /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob = 0.0;	   /* probability that a packet is dropped */
float corruptprob = 0.0;   /* probability that one bit is packet is flipped */
float lambda = 0.0; 	   /* arrival rate of messages from layer 5 */
int ntolayer3 = 0; 	   /* number sent into layer 3 */
int nlost = 0; 	  	   /* number lost in media */
int ncorrupt = 0; 	   /* number corrupted by media*/

/**
 * Checks if the array pointed to by input holds a valid number.
 *
 * @param  input char* to the array holding the value.
 * @return TRUE or FALSE
 */
int isNumber(char *input)
{
    while (*input){
        if (!isdigit(*input))
            return 0;
        else
            input += 1;
    }

    return 1;
}

int main(int argc, char **argv)
{
   struct event *eventptr;
   struct msg  msg2give;
   struct pkt  pkt2give;

   int i,j;
   char c;

   int opt;
   int seed;

   //Check for number of arguments
   if(argc < 3){
   	fprintf(stderr, "Missing arguments\n");
	printf("Usage: %s -s SEED\n", argv[0]);
   	return -1;
   }

   /*
    * Parse the arguments
    * http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
    */
   while((opt = getopt(argc, argv,"s:w")) != -1){
       switch (opt){
           case 's':   if(!isNumber(optarg)){
                           fprintf(stderr, "Invalid value for -s\n");
                           return -1;
                       }
                       seed = atoi(optarg);
                       break;
	   case 'w':   break;
           case '?':   break;

           default:    printf("Usage: %s -s SEED\n", argv[0]);
                       return -1;
       }
   }

   init(seed);
   A_init();
   B_init();

   while (1) {
        eventptr = evlist;            /* get next event to simulate */
        if (eventptr==NULL)
           goto terminate;
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist!=NULL)
           evlist->prev=NULL;
        if (TRACE>=2) {
           printf("\nEVENT time: %f,",eventptr->evtime);
           printf("  type: %d",eventptr->evtype);
           if (eventptr->evtype==0)
	       printf(", timerinterrupt  ");
             else if (eventptr->evtype==1)
               printf(", fromlayer5 ");
             else
	     printf(", fromlayer3 ");
           printf(" entity: %d\n",eventptr->eventity);
           }
        time = eventptr->evtime;        /* update time to next event time */
        if (nsim==nsimmax)
	  break;                        /* all done with simulation */
        if (eventptr->evtype == FROM_LAYER5 ) {
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */
            j = nsim % 26;
            for (i=0; i<20; i++)
               msg2give.data[i] = 97 + j;
            if (TRACE>2) {
               printf("          MAINLOOP: data given to student: ");
                 for (i=0; i<20; i++)
                  printf("%c", msg2give.data[i]);
               printf("\n");
	     }
            nsim++;
            if (eventptr->eventity == A)
               A_output(msg2give);
             else
               B_output(msg2give);
            }
          else if (eventptr->evtype ==  FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i=0; i<20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
	    if (eventptr->eventity ==A)      /* deliver packet by calling */
   	       A_input(pkt2give);            /* appropriate entity */
            else
   	       B_input(pkt2give);
	    free(eventptr->pktptr);          /* free the memory for packet */
            }
          else if (eventptr->evtype ==  TIMER_INTERRUPT) {
            if (eventptr->eventity == A)
	       A_timerinterrupt();
             else
	       B_timerinterrupt();
             }
          else  {
	     printf("INTERNAL PANIC: unknown event type \n");
             }
        free(eventptr);
        }

terminate:
	//Do NOT change any of the following printfs
	printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n",time,nsim);

	printf("\n");
	printf("Protocol: ABT\n");
	printf("[PA2]%d packets sent from the Application Layer of Sender A[/PA2]\n", A_application);
	printf("[PA2]%d packets sent from the Transport Layer of Sender A[/PA2]\n", A_transport);
	printf("[PA2]%d packets received at the Transport layer of Receiver B[/PA2]\n", B_transport);
	printf("[PA2]%d packets received at the Application layer of Receiver B[/PA2]\n", B_application);
	printf("[PA2]Total time: %f time units[/PA2]\n", time);
	printf("[PA2]Throughput: %f packets/time units[/PA2]\n", B_application/time);
	return 0;
}



void init(int seed)                         /* initialize the simulator */
{
  int i;
  float sum, avg;
  float jimsrand();


   printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
   printf("Enter the number of messages to simulate: ");
   scanf("%d",&nsimmax);
   printf("Enter  packet loss probability [enter 0.0 for no loss]:");
   scanf("%f",&lossprob);
   printf("Enter packet corruption probability [0.0 for no corruption]:");
   scanf("%f",&corruptprob);
   printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
   scanf("%f",&lambda);
   printf("Enter TRACE:");
   scanf("%d",&TRACE);

   srand(seed);              /* init random number generator */
   sum = 0.0;                /* test random number generator for students */
   for (i=0; i<1000; i++)
      sum=sum+jimsrand();    /* jimsrand() should be uniform in [0,1] */
   avg = sum/1000.0;
   if (avg < 0.25 || avg > 0.75) {
    printf("It is likely that random number generation on your machine\n" );
    printf("is different from what this emulator expects.  Please take\n");
    printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
    exit(0);
    }

   ntolayer3 = 0;
   nlost = 0;
   ncorrupt = 0;

   time=0.0;                    /* initialize time to 0.0 */
   generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand()
{
  double mmm = 2147483647;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
  float x;                   /* individual students may need to change mmm */
  x = rand()/mmm;            /* x should be uniform in [0,1] */
  return(x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

void generate_next_arrival()
{
   double x,log(),ceil();
   struct event *evptr;
    //char *malloc();
   float ttime;
   int tempint;

   if (TRACE>2)
       printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

   x = lambda*jimsrand()*2;  /* x is uniform on [0,2*lambda] */
                             /* having mean of lambda        */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time + x;
   evptr->evtype =  FROM_LAYER5;
   if (BIDIRECTIONAL && (jimsrand()>0.5) )
      evptr->eventity = B;
    else
      evptr->eventity = A;
   insertevent(evptr);
}


void insertevent(p)
   struct event *p;
{
   struct event *q,*qold;

   if (TRACE>2) {
      printf("            INSERTEVENT: time is %lf\n",time);
      printf("            INSERTEVENT: future time will be %lf\n",p->evtime);
      }
   q = evlist;     /* q points to header of list in which p struct inserted */
   if (q==NULL) {   /* list is empty */
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
        }
     else {
        for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
              qold=q;
        if (q==NULL) {   /* end of list */
             qold->next = p;
             p->prev = qold;
             p->next = NULL;
             }
           else if (q==evlist) { /* front of list */
             p->next=evlist;
             p->prev=NULL;
             p->next->prev=p;
             evlist = p;
             }
           else {     /* middle of list */
             p->next=q;
             p->prev=q->prev;
             q->prev->next=p;
             q->prev=p;
             }
         }
}

void printevlist()
{
  struct event *q;
  int i;
  printf("--------------\nEvent List Follows:\n");
  for(q = evlist; q!=NULL; q=q->next) {
    printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    }
  printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(AorB)
int AorB;  /* A or B is trying to stop timer */
{
 struct event *q,*qold;

 if (TRACE>2)
    printf("          STOP TIMER: stopping timer at %f\n",time);
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
 for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) {
       /* remove this event */
       if (q->next==NULL && q->prev==NULL)
             evlist=NULL;         /* remove first and only event on list */
          else if (q->next==NULL) /* end of list - there is one in front */
             q->prev->next = NULL;
          else if (q==evlist) { /* front of list - there must be event after */
             q->next->prev=NULL;
             evlist = q->next;
             }
           else {     /* middle of list */
             q->next->prev = q->prev;
             q->prev->next =  q->next;
             }
       free(q);
       return;
     }
  printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


void starttimer(AorB,increment)
int AorB;  /* A or B is trying to stop timer */
float increment;
{

 struct event *q;
 struct event *evptr;
 //char *malloc();

 if (TRACE>2)
    printf("          START TIMER: starting timer at %f\n",time);
 /* be nice: check to see if timer is already started, if so, then  warn */
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
   for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) {
      printf("Warning: attempt to start a timer that is already started\n");
      return;
      }

/* create future event for when timer goes off */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time + increment;
   evptr->evtype =  TIMER_INTERRUPT;
   evptr->eventity = AorB;
   insertevent(evptr);
}


/************************** TOLAYER3 ***************/
void tolayer3(AorB,packet)
int AorB;  /* A or B is trying to stop timer */
struct pkt packet;
{
 struct pkt *mypktptr;
 struct event *evptr,*q;
 //char *malloc();
 float lastime, x, jimsrand();
 int i;


 ntolayer3++;

 /* simulate losses: */
 if (jimsrand() < lossprob)  {
      nlost++;
      if (TRACE>0)
	printf("          TOLAYER3: packet being lost\n");
      return;
    }

/* make a copy of the packet student just gave me since he/she may decide */
/* to do something with the packet after we return back to him/her */
 mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
 mypktptr->seqnum = packet.seqnum;
 mypktptr->acknum = packet.acknum;
 mypktptr->checksum = packet.checksum;
 for (i=0; i<20; i++)
    mypktptr->payload[i] = packet.payload[i];
 if (TRACE>2)  {
   printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
	  mypktptr->acknum,  mypktptr->checksum);
    for (i=0; i<20; i++)
        printf("%c",mypktptr->payload[i]);
    printf("\n");
   }

/* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER3;   /* packet will pop out from layer3 */
  evptr->eventity = (AorB+1) % 2; /* event occurs at other entity */
  evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
 lastime = time;
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
 for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==FROM_LAYER3  && q->eventity==evptr->eventity) )
      lastime = q->evtime;
 evptr->evtime =  lastime + 1 + 9*jimsrand();



 /* simulate corruption: */
 if (jimsrand() < corruptprob)  {
    ncorrupt++;
    if ( (x = jimsrand()) < .75)
       mypktptr->payload[0]='Z';   /* corrupt payload */
      else if (x < .875)
       mypktptr->seqnum = 999999;
      else
       mypktptr->acknum = 999999;
    if (TRACE>0)
	printf("          TOLAYER3: packet being corrupted\n");
    }

  if (TRACE>2)
     printf("          TOLAYER3: scheduling arrival on other side\n");
  insertevent(evptr);
}

void tolayer5(AorB,datasent)
  int AorB;
  char datasent[20];
{
  int i;
  if (TRACE>2) {
     printf("          TOLAYER5: data received: ");
     for (i=0; i<20; i++)
        printf("%c",datasent[i]);
     printf("\n");
   }

}
