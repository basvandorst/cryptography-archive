/* vox.c

   Dave Hawkes 27/IX/95

   This rle codes the GSM or other compression to reduce data
   at low signal levels.  */

#include "speakfree.h"

extern gsm gsmh;		      /* GSM handle */

#define FAR

#ifdef NEEDED
static int rlecomp(int len, char FAR *lpData, char FAR *lpBuffer)
{
    char FAR *lpDCount;
    char FAR *lpStart;
    int i, scnt, dcnt;
    scnt = 1;
    dcnt = 0;
    lpDCount = NULL;
    lpStart = lpBuffer;
    for(i = 1; i < len; ++i) {
	++lpData;
	if(*lpData == *(lpData - 1)) {
	    if(scnt > 0) {
		++scnt;
		if(scnt >= 3) {
		    if(dcnt > 0) {
			*lpDCount = dcnt - 1;
			--lpBuffer;
			dcnt = 0;
		    }
		    if(scnt >= 120) {
			*lpBuffer++ = scnt | 0x80;
			*lpBuffer++ = *lpData;
			scnt = 0;
		    }
		}
	    } else {
		scnt = (dcnt > 0) ? 2 : 1;
	    }
	} else {
	    if(dcnt > 0) {
		if(scnt > 0) {
		    dcnt += scnt - 1;
		    while(--scnt)
			*lpBuffer++ = *(lpData - 1);
		}
		++dcnt;
		if(dcnt >= 120) {
		    *lpDCount = dcnt;
		    scnt = 0;
		    dcnt = 0;
		}
		*lpBuffer++ = *lpData;
	    } else {
		if(scnt >= 3) {
		    *lpBuffer++ = scnt | 0x80;
		    *lpBuffer++ = *(lpData - 1);
		    scnt = 1;
		    dcnt = 0;
		} else if(scnt == 0) {
		    scnt = 1;
		} else {
		    lpDCount = lpBuffer++;
		    dcnt = 1 + scnt;
		    while(scnt--) {
			*lpBuffer++ = *(lpData - 1);
		    }
		    *lpBuffer++ = *lpData;
		}
	    }
	}
    }
    if(dcnt > 0) {
	if(scnt > 0) {
	    dcnt += scnt - 1;
	    while(--scnt)
		*lpBuffer++ = *(lpData - 1);
	}
	*lpDCount = dcnt;
    } else if(scnt >= 3) {
	*lpBuffer++ = scnt | 0x80;
	*lpBuffer++ = *(lpData - 1);
    } else if(scnt > 0) {
	if(!lpDCount) {
	    lpDCount = lpBuffer++;
	    *lpDCount = 0;
	}
	*lpDCount += scnt;
	while(scnt--)
	    *lpBuffer++ = *(lpData - 1);
    }
    return lpBuffer - lpStart;
}
#endif

static int rledecomp(len, lpData, lpBuffer)
  int len;
  char *lpData, *lpBuffer;
{
    int j, cnt, val;
    char FAR *lpStart;
    char FAR *lpEnd;
    if(*lpBuffer == (char) ((unsigned char) 0x80)) {
	bzero(lpData, len);
	return len;
    }
    lpStart = lpData;
    lpEnd = lpData + len;
    while(lpData < lpEnd) {
	if(*lpBuffer & 0x80) {
	    cnt = *lpBuffer++ & 0x7f;
	    val = *lpBuffer++;
	    for(j = 0; j < cnt; ++j)
		*lpData++ = val;
	} else {
	    cnt = *lpBuffer++;
	    for(j = 0; j < cnt; ++j)
		*lpData++ = *lpBuffer++;
	}
    }
    return lpData - lpStart;
}

static gsm_frame gsm_zero = {
    216, 32, 162, 225, 90, 80, 0, 73, 36, 146, 73, 36, 80, 0, 73, 36, 146, 73,
    36, 80, 0, 73, 36, 146, 73, 36, 80, 0, 73, 36, 146, 73, 36 };

static char buffer[BUFL];

#ifdef NEEDED

#define GAIN_LEVELS	64
#define GAIN_HANG	    10

static int av_var = 2;
static int av_samples = 250;

static long packet_size = 0;
static int frame_size = 160;

