
/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/
#include "config.h"
#include "board.h"
#include "timer.h"
#include "communication.h"
#include "ias.h"
#include "Keypad_driver.h"
#include "lcd.h"
#include "string.h"
#include "eep.h"
#include "ui.h"


//#define SIMULATION
/*
*------------------------------------------------------------------------------
* Structures
*------------------------------------------------------------------------------
*/
typedef struct _ISSUE
{

	UINT8 data[MAX_KEYPAD_ENTRIES + 1];
	UINT16 timeout;
	UINT8 state;
	UINT8 ackStatus;
}ISSUE;


typedef struct _ISSUE_INFO
{
	BOOL set;
	ISSUE_TYPE issueType;
	UINT8 depID;

}ISSUE_INFO;																			//Object to store data of raised issue

typedef struct _LOG
{
	UINT8 prevIndex;
	UINT8 index;
	UINT8 entries[MAX_LOG_ENTRIES][LOG_BUFF_SIZE];
}LOG;																			//Object to store log entries

typedef struct _IAS
{
	UINT8 state;
	UINT8 issues_raised, issues_critical;
	UINT32 preAppTime,curAppTime;
	UINT8 stationCount[2];
	UINT8 password[5];
	UINT8 logonPassword[5];
	ISSUE issues[MAX_ISSUES];
	UINT8 logonStatus;
	UINT8 openIssue;
	INT8 buzzerTimeout;
}IAS;																			//This object contains all the varibles used in this application



typedef enum 
{
	ISSUE_ENTRY_DATA_ADDR = 0,
	ISSUE_TIMEOUT = MAX_KEYPAD_ENTRIES + 1,
	ISSUE_STATE =  ISSUE_TIMEOUT + 2,
	ISSUE_ACKSTATUS = ISSUE_STATE+1,
	ISSUE_ENTRY_SIZE = sizeof(ISSUE)

};

/*
*------------------------------------------------------------------------------
* Variables
*------------------------------------------------------------------------------
*/
#pragma idata iasdata
IAS ias = {0};
#pragma idata

#pragma idata logdata
ISSUE_INFO issueInfo = {FALSE,0,0};
LOG log = {0};
#pragma idata

static rom UINT16 timeout = (UINT16)35;

/*------------------------------------------------------------------------------
* Private Functions
*------------------------------------------------------------------------------
*/

static void updateLog(far UINT8* data);
static void updateIndication(BOOL issueResolved,BOOL issueRaised,BOOL issueCritical,BOOL buzIndication);	//Switches the indicators
INT8 issueResolved(far UINT8* data);
static BOOL update_timeouts(void);

void resolveIssue(far UINT8* data);
UINT8 getStatusLog(far UINT8** logBuff);
void login(void);
UINT8 APP_comCallBack( far UINT8 *rxPacket, far UINT8* txCode,far UINT8** txPacket);


/*
*------------------------------------------------------------------------------
* void IAS-init(void)
*------------------------------------------------------------------------------
*/

void IAS_init(void)
{

	UINT16 i;
	UINT8 j,*ptr;
	UINT8 ackStatus = 0;
	UINT8 grn =0 ,org = 0,red = 0,buz = 0; 

#ifdef __FACTORY_CONFIGURATION__

	ias.state = ISSUE_RESOLVED;
	ias.issues_raised = 0;														//No. of raised issues should be 0 initially
	ias.issues_critical = 0;													//No. of critical issues should be 0 initially
	ias.preAppTime = 0;
	ias.curAppTime = 0;
	log.index = 0;
	ias.logonStatus = 0;
	updateIndication(1,0,0,0);
	for(i = 0; i < 256; i++)
	{
		Write_b_eep(i,0);
		Busy_eep();
		ClrWdt();

	}

#else

	ias.preAppTime = 0;
	ias.curAppTime = 0;
	log.index = 0;

	for( i = 0; i < MAX_ISSUES; i++)
	{
		ptr = (UINT8*) &ias.issues[i];
		for( j = 0 ; j < ISSUE_ENTRY_SIZE; j++)
		{
			Busy_eep();
			ClrWdt();
			*(ptr+j) = Read_b_eep((i*ISSUE_ENTRY_SIZE)+j );

			Busy_eep();
			ClrWdt();

		}

		if( ias.issues[i].state == ISSUE_RAISED)
			ias.issues_raised++;
		if(ias.issues[i].state == ISSUE_CRITICAL)
			ias.issues_critical++;
		if(ias.issues[i].ackStatus == 1)
			ackStatus = 1;
	}

	if( ias.issues_critical > 0 )
		red = 1;
	else if( ias.issues_raised > 0 )
		org = 1;
	else grn = 1;
	
	if( ackStatus == 1 )
		buz = 1;


	ias.buzzerTimeout = 6;
	Busy_eep();
	ClrWdt();

	updateIndication(0,0,0,0);
	
	
	login();
	ias.logonStatus = 1;
	
	updateIndication(grn,org,red,buz);

#endif

COM_init(CMD_SOP , CMD_EOP ,RESP_SOP , RESP_EOP , APP_comCallBack);



}


