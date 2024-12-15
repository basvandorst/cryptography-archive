/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMPhoto.c - photoID routines
	

	$Id: KMPhoto.c,v 1.16 1998/08/11 14:43:35 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// pgp header files
#include "pgpJPEG.h"
#include "pgpClientShared.h"
#include "pgpEndianConversion.h"

// external global variables
extern HINSTANCE g_hInst;

// constant definitions
#define DISPLAY_WIDTH		120
#define MAX_FILE_WIDTH		1000
#define MAX_FILE_HEIGHT		1200
#define MAX_PHOTO_WIDTH		120
#define MAX_PHOTO_HEIGHT	144

// error handler struct for JPEG library
struct my_error_mgr {
	struct jpeg_error_mgr pub;	
	jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

//	___________________________________________________
//
//	extract size information from DIB

INT
KMGetDIBSize (LPBITMAPINFO lpbi, INT* piWidth, INT* piHeight)
{
	INT	iRowSize;
	INT	iDIBSize;

	// return image dimensions
	if (piWidth)
		*piWidth = lpbi->bmiHeader.biWidth;
	if (piHeight)
		*piHeight = lpbi->bmiHeader.biHeight;

	// rows length must be a multiple of sizeof(LONG)
	iRowSize = (lpbi->bmiHeader.biWidth * lpbi->bmiHeader.biBitCount) >>3;
	iRowSize = ((iRowSize+3)>>2)<<2;

	// calculate image size by multiplying row length by number of rows
	// and adding in header and color table sizes
	iDIBSize = lpbi->bmiHeader.biSize;
	iDIBSize += lpbi->bmiHeader.biClrUsed * sizeof(RGBQUAD);
	iDIBSize += iRowSize * lpbi->bmiHeader.biHeight;

	return iDIBSize;
}
 
//	___________________________________________________
//
//	create logical palette from bitmap color table

static HPALETTE 
sPaletteFromDIB (
		  LPBITMAPINFO	lpbmi, 
		  INT*			piNumColors) 
{
	HPALETTE			hPal	= NULL;
	LPBITMAPINFOHEADER	lpbi;
	LPLOGPALETTE		lpPal;
	HANDLE				hLogPal;
	INT					i;
 
	lpbi = (LPBITMAPINFOHEADER)lpbmi;

	// palettes are only used on images with 8 bits/pixel or less
	switch (lpbi->biBitCount) {
	case 1 :
		*piNumColors = 2;
		break;

	case 4 :
		*piNumColors = 16;
		break;

	case 8 :
		if (lpbi->biClrUsed) 
			*piNumColors = lpbi->biClrUsed;
		else 
			*piNumColors = 256;
		break;

	default :
		*piNumColors = 0;
		break;
	}
 
	// if a palette is indicated, extract it from bitmap color table
	if (*piNumColors) {
		hLogPal = GlobalAlloc (GHND, sizeof (LOGPALETTE) +
                             sizeof (PALETTEENTRY) * (*piNumColors));
		lpPal = (LPLOGPALETTE) GlobalLock (hLogPal);
		lpPal->palVersion = 0x300;
		lpPal->palNumEntries = *piNumColors;
 
		for (i=0; i<*piNumColors; i++) {
			lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
		}
		hPal = CreatePalette (lpPal);
		GlobalUnlock (hLogPal);
		GlobalFree (hLogPal);
	}

	return hPal;
}
 
//	___________________________________________________
//
//	derive DDB and palette from DIB 

HBITMAP
KMDDBfromDIB (
		LPBITMAPINFO	lpbi, 
		HPALETTE*		lphPalette)
{
	HBITMAP		hBitmapFinal = NULL;
	HDC			hdc;
    INT			iNumColors;
	LPVOID		lpPixels;
 
	hdc = GetDC (NULL);

	*lphPalette = NULL;
	iNumColors = 0;

	if (GetDeviceCaps (hdc, RASTERCAPS) & RC_PALETTE) 
		*lphPalette = sPaletteFromDIB ((LPBITMAPINFO)lpbi, &iNumColors);

	if (*lphPalette) {
		SelectPalette (hdc, *lphPalette, FALSE);
		RealizePalette (hdc);
	}
 
	lpPixels = (LPBYTE)lpbi + 
				lpbi->bmiHeader.biSize +
				(iNumColors * sizeof(RGBQUAD));

	hBitmapFinal = CreateDIBitmap (hdc,
                   &(lpbi->bmiHeader),
                   CBM_INIT,
                   lpPixels,
                   lpbi,
                   DIB_RGB_COLORS);
 
	ReleaseDC (NULL, hdc);

	return (hBitmapFinal);
}

//	___________________________________________________
//
//	error handler routine for JPEG library
//
//	converts error code to string and displays it in MessageBox

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
//	CHAR szError [JMSG_LENGTH_MAX];

