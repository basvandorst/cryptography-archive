#ifndef __MACHINE_H__
#define __MACHINE_H__
     
typedef enum _nntp_state {
 st_none,
 st_establish_comm,
 st_group_resp,
 st_article_resp,
 st_rec_article,
 st_post_wait_permission,
 st_post_ok_to_send,
 st_post_wait_end,
 st_list_start,
 st_list_data,
 st_xhdr_request_start,
 st_xhdr_request_data,
 st_xover_start,
 st_xover_data,
 st_xover_check,
 st_closed_comm,
 st_initializing,
 st_confused,
 st_fatal_error,
 st_closing
} nntp_state;

#define ACTIVE_INITIAL_SIZE 131072
#define ARTICLE_INITIAL_SIZE 4096 

int news_machine(news_server_type * server);

#endif /* __MACHINE_H__ */