void login()
{
	UINT8 grn =0 ,org = 0,red = 0,buz = 0; 
	UINT8 i;
	
	if( ias.issues_critical > 0 )
	{
		ias.state = ISSUE_CRITICAL;
		red = 1;
	}
	else if( ias.issues_raised > 0 )
	{
		ias.state = ISSUE_RAISED;
		org = 1;
	}
	else
	{
		ias.state = ISSUE_RESOLVED;	
		grn = 1;
		buz = 0;
	}
	for( i = 0; i < MAX_ISSUES; i++)
	{
		if(ias.issues[i].ackStatus == 1)
		{
			buz = 1;
			break;
		}
	}


	updateIndication(grn,org,red,buz);
}

BOOL IAS_login(far UINT8 *password,far UINT8 *data)
{
	UINT8 grn =0 ,org = 0,red = 0,buz = 0; 
	UINT8 i;

//	if( strcmp(ias.logonPassword , password) )
//		return FALSE;


	if( ias.issues_critical > 0 )
	{
		ias.state = ISSUE_CRITICAL;
		red = 1;
	}
	else if( ias.issues_raised > 0 )
	{
		ias.state = ISSUE_RAISED;
		org = 1;
	}
	else
	{
		ias.state = ISSUE_RESOLVED;	
		grn = 1;
		buz = 0;
	}

//	for( i = 0; i < MAX_ISSUES; i++)
//	{
//		if(ias.issues[i].ackStatus == 1)
//		{
//			buz = 1;
//			break;
//		}
//	}


	updateIndication(grn,org,red,buz);

//	updateLog(data);

//	Write_b_eep(LOGON_STATUS, 1);
//	Busy_eep();
//	ClrWdt();

	ias.logonStatus = 1;

	return TRUE;
}


BOOL IAS_logout(far UINT8 *password,far UINT8 *data)
{
	UINT8 grn =0 ,org = 0,red = 0,buz = 0; 
	UINT8 i;
	if( strcmp(ias.logonPassword , password) )
		return FALSE;


	updateIndication(grn,org,red,buz);

	updateLog(data);
/*
	Write_b_eep(LOGON_STATUS, 0);
	Busy_eep();
	ClrWdt();
*/
	ias.logonStatus = 0;
	return TRUE;
}




/*
*------------------------------------------------------------------------------
* void IAS-task(void)
*------------------------------------------------------------------------------
*/
void IAS_task(void)
{
	UINT8 i,*ptr, data;

	UINT32 addr;
	UINT8 resetBuzzer = TRUE;
	ias.curAppTime = GetAppTime();											//Fetches the application time from timer driver
	if(ias.preAppTime != ias.curAppTime)
	{
		ias.preAppTime = ias.curAppTime;
		for(i = 0; i <MAX_ISSUES; i++)										//check for timeout of issues raised
		{
			if(ias.issues[i].state == ISSUE_RAISED )
			{
				ias.issues[i].timeout -= 1;

			

				ptr = (UINT8*)&ias.issues[i].timeout;

				data = *ptr;

				addr = (i*ISSUE_ENTRY_SIZE)+ISSUE_TIMEOUT;
				Write_b_eep((i*ISSUE_ENTRY_SIZE)+ISSUE_TIMEOUT, *ptr);
				Busy_eep();
				ClrWdt();

				data=*(ptr+1);
				addr = (i*ISSUE_ENTRY_SIZE)+ISSUE_TIMEOUT+1;
				Write_b_eep((i*ISSUE_ENTRY_SIZE)+ISSUE_TIMEOUT+1, *(ptr+1));
				Busy_eep();
				ClrWdt();

			}
		}

		if(update_timeouts() == TRUE)
		{
			LAMP_RED = 1;
			if( ias.issues_raised == 0 )
			{
				LAMP_YELLOW = 0;
				LAMP_GREEN = 0;
			}
		}

	}

}