	my_error_ptr myerr = (my_error_ptr) cinfo->err;

//	(*cinfo->err->format_message) (cinfo, szError);
//	MessageBox (NULL, szError, "JPEG Library Error", MB_OK);
//
	longjmp(myerr->setjmp_buffer, 1);
}

//	___________________________________________________
//
//	convert DIB to JPEG buffer

static PGPError
sJPEGFromDIB (
		LPBITMAPINFO	lpbmi, 
		INT				iQuality,
		INT				iMaxWidth,
		INT				iMaxHeight,
		BOOL			bShrink,
		LPBYTE*			pbuf, 
		INT*			piLength)
{
	PGPError	err				= kPGPError_NoErr;

	INT			iColorTableLength;
	INT			iRowOrigLen, iRowJPEGLen, iRowDIBLen, iDIBsize;
	INT			i, j, index;
	INT			iWidth, iHeight, iZoom;
	INT			idxX, idxY;
	struct		my_error_mgr jerr;
	struct		jpeg_compress_struct cinfo;
	FILE*		outfile;
	LPBYTE		pDIB;
	LPBYTE		pRGBorig;
	LPBYTE		pRGBjpeg;
	JSAMPROW	pSampRow[1];	
	double		fZoom, fImageRatio, fMaxRatio, fTemp;

	// do we really need to shrink image ?
	fZoom = 1.0;
	iZoom = 1000;
	iWidth = lpbmi->bmiHeader.biWidth;
	iHeight = lpbmi->bmiHeader.biHeight;

	if (bShrink) {
		if ((lpbmi->bmiHeader.biWidth > iMaxWidth) ||
			(lpbmi->bmiHeader.biHeight > iMaxHeight)) {

			fImageRatio = (double)lpbmi->bmiHeader.biWidth /
						  (double)lpbmi->bmiHeader.biHeight;
			fMaxRatio   = (double)iMaxWidth / (double)iMaxHeight;

			if (fImageRatio >= fMaxRatio) {
				fZoom = (double)iMaxWidth / 
						(double)lpbmi->bmiHeader.biWidth;
				fTemp = (1000.0 / fZoom);

				iWidth = iMaxWidth;
				iHeight = (INT)(lpbmi->bmiHeader.biHeight * fZoom);

				iZoom = (INT)fTemp;
			}
			else {
				fZoom = (double)iMaxHeight / 
						(double)lpbmi->bmiHeader.biHeight;
				fTemp = (1000.0 / fZoom);

				iHeight = iMaxHeight;
				iWidth = (INT)(lpbmi->bmiHeader.biWidth * fZoom);

				iZoom = (INT)fTemp;
			}
		}
	}

	// calculate length of row of image in bytes (3 color channels)
	iRowOrigLen = lpbmi->bmiHeader.biWidth * 3;
	iRowJPEGLen = (iWidth+1) * 3;	

	// calculate length of row of image in DIB structure
	iRowDIBLen = (lpbmi->bmiHeader.biWidth * lpbmi->bmiHeader.biBitCount) >>3;
	iRowDIBLen = ((iRowDIBLen+3)>>2)<<2;

	// allocate buffer used for passing rows of RGB data into library
	pRGBorig = KMAlloc (iRowOrigLen);
	if (pRGBorig == NULL) 
		return kPGPError_OutOfMemory;

	pRGBjpeg = KMAlloc (iRowJPEGLen);
	if (pRGBjpeg == NULL) 
		return kPGPError_OutOfMemory;

	// calculate size of DIB structure
	iColorTableLength = lpbmi->bmiHeader.biClrUsed;
	if (iColorTableLength == 0) {
		switch (lpbmi->bmiHeader.biBitCount) {
		case 1 :
			iColorTableLength = 2;
			break;
		case 4 :
			iColorTableLength = 16;
			break;
		case 8 :
			iColorTableLength = 256;
			break;
		default :
			iColorTableLength = 0;
			break;
		}
	}
	iDIBsize = sizeof(BITMAPINFOHEADER);
	iDIBsize += (iColorTableLength * sizeof (RGBQUAD));
	iDIBsize += (iRowDIBLen * lpbmi->bmiHeader.biHeight);

	// initialize pointer to last row of DIB (DIB is stored bottom-up)
	pDIB = (LPBYTE)lpbmi;
	pDIB += iDIBsize;
	pDIB -= iRowDIBLen;

	// JPEG library will write to temporary file
	outfile = tmpfile ();

	// setup error handler for JPEG library
	cinfo.err = jpeg_std_error (&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
		// will be here only on error in library
		jpeg_destroy_compress (&cinfo);
		fclose (outfile);
		_rmtmp ();
		return kPGPError_Win32_InvalidImage;
	}

	// initialize compression structure
	jpeg_create_compress (&cinfo);

	// specify destination of JPEG stream
	jpeg_stdio_dest (&cinfo, outfile);

	// specify image characteristics
	cinfo.image_width = iWidth;
	cinfo.image_height = iHeight;
	cinfo.input_components = 3;	
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults (&cinfo);

	// specify quality of compressed image
	jpeg_set_quality (&cinfo, iQuality, TRUE);

	// startup compression
	jpeg_start_compress (&cinfo, TRUE);

	pSampRow[0] = pRGBjpeg;
	idxY = 0;
	while (cinfo.next_scanline < cinfo.image_height) {

		// fill buffer of data on basis of number of bits/pixel
		switch (lpbmi->bmiHeader.biBitCount) {
		case 1 :
			// B/W image
			for (j=i=0; i<iRowOrigLen; i+=3) {
				index = pDIB [j>>3];
				index = (index >> (7 - (j&0x07))) & 0x01;
				j++;

				pRGBorig[i+0] = lpbmi->bmiColors[index].rgbRed;
				pRGBorig[i+1] = lpbmi->bmiColors[index].rgbGreen;
				pRGBorig[i+2] = lpbmi->bmiColors[index].rgbBlue;
			}
			break;

		case 4 :
			// 16 color image, derive RGB values from color table
			for (j=i=0; i<iRowOrigLen; i+=3) {
				if (j & 0x1) 
					index = pDIB [j>>1] & 0x0F;
				else 
					index = pDIB [j>>1] >> 4;
				j++;

				pRGBorig[i+0] = lpbmi->bmiColors[index].rgbRed;
				pRGBorig[i+1] = lpbmi->bmiColors[index].rgbGreen;
				pRGBorig[i+2] = lpbmi->bmiColors[index].rgbBlue;
			}
			break;

		case 8 :
			// 256 color image, derive RGB values from color table
			for (j=i=0; i<iRowOrigLen; i+=3) {
				index = pDIB[j++];
				pRGBorig[i+0] = lpbmi->bmiColors[index].rgbRed;
				pRGBorig[i+1] = lpbmi->bmiColors[index].rgbGreen;
				pRGBorig[i+2] = lpbmi->bmiColors[index].rgbBlue;
			}
			break;

		case 24 :
			// RGB image, but DIB stores it as BGR, so invert
			for (i=0; i<iRowOrigLen; i+=3) {
				pRGBorig[i+0] = pDIB[i+2];
				pRGBorig[i+1] = pDIB[i+1];
				pRGBorig[i+2] = pDIB[i+0];
			}
			break;

		case 32 :
			// RGB image, but in DIB it's 0BGR, so invert and discard byte
			for (i=j=0; i<iRowOrigLen; i+=3) {
				pRGBorig[i+0] = pDIB[j+3];
				pRGBorig[i+1] = pDIB[j+2];
				pRGBorig[i+2] = pDIB[j+1];
				j += 4;
			}
			break;
		}

		// subsample the row of data
		idxX = 0;
		for (i=j=0; j<iRowOrigLen; j+=3) {
			idxX -= 1000;
			while (idxX < 0) {
				pRGBjpeg[i+0] = pRGBorig[j+0];
				pRGBjpeg[i+1] = pRGBorig[j+1];
				pRGBjpeg[i+2] = pRGBorig[j+2];

				i += 3;
				idxX += iZoom;
			}
		}

		// pass the RGB data into the library
		(void) jpeg_write_scanlines (&cinfo, pSampRow, 1);

		// step to previous DIB scan line
		idxY += iZoom;
		while (idxY > 0) {
			pDIB -= iRowDIBLen;
			idxY -= 1000;
		}
	}

	// done with image
	jpeg_finish_compress (&cinfo);
	jpeg_destroy_compress (&cinfo);
	KMFree (pRGBorig);
	KMFree (pRGBjpeg);

	// now extract JPEG data from file -- first calculate buffer length
	fseek (outfile, 0, SEEK_END);
	*piLength = ftell (outfile);

	// allocate buffer
	*pbuf = KMAlloc (*piLength);

	// read data from file into buffer
	if (*pbuf != NULL) {
		rewind (outfile);
		fread (*pbuf, 1, *piLength, outfile); 
	}

	// memory allocation error
	else 
		err = kPGPError_OutOfMemory;

	fclose (outfile);
	_rmtmp ();

	return err;
}


