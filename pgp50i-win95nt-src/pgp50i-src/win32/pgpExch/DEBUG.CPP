/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */

#include "stdinc.h"
#include "pgpexch.h"
#include "assert.h"


#if defined(_DEBUG)
void DumpPropTag(ULONG ulPropTag)
{
    char szUnkTag[32];
    char * pszTag;
    switch (PROP_ID(ulPropTag))
	{

	// Message envelope properties

	case PROP_ID(PR_ACKNOWLEDGEMENT_MODE) :
		pszTag = "PR_ACKNOWLEDGEMENT_MODE";
		break;

    case PROP_ID(PR_ALTERNATE_RECIPIENT_ALLOWED) :
		pszTag = "PR_ALTERNATE_RECIPIENT_ALLOWED";
		break;

    case PROP_ID(PR_AUTHORIZING_USERS) :
		pszTag = "PR_AUTHORIZING_USERS";
		break;

      case PROP_ID(PR_AUTO_FORWARD_COMMENT) :
		pszTag = "PR_AUTO_FORWARD_COMMENT";
		break;

    case PROP_ID(PR_AUTO_FORWARDED) :
		pszTag = "PR_AUTO_FORWARDED";
		break;

    case PROP_ID(PR_CONTENT_CONFIDENTIALITY_ALGORITHM_ID) :
		pszTag = "PR_CONTENT_CONFIDENTIALITY_ALGORITHM_ID";
		break;

    case PROP_ID(PR_CONTENT_CORRELATOR) :
		pszTag = "PR_CONTENT_CORRELATOR";
		break;

    case PROP_ID(PR_CONTENT_IDENTIFIER) :
		pszTag = "PR_CONTENT_IDENTIFIER";
		break;

    case PROP_ID(PR_CONTENT_LENGTH) :
		pszTag = "PR_CONTENT_LENGTH";
		break;

    case PROP_ID(PR_CONTENT_RETURN_REQUESTED) :
		pszTag = "PR_CONTENT_RETURN_REQUESTED";
		break;

    case PROP_ID(PR_CONVERSATION_KEY) :
		pszTag = "PR_CONVERSATION_KEY";
		break;

    case PROP_ID(PR_CONVERSION_EITS) :
		pszTag = "PR_CONVERSION_EITS";                              break;
    case PROP_ID(PR_CONVERSION_WITH_LOSS_PROHIBITED) :
		pszTag = "PR_CONVERSION_WITH_LOSS_PROHIBITED";              break;
    case PROP_ID(PR_CONVERTED_EITS) :
		pszTag = "PR_CONVERTED_EITS";                               break;
    case PROP_ID(PR_DEFERRED_DELIVERY_TIME) :
		pszTag = "PR_DEFERRED_DELIVERY_TIME";                       break;
    case PROP_ID(PR_DELIVER_TIME) :
		pszTag = "PR_DELIVER_TIME";                                 break;
    case PROP_ID(PR_DISCARD_REASON) :
		pszTag = "PR_DISCARD_REASON";                               break;
    case PROP_ID(PR_DISCLOSURE_OF_RECIPIENTS) :
		pszTag = "PR_DISCLOSURE_OF_RECIPIENTS";                     break;
    case PROP_ID(PR_DL_EXPANSION_HISTORY) :
		pszTag = "PR_DL_EXPANSION_HISTORY";                         break;
    case PROP_ID(PR_DL_EXPANSION_PROHIBITED) :
		pszTag = "PR_DL_EXPANSION_PROHIBITED";                      break;
    case PROP_ID(PR_EXPIRY_TIME) :
		pszTag = "PR_EXPIRY_TIME";                                  break;
    case PROP_ID(PR_IMPLICIT_CONVERSION_PROHIBITED) :
		pszTag = "PR_IMPLICIT_CONVERSION_PROHIBITED";               break;
    case PROP_ID(PR_IMPORTANCE) :
		pszTag = "PR_IMPORTANCE";                                   break;
    case PROP_ID(PR_IPM_ID) :
		pszTag = "PR_IPM_ID";                                       break;
    case PROP_ID(PR_LATEST_DELIVERY_TIME) :
		pszTag = "PR_LATEST_DELIVERY_TIME";                         break;
    case PROP_ID(PR_MESSAGE_CLASS) :
		pszTag = "PR_MESSAGE_CLASS";                                break;
    case PROP_ID(PR_MESSAGE_DELIVERY_ID) :
		pszTag = "PR_MESSAGE_DELIVERY_ID";                          break;
    case PROP_ID(PR_MESSAGE_SECURITY_LABEL) :
		pszTag = "PR_MESSAGE_SECURITY_LABEL";                       break;
    case PROP_ID(PR_OBSOLETED_IPMS) :
		pszTag = "PR_OBSOLETED_IPMS";                               break;
    case PROP_ID(PR_ORIGINALLY_INTENDED_RECIPIENT_NAME) :
		pszTag = "PR_ORIGINALLY_INTENDED_RECIPIENT_NAME";           break;
    case PROP_ID(PR_ORIGINAL_EITS) :
		pszTag = "PR_ORIGINAL_EITS";                                break;
    case PROP_ID(PR_ORIGINATOR_CERTIFICATE) :
		pszTag = "PR_ORIGINATOR_CERTIFICATE";                       break;
    case PROP_ID(PR_ORIGINATOR_DELIVERY_REPORT_REQUESTED) :
		pszTag = "PR_ORIGINATOR_DELIVERY_REPORT_REQUESTED";         break;
    case PROP_ID(PR_ORIGINATOR_RETURN_ADDRESS) :
		pszTag = "PR_ORIGINATOR_RETURN_ADDRESS";                    break;
    case PROP_ID(PR_PARENT_KEY) :
		pszTag = "PR_PARENT_KEY";                                   break;
     case PROP_ID(PR_PRIORITY) :
		pszTag = "PR_PRIORITY";                                     break;
     case PROP_ID(PR_ORIGIN_CHECK) :
		pszTag = "PR_ORIGIN_CHECK";                                 break;
     case PROP_ID(PR_PROOF_OF_SUBMISSION_REQUESTED) :
		pszTag = "PR_PROOF_OF_SUBMISSION_REQUESTED";                break;
     case PROP_ID(PR_READ_RECEIPT_REQUESTED) :
		pszTag = "PR_READ_RECEIPT_REQUESTED";                       break;
     case PROP_ID(PR_RECEIPT_TIME) :
		pszTag = "PR_RECEIPT_TIME";                                 break;
     case PROP_ID(PR_RECIPIENT_REASSIGNMENT_PROHIBITED) :
		pszTag = "PR_RECIPIENT_REASSIGNMENT_PROHIBITED";            break;
     case PROP_ID(PR_REDIRECTION_HISTORY) :
		pszTag = "PR_REDIRECTION_HISTORY";                          break;
    case PROP_ID(PR_RELATED_IPMS) :
		pszTag = "PR_RELATED_IPMS";                                 break;
    case PROP_ID(PR_LANGUAGES) :
		pszTag = "PR_LANGUAGES";                                    break;
    case PROP_ID(PR_REPLY_TIME) :
		pszTag = "PR_REPLY_TIME";                                   break;
    case PROP_ID(PR_REPORT_TAG) :
		pszTag = "PR_REPORT_TAG";                                   break;
    case PROP_ID(PR_REPORT_TIME) :
		pszTag = "PR_REPORT_TIME";                                  break;
    case PROP_ID(PR_RETURNED_IPM) :
		pszTag = "PR_RETURNED_IPM";                                 break;
    case PROP_ID(PR_SECURITY) :
		pszTag = "PR_SECURITY";                                     break;
    case PROP_ID(PR_INCOMPLETE_COPY) :
		pszTag = "PR_INCOMPLETE_COPY";                              break;
    case PROP_ID(PR_SENSITIVITY) :
		pszTag = "PR_SENSITIVITY";                                  break;
    case PROP_ID(PR_SUBJECT) :
		pszTag = "PR_SUBJECT";                                      break;
    case PROP_ID(PR_SUBJECT_IPM) :
		pszTag = "PR_SUBJECT_IPM";                                  break;
	case PROP_ID(PR_SUBJECT_PREFIX) :
		pszTag = "PR_SUBJECT_PREFIX";								break;
    case PROP_ID(PR_CLIENT_SUBMIT_TIME) :
		pszTag = "PR_CLIENT_SUBMIT_TIME";                           break;
    case PROP_ID(PR_REPORT_NAME) :
		pszTag = "PR_REPORT_NAME";                                  break;
    case PROP_ID(PR_SENT_REPRESENTING_SEARCH_KEY) :
		pszTag = "PR_SENT_REPRESENTING_SEARCH_KEY";                 break;
    case PROP_ID(PR_X400_CONTENT_TYPE) :
		pszTag = "PR_X400_CONTENT_TYPE";                            break;
    case PROP_ID(PR_NON_RECEIPT_REASON) :
		pszTag = "PR_NON_RECEIPT_REASON";                           break;
    case PROP_ID(PR_RECEIVED_BY_ENTRYID) :
		pszTag = "PR_RECEIVED_BY_ENTRYID";                          break;
    case PROP_ID(PR_RECEIVED_BY_NAME) :
		pszTag = "PR_RECEIVED_BY_NAME";                             break;
    case PROP_ID(PR_SENT_REPRESENTING_ENTRYID) :
		pszTag = "PR_SENT_REPRESENTING_ENTRYID";                    break;
    case PROP_ID(PR_SENT_REPRESENTING_NAME) :
		pszTag = "PR_SENT_REPRESENTING_NAME";                       break;
    case PROP_ID(PR_RCVD_REPRESENTING_ENTRYID) :
		pszTag = "PR_RCVD_REPRESENTING_ENTRYID";                    break;
    case PROP_ID(PR_RCVD_REPRESENTING_NAME) :
		pszTag = "PR_RCVD_REPRESENTING_NAME";                       break;
    case PROP_ID(PR_REPORT_ENTRYID) :
		pszTag = "PR_REPORT_ENTRYID";                               break;
    case PROP_ID(PR_READ_RECEIPT_ENTRYID) :
		pszTag = "PR_READ_RECEIPT_ENTRYID";                         break;
    case PROP_ID(PR_MESSAGE_SUBMISSION_ID) :
		pszTag = "PR_MESSAGE_SUBMISSION_ID";                        break;
    case PROP_ID(PR_PROVIDER_SUBMIT_TIME) :
		pszTag = "PR_PROVIDER_SUBMIT_TIME";                         break;
    case PROP_ID(PR_ORIGINAL_SUBJECT) :
		pszTag = "PR_ORIGINAL_SUBJECT";                             break;
    case PROP_ID(PR_DISC_VAL) :
		pszTag = "PR_DISC_VAL";                                     break;
    case PROP_ID(PR_ORIG_MESSAGE_CLASS) :
		pszTag = "PR_ORIG_MESSAGE_CLASS";                           break;
    case PROP_ID(PR_ORIGINAL_AUTHOR_ENTRYID) :
		pszTag = "PR_ORIGINAL_AUTHOR_ENTRYID";                      break;
    case PROP_ID(PR_ORIGINAL_AUTHOR_NAME) :
		pszTag = "PR_ORIGINAL_AUTHOR_NAME";                         break;
    case PROP_ID(PR_ORIGINAL_SUBMIT_TIME) :
		pszTag = "PR_ORIGINAL_SUBMIT_TIME";                         break;
    case PROP_ID(PR_REPLY_RECIPIENT_ENTRIES) :
		pszTag = "PR_REPLY_RECIPIENT_ENTRIES";                      break;
    case PROP_ID(PR_REPLY_RECIPIENT_NAMES) :
		pszTag = "PR_REPLY_RECIPIENT_NAMES";                        break;
    case PROP_ID(PR_RECEIVED_BY_SEARCH_KEY) :
		pszTag = "PR_RECEIVED_BY_SEARCH_KEY";                       break;
    case PROP_ID(PR_RCVD_REPRESENTING_SEARCH_KEY) :
		pszTag = "PR_RCVD_REPRESENTING_SEARCH_KEY";                 break;
    case PROP_ID(PR_READ_RECEIPT_SEARCH_KEY) :
		pszTag = "PR_READ_RECEIPT_SEARCH_KEY";                      break;
    case PROP_ID(PR_REPORT_SEARCH_KEY) :
		pszTag = "PR_REPORT_SEARCH_KEY";                            break;
    case PROP_ID(PR_ORIGINAL_DELIVERY_TIME) :
		pszTag = "PR_ORIGINAL_DELIVERY_TIME";                       break;
    case PROP_ID(PR_ORIGINAL_AUTHOR_SEARCH_KEY) :
		pszTag = "PR_ORIGINAL_AUTHOR_SEARCH_KEY";                   break;
    case PROP_ID(PR_MESSAGE_TO_ME) :
		pszTag = "PR_MESSAGE_TO_ME";                                break;
    case PROP_ID(PR_MESSAGE_CC_ME) :
		pszTag = "PR_MESSAGE_CC_ME";                                break;
    case PROP_ID(PR_MESSAGE_RECIP_ME) :
		pszTag = "PR_MESSAGE_RECIP_ME";                             break;
    case PROP_ID(PR_ORIGINAL_SENDER_NAME) :
		pszTag = "PR_ORIGINAL_SENDER_NAME";                         break;
    case PROP_ID(PR_ORIGINAL_SENDER_ENTRYID) :
		pszTag = "PR_ORIGINAL_SENDER_ENTRYID";                      break;
    case PROP_ID(PR_ORIGINAL_SENDER_SEARCH_KEY) :
		pszTag = "PR_ORIGINAL_SENDER_SEARCH_KEY";                   break;
    case PROP_ID(PR_ORIGINAL_SENT_REPRESENTING_NAME) :
		pszTag = "PR_ORIGINAL_SENT_REPRESENTING_NAME";              break;
     case PROP_ID(PR_ORIGINAL_SENT_REPRESENTING_ENTRYID) :
		pszTag = "PR_ORIGINAL_SENT_REPRESENTING_ENTRYID";           break;
    case PROP_ID(PR_ORIGINAL_SENT_REPRESENTING_SEARCH_KEY) :
		pszTag = "PR_ORIGINAL_SENT_REPRESENTING_SEARCH_KEY";        break;
    case PROP_ID(PR_START_DATE) :
		pszTag = "PR_START_DATE";                                   break;
    case PROP_ID(PR_END_DATE) :
		pszTag = "PR_END_DATE";                                     break;
    case PROP_ID(PR_OWNER_APPT_ID) :
		pszTag = "PR_OWNER_APPT_ID";                                break;
    case PROP_ID(PR_RESPONSE_REQUESTED) :
		pszTag = "PR_RESPONSE_REQUESTED";                           break;
    case PROP_ID(PR_SENT_REPRESENTING_ADDRTYPE) :
		pszTag = "PR_SENT_REPRESENTING_ADDRTYPE";                   break;
    case PROP_ID(PR_SENT_REPRESENTING_EMAIL_ADDRESS) :
		pszTag = "PR_SENT_REPRESENTING_EMAIL_ADDRESS";              break;
    case PROP_ID(PR_ORIGINAL_SENDER_ADDRTYPE) :
		pszTag = "PR_ORIGINAL_SENDER_ADDRTYPE";                     break;
    case PROP_ID(PR_ORIGINAL_SENDER_EMAIL_ADDRESS) :
		pszTag = "PR_ORIGINAL_SENDER_EMAIL_ADDRESS";                break;
    case PROP_ID(PR_ORIGINAL_SENT_REPRESENTING_ADDRTYPE) :
		pszTag = "PR_ORIGINAL_SENT_REPRESENTING_ADDRTYPE";          break;
    case PROP_ID(PR_ORIGINAL_SENT_REPRESENTING_EMAIL_ADDRESS) :
		pszTag = "PR_ORIGINAL_SENT_REPRESENTING_EMAIL_ADDRESS";     break;
    case PROP_ID(PR_CONVERSATION_TOPIC) :
		pszTag = "PR_CONVERSATION_TOPIC";                           break;
    case PROP_ID(PR_CONVERSATION_INDEX) :
		pszTag = "PR_CONVERSATION_INDEX";                           break;
    case PROP_ID(PR_ORIGINAL_DISPLAY_BCC) :
		pszTag = "PR_ORIGINAL_DISPLAY_BCC";                         break;
    case PROP_ID(PR_ORIGINAL_DISPLAY_CC) :
		pszTag = "PR_ORIGINAL_DISPLAY_CC";                          break;
    case PROP_ID(PR_ORIGINAL_DISPLAY_TO) :
		pszTag = "PR_ORIGINAL_DISPLAY_TO";                          break;
    case PROP_ID(PR_RECEIVED_BY_ADDRTYPE) :
		pszTag = "PR_RECEIVED_BY_ADDRTYPE";                         break;
    case PROP_ID(PR_RECEIVED_BY_EMAIL_ADDRESS) :
		pszTag = "PR_RECEIVED_BY_EMAIL_ADDRESS";                    break;
    case PROP_ID(PR_RCVD_REPRESENTING_ADDRTYPE) :
		pszTag = "PR_RCVD_REPRESENTING_ADDRTYPE";                   break;
    case PROP_ID(PR_RCVD_REPRESENTING_EMAIL_ADDRESS) :
		pszTag = "PR_RCVD_REPRESENTING_EMAIL_ADDRESS";              break;
    case PROP_ID(PR_ORIGINAL_AUTHOR_ADDRTYPE) :
		pszTag = "PR_ORIGINAL_AUTHOR_ADDRTYPE";                     break;
    case PROP_ID(PR_ORIGINAL_AUTHOR_EMAIL_ADDRESS) :
		pszTag = "PR_ORIGINAL_AUTHOR_EMAIL_ADDRESS";                break;
    case PROP_ID(PR_ORIGINALLY_INTENDED_RECIP_ADDRTYPE) :
		pszTag = "PR_ORIGINALLY_INTENDED_RECIP_ADDRTYPE";           break;
    case PROP_ID(PR_ORIGINALLY_INTENDED_RECIP_EMAIL_ADDRESS) :
		pszTag = "PR_ORIGINALLY_INTENDED_RECIP_EMAIL_ADDRESS";      break;
    case PROP_ID(PR_TRANSPORT_MESSAGE_HEADERS) :
		pszTag = "PR_TRANSPORT_MESSAGE_HEADERS";                    break;

    // Message content properties

	case PROP_ID(PR_BODY) :
		pszTag = "PR_BODY";                                         break;
    case PROP_ID(PR_REPORT_TEXT) :
		pszTag = "PR_REPORT_TEXT";                                  break;
    case PROP_ID(PR_ORIGINATOR_AND_DL_EXPANSION_HISTORY) :
		pszTag = "PR_ORIGINATOR_AND_DL_EXPANSION_HISTORY";          break;
    case PROP_ID(PR_REPORTING_DL_NAME) :
		pszTag = "PR_REPORTING_DL_NAME";                            break;
    case PROP_ID(PR_REPORTING_MTA_CERTIFICATE) :
		pszTag = "PR_REPORTING_MTA_CERTIFICATE";                    break;
    case PROP_ID(PR_RTF_SYNC_BODY_CRC) :
		pszTag = "PR_RTF_SYNC_BODY_CRC";                            break;
    case PROP_ID(PR_RTF_SYNC_BODY_COUNT) :
		pszTag = "PR_RTF_SYNC_BODY_COUNT";                          break;
    case PROP_ID(PR_RTF_SYNC_BODY_TAG) :
		pszTag = "PR_RTF_SYNC_BODY_TAG";                            break;
     case PROP_ID(PR_RTF_COMPRESSED) :
		pszTag = "PR_RTF_COMPRESSED";                               break;
     case PROP_ID(PR_RTF_SYNC_PREFIX_COUNT) :
		pszTag = "PR_RTF_SYNC_PREFIX_COUNT";                        break;
     case PROP_ID(PR_RTF_SYNC_TRAILING_COUNT) :
		pszTag = "PR_RTF_SYNC_TRAILING_COUNT";                      break;
     case PROP_ID(PR_ORIGINALLY_INTENDED_RECIP_ENTRYID) :
		pszTag = "PR_ORIGINALLY_INTENDED_RECIP_ENTRYID";            break;

     // Message recipient properties

	case PROP_ID(PR_CONTENT_INTEGRITY_CHECK) :
		pszTag = "PR_CONTENT_INTEGRITY_CHECK";                      break;
     case PROP_ID(PR_EXPLICIT_CONVERSION) :
		pszTag = "PR_EXPLICIT_CONVERSION";                          break;
    case PROP_ID(PR_IPM_RETURN_REQUESTED) :
		pszTag = "PR_IPM_RETURN_REQUESTED";                         break;
     case PROP_ID(PR_MESSAGE_TOKEN) :
		pszTag = "PR_MESSAGE_TOKEN";                                break;
    case PROP_ID(PR_NDR_REASON_CODE) :
		pszTag = "PR_NDR_REASON_CODE";                              break;
    case PROP_ID(PR_NDR_DIAG_CODE) :
		pszTag = "PR_NDR_DIAG_CODE";                                break;
    case PROP_ID(PR_NON_RECEIPT_NOTIFICATION_REQUESTED) :
		pszTag = "PR_NON_RECEIPT_NOTIFICATION_REQUESTED";           break;
    case PROP_ID(PR_ORIGINATOR_NON_DELIVERY_REPORT_REQUESTED) :
		pszTag = "PR_ORIGINATOR_NON_DELIVERY_REPORT_REQUESTED";     break;
    case PROP_ID(PR_ORIGINATOR_REQUESTED_ALTERNATE_RECIPIENT) :
		pszTag = "PR_ORIGINATOR_REQUESTED_ALTERNATE_RECIPIENT";     break;
    case PROP_ID(PR_PHYSICAL_DELIVERY_BUREAU_FAX_DELIVERY) :
		pszTag = "PR_PHYSICAL_DELIVERY_BUREAU_FAX_DELIVERY";        break;
    case PROP_ID(PR_PHYSICAL_DELIVERY_MODE) :
		pszTag = "PR_PHYSICAL_DELIVERY_MODE";                       break;
    case PROP_ID(PR_PHYSICAL_DELIVERY_REPORT_REQUEST) :
		pszTag = "PR_PHYSICAL_DELIVERY_REPORT_REQUEST";             break;
    case PROP_ID(PR_PHYSICAL_FORWARDING_ADDRESS) :
		pszTag = "PR_PHYSICAL_FORWARDING_ADDRESS";                  break;
    case PROP_ID(PR_PHYSICAL_FORWARDING_ADDRESS_REQUESTED) :
		pszTag = "PR_PHYSICAL_FORWARDING_ADDRESS_REQUESTED";        break;
    case PROP_ID(PR_PHYSICAL_FORWARDING_PROHIBITED) :
		pszTag = "PR_PHYSICAL_FORWARDING_PROHIBITED";               break;
    case PROP_ID(PR_PHYSICAL_RENDITION_ATTRIBUTES) :
		pszTag = "PR_PHYSICAL_RENDITION_ATTRIBUTES";                break;
    case PROP_ID(PR_PROOF_OF_DELIVERY) :
		pszTag = "PR_PROOF_OF_DELIVERY";                            break;
    case PROP_ID(PR_PROOF_OF_DELIVERY_REQUESTED) :
		pszTag = "PR_PROOF_OF_DELIVERY_REQUESTED";                  break;
    case PROP_ID(PR_RECIPIENT_CERTIFICATE) :
		pszTag = "PR_RECIPIENT_CERTIFICATE";                        break;
    case PROP_ID(PR_RECIPIENT_NUMBER_FOR_ADVICE) :
		pszTag = "PR_RECIPIENT_NUMBER_FOR_ADVICE";                  break;
    case PROP_ID(PR_RECIPIENT_TYPE) :
		pszTag = "PR_RECIPIENT_TYPE";                               break;
    case PROP_ID(PR_REGISTERED_MAIL_TYPE) :
		pszTag = "PR_REGISTERED_MAIL_TYPE";                         break;
    case PROP_ID(PR_REPLY_REQUESTED) :
		pszTag = "PR_REPLY_REQUESTED";                              break;
    case PROP_ID(PR_REQUESTED_DELIVERY_METHOD) :
		pszTag = "PR_REQUESTED_DELIVERY_METHOD";                    break;
    case PROP_ID(PR_SENDER_ENTRYID) :
		pszTag = "PR_SENDER_ENTRYID";                               break;
    case PROP_ID(PR_SENDER_NAME) :
		pszTag = "PR_SENDER_NAME";                                  break;
    case PROP_ID(PR_SUPPLEMENTARY_INFO) :
		pszTag = "PR_SUPPLEMENTARY_INFO";                           break;
    case PROP_ID(PR_TYPE_OF_MTS_USER) :
		pszTag = "PR_TYPE_OF_MTS_USER";                             break;
    case PROP_ID(PR_SENDER_SEARCH_KEY) :
		pszTag = "PR_SENDER_SEARCH_KEY";                            break;
    case PROP_ID(PR_SENDER_ADDRTYPE) :
		pszTag = "PR_SENDER_ADDRTYPE";                              break;
    case PROP_ID(PR_SENDER_EMAIL_ADDRESS) :
		pszTag = "PR_SENDER_EMAIL_ADDRESS";                         break;

    // Message nontransmittable properties

	case PROP_ID(PR_CURRENT_VERSION) :
		pszTag = "PR_CURRENT_VERSION";                              break;
    case PROP_ID(PR_DELETE_AFTER_SUBMIT) :
		pszTag = "PR_DELETE_AFTER_SUBMIT";                          break;
    case PROP_ID(PR_DISPLAY_BCC) :
		pszTag = "PR_DISPLAY_BCC";                                  break;
    case PROP_ID(PR_DISPLAY_CC) :
		pszTag = "PR_DISPLAY_CC";                                   break;
    case PROP_ID(PR_DISPLAY_TO) :
		pszTag = "PR_DISPLAY_TO";                                   break;
    case PROP_ID(PR_PARENT_DISPLAY) :
		pszTag = "PR_PARENT_DISPLAY";                               break;
    case PROP_ID(PR_MESSAGE_DELIVERY_TIME) :
		pszTag = "PR_MESSAGE_DELIVERY_TIME";                        break;
    case PROP_ID(PR_MESSAGE_FLAGS) :
		pszTag = "PR_MESSAGE_FLAGS";                                break;
    case PROP_ID(PR_MESSAGE_SIZE) :
		pszTag = "PR_MESSAGE_SIZE";                                 break;
    case PROP_ID(PR_PARENT_ENTRYID) :
		pszTag = "PR_PARENT_ENTRYID";                               break;
    case PROP_ID(PR_SENTMAIL_ENTRYID) :
		pszTag = "PR_SENTMAIL_ENTRYID";                             break;
    case PROP_ID(PR_CORRELATE) :
		pszTag = "PR_CORRELATE";                                    break;
    case PROP_ID(PR_CORRELATE_MTSID) :
		pszTag = "PR_CORRELATE_MTSID";                              break;
    case PROP_ID(PR_DISCRETE_VALUES) :
		pszTag = "PR_DISCRETE_VALUES";                              break;
    case PROP_ID(PR_RESPONSIBILITY) :
		pszTag = "PR_RESPONSIBILITY";                               break;
    case PROP_ID(PR_SPOOLER_STATUS) :
		pszTag = "PR_SPOOLER_STATUS";                               break;
    case PROP_ID(PR_TRANSPORT_STATUS) :
		pszTag = "PR_TRANSPORT_STATUS";                             break;
    case PROP_ID(PR_MESSAGE_RECIPIENTS) :
		pszTag = "PR_MESSAGE_RECIPIENTS";                           break;
    case PROP_ID(PR_MESSAGE_ATTACHMENTS) :
		pszTag = "PR_MESSAGE_ATTACHMENTS";                          break;
    case PROP_ID(PR_SUBMIT_FLAGS) :
		pszTag = "PR_SUBMIT_FLAGS";                                 break;
    case PROP_ID(PR_RECIPIENT_STATUS) :
		pszTag = "PR_RECIPIENT_STATUS";                             break;
    case PROP_ID(PR_TRANSPORT_KEY) :
		pszTag = "PR_TRANSPORT_KEY";                                break;
    case PROP_ID(PR_MSG_STATUS) :
		pszTag = "PR_MSG_STATUS";                                   break;
    case PROP_ID(PR_MESSAGE_DOWNLOAD_TIME) :
		pszTag = "PR_MESSAGE_DOWNLOAD_TIME";                        break;
    case PROP_ID(PR_CREATION_VERSION) :
		pszTag = "PR_CREATION_VERSION";                             break;
    case PROP_ID(PR_MODIFY_VERSION) :
		pszTag = "PR_MODIFY_VERSION";                               break;
    case PROP_ID(PR_HASATTACH) :
		pszTag = "PR_HASATTACH";                                    break;
    case PROP_ID(PR_BODY_CRC) :
		pszTag = "PR_BODY_CRC";                                     break;
    case PROP_ID(PR_NORMALIZED_SUBJECT) :
		pszTag = "PR_NORMALIZED_SUBJECT";                           break;
    case PROP_ID(PR_RTF_IN_SYNC) :
		pszTag = "PR_RTF_IN_SYNC";                                  break;
    case PROP_ID(PR_ATTACH_SIZE) :
		pszTag = "PR_ATTACH_SIZE";                                  break;
    case PROP_ID(PR_ATTACH_NUM) :
		pszTag = "PR_ATTACH_NUM";                                   break;
    case PROP_ID(PR_PREPROCESS) :
		pszTag = "PR_PREPROCESS";                                   break;

    case PROP_ID(PR_ORIGINATING_MTA_CERTIFICATE) :
		pszTag = "PR_ORIGINATING_MTA_CERTIFICATE";                  break;
    case PROP_ID(PR_PROOF_OF_SUBMISSION) :
		pszTag = "PR_PROOF_OF_SUBMISSION";                          break;

    // Properties common to multiple objects (including message objects)

    case PROP_ID(PR_ENTRYID) :
		pszTag = "PR_ENTRYID";                                      break;
    case PROP_ID(PR_OBJECT_TYPE) :
		pszTag = "PR_OBJECT_TYPE";                                  break;
    case PROP_ID(PR_ICON) :
		pszTag = "PR_ICON";                                         break;
    case PROP_ID(PR_MINI_ICON) :
		pszTag = "PR_MINI_ICON";                                    break;
    case PROP_ID(PR_STORE_ENTRYID) :
		pszTag = "PR_STORE_ENTRYID";                                break;
    case PROP_ID(PR_STORE_RECORD_KEY) :
		pszTag = "PR_STORE_RECORD_KEY";                             break;
    case PROP_ID(PR_RECORD_KEY) :
		pszTag = "PR_RECORD_KEY";                                   break;
    case PROP_ID(PR_MAPPING_SIGNATURE) :
		pszTag = "PR_MAPPING_SIGNATURE";                            break;
    case PROP_ID(PR_ACCESS_LEVEL) :
		pszTag = "PR_ACCESS_LEVEL";                                 break;
    case PROP_ID(PR_INSTANCE_KEY) :
		pszTag = "PR_INSTANCE_KEY";                                 break;
    case PROP_ID(PR_ROW_TYPE) :
		pszTag = "PR_ROW_TYPE";                                     break;
    case PROP_ID(PR_ACCESS) :
		pszTag = "PR_ACCESS";                                       break;

    // Properties common to multiple objects (NOT including message objects)

	case PROP_ID(PR_ROWID) :
		pszTag = "PR_ROWID";                                        break;
    case PROP_ID(PR_DISPLAY_NAME) :
		pszTag = "PR_DISPLAY_NAME";                                 break;
    case PROP_ID(PR_ADDRTYPE) :
		pszTag = "PR_ADDRTYPE";                                     break;
    case PROP_ID(PR_EMAIL_ADDRESS) :
		pszTag = "PR_EMAIL_ADDRESS";                                break;
    case PROP_ID(PR_COMMENT) :
		pszTag = "PR_COMMENT";                                      break;
    case PROP_ID(PR_DEPTH) :
		pszTag = "PR_DEPTH";                                        break;
    case PROP_ID(PR_PROVIDER_DISPLAY) :
		pszTag = "PR_PROVIDER_DISPLAY";                             break;
    case PROP_ID(PR_CREATION_TIME) :
		pszTag = "PR_CREATION_TIME";                                break;
    case PROP_ID(PR_LAST_MODIFICATION_TIME) :
		pszTag = "PR_LAST_MODIFICATION_TIME";                       break;
    case PROP_ID(PR_RESOURCE_FLAGS) :
		pszTag = "PR_RESOURCE_FLAGS";                               break;
    case PROP_ID(PR_PROVIDER_DLL_NAME) :
		pszTag = "PR_PROVIDER_DLL_NAME";                            break;
    case PROP_ID(PR_SEARCH_KEY) :
		pszTag = "PR_SEARCH_KEY";                                   break;
    case PROP_ID(PR_PROVIDER_UID) :
		pszTag = "PR_PROVIDER_UID";                                 break;
    case PROP_ID(PR_PROVIDER_ORDINAL) :
		pszTag = "PR_PROVIDER_ORDINAL";                             break;

    // MAPI form properties

	case PROP_ID(PR_FORM_VERSION) :
		pszTag = "PR_FORM_VERSION";                                 break;
    case PROP_ID(PR_FORM_CLSID) :
		pszTag = "PR_FORM_CLSID";                                   break;
    case PROP_ID(PR_FORM_CONTACT_NAME) :
		pszTag = "PR_FORM_CONTACT_NAME";                            break;
    case PROP_ID(PR_FORM_CATEGORY) :
		pszTag = "PR_FORM_CATEGORY";                                break;
    case PROP_ID(PR_FORM_CATEGORY_SUB) :
		pszTag = "PR_FORM_CATEGORY_SUB";                            break;
    case PROP_ID(PR_FORM_HOST_MAP) :
		pszTag = "PR_FORM_HOST_MAP";                                break;
    case PROP_ID(PR_FORM_HIDDEN) :
		pszTag = "PR_FORM_HIDDEN";                                  break;
    case PROP_ID(PR_FORM_DESIGNER_NAME) :
		pszTag = "PR_FORM_DESIGNER_NAME";                           break;
    case PROP_ID(PR_FORM_DESIGNER_GUID) :
		pszTag = "PR_FORM_DESIGNER_GUID";                           break;
    case PROP_ID(PR_FORM_MESSAGE_BEHAVIOR) :
		pszTag = "PR_FORM_MESSAGE_BEHAVIOR";                        break;


	//Message store properties

	case PROP_ID(PR_DEFAULT_STORE) :
		pszTag = "PR_DEFAULT_STORE";                                break;
    case PROP_ID(PR_STORE_SUPPORT_MASK) :
		pszTag = "PR_STORE_SUPPORT_MASK";                           break;
    case PROP_ID(PR_STORE_STATE) :
		pszTag = "PR_STORE_STATE";                                  break;
    case PROP_ID(PR_IPM_SUBTREE_SEARCH_KEY) :
		pszTag = "PR_IPM_SUBTREE_SEARCH_KEY";                       break;
    case PROP_ID(PR_IPM_OUTBOX_SEARCH_KEY) :
		pszTag = "PR_IPM_OUTBOX_SEARCH_KEY";                        break;
    case PROP_ID(PR_IPM_WASTEBASKET_SEARCH_KEY) :
		pszTag = "PR_IPM_WASTEBASKET_SEARCH_KEY";                   break;
    case PROP_ID(PR_IPM_SENTMAIL_SEARCH_KEY) :
		pszTag = "PR_IPM_SENTMAIL_SEARCH_KEY";                      break;
    case PROP_ID(PR_MDB_PROVIDER) :
		pszTag = "PR_MDB_PROVIDER";                                 break;
    case PROP_ID(PR_RECEIVE_FOLDER_SETTINGS) :
		pszTag = "PR_RECEIVE_FOLDER_SETTINGS";                      break;
    case PROP_ID(PR_VALID_FOLDER_MASK) :
		pszTag = "PR_VALID_FOLDER_MASK";                            break;
    case PROP_ID(PR_IPM_SUBTREE_ENTRYID) :
		pszTag = "PR_IPM_SUBTREE_ENTRYID";                          break;
    case PROP_ID(PR_IPM_OUTBOX_ENTRYID) :
		pszTag = "PR_IPM_OUTBOX_ENTRYID";                           break;
    case PROP_ID(PR_IPM_WASTEBASKET_ENTRYID) :
		pszTag = "PR_IPM_WASTEBASKET_ENTRYID";                      break;
    case PROP_ID(PR_IPM_SENTMAIL_ENTRYID) :
		pszTag = "PR_IPM_SENTMAIL_ENTRYID";                         break;
    case PROP_ID(PR_VIEWS_ENTRYID) :
		pszTag = "PR_VIEWS_ENTRYID";                                break;
    case PROP_ID(PR_COMMON_VIEWS_ENTRYID) :
		pszTag = "PR_COMMON_VIEWS_ENTRYID";                         break;
    case PROP_ID(PR_FINDER_ENTRYID) :
		pszTag = "PR_FINDER_ENTRYID";                               break;


	// Folder and address book container properties

	case PROP_ID(PR_CONTAINER_FLAGS) :
		pszTag = "PR_CONTAINER_FLAGS";                              break;
    case PROP_ID(PR_FOLDER_TYPE) :
		pszTag = "PR_FOLDER_TYPE";                                  break;
    case PROP_ID(PR_CONTENT_COUNT) :
		pszTag = "PR_CONTENT_COUNT";                                break;
    case PROP_ID(PR_CONTENT_UNREAD) :
		pszTag = "PR_CONTENT_UNREAD";                               break;
    case PROP_ID(PR_CREATE_TEMPLATES) :
		pszTag = "PR_CREATE_TEMPLATES";                             break;
    case PROP_ID(PR_DETAILS_TABLE) :
		pszTag = "PR_DETAILS_TABLE";                                break;
    case PROP_ID(PR_SEARCH) :
		pszTag = "PR_SEARCH";                                       break;
    case PROP_ID(PR_SELECTABLE) :
		pszTag = "PR_SELECTABLE";                                   break;
    case PROP_ID(PR_SUBFOLDERS) :
		pszTag = "PR_SUBFOLDERS";                                   break;
    case PROP_ID(PR_STATUS) :
		pszTag = "PR_STATUS";                                       break;
    case PROP_ID(PR_ANR) :
		pszTag = "PR_ANR";                                          break;
    case PROP_ID(PR_CONTENTS_SORT_ORDER) :
		pszTag = "PR_CONTENTS_SORT_ORDER";                          break;
    case PROP_ID(PR_CONTAINER_HIERARCHY) :
		pszTag = "PR_CONTAINER_HIERARCHY";                          break;
    case PROP_ID(PR_CONTAINER_CONTENTS) :
		pszTag = "PR_CONTAINER_CONTENTS";                           break;
    case PROP_ID(PR_FOLDER_ASSOCIATED_CONTENTS) :
		pszTag = "PR_FOLDER_ASSOCIATED_CONTENTS";                   break;
    case PROP_ID(PR_DEF_CREATE_DL) :
		pszTag = "PR_DEF_CREATE_DL";                                break;
    case PROP_ID(PR_DEF_CREATE_MAILUSER) :
		pszTag = "PR_DEF_CREATE_MAILUSER";                          break;
    case PROP_ID(PR_CONTAINER_CLASS) :
		pszTag = "PR_CONTAINER_CLASS";                              break;
    case PROP_ID(PR_CONTAINER_MODIFY_VERSION) :
		pszTag = "PR_CONTAINER_MODIFY_VERSION";                     break;
    case PROP_ID(PR_AB_PROVIDER_ID) :
		pszTag = "PR_AB_PROVIDER_ID";                               break;
    case PROP_ID(PR_DEFAULT_VIEW_ENTRYID) :
		pszTag = "PR_DEFAULT_VIEW_ENTRYID";                         break;
    case PROP_ID(PR_ASSOC_CONTENT_COUNT) :
		pszTag = "PR_ASSOC_CONTENT_COUNT";                          break;


    // Attachment properties

	case PROP_ID(PR_ATTACHMENT_X400_PARAMETERS) :
		pszTag = "PR_ATTACHMENT_X400_PARAMETERS";                   break;
    case PROP_ID(PR_ATTACH_DATA_OBJ) :
		pszTag = "PR_ATTACH_DATA_BIN or PR_ATTACH_DATA_OBJ";        break;
	case PROP_ID(PR_ATTACH_ENCODING) :
		pszTag = "PR_ATTACH_ENCODING";                              break;
    case PROP_ID(PR_ATTACH_EXTENSION) :
		pszTag = "PR_ATTACH_EXTENSION";                             break;
   case PROP_ID(PR_ATTACH_FILENAME) :
		pszTag = "PR_ATTACH_FILENAME";                              break;
    case PROP_ID(PR_ATTACH_METHOD) :
		pszTag = "PR_ATTACH_METHOD";                                break;
    case PROP_ID(PR_ATTACH_LONG_FILENAME) :
		pszTag = "PR_ATTACH_LONG_FILENAME";                         break;
    case PROP_ID(PR_ATTACH_PATHNAME) :
		pszTag = "PR_ATTACH_PATHNAME";                              break;
    case PROP_ID(PR_ATTACH_RENDERING) :
		pszTag = "PR_ATTACH_RENDERING";                             break;
    case PROP_ID(PR_ATTACH_TAG) :
		pszTag = "PR_ATTACH_TAG";                                   break;
    case PROP_ID(PR_RENDERING_POSITION) :
		pszTag = "PR_RENDERING_POSITION";                           break;
    case PROP_ID(PR_ATTACH_TRANSPORT_NAME) :
		pszTag = "PR_ATTACH_TRANSPORT_NAME";                        break;


	// Address book object properties

    case PROP_ID(PR_DISPLAY_TYPE) :
		pszTag = "PR_DISPLAY_TYPE";                                 break;
    case PROP_ID(PR_TEMPLATEID) :
		pszTag = "PR_TEMPLATEID";                                   break;
    case PROP_ID(PR_PRIMARY_CAPABILITY) :
		pszTag = "PR_PRIMARY_CAPABILITY";                           break;


	// Mail user properties

	case PROP_ID(PR_ACCOUNT) :
		pszTag = "PR_ACCOUNT";                                      break;
    case PROP_ID(PR_ALTERNATE_RECIPIENT) :
		pszTag = "PR_ALTERNATE_RECIPIENT";                          break;
    case PROP_ID(PR_CALLBACK_TELEPHONE_NUMBER) :
		pszTag = "PR_CALLBACK_TELEPHONE_NUMBER";                    break;
    case PROP_ID(PR_CONVERSION_PROHIBITED) :
		pszTag = "PR_CONVERSION_PROHIBITED";                        break;
    case PROP_ID(PR_DISCLOSE_RECIPIENTS) :
		pszTag = "PR_DISCLOSE_RECIPIENTS";                          break;
    case PROP_ID(PR_GENERATION) :
		pszTag = "PR_GENERATION";                                   break;
    case PROP_ID(PR_GIVEN_NAME) :
		pszTag = "PR_GIVEN_NAME";                                   break;
    case PROP_ID(PR_GOVERNMENT_ID_NUMBER) :
		pszTag = "PR_GOVERNMENT_ID_NUMBER";                         break;
    case PROP_ID(PR_OFFICE_TELEPHONE_NUMBER) :
		pszTag = "PR_OFFICE_TELEPHONE_NUMBER";                      break;
    case PROP_ID(PR_HOME_TELEPHONE_NUMBER) :
		pszTag = "PR_HOME_TELEPHONE_NUMBER";                        break;
    case PROP_ID(PR_INITIALS) :
		pszTag = "PR_INITIALS";                                     break;
    case PROP_ID(PR_KEYWORD) :
		pszTag = "PR_KEYWORD";                                      break;
    case PROP_ID(PR_LANGUAGE) :
		pszTag = "PR_LANGUAGE";                                     break;
    case PROP_ID(PR_LOCATION) :
		pszTag = "PR_LOCATION";                                     break;
    case PROP_ID(PR_MAIL_PERMISSION) :
		pszTag = "PR_MAIL_PERMISSION";                              break;
    case PROP_ID(PR_MHS_COMMON_NAME) :
		pszTag = "PR_MHS_COMMON_NAME";                              break;
    case PROP_ID(PR_ORGANIZATIONAL_ID_NUMBER) :
		pszTag = "PR_ORGANIZATIONAL_ID_NUMBER";                     break;
    case PROP_ID(PR_SURNAME) :
		pszTag = "PR_SURNAME";                                      break;
    case PROP_ID(PR_ORIGINAL_ENTRYID) :
		pszTag = "PR_ORIGINAL_ENTRYID";                             break;
    case PROP_ID(PR_ORIGINAL_DISPLAY_NAME) :
		pszTag = "PR_ORIGINAL_DISPLAY_NAME";                        break;
    case PROP_ID(PR_ORIGINAL_SEARCH_KEY) :
		pszTag = "PR_ORIGINAL_SEARCH_KEY";                          break;
    case PROP_ID(PR_POSTAL_ADDRESS) :
		pszTag = "PR_POSTAL_ADDRESS";                               break;
    case PROP_ID(PR_COMPANY_NAME) :
		pszTag = "PR_COMPANY_NAME";                                 break;
    case PROP_ID(PR_TITLE) :
		pszTag = "PR_TITLE";                                        break;
    case PROP_ID(PR_DEPARTMENT_NAME) :
		pszTag = "PR_DEPARTMENT_NAME";                              break;
    case PROP_ID(PR_OFFICE_LOCATION) :
		pszTag = "PR_OFFICE_LOCATION";                              break;
    case PROP_ID(PR_PRIMARY_TELEPHONE_NUMBER) :
		pszTag = "PR_PRIMARY_TELEPHONE_NUMBER";                     break;
    case PROP_ID(PR_OFFICE2_TELEPHONE_NUMBER) :
		pszTag = "PR_OFFICE2_TELEPHONE_NUMBER";                     break;
    case PROP_ID(PR_CELLULAR_TELEPHONE_NUMBER) :
		pszTag = "PR_CELLULAR_TELEPHONE_NUMBER";                    break;
    case PROP_ID(PR_RADIO_TELEPHONE_NUMBER) :
		pszTag = "PR_RADIO_TELEPHONE_NUMBER";                       break;
    case PROP_ID(PR_CAR_TELEPHONE_NUMBER) :
		pszTag = "PR_CAR_TELEPHONE_NUMBER";                         break;
    case PROP_ID(PR_OTHER_TELEPHONE_NUMBER) :
		pszTag = "PR_OTHER_TELEPHONE_NUMBER";                       break;
    case PROP_ID(PR_TRANSMITABLE_DISPLAY_NAME) :
		pszTag = "PR_TRANSMITABLE_DISPLAY_NAME";                    break;
    case PROP_ID(PR_BEEPER_TELEPHONE_NUMBER) :
		pszTag = "PR_BEEPER_TELEPHONE_NUMBER";                      break;
    case PROP_ID(PR_USER_CERTIFICATE) :
		pszTag = "PR_USER_CERTIFICATE";                             break;
    case PROP_ID(PR_PRIMARY_FAX_NUMBER) :
		pszTag = "PR_PRIMARY_FAX_NUMBER";                           break;
    case PROP_ID(PR_BUSINESS_FAX_NUMBER) :
		pszTag = "PR_BUSINESS_FAX_NUMBER";                          break;
    case PROP_ID(PR_HOME_FAX_NUMBER) :
		pszTag = "PR_HOME_FAX_NUMBER";                              break;
    case PROP_ID(PR_COUNTRY) :
		pszTag = "PR_COUNTRY";                                      break;
    case PROP_ID(PR_LOCALITY) :
		pszTag = "PR_LOCALITY";                                     break;
    case PROP_ID(PR_STATE_OR_PROVINCE) :
		pszTag = "PR_STATE_OR_PROVINCE";                            break;
    case PROP_ID(PR_STREET_ADDRESS) :
		pszTag = "PR_STREET_ADDRESS";                               break;
    case PROP_ID(PR_POSTAL_CODE) :
		pszTag = "PR_POSTAL_CODE";                                  break;
    case PROP_ID(PR_POST_OFFICE_BOX) :
		pszTag = "PR_POST_OFFICE_BOX";                              break;
    case PROP_ID(PR_TELEX_NUMBER) :
		pszTag = "PR_TELEX_NUMBER";                                 break;
    case PROP_ID(PR_ISDN_NUMBER) :
		pszTag = "PR_ISDN_NUMBER";                                  break;
    case PROP_ID(PR_ASSISTANT_TELEPHONE_NUMBER) :
		pszTag = "PR_ASSISTANT_TELEPHONE_NUMBER";                   break;
    case PROP_ID(PR_HOME2_TELEPHONE_NUMBER) :
		pszTag = "PR_HOME2_TELEPHONE_NUMBER";                       break;
    case PROP_ID(PR_ASSISTANT) :
		pszTag = "PR_ASSISTANT";                                    break;
    case PROP_ID(PR_SEND_RICH_INFO) :
		pszTag = "PR_SEND_RICH_INFO";                               break;


	// Profile section properties

	case PROP_ID(PR_STORE_PROVIDERS) :
		pszTag = "PR_STORE_PROVIDERS";                              break;
    case PROP_ID(PR_AB_PROVIDERS) :
		pszTag = "PR_AB_PROVIDERS";                                 break;
    case PROP_ID(PR_TRANSPORT_PROVIDERS) :
		pszTag = "PR_TRANSPORT_PROVIDERS";                          break;
    case PROP_ID(PR_DEFAULT_PROFILE) :
		pszTag = "PR_DEFAULT_PROFILE";                              break;
    case PROP_ID(PR_AB_SEARCH_PATH) :
		pszTag = "PR_AB_SEARCH_PATH";                               break;
    case PROP_ID(PR_AB_DEFAULT_DIR) :
		pszTag = "PR_AB_DEFAULT_DIR";                               break;
    case PROP_ID(PR_AB_DEFAULT_PAB) :
		pszTag = "PR_AB_DEFAULT_PAB";                               break;
    case PROP_ID(PR_FILTERING_HOOKS) :
		pszTag = "PR_FILTERING_HOOKS";                              break;
    case PROP_ID(PR_SERVICE_NAME) :
		pszTag = "PR_SERVICE_NAME";                                 break;
    case PROP_ID(PR_SERVICE_DLL_NAME) :
		pszTag = "PR_SERVICE_DLL_NAME";                             break;
    case PROP_ID(PR_SERVICE_ENTRY_NAME) :
		pszTag = "PR_SERVICE_ENTRY_NAME";                           break;
    case PROP_ID(PR_SERVICE_UID) :
		pszTag = "PR_SERVICE_UID";                                  break;
    case PROP_ID(PR_SERVICE_EXTRA_UIDS) :
		pszTag = "PR_SERVICE_EXTRA_UIDS";                           break;
    case PROP_ID(PR_SERVICES) :
		pszTag = "PR_SERVICES";                                     break;
    case PROP_ID(PR_SERVICE_SUPPORT_FILES) :
		pszTag = "PR_SERVICE_SUPPORT_FILES";                        break;
    case PROP_ID(PR_SERVICE_DELETE_FILES) :
		pszTag = "PR_SERVICE_DELETE_FILES";                         break;
    case PROP_ID(PR_AB_SEARCH_PATH_UPDATE) :
		pszTag = "PR_AB_SEARCH_PATH_UPDATE";                        break;


	// Status object properties

	case PROP_ID(PR_IDENTITY_DISPLAY) :
		pszTag = "PR_IDENTITY_DISPLAY";                             break;
    case PROP_ID(PR_IDENTITY_ENTRYID) :
		pszTag = "PR_IDENTITY_ENTRYID";                             break;
    case PROP_ID(PR_RESOURCE_METHODS) :
		pszTag = "PR_RESOURCE_METHODS";                             break;
    case PROP_ID(PR_RESOURCE_TYPE) :
		pszTag = "PR_RESOURCE_TYPE";                                break;
    case PROP_ID(PR_STATUS_CODE) :
		pszTag = "PR_STATUS_CODE";                                  break;
    case PROP_ID(PR_IDENTITY_SEARCH_KEY) :
		pszTag = "PR_IDENTITY_SEARCH_KEY";                          break;
    case PROP_ID(PR_OWN_STORE_ENTRYID) :
		pszTag = "PR_OWN_STORE_ENTRYID";                            break;
    case PROP_ID(PR_RESOURCE_PATH) :
		pszTag = "PR_RESOURCE_PATH";                                break;
    case PROP_ID(PR_STATUS_STRING) :
		pszTag = "PR_STATUS_STRING";                                break;
    case PROP_ID(PR_X400_DEFERRED_DELIVERY_CANCEL) :
		pszTag = "PR_X400_DEFERRED_DELIVERY_CANCEL";                break;
    case PROP_ID(PR_HEADER_FOLDER_ENTRYID) :
		pszTag = "PR_HEADER_FOLDER_ENTRYID";                        break;
    case PROP_ID(PR_REMOTE_PROGRESS) :
		pszTag = "PR_REMOTE_PROGRESS";                              break;
    case PROP_ID(PR_REMOTE_PROGRESS_TEXT) :
		pszTag = "PR_REMOTE_PROGRESS_TEXT";                         break;
    case PROP_ID(PR_REMOTE_VALIDATE_OK) :
		pszTag = "PR_REMOTE_VALIDATE_OK";                           break;


	// Display table properties

	case PROP_ID(PR_CONTROL_FLAGS) :
		pszTag = "PR_CONTROL_FLAGS";                                break;
    case PROP_ID(PR_CONTROL_STRUCTURE) :
		pszTag = "PR_CONTROL_STRUCTURE";                            break;
    case PROP_ID(PR_CONTROL_TYPE) :
		pszTag = "PR_CONTROL_TYPE";                                 break;
    case PROP_ID(PR_DELTAX) :
		pszTag = "PR_DELTAX";                                       break;
    case PROP_ID(PR_DELTAY) :
		pszTag = "PR_DELTAY";                                       break;
    case PROP_ID(PR_XPOS) :
		pszTag = "PR_XPOS";                                         break;
    case PROP_ID(PR_YPOS) :
		pszTag = "PR_YPOS";                                         break;
    case PROP_ID(PR_CONTROL_ID) :
		pszTag = "PR_CONTROL_ID";                                   break;
    case PROP_ID(PR_INITIAL_DETAILS_PANE) :
		pszTag = "PR_INITIAL_DETAILS_PANE";                         break;

    default :
        wsprintf (szUnkTag, "UNKNOWN (%x)", PROP_ID(ulPropTag));
        pszTag = szUnkTag;
        break;
    }

    char * pszType;
    char szUnkType[32];
    switch (PROP_TYPE(ulPropTag))
    {
    case PT_UNSPECIFIED :   pszType = "PT_UNSPECIFIED"; break;
    case PT_NULL :          pszType = "PT_NULL";         break;
    case PT_SHORT :          pszType = "PT_SHORT";        break;
    case PT_LONG :          pszType = "PT_LONG";         break;
    case PT_FLOAT :          pszType = "PT_FLOAT";        break;
    case PT_DOUBLE :         pszType = "PT_DOUBLE";      break;
    case PT_CURRENCY :       pszType = "PT_CURRENCY";    break;
    case PT_APPTIME :        pszType = "PT_APPTIME";     break;
    case PT_ERROR :          pszType = "PT_ERROR";        break;
    case PT_BOOLEAN :        pszType = "PT_BOOLEAN";     break;
    case PT_OBJECT :         pszType = "PT_OBJECT";      break;
    case PT_I8 :          pszType = "PT_I8";          break;
    case PT_STRING8 :        pszType = "PT_STRING8";     break;
    case PT_UNICODE :        pszType = "PT_UNICODE";     break;
    case PT_SYSTIME :        pszType = "PT_SYSTIME";     break;
    case PT_CLSID :          pszType = "PT_CLSID";        break;
    case PT_BINARY :         pszType = "PT_BINARY";      break;
    default :
         wsprintf (szUnkType, "UNKNOWN (%x)", PROP_TYPE(ulPropTag));
         pszType = szUnkType;
         break;
    }

    char message [128];
	int cch = wsprintf(message, "Tag: %s  Type: %s\r\n", pszTag, pszType);
    OutputDebugString(message);
}
#endif

#if defined(_DEBUG)
void xxx(LPEXCHEXTCALLBACK peecb)
{
	IMessage* pmsg;
	HRESULT hr = peecb->GetObject(NULL, (IMAPIProp**)&pmsg);
	assert(SUCCEEDED(hr));
	ULONG cprp;
	SPropValue* pprp;
	hr = pmsg->GetProps(NULL, 0, &cprp, &pprp);
	assert(SUCCEEDED(hr));
	while (cprp-- > 0)
	{
		SPropValue* pprpHere = pprp+cprp;
		DumpPropTag(pprpHere->ulPropTag);
	}
	MAPIFreeBuffer(pprp);
	pmsg->Release();
}
#endif