void IAS_getOpenIssue(far OpenIssue *openIssue)
{
	UINT8 i =openIssue->ID+1, j=0,k=0,l;

	if( i >= MAX_ISSUES )
		i = 0;
	
	openIssue->ID = -1;

	for(l=0; l < MAX_ISSUES; l++,i++)
	{
		i = i%MAX_ISSUES;
		if( (ias.issues[i].state == ISSUE_RESOLVED) || (ias.issues[i].ackStatus == 0 ))
			continue;
		openIssue->tag[j++] = 'S';openIssue->tag[j++] = 'T';openIssue->tag[j++] = 'N';openIssue->tag[j++] = ':';
		openIssue->tag[j++] = ias.issues[i].data[k++];openIssue->tag[j++] =ias.issues[i].data[k++];openIssue->tag[j++] = ';';
		
		switch( ias.issues[i].data[k] )
		{
			case '1': 
				openIssue->tag[j++] = 'B';openIssue->tag[j++] = 'R';openIssue->tag[j++] = 'E';openIssue->tag[j++] = 'A';openIssue->tag[j++] = 'K';openIssue->tag[j++] = 'D';
				openIssue->tag[j++] = 'O';openIssue->tag[j++] = 'W';openIssue->tag[j++] = 'N';openIssue->tag[j++]= ';';
				//openIssue->tag[j++] = 'C';openIssue->tag[j++] = 'L';openIssue->tag[j++] = 'A';openIssue->tag[j++] = 'S';openIssue->tag[j++] = 'S';openIssue->tag[j++] = ':';
				
				break;

			case '2': 
				openIssue->tag[j++] = 'Q';openIssue->tag[j++] = 'U';openIssue->tag[j++] = 'A';openIssue->tag[j++] = 'L';openIssue->tag[j++] = 'I';openIssue->tag[j++] = 'T';
				openIssue->tag[j++] = 'Y';openIssue->tag[j++]= ';';
				//openIssue->tag[j++] = 'C';openIssue->tag[j++] = 'L';openIssue->tag[j++] = 'A';openIssue->tag[j++] = 'S';openIssue->tag[j++] = 'S';openIssue->tag[j++] = ':';
				//openIssue->tag[j++] = ias.issues[i].data[3];openIssue->tag[j++] = ias.issues[i].data[4];
				break;	
			case '3':
				openIssue->tag[j++]='#';
				break;
			case '4': 
				openIssue->tag[j++] = 'O';openIssue->tag[j++]= 'T';openIssue->tag[j++]= 'H';openIssue->tag[j++]= 'E';openIssue->tag[j++]= 'R';openIssue->tag[j++]= 'S';
				openIssue->tag[j++]= ';';
				break;			
		}
		++k;
		while( ias.issues[i].data[k] !='\0')
		{
			openIssue->tag[j] = ias.issues[i].data[k];
			j++;
			k++;
		}
		openIssue->tag[j] = '\0';
		openIssue->ID = ias.openIssue = i;

		break;
	}
	
}