static long gain_threshold = 0;
static int kill_count = 0;
static int vox_speed = 0;
int noise_threshold = 32;
static int gain_max = GAIN_LEVELS + GAIN_HANG;
static int gain_kill = 25;
static int decay_mask = 160;
static int attack_mask = 4;
int VoxForceReset = 0;

static int gain = GAIN_LEVELS + GAIN_HANG;

void vox_gsmcomp(struct soundbuf *asb, int use_GSM)
{
    gsm_signal src[244];    /* max frame size from load_vox_type_params */
    gsm_frame dst;
    int i, j, sz, l;
    char *dp = (asb->buffer.buffer_val) + sizeof(short);
    LONG ldata = asb->buffer.buffer_len; 
    int nframes;
    int sv, mx, mn, allzero;
    static long mx_diff;
    static int mx_av, mn_av, av_cnt = 0;
    static int decay_rem = 0, attack_rem = 0;
    long diff, av, local_mx_diff;
    static int NotDetectNoise = 0;
    static int DetectNoise = 0;
    
    if(packet_size != ldata) {
	if(!load_vox_type_params(0))
	    return; /* We don't support this size */
    }
    
    nframes = (int) ((ldata + frame_size - 1) / frame_size);

    l = nframes * sizeof(dst);
    allzero = 1;
    local_mx_diff = 0;
    for (i = 0; i < ldata; i += frame_size) {
	mx = mn = audio_u2s(asb->buffer.buffer_val[0]);
	av = 0;
	for (j = 0; j < frame_size; j++) {
	    if ((i + j) < ldata) {
		sv = audio_u2s(asb->buffer.buffer_val[i + j]);
		if(sv > mx)
		    mx = sv;
		else if(sv < mn)
		    mn = sv;
		src[j] = sv;
		av += abs(sv);
	    } else {
		sv = audio_u2s(asb->buffer.buffer_val[i + j - frame_size]);
		if(sv > mx)
		    mx = sv;
		else if(sv < mn)
		    mn = sv;
		av += abs(sv);
		src[j] = 0;
	    }
	}
	diff = (long) mx - mn;
	if(diff > local_mx_diff)
	  local_mx_diff = diff;
	av /= frame_size;
#ifdef ALLOW_VOX_WAVE
	if(1)
#else
	if(!isWaveSound) {  /* stop wave sounds upsetting VOX levels */
#endif
	    if(av_cnt++ == 0) {
		mx_av = mn_av = (int) av;
		mx_diff = diff;
	    } else {
		if(av_var * mn_av >= mx_av) {
		    if((int) av > mx_av)
			mx_av = (int) av;
		    if((int) av < mn_av)
			mn_av = (int) av;
		    if(diff > mx_diff)
			mx_diff = diff;
		    if(av_cnt >= av_samples) {
			av_cnt = 0;
			if(VoxForceReset || (gain_threshold == 0)) {
			    gain_threshold = mx_diff;
			    VoxForceReset = 0;
			} else
			    gain_threshold = ((15 * gain_threshold + mx_diff) / 16);
			NotDetectNoise = 0;
			if(++DetectNoise > 20) {
			    if(--av_var < 2)
			      av_var = 2;
			    DetectNoise = 0;
			}
		    }
		} else {
		    av_cnt = 0;
		    if(++NotDetectNoise > 3000) {
			NotDetectNoise = 2500;
			++av_var;
		    }
		    DetectNoise = 0;
		}
	    }
	    if((16 * diff) < (noise_threshold * gain_threshold)) {
		if(gain <= GAIN_LEVELS) {
		    attack_rem = 0;
		    if(gain > 0) {
			allzero = 0;
			for (j = 0; j < frame_size; j++) {
			    if(((j + decay_rem) % decay_mask) == 0)
				if(gain > 0)
				    --gain;
			    src[j] = (int) ((src[j] * (long) gain) / GAIN_LEVELS);
			}
			decay_rem = (decay_rem + frame_size) % decay_mask;
		    } else {
			bzero(src, frame_size * sizeof(src[0]));
			decay_rem = 0;
		    }
		    if(!use_GSM) {
			for (j = 0; j < frame_size; j++) {
			    asb->buffer.buffer_val[i + j] = audio_s2u(src[j]);
			}
		    }
		} else {
		    attack_rem = decay_rem = 0;
		    allzero = 0;
		    --gain;
		}
	    } else if(gain < GAIN_LEVELS) {
		decay_rem = 0;
		allzero = 0;
		for (j = 0; j < frame_size; j++) {
		    if(((j + attack_rem) % attack_mask) == 0)
			if(++gain >= GAIN_LEVELS) {
			    gain = gain_max;
			    attack_rem = 0;
			    break;
			}
		    src[j] = (int) ((src[j] * (long) gain) / GAIN_LEVELS);
		}
		attack_rem = (attack_rem + frame_size) % attack_mask;
		if(!use_GSM) {
		    for (j = 0; j < frame_size; j++) {
			asb->buffer.buffer_val[i + j] = audio_s2u(src[j]);
		    }
		}
	    }
	} else {
	    decay_rem = 0;
	    attack_rem = 0;
	    allzero = 0;
	    gain = gain_max;
	    kill_count = 0;
	}
	
	if(use_GSM) {
	    gsm_encode(gsmh, src, dst);
	    for(j = 0; j < sizeof(dst); ++j) {
		dp[j] = dst[j] - gsm_zero[j];
	    }
	    dp += sizeof dst;
	}
    }
    
    voxMonitorUpdate(local_mx_diff, (noise_threshold * gain_threshold) / 16);
    if(!use_GSM) {
	if(allzero) {
	    if(++kill_count > gain_kill) {
		asb->buffer.buffer_len = 0;
		kill_count = gain_kill;
	    }
	} else
	    kill_count = 0;
	return;
    }

    sz = rlecomp(l, (char *) (asb->buffer.buffer_val) + sizeof(short), buffer);

    /* Hide original uncompressed buffer length in first 2 bytes of buffer. */
    
    if(sz >= l) {
	asb->buffer.buffer_len = l + sizeof(short);
	*((short *) asb->buffer.buffer_val) = (short) ldata;
	kill_count = 0;
    } else {
	if(allzero) {
	    if(++kill_count > gain_kill) {
		ldata |= 0x4000;
		kill_count = gain_kill;
	    }
	    sz = 1;
	    *((char *) (asb->buffer.buffer_val) + sizeof(short)) = (char) ((unsigned char) 0x80);
	} else {
	    memcpy((char *) (asb->buffer.buffer_val) + sizeof(short), buffer, sz);
	    kill_count = 0;
	}
	asb->buffer.buffer_len = sz + sizeof(short);
	*((short *) asb->buffer.buffer_val) = (short) ldata | 0x8000;
    }
    *((short *)  asb->buffer.buffer_val) = htonl(*(short *) asb->buffer.buffer_val);
}
#endif