//	___________________________________________________
//
//	convert DIB to PhotoID buffer

static PGPError
sPhotoFromDIB (
		LPBITMAPINFO	lpbmi, 
		INT				iMaxWidth,
		INT				iMaxHeight,
		BOOL			bShrink,
		LPBYTE*			pbuf, 
		INT*			piLength)
{
	PGPError				err;
	LPBYTE					pJPEGbuf;
	INT						iJPEGlen;
	PGPPhotoUserIDHeader*	pheader;
	LPBYTE					pdata;
	PGPUInt16				usHeaderLength;
	
	// currently we only support JPEG photoids
	err = sJPEGFromDIB (lpbmi, kPGPDefaultJPEQQuality, 
		iMaxWidth, iMaxHeight, bShrink, &pJPEGbuf, &iJPEGlen);

	if (IsntPGPError (err)) {
		*piLength = iJPEGlen + sizeof(PGPPhotoUserIDHeader);
		*pbuf = KMAlloc (*piLength);

		if (*pbuf) {
			pheader = (PGPPhotoUserIDHeader*)*pbuf;
			usHeaderLength = sizeof(PGPPhotoUserIDHeader);
			PGPUInt16ToStorage (usHeaderLength, 
								(PGPByte*)&(pheader->headerLength));
			pheader->headerVersion 	= kPGPPhotoIDHeaderVersion;
			pheader->photoIDFormat	= kPGPPhotoIDFormat_JPEG;

			pdata = *pbuf + usHeaderLength;

			memcpy (pdata, pJPEGbuf, iJPEGlen);
		}
		else {
			*piLength = 0;
			err = kPGPError_OutOfMemory;
		}

	}

	return err;
}