void IAS_getAcknowledgedIssue(far OpenIssue *openIssue)
{
	UINT8 i =openIssue->ID+1, j=0,k=0,l;

	if( i >= MAX_ISSUES )
		i = 0;
	
	openIssue->ID = -1;

	for(l=0; l < MAX_ISSUES; l++,i++)
	{
		i = i%MAX_ISSUES;
		if( (ias.issues[i].state == ISSUE_RESOLVED) || (ias.issues[i].ackStatus == 1 ) )
			continue;
		openIssue->tag[j++] = 'S';openIssue->tag[j++] = 'T';openIssue->tag[j++] = 'N';openIssue->tag[j++] = ':';
		openIssue->tag[j++] = ias.issues[i].data[k++];openIssue->tag[j++] =ias.issues[i].data[k++];openIssue->tag[j++] = ';';
		
		switch( ias.issues[i].data[k] )
		{
			case '1': 
				openIssue->tag[j++] = 'B';openIssue->tag[j++] = 'R';openIssue->tag[j++] = 'E';openIssue->tag[j++] = 'A';openIssue->tag[j++] = 'K';openIssue->tag[j++] = 'D';
				openIssue->tag[j++] = 'O';openIssue->tag[j++] = 'W';openIssue->tag[j++] = 'N';openIssue->tag[j++]= ';';
				//openIssue->tag[j++] = 'C';openIssue->tag[j++] = 'L';openIssue->tag[j++] = 'A';openIssue->tag[j++] = 'S';openIssue->tag[j++] = 'S';openIssue->tag[j++] = ':';
				
				break;

			case '2': 
				openIssue->tag[j++] = 'Q';openIssue->tag[j++] = 'U';openIssue->tag[j++] = 'A';openIssue->tag[j++] = 'L';openIssue->tag[j++] = 'I';openIssue->tag[j++] = 'T';
				openIssue->tag[j++] = 'Y';openIssue->tag[j++]= ';';
				//openIssue->tag[j++] = 'C';openIssue->tag[j++] = 'L';openIssue->tag[j++] = 'A';openIssue->tag[j++] = 'S';openIssue->tag[j++] = 'S';openIssue->tag[j++] = ':';
				//openIssue->tag[j++] = ias.issues[i].data[3];openIssue->tag[j++] = ias.issues[i].data[4];
				break;	
			case '3':
				openIssue->tag[j++]='#';
				break;
			case '4': 
				openIssue->tag[j++] = 'O';openIssue->tag[j++]= 'T';openIssue->tag[j++]= 'H';openIssue->tag[j++]= 'E';openIssue->tag[j++]= 'R';openIssue->tag[j++]= 'S';
				openIssue->tag[j++]= ';';
				break;			
		}
		++k;
		while( ias.issues[i].data[k] !='\0')
		{
			openIssue->tag[j] = ias.issues[i].data[k];
			j++;
			k++;
		}
		openIssue->tag[j] = '\0';
		openIssue->ID = ias.openIssue = i;

		break;
	}
	
}




void IAS_resolveIssues(UINT8 id)
{
	UINT8 i,j,issueIndex;
	UINT8 *ptr;
	far UINT8* data;


								
		if(ias.issues[id].state != ISSUE_RESOLVED )						//if it is raised
			if(strcmp((INT8*)data,(INT8 *)ias.issues[id].data) == 0)		//if input matches
				if(ias.issues[id].ackStatus == 1) 						//check whether acknowledged
					return;												//if not return

	data = ias.issues[id].data;

	switch(ias.state)
	{

		case ISSUE_RAISED:
			issueIndex = issueResolved(data);
			if(  issueIndex != -1)
			{
				updateLog(data);
				memset(ias.issues[issueIndex].data , 0 , MAX_KEYPAD_ENTRIES + 1);
							
				ClrWdt();
				
				ptr = (UINT8*)&ias.issues[issueIndex];
				for( j = 0; j < ISSUE_ENTRY_SIZE; j++)
				{
					Write_b_eep(j+(issueIndex*ISSUE_ENTRY_SIZE),*(ptr+j));
					Busy_eep();
					
					ClrWdt();
				}
				
				
				if(ias.issues_raised == 0)
				{
					ias.state = ISSUE_RESOLVED;
	
					updateIndication(1,0,0,0);
				}
				return;
			}
	
		break;

		case ISSUE_CRITICAL:
			issueIndex = issueResolved(data);
			if(  issueIndex != -1)
			{
				updateLog(data);
				memset(ias.issues[issueIndex].data , 0 , MAX_KEYPAD_ENTRIES + 1);
							
				ClrWdt();
				
				ptr = (UINT8*)&ias.issues[issueIndex];
				for( j = 0; j < ISSUE_ENTRY_SIZE; j++)
				{
					Write_b_eep(j+(issueIndex*ISSUE_ENTRY_SIZE),*(ptr+j));
					Busy_eep();
					
					ClrWdt();
				}
				
				
				if(ias.issues_critical == 0)
				{
					if(ias.issues_raised == 0)
					{
						ias.state = ISSUE_RESOLVED;
						updateIndication(1,0,0,0);
					}
					else
					{
						ias.state = ISSUE_RAISED;
						updateIndication(0,1,0,0);
					}
				}
				else
				
				{
                     if(ias.issues_raised == 0)
					{
					//	ias.state = ISSUE_RESOLVED;
						updateIndication(0,0,1,0);
					}
					else
					{
						//ias.state = ISSUE_RAISED;
						updateIndication(0,1,1,0);
					}
				
     			}                            
		
				return;
			}
		

		break;

		default:
		break;

	}
for(i=0;i<MAX_ISSUES;i++)
{
if(ias.issues[i].ackStatus==1)
{
 BUZ_OP=1;
 return;
}
}

}

