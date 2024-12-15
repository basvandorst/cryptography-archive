#include <stdio.h>

#ifdef __STDC__

/***************************************************************
 *
 * Headers from file secure/readconf.c
 *
 ***************************************************************/ 
       RC	read_SC_configuration		(WhichSCConfig which_SCconfig);
       void	display_SC_configuration	();
       RC	get_default_configuration	();
       RC	check_SCapp_configuration	(char *app_name, Boolean onekeypair);
/***************************************************************
 *
 * Headers from file secure/secsc.c
 *
 ***************************************************************/ 
       RC	secsc_create	(PSESel *pse_sel);
       RC	secsc_open	(PSESel *pse_sel);
       RC	secsc_close	(PSESel *pse_sel);
       RC	secsc_delete	(PSESel *pse_sel);
       RC	secsc_write	(PSESel *pse_sel, OctetString *content);
       RC	secsc_read	(PSESel *pse_sel, OctetString *content);
       RC	secsc_chpin	(PSESel *pse_sel);
       RC	secsc_gen_key	(Key *key, Boolean replace);
       RC	secsc_sign	(OctetString *in_octets, Signature *signature, More more, Key *key, HashInput *hash_input);
       RC	secsc_verify	(OctetString *in_octets, Signature *signature, More more, Key *key, HashInput *hash_input);
       RC	secsc_encrypt	(OctetString *in_octets, BitString *out_bits, More more, Key *key);
       RC	secsc_decrypt	(BitString *in_bits, OctetString *out_octets, More more, Key *key);
       RC	secsc_get_EncryptedKey	(EncryptedKey *encrypted_key, Key *plain_key, Key *encryption_key);
       RC	secsc_put_EncryptedKey	(EncryptedKey *encrypted_key, Key *plain_key, Key *decryption_key, Boolean replace);
       RC	secsc_del_key		(KeyRef keyref);
       RC	secsc_unblock_SCpin	(PSESel *pse_sel);
#ifdef _SECSC_
       RC	secsc_sc_eject		(SCTSel sct_sel);
#endif
       char	*get_pse_pin_from_SC	(char *app_name);
       SCObjEntry	*aux_AppObjName2SCObj	(char *app_name, char *obj_name);
       SCAppEntry	*aux_AppName2SCApp	(char *app_name);
       int	SC_configuration	();

       int	SCT_configuration	();
       int 	handle_SC_app		(char *app_name);
#ifdef _SCA_
       int 	delete_devkeyset	(OctetString *app_id, KeyDevSel *key_dev_info[], unsigned int no_of_devkeys, KeyDevStatus keyset_status);
       int 	gen_devkeyset		(OctetString *app_id, KeyDevSel *key_dev_info[], unsigned int no_of_devkeys);
       int 	load_devkeyset		(OctetString *app_id, KeyDevSel *key_dev_info[], unsigned int no_of_devkeys, KeyDevStatus keyset_status);
       int	pre_devkeyset		(char *app_name, OctetString **app_id, KeyDevSel *key_dev_info[], unsigned int *no_of_devkeys);
       int 	write_on_keycards	(OctetString *app_id, KeyDevSel *key_dev_info[], unsigned int no_of_devkeys, char *keycard_pin);
#endif
       int	re_devkeyset		(char *app_name);
       int 	release_SCT		(int sct_id);
       int	request_keycard		(Boolean new_keycard);
       int 	reset_SCT		(int sct_id);


#else
/***************************************************************
 *
 * Headers from file secure/readconf.c
 *
 ***************************************************************/ 
       RC	read_SC_configuration		();
       void	display_SC_configuration	();
       RC	get_default_configuration	();
       RC	check_SCapp_configuration	();

/***************************************************************
 *
 * Headers from file secure/secsc.c
 *
 ***************************************************************/ 
       RC	secsc_create	();
       RC	secsc_open	();
       RC	secsc_close	();
       RC	secsc_delete	();
       RC	secsc_write	();
       RC	secsc_read	();
       RC	secsc_chpin	();
       RC	secsc_gen_key	();
       RC	secsc_sign	();
       RC	secsc_verify	();
       RC	secsc_encrypt	();
       RC	secsc_decrypt	();
       RC	secsc_get_EncryptedKey	();
       RC	secsc_put_EncryptedKey	();
       RC	secsc_del_key	();
       RC	secsc_unblock_SCpin	();
       RC	secsc_sc_eject	();
       char	*get_pse_pin_from_SC	();
       SCObjEntry	*aux_AppObjName2SCObj	();
       SCAppEntry	*aux_AppName2SCApp	();
       int	SC_configuration	();
       int	SCT_configuration	();
       int 	handle_SC_app		();
       int 	delete_devkeyset	();
       int 	gen_devkeyset		();
       int 	load_devkeyset		();
       int	pre_devkeyset		();
       int	re_devkeyset		();
       int 	release_SCT		();
       int	request_keycard		();
       int 	reset_SCT		();
       int 	write_on_keycards	();


#endif