//	___________________________________________________
//
//	convert JPEG buffer to DIB 

static LPBITMAPINFO
sDIBfromJPEGFile (
		FILE*	infile, 
		BOOL	bForDisplay)
{
	LPBITMAPINFO	lpbmi	= NULL;

	INT				i, iNumBits, iDIBsize;
	INT				iRowDataLen, iRowDIBLen;
	struct			my_error_mgr jerr;
	struct			jpeg_decompress_struct cinfo;
	HDC				hdc;
	JSAMPARRAY		buffer;
	JSAMPROW		pSamp;
	LPBYTE			pDIB;
	BOOL			bQuantize;


	// get pixel depth supported by the current display device
	hdc = GetDC (NULL);
	iNumBits = GetDeviceCaps (hdc, BITSPIXEL) * GetDeviceCaps (hdc, PLANES);
	ReleaseDC (NULL, hdc);

	// quantize colors only if a 256 color display
	//	if more than 256, display is RGB and quantization is not needed
	//	if less than 256, quantization does no good as system palette will
	//		be used regardless
	if (bForDisplay && (iNumBits == 8)) bQuantize = TRUE;
	else bQuantize = FALSE;

	// prepare error handling structure for JPEG library
	cinfo.err = jpeg_std_error (&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
		// here only when JPEG library encounters an error
		jpeg_destroy_decompress (&cinfo);
		return NULL;
	}

	// initialize decompression structure
	jpeg_create_decompress (&cinfo);

	// identify source of JPEG data as input file
	jpeg_stdio_src (&cinfo, infile);

	// load header data from file
	(void) jpeg_read_header (&cinfo, TRUE);

	// set scaling factor for decompression
	if (bForDisplay) {
		cinfo.scale_num = 1;
		if (cinfo.image_width > 4*DISPLAY_WIDTH) 
			cinfo.scale_denom = 8;
		else if (cinfo.image_width > 2*DISPLAY_WIDTH) 
			cinfo.scale_denom = 4;
		else if (cinfo.image_width > 1*DISPLAY_WIDTH) 
			cinfo.scale_denom = 2;
		else 
			cinfo.scale_denom = 1;
	}
	else {
		cinfo.scale_num = 1;
		cinfo.scale_denom = 1;
	}

	// set decompression parameters
	if (bQuantize) {
		cinfo.quantize_colors = TRUE;
		cinfo.two_pass_quantize = TRUE;
		cinfo.dither_mode = JDITHER_FS;
		cinfo.desired_number_of_colors = 128;
	}
	else {
		cinfo.quantize_colors = FALSE;
	}

	// start decompression
	(void) jpeg_start_decompress (&cinfo);

	// calculate bitmap size parameters and set DIB header info
	if (bQuantize) {
		iRowDataLen = cinfo.output_width * 1;
		iRowDIBLen = ((iRowDataLen+3)>>2)<<2;
		iDIBsize = sizeof(BITMAPINFOHEADER);
		iDIBsize += (cinfo.actual_number_of_colors * sizeof (RGBQUAD));
		iDIBsize += (iRowDIBLen * cinfo.output_height);

		lpbmi = KMAlloc (iDIBsize);
		lpbmi->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
		lpbmi->bmiHeader.biWidth = cinfo.output_width;
		lpbmi->bmiHeader.biHeight = cinfo.output_height;
		lpbmi->bmiHeader.biPlanes = 1;
		lpbmi->bmiHeader.biBitCount = 8;
		lpbmi->bmiHeader.biCompression = BI_RGB;
		lpbmi->bmiHeader.biSizeImage = iRowDIBLen * cinfo.output_height;
		lpbmi->bmiHeader.biClrUsed = cinfo.actual_number_of_colors;
		lpbmi->bmiHeader.biClrImportant = cinfo.actual_number_of_colors;
	}
	else {
		iRowDataLen = cinfo.output_width * 3;
		iRowDIBLen = ((iRowDataLen+3)>>2)<<2;
		iDIBsize = sizeof(BITMAPINFOHEADER);
		iDIBsize += (iRowDIBLen * cinfo.output_height);

		lpbmi = KMAlloc (iDIBsize);
		lpbmi->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
		lpbmi->bmiHeader.biWidth = cinfo.output_width;
		lpbmi->bmiHeader.biHeight = cinfo.output_height;
		lpbmi->bmiHeader.biPlanes = 1;
		lpbmi->bmiHeader.biBitCount = 24;
		lpbmi->bmiHeader.biCompression = BI_RGB;
		lpbmi->bmiHeader.biSizeImage = iRowDIBLen * cinfo.output_height;
		lpbmi->bmiHeader.biClrUsed = 0;
		lpbmi->bmiHeader.biClrImportant = 0;
	}

	// have JPEG library allocate buffer for data output
	buffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr) &cinfo, JPOOL_IMAGE, iRowDataLen, 1);

	// put color table into DIB
	if (bQuantize) {
		pSamp = cinfo.colormap[0];
		for (i = 0;  i < cinfo.actual_number_of_colors;  i++) 
			lpbmi->bmiColors[i].rgbRed = pSamp[i];

		pSamp = cinfo.colormap[1];
		for (i = 0;  i < cinfo.actual_number_of_colors;  i++) 
			lpbmi->bmiColors[i].rgbGreen = pSamp[i];

		pSamp = cinfo.colormap[2];
		for (i = 0;  i < cinfo.actual_number_of_colors;  i++) 
			lpbmi->bmiColors[i].rgbBlue = pSamp[i];
	}

	// initialize pointer to last scan row of DIB (DIB is bottom-up)
	pDIB = (LPBYTE)lpbmi;
	pDIB += iDIBsize;
	pDIB -= iRowDIBLen;

	// decompress the image, one line at a time
	while (cinfo.output_scanline < cinfo.output_height) {
		(void) jpeg_read_scanlines (&cinfo, buffer, 1);

		// if quantized, just store the colormap index in the DIB
		if (bQuantize) {
			pSamp = buffer[0];
			for (i=0; i<iRowDataLen; i++) {
				pDIB[i] = pSamp[i];
			}
		}
		// otherwise data is RGB, but DIB are in BGR order, so invert
		else {
			pSamp = buffer[0];
			for (i=0; i<iRowDataLen; i+=3) {
				pDIB[i+2] = pSamp[i+0];
				pDIB[i+1] = pSamp[i+1];
				pDIB[i+0] = pSamp[i+2];
			}
		}

		// step to previous row
		pDIB -= iRowDIBLen;
	}

	// finished decompressing
	jpeg_finish_decompress (&cinfo);
	jpeg_destroy_decompress (&cinfo);

	return lpbmi;
}