void IAS_raiseIssues(far UINT8* data)
{
	UINT8 i,j,issueIndex;
	UINT8 *ptr;



	for(i = 0; i < MAX_ISSUES; i++)										//for each issue
		if(ias.issues[i].state != ISSUE_RESOLVED )						//if it is raised
			if(strcmp((INT8*)data,(INT8 *)ias.issues[i].data) == 0)		//if input matches				
					return;												//do nothing


	for( i = 0 ; i < MAX_ISSUES ;i++)
	{
		if( ias.issues[i].state == ISSUE_RESOLVED )
		{
			ias.issues[i].state = ISSUE_RAISED;
			ias.issues[i].timeout =  timeout;
			ias.issues[i].ackStatus = 1;
			strcpy((INT8*) ias.issues[i].data, (INT8*)data);

			ptr = (UINT8*)&ias.issues[i];
			for(j = 0; j < ISSUE_ENTRY_SIZE; j++)
			{
				Write_b_eep(j+(i*ISSUE_ENTRY_SIZE),*(ptr+j));
				Busy_eep();
				ClrWdt();
			}

			ias.issues_raised++;
            if(ias.issues_critical == 0)
            {
			  updateIndication(0,1,0,1);
            }
            else
            {
			  updateIndication(0,1,1,1);
			}
			updateLog(data);
			break;
		}
	}

	if( ias.issues_critical ==  0 )
		ias.state = ISSUE_RAISED;


}



void resolveIssue(far UINT8* data)
{
	UINT8 i,j,issueIndex;
	UINT8 *ptr;
	if( data[2] == '0')
	{
		updateLog(data);
		return;
	}



	for(i = 0; i < MAX_ISSUES; i++)										//for each issue
		if(ias.issues[i].state != ISSUE_RESOLVED )						//if it is raised
			if(strcmp((INT8*)data,(INT8 *)ias.issues[i].data) == 0)		//if input matches
				if(ias.issues[i].ackStatus == 1) 						//check whether acknowledged
					return;												//if not return


	switch(ias.state)
	{
	

		case ISSUE_RAISED:

		for(i = 0; i< MAX_ISSUES ; i++)
		{
			issueIndex = issueResolved(data);
			if(  issueIndex != -1)
			{
				//updateLog(data);
				memset(ias.issues[issueIndex].data , 0 , MAX_KEYPAD_ENTRIES + 1);
							
				ClrWdt();
				
				ptr = (UINT8*)&ias.issues[issueIndex];
				for( j = 0; j < ISSUE_ENTRY_SIZE; j++)
				{
					Write_b_eep(j+(issueIndex*ISSUE_ENTRY_SIZE),*(ptr+j));
					Busy_eep();
					
					ClrWdt();
				}
				
				
				if(ias.issues_raised == 0)
				{
					ias.state = ISSUE_RESOLVED;
	
					updateIndication(1,0,0,0);
				}
				return;
			}
		}

		
	


		break;

		case ISSUE_CRITICAL:

		for(i = 0; i< MAX_ISSUES ; i++)
		{
			issueIndex = issueResolved(data);
			if(  issueIndex != -1)
			{
				//updateLog(data);
				memset(ias.issues[issueIndex].data , 0 , MAX_KEYPAD_ENTRIES + 1);
							
				ClrWdt();
				
				ptr = (UINT8*)&ias.issues[issueIndex];
				for( j = 0; j < ISSUE_ENTRY_SIZE; j++)
				{
					Write_b_eep(j+(issueIndex*ISSUE_ENTRY_SIZE),*(ptr+j));
					Busy_eep();
					
					ClrWdt();
				}
				
				
				if(ias.issues_critical == 0)
				{
				   	if(ias.issues_raised == 0)
					{
						ias.state = ISSUE_RESOLVED;
						updateIndication(1,0,0,0);
					}
					else
					{
						ias.state = ISSUE_RAISED;
						updateIndication(0,1,0,0);
					}
				}
				else
				
				{
                    if(ias.issues_raised == 0)
					{
					//	ias.state = ISSUE_RESOLVED;
						updateIndication(0,0,1,0);
					}
					else
					{
						//ias.state = ISSUE_RAISED;
						updateIndication(0,1,1,0);
					}
				
     			}                            
				return;
			}
		}

		break;

		default:
		break;

	}

}