void vox_gsmdecomp(sb)
  struct soundbuf *sb;
{
    gsm_signal dst[160];
    int i, j, isz, l = 0;
    static char dcb[BUFL];
    short declen = ntohl(*((short *) sb->buffer.buffer_val));
    int cmpr;
    char *pbuffer;
    
    cmpr = (declen & 0x8000);
    declen &= ~(0x8000 | 0x4000);
    if (declen <= 0 || declen > 1600) {
	declen = 1600;
    }
    if(cmpr) {
	rledecomp(declen, buffer, ((char *) sb->buffer.buffer_val) + sizeof(short));
	pbuffer = buffer;
    } else
	pbuffer = ((char *) sb->buffer.buffer_val) + sizeof(short);
    for (i = 0; i < sb->buffer.buffer_len - sizeof(short);
		i += sizeof(gsm_frame)) {
	isz = 0;
	for(j = 0; j < sizeof(gsm_frame); ++j) {
	    isz |= pbuffer[j];
	    pbuffer[j] += gsm_zero[j];
	}
	if(isz == 0) {
	    memset(dcb, audio_s2u(0), sizeof(dcb));
	    l += 160;
	} else {
	    gsm_decode(gsmh, (gsm_byte *) pbuffer, dst);
	    for (j = 0; j < 160; j++) {
		dcb[l++] = audio_s2u(dst[j]);
	    }
	}
	pbuffer += sizeof(gsm_frame);
    }
    bcopy(dcb, sb->buffer.buffer_val, declen);
    sb->buffer.buffer_len = declen;
}