//	___________________________________________________
//
//	convert JPEG buffer to DIB 

static LPBITMAPINFO
sDIBfromJPEGBuffer (
		LPBYTE	buf, 
		INT		isize, 
		BOOL	bForDisplay)
{
	LPBITMAPINFO	lpbmi	= NULL;

	FILE*			infile;

	// write JPEG data to temp file for library to read from
	infile = tmpfile ();
	fwrite (buf, 1, isize, infile);
	rewind (infile);

	// convert the file
	lpbmi = sDIBfromJPEGFile (infile, bForDisplay);

	// close and delete temporary file
	fclose (infile);
	_rmtmp ();

	return lpbmi;
}

//	___________________________________________________
//
//	convert PhotoID buffer to DIB 

PGPError
KMDIBfromPhoto (
		LPBYTE			buf, 
		INT				isize, 
		BOOL			bForDisplay,
		LPBITMAPINFO*	plpbmi)
{
	PGPError				err				= kPGPError_NoErr;
	PGPPhotoUserIDHeader*	pheader;
	LPBYTE					pdata;
	PGPUInt16				usHeaderLength;

	if (!plpbmi) return kPGPError_BadParams;

	*plpbmi = NULL;
	pheader = (PGPPhotoUserIDHeader*)buf;

	if (pheader->headerVersion != kPGPPhotoIDHeaderVersion)
		err = kPGPError_BadParams;
	else {
		switch (pheader->photoIDFormat) {
		case kPGPPhotoIDFormat_JPEG :
			usHeaderLength = 
				PGPStorageToUInt16 ((PGPByte*)&pheader->headerLength);
			pdata = buf + usHeaderLength;
			*plpbmi = sDIBfromJPEGBuffer (pdata, 
						isize - usHeaderLength, bForDisplay);
			break;

		default :
			err = kPGPError_BadParams;
		}
	}

	return err;
}