/*---------------------------------------------------------------------------------------------------------------
*	UINT8 getStatusLog(UINT8** logBuff)
*
*----------------------------------------------------------------------------------------------------------------
*/

UINT8 getStatusLog(far UINT8** logBuff)
{
	UINT8 i;
	UINT8 length;
	if(log.prevIndex == log.index)
	{
		*logBuff = 0;
		 length = 0;
	}
	else
	{
		*logBuff = log.entries[log.prevIndex];
		length = strlen(log.entries[log.prevIndex]);
		log.prevIndex++;
		if( log.prevIndex >= MAX_LOG_ENTRIES )
			log.prevIndex = 0;
	}
	
	return(length);
}





/*----------------------------------------------------------------------------------------------------------------
*	void IAS_updateIssueInfo( UINT8 depId , ISSUE_TYPE issueType)
* Function to update the issue info object
*----------------------------------------------------------------------------------------------------------------
*/


BOOL IAS_updateIssueInfo( UINT8 depId , ISSUE_TYPE issueType)
{
	if( issueInfo.set == TRUE)	//if there is a previous cmd pending
	{
		return FALSE;
	}
	issueInfo.set = TRUE;
	issueInfo.depID = depId,
	issueInfo.issueType = issueType;
	return TRUE;
}



void IAS_acknowledgeIssues(UINT8 ID)
{
	UINT8 i;
	ias.issues[ID].ackStatus = 0;

	updateLog(ias.issues[ID].data);

	


	Write_b_eep((ID*ISSUE_ENTRY_SIZE)+ISSUE_ACKSTATUS, ias.issues[ID].ackStatus);
	Busy_eep();
	ClrWdt();

	//IAS_updateIssues(ias.issues[ID].data);

	for( i = 0 ; i < MAX_ISSUES ;i++)
	{
		if( ias.issues[i].ackStatus == 1)
			return;
	}
	BUZ_OP = 0;
			
	
}




/*---------------------------------------------------------------------------------------------------------------
*	void updateLog(void)
*----------------------------------------------------------------------------------------------------------------
*/
void updateLog(far UINT8 *data)
{
	UINT8 i = 0;
	while( *data != '\0')
	{
		log.entries[log.index][i] = *data;
		i++;
		data++;
	}
	log.entries[log.index][i]= '\0';
	log.index++;
	if( log.index >= MAX_LOG_ENTRIES)
		log.index = 0;
}



/*---------------------------------------------------------------------------------------------------------------
*	void updateIndication(BOOL issueResolved,BOOL issueRaised,BOOL issueCritical,BOOL buzIndication)
*----------------------------------------------------------------------------------------------------------------
*/
void updateIndication(BOOL issueResolved,BOOL issueRaised,BOOL issueCritical,BOOL buzIndication)
{
	LAMP_GREEN = issueResolved;
	LAMP_YELLOW = issueRaised;
	LAMP_RED = issueCritical;
	BUZ_OP = buzIndication;
}

