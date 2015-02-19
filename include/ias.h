#ifndef _APC_IAS_H_
#define _APC_IAS_H_


/*
*----------------------------------------------------------------------------------------------------------------
*	MACROS
*-----------------------------------------------------------------------------------------------------------------
*/

//#define __FACTORY_CONFIGURATION__


/*
*----------------------------------------------------------------------------------------------------------------
*	Enumerations
*-----------------------------------------------------------------------------------------------------------------
*/
typedef enum 
{
	OFF,
	ON
}INDICATOR_STATE;

typedef enum _ISSUE_TYPE
{
	NO_ISSUE,
	RAISED,
	RESOLVED
}ISSUE_TYPE;



typedef enum _IAS_PARAM
{
	MAX_KEYPAD_ENTRIES = 24,
	MAX_ISSUES = 12,
	MAX_DEPARTMENTS = 20,
	MAX_LOG_ENTRIES = 2,
	LOG_BUFF_SIZE = MAX_KEYPAD_ENTRIES+1

}IAS_PARAM;

typedef enum _LOGDATA
{
	HW_TMEOUT = 10,
	APP_TIMEOUT = 1000,
	TIMESTAMP_UPDATE_VALUE = (APP_TIMEOUT/HW_TMEOUT)
}LOGDATA;

typedef enum
{
	ISSUE_RESOLVED,
	ISSUE_RAISED,
	ISSUE_ACKNOWLEDGED,
	ISSUE_CRITICAL
}IAS_STATE;

enum
{
	CMD_GET_STATUS = 0x80,
	CMD_GET_ADMIN_PASSWORD = 0x81,
	CMD_GET_LOGON_PASSWORD = 0x82,
	CMD_GET_BUZZER_TIMEOUT = 0x83,


	CMD_SET_ADMIN_PASSWORD = 0x91,
	CMD_SET_LOGON_PASSWORD = 0x92,
	CMD_SET_BUZZER_TIMEOUT = 0x93,


	CMD_PING = 0xA0,
	CMD_CLEAR_ISSUES = 0xA1,
	CMD_RESOLVE_ISSUE = 0xA2

	
};

typedef struct _OpenIssue
{
	UINT8 tag[32];
	INT8 ID;
}OpenIssue;

extern void IAS_init(void);
extern void IAS_task(void);
extern BOOL  IAS_updateIssueInfo( UINT8 depId , ISSUE_TYPE issueType);
void IAS_raiseIssues(far UINT8* data);
void IAS_acknowledgeIssues(UINT8 ID);
void IAS_resolveIssues(UINT8 id);
void IAS_clearIssues(void);
BOOL IAS_checkPassword(UINT8 *password);
BOOL IAS_login(UINT8 *password,UINT8 *data);
BOOL IAS_logout(UINT8 *password,UINT8 *data);
void IAS_getOpenIssue(OpenIssue *);
void IAS_getAcknowledgedIssue(far OpenIssue *openIssue);
#endif