//	___________________________________________________
//
//	copy PhotoID buffer to clipboard in DIB format

PGPError
KMCopyPhotoToClipboard (HWND hWnd, LPBYTE buf, INT isize)
{
	PGPError		err;
	LPBITMAPINFO	lpbmi;
	HANDLE			hMem;
	LPBYTE			pMem;
	INT				iDIBsize;

	err = KMDIBfromPhoto (buf, isize, FALSE, &lpbmi);

	if (IsntPGPError (err)) {
		iDIBsize = KMGetDIBSize (lpbmi, NULL, NULL);
		hMem = GlobalAlloc (GMEM_MOVEABLE|GMEM_DDESHARE, iDIBsize);
		pMem = GlobalLock (hMem);
		CopyMemory (pMem, lpbmi, iDIBsize);
		GlobalUnlock (hMem);

		OpenClipboard (hWnd);
		SetClipboardData (CF_DIB, hMem);
		CloseClipboard ();

		KMFree (lpbmi);
	}

	return err;
}

//	___________________________________________________
//
//	paste data in clipboard (DIB format) into JPEG buffer

PGPError
KMPastePhotoFromClipboard (HWND hWnd, LPBYTE* pbuf, INT* pisize)
{
	HANDLE			hMem;
	LPBITMAPINFO	lpbmi;
	PGPError		err;

	if (OpenClipboard (hWnd)) { 
		hMem = GetClipboardData (CF_DIB); 
		lpbmi = (LPBITMAPINFO)GlobalLock (hMem);

		err = sPhotoFromDIB (lpbmi, MAX_PHOTO_WIDTH,
							 MAX_PHOTO_HEIGHT, TRUE, pbuf, pisize);

		GlobalUnlock (hMem);
		CloseClipboard(); 
	}
	return err;
}