/*---------------------------------------------------------------------------------------------------------------
*	void issueResolved(void)
*----------------------------------------------------------------------------------------------------------------
*/
INT8 issueResolved(far UINT8* data)
{
	UINT8 i,j,*ptr;

	for(i = 0; i < MAX_ISSUES; i++)
	{
		if(ias.issues[i].state != ISSUE_RESOLVED && (ias.issues[i].ackStatus != 1) )
		{
			if(strcmp((INT8*)data,(INT8 *)ias.issues[i].data) == 0)
			{
				if(ias.issues[i].state == ISSUE_RAISED)
					ias.issues_raised--;
				else if(ias.issues[i].state == ISSUE_CRITICAL)
					ias.issues_critical--;

				ias.issues[i].state = ISSUE_RESOLVED;
				ias.issues[i].timeout = 0;
				ias.issues[i].ackStatus = 0;
			
				
				return i;
			}
		}
	}
	return -1;
}

/*---------------------------------------------------------------------------------------------------------------
*	BOOL update_timeouts(void)
*----------------------------------------------------------------------------------------------------------------
*/
BOOL update_timeouts(void)
{
	UINT8 i;
	BOOL result = FALSE;
	for(i = 0; i <MAX_ISSUES; i++)						//check for timeout of issues raised
	{
		if(ias.issues[i].state == ISSUE_RAISED)
		{
			if(ias.issues[i].timeout == 0)
			{
				ias.issues_critical++;
				ias.issues_raised--;
				ias.issues[i].state = ISSUE_CRITICAL;
				
				Write_b_eep((i*ISSUE_ENTRY_SIZE)+ISSUE_STATE,ias.issues[i].state);
				Busy_eep();
				ClrWdt();
		

				ias.state = ISSUE_CRITICAL;//put up
				result = TRUE;
			}
		}
	}
	return result;
}



void acknowledgeIssue(far UINT8 *data)
{
	UINT8 i;
	for(i = 0; i < MAX_ISSUES; i++)										//for each issue
		if(ias.issues[i].state != ISSUE_RESOLVED )						//if it is active
			if(strcmp((INT8*)data,(INT8 *)ias.issues[i].data) == 0)		//if data matches
				ias.issues[i].ackStatus = 0;							//acknowledge 
}						

BOOL IAS_checkPassword(UINT8 *password)
{

	if( strcmp(ias.password , password) )
		return FALSE;
	return TRUE;
}






void IAS_clearIssues()
{

	UINT16 i;

	ias.state = ISSUE_RESOLVED;
	ias.issues_raised = 0;														//No. of raised issues should be 0 initially
	ias.issues_critical = 0;													//No. of critical issues should be 0 initially
	ias.preAppTime = 0;
	ias.curAppTime = 0;
	ias.openIssue = 0;
	log.index = 0;
	log.prevIndex = 0;


	for( i = 0 ; i < MAX_ISSUES; i++)
	{
		ias.issues[i].state = ISSUE_RESOLVED;
		ias.issues[i].timeout = 0;
		ias.issues[i].ackStatus = 0;
		memset(ias.issues[i].data , 0 , MAX_KEYPAD_ENTRIES + 1);
		ClrWdt();
	}



	for(i = 0; i < (ISSUE_ENTRY_SIZE * MAX_ISSUES); i++)
	{
		Write_b_eep(i,0);
		Busy_eep();
		ClrWdt();

	}

	if( ias.logonStatus == 1 )
	{
		updateIndication(1,0,0,0);
	}
	else
		updateIndication(0,0,0,0);


	
}


UINT8 APP_comCallBack( far UINT8 *rxPacket, far UINT8* txCode,far UINT8** txPacket)
{

	UINT8 i;

	UINT8 rxCode = rxPacket[0];
	UINT8 length = 0;

	switch( rxCode )
	{
		case CMD_GET_STATUS:
			length = getStatusLog(txPacket);
			*txCode = CMD_GET_STATUS;
			break;

		case CMD_RESOLVE_ISSUE:
			acknowledgeIssue(&rxPacket[1]);
			resolveIssue(&rxPacket[1]);
			*txCode = CMD_RESOLVE_ISSUE;
			length = 0;
			
			break;

		case CMD_CLEAR_ISSUES:
			IAS_clearIssues() ;
			*txCode = CMD_CLEAR_ISSUES;
			length = 0;
			
			break;

	

		case CMD_PING:
			length = 0;
			*txCode = CMD_PING;
			break;

		default:
			length = 0;
			*txCode = COM_RESP_INVALID_CMD;
			break;

	}

	return length;

}
	
		

		
	