#ifdef NEEDED
int load_vox_type_params(int Force)
{
    static int OldVoxMode = IDM_VOX_NONE;
    const int speed_ids[] = { IDS_PF_VOX_SLOW, IDS_PF_VOX_MEDIUM, IDS_PF_VOX_FAST };
    int i;
    char buf[16];
    char *pfn = rstring(IDS_PF_PROFILE_FILE);
    int sample_count = inputSampleCount();
    
    if(!Force && (sample_count == packet_size))
	return 1;
    
    switch(sample_count) {
	case 3200:
	case 1600:
	    packet_size = sample_count;
	    frame_size = 160;
	    break;
	case 3600:
	case 1800:
	    packet_size = sample_count;
	    frame_size = 180;
	    break;
	case 1948:
	    packet_size = sample_count;
	    frame_size = 150;	/* 2 bytes over - but should be OK */
	    break;
	case 972:
	    packet_size = sample_count;
	    frame_size = 162;
	    break;
	case 976:
	case 488:
	    packet_size = sample_count;
	    frame_size = 244;
	    break;
	default:
	    return 0;
    }

    switch(voxmode) {
	case IDM_VOX_FAST: vox_speed = 2; break;
	case IDM_VOX_MEDIUM: vox_speed = 1; break;
	case IDM_VOX_SLOW: vox_speed = 0; break;
	case IDM_VOX_NONE: vox_speed = 0; break;
	default: vox_speed = 0; voxmode = IDM_VOX_NONE; break;
    }
    
    if(OldVoxMode != IDM_VOX_NONE) {
	switch(OldVoxMode) {
	    case IDM_VOX_FAST: i = 2; break;
	    case IDM_VOX_MEDIUM: i = 1; break;
	    case IDM_VOX_SLOW: i = 0; break;
	    default: i = 0; break;
	}
	_itoa(noise_threshold, buf, 10);
	WritePrivateProfileString(rstring(speed_ids[i]), rstring(IDS_PF_VOX_NOISE_THRESH), buf, pfn);
    }
    OldVoxMode = voxmode;
    
    av_samples = (int) (((long) 5000 * 8 + frame_size / 2) / frame_size);   /* 5 seconds */
    noise_threshold = GetPrivateProfileInt(rstring(speed_ids[vox_speed]), rstring(IDS_PF_VOX_NOISE_THRESH), noise_threshold, pfn);
    pfn = rstring(IDS_PF_PROFILE_FILE); /* refresh */
    i = (int) (((long) attack_mask * GAIN_LEVELS + 4) / 8);
    i = GetPrivateProfileInt(rstring(speed_ids[vox_speed]), rstring(IDS_PF_VOX_ATTACK), i, pfn);
    attack_mask = (int) (((long) i * 8 + GAIN_LEVELS / 2) / GAIN_LEVELS);
    if(attack_mask < 1)
	attack_mask = 1;
    i = (int) (((long) (gain_max - GAIN_LEVELS) * frame_size + 4) / 8);
    i = GetPrivateProfileInt(rstring(speed_ids[vox_speed]), rstring(IDS_PF_VOX_HOLD), i, pfn);
    gain = gain_max = (int) (((long) i * 8 + frame_size / 2) / frame_size) + GAIN_LEVELS;
    pfn = rstring(IDS_PF_PROFILE_FILE); /* refresh */
    i = (int) (((long) decay_mask * GAIN_LEVELS + 4) / 8);
    i = GetPrivateProfileInt(rstring(speed_ids[vox_speed]), rstring(IDS_PF_VOX_DECAY), i, pfn);
    decay_mask = (int) (((long) i * 8 + GAIN_LEVELS / 2) / GAIN_LEVELS);
    if(decay_mask < 1)
	decay_mask = 1;
    i = (int) (((long) gain_kill * packet_size + 4) / 8);
    i = GetPrivateProfileInt(rstring(speed_ids[vox_speed]), rstring(IDS_PF_VOX_RELEASE), i, pfn);
    gain_kill = (int) (((long) i * 8 + packet_size / 2) / packet_size);
    voxMonitorUpdate(0, (noise_threshold * gain_threshold) / 16);
    return 1;
}

