/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: PTwipedl.h,v 1.1 1999/03/23 20:44:53 wjb Exp $
____________________________________________________________________________*/

#define VXD_NAME		"\\\\.\\functrap.VXD"

#define VXD_GET_VER 1
#define VXD_REGISTER_CB 2       //register a callback and logging conditions
#define VXD_REC_DONE    3       //release the buffer VxD reported to us
#define VXD_UPDATE 4            //update logging conditions

#define DISABLE_LOGGING 1
#define APP_EXITING 16
#define APP_CANCELED 0x20       //report to VxD that no processing occurred
#define FAKE_AND_WAIT 4
#define FAKE_SUCCESS 2
#define SKIP_ZERO_LEN 0x40

#define IFSFN_OPEN 36           //function code for file opens
#define IFSFN_DELETE 31
#define LOG_ALL 0x0ffffffff
#define LOG_ALL_DOS 0x0fffffffe
#define LOG_ONLY_ERRORS 0xfffe
#define MAX_PATH 260
#define DOS_ACCESS_DENIED_ERR 5

struct	        trap_criteria   {
DWORD           tc_func_num;	//function app wants trapped (-1 = all)
DWORD           tc_drive;       //drive (-1 = all drives)
DWORD           tc_proc_handle; //process (-1 if all processes)
DWORD           tc_vm_num;      //VMs (-1 if all) (0x0fffffffe if only DOS)
WORD            tc_status;      //error code (-1 if all status)
DWORD           tc_callback;    //address to call back
WORD            tc_misc;        //misc flags for logging VxD (bit 0=disable
                                //logging, bit 1=fake the operation, bit 2=
                                //fake the operation and wait until we've
                                //acknowledged the callback (and the status)
                        };



struct  trap_record     {
DWORD           tr_function;   
DWORD           tr_drive;      
unsigned char   tr_ir_flags;   
WORD            tr_ir_options; 
DWORD           tr_res_type; 
DWORD			tr_handle;  
char            tr_file1[MAX_PATH];
char            tr_file2[MAX_PATH];
char            tr_program[9];
DWORD           tr_vm;         
DWORD           tr_proc_handle;
WORD            tr_error;      
DWORD           tr_drv_context;
WORD            tr_drv_status;
WORD            tr_drv_miscflag;

                            };


struct          upcall  {
DWORD           trap_rec_num;
DWORD           trap_dat_ptr;
WORD            trap_dat_len;
                        };


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
