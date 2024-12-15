// vox.h
// Dave Hawkes 27/IX/95

extern void vox_gsmcomp(struct soundbuf *asb, int use_GSM);
extern void vox_gsmdecomp(struct soundbuf *sb);
extern int load_vox_type_params(int Force);
extern void vox_reset_parameters(void);
extern int load_vox_params(void);
extern int save_vox_params(void);

extern int noise_threshold;
extern int VoxForceReset;