//	___________________________________________________
//
//	read BMP file into PhotoID buffer

static PGPError
sReadPhotoFromBMPFile (
		FILE*		pfile, 
		LPBYTE*		pbuf, 
		INT*		pisize)
{
	PGPError			err		= kPGPError_Win32_InvalidImage;
	LPBYTE				pfilebuf;
	BITMAPFILEHEADER	bmfh;
	LPBITMAPINFO		lpbmi;
	ULONG				ulMaxFileSize;

	// read file header to get size of file
	rewind (pfile);
	if (fread (&bmfh, 1, sizeof(bmfh), pfile) != sizeof(bmfh))
		return err;

	// check for internal magic number and valid size
	if ((bmfh.bfType != 0x4d42) ||		// 'BM'
		(bmfh.bfSize < (sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFO)))) {
		return err;
	}

	// check for giant files
	ulMaxFileSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
	ulMaxFileSize += 256*sizeof(RGBQUAD);
	ulMaxFileSize += MAX_FILE_WIDTH*MAX_FILE_HEIGHT*4;
	if (bmfh.bfSize > ulMaxFileSize) {
		return kPGPError_Win32_ImageTooBig;
	}

	// allocate buffer to hold entire file
	pfilebuf = KMAlloc (bmfh.bfSize);
	if (!pfilebuf) {
		return kPGPError_OutOfMemory;
	}

	// read the entire file into the buffer
	rewind (pfile);
	if (fread (pfilebuf, 1, bmfh.bfSize, pfile) != bmfh.bfSize)
		return err;

	// set the pointer to byte following file header
	lpbmi = (LPBITMAPINFO)(pfilebuf + sizeof(bmfh));

	// check for compressed images (not supported)
	if (lpbmi->bmiHeader.biCompression != BI_RGB) {
		KMFree (pfilebuf);
		return err;
	}

	// convert
	err = sPhotoFromDIB (lpbmi, MAX_PHOTO_WIDTH,
						 MAX_PHOTO_HEIGHT, TRUE, pbuf, pisize);

	KMFree (pfilebuf);

	return err;
}

//	___________________________________________________
//
//	read JPEG file into PhotoID buffer

static PGPError
sReadPhotoFromJPEGFile (
		FILE*		pfile, 
		LPBYTE*		pbuf, 
		INT*		pisize)
{
	PGPError			err		= kPGPError_Win32_InvalidImage;
	LPBITMAPINFO		lpbmi;

	// convert the JPEG file to a BMP buffer
	rewind (pfile);
	lpbmi = sDIBfromJPEGFile (pfile, FALSE);

	// convert to photoid format
	if (lpbmi) {
		err = sPhotoFromDIB (lpbmi, MAX_PHOTO_WIDTH,
						 MAX_PHOTO_HEIGHT, TRUE, pbuf, pisize);
	}

	return err;
}

//	___________________________________________________
//
//	read image file into PhotoID buffer

PGPError
KMReadPhotoFromFile (
		LPSTR		pszFile, 
		LPBYTE*		pbuf, 
		INT*		pisize)
{
	PGPError			err		= kPGPError_Win32_InvalidImage;
	FILE*				pfile;

	pfile = fopen (pszFile, "rb");

	if (pfile) {
		// attempt to read file as a JPEG file
		err = sReadPhotoFromJPEGFile (pfile, pbuf, pisize);

		// if that didn't work, try it as a BMP file
		if (IsPGPError (err)) {
			err = sReadPhotoFromBMPFile (pfile, pbuf, pisize);
		}

		fclose (pfile); 
	}

	return err;
}