/*  VOX_RESET_PARAMETERS  --  Reset all VOX parameters to defaults */

void vox_reset_parameters(void)
{
    gain_threshold = 0;
    kill_count = 0;
    vox_speed = 0;
    noise_threshold = 32;
    gain_max = GAIN_LEVELS + GAIN_HANG;
    gain_kill = 25;
    decay_mask = 160;
    attack_mask = 4;
    load_vox_type_params(1);	
}

int load_vox_params(void)
{
    char *pfn = rstring(IDS_PF_PROFILE_FILE);
    const int speed_ids[] = { IDS_PF_VOX_SLOW, IDS_PF_VOX_MEDIUM, IDS_PF_VOX_FAST };
    int h[] = { 1000, 500, 250 };
    int d[] = { 1000, 500, 250 };
    int r[] = { 5000, 1000, 0 };
    char buf[16];
    int i, n;
    
    gain_threshold = GetPrivateProfileInt(rstring(IDS_PF_VOX), rstring(IDS_PF_VOX_GAIN), (int) gain_threshold, pfn);
    voxmode = GetPrivateProfileInt(rstring(IDS_PF_VOX), rstring(IDS_PF_VOX_SPEED), voxmode, pfn);
    breakinput = GetPrivateProfileInt(rstring(IDS_PF_VOX), rstring(IDS_PF_VOX_BREAK), 0, pfn);
    for(i = 0; i < 3; ++i) {
	pfn = rstring(IDS_PF_PROFILE_FILE); /* refresh */
	n = GetPrivateProfileString(rstring(speed_ids[i]), rstring(IDS_PF_VOX_NOISE_THRESH), "", buf, sizeof(buf), pfn);
	if(n == 0) {
	    _itoa(32, buf, 10);
	    WritePrivateProfileString(rstring(speed_ids[i]), rstring(IDS_PF_VOX_NOISE_THRESH), buf, pfn);
	    _itoa(32, buf, 10);
	    WritePrivateProfileString(rstring(speed_ids[i]), rstring(IDS_PF_VOX_ATTACK), buf, pfn);
	    pfn = rstring(IDS_PF_PROFILE_FILE); /* refresh */
	    _itoa(h[i], buf, 10);
	    WritePrivateProfileString(rstring(speed_ids[i]), rstring(IDS_PF_VOX_HOLD), buf, pfn);
	    _itoa(d[i], buf, 10);
	    WritePrivateProfileString(rstring(speed_ids[i]), rstring(IDS_PF_VOX_DECAY), buf, pfn);
	    _itoa(r[i], buf, 10);
	    WritePrivateProfileString(rstring(speed_ids[i]), rstring(IDS_PF_VOX_RELEASE), buf, pfn);
	}
    }
    load_vox_type_params(1);
    return 1;
}

int save_vox_params(void)
{
    const int speed_ids[] = { IDS_PF_VOX_SLOW, IDS_PF_VOX_MEDIUM, IDS_PF_VOX_FAST };
    char *pfn = rstring(IDS_PF_PROFILE_FILE);
    char buf[16];
    int i;
    _ltoa(gain_threshold, buf, 10);
    WritePrivateProfileString(rstring(IDS_PF_VOX), rstring(IDS_PF_VOX_GAIN), buf, pfn);
    _ltoa(breakinput, buf, 10);
    WritePrivateProfileString(rstring(IDS_PF_VOX), rstring(IDS_PF_VOX_BREAK), buf, pfn);
    _ltoa(voxmode, buf, 10);
    WritePrivateProfileString(rstring(IDS_PF_VOX), rstring(IDS_PF_VOX_SPEED), buf, pfn);
    pfn = rstring(IDS_PF_PROFILE_FILE);
    switch(voxmode) {
	case IDM_VOX_FAST: i = 2; break;
	case IDM_VOX_MEDIUM: i = 1; break;
	case IDM_VOX_SLOW: i = 0; break;
	default: i = 0; break;
    }
    _itoa(noise_threshold, buf, 10);
    WritePrivateProfileString(rstring(speed_ids[i]), rstring(IDS_PF_VOX_NOISE_THRESH), buf, pfn);
    return 1;
}
#endif
