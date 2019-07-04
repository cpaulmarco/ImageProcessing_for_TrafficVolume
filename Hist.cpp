#include "stdafx.h"
#include "Imagr.h"
#include "ImagrDoc.h"
#include "ImagrView.h"
#include "ConstArithDlg.h"

/*----------------------------------------------------------------------
  This function computes a 256 level (and out of bounds <0 and >255) 
  histogram of the active image. RGB bitmaps have ea. color component 
  counted as well.
  
  Showing the histogram dialog window is handled by HistDlg. If the user
  hasn't selected to display the hist. then the hist calc. can proceed
  quietly, so some functions (like Eqliz()) can call Hist() w/o 
  displaying it.

  Returns true if successful.
----------------------------------------------------------------------*/
bool CImagrDoc::Hist()
{
	if (!m_image.minmax.found) if (!ChkData(false)) return(false);	

	m_image.histg_ovr = m_image.histg_neg = 0;

	int *p = (int *) m_image.GetBits();	// Ptr to bitmap
	unsigned long n = GetImageSize();
	
	BeginWaitCursor();

	/* Hash *p to a h level index & accumulate count */
	switch (m_image.ptype) {
		case GREY:
			ZeroMemory(m_image.histg, 256*sizeof(unsigned long));
			//(Faster to cmp 0) for (i = 0; i < n; i++, p++) {
			for ( ; n > 0; n--, p++) {
				m_image.histg[0][RED(*p)]++;	
			}
			break;
		case cRGB:
			ZeroMemory(m_image.histg, 3*256*sizeof(unsigned long));

			for ( ; n > 0; n--, p++) {
				m_image.histg[0][RED(*p)]++;
				m_image.histg[1][GRN(*p)]++;
				m_image.histg[2][BLU(*p)]++;
			}
			break;
		default:	// INTG
			ZeroMemory(m_image.histg, 256*sizeof(int));
			for ( ; n > 0; n--, p++) {
				if (*p < 0)			
					m_image.histg_neg++;
				else if (*p > 255)	
					m_image.histg_ovr++;
				else                
					m_image.histg[0][*p]++;
			}
			break;
	}
	EndWaitCursor();

	return(true);
}

/*----------------------------------------------------------------------
  This function checks range of bitmap and updates min and max.
  Returns true if successful.
----------------------------------------------------------------------*/
bool CImagrDoc::ChkData(bool Quiet /*= true*/)
{
	if (!ChkDIB()) return(false);

	//ATLTRACE2("***"__FUNCTION__" %s\n", GetTitle());

	int /*(faster?) byte*/ r, g, b;
	int *p = (int *) m_image.GetBits();	// Ptr to bitmap
	unsigned long n = GetImageSize();
	int *min = &(m_image.minmax.min);
	int *max = &(m_image.minmax.max);

	//(Removed for interactive fctns) BeginWaitCursor();
	switch (m_image.ptype) {
		case GREY:
			*max = *min = RED(*p);	// Init. min, max
			for (n--, p++ /* Already did 1st */; n > 0; n--, p++) {
				r = RED(*p);
				if (r < *min) *min = r; else if (r > *max) *max = r;
			}
			break;
		case cRGB:
			*max = *min = RED(*p); 
			g = GRN(*p);
			b = BLU(*p);
			if (g < *min) *min = g; else if (g > *max) *max = g;
			if (b < *min) *min = b; else if (b > *max) *max = b;
			
			for (n--, p++ /* Already did 1st */; n > 0; n--, p++) {
				r = RED(*p);
				g = GRN(*p);
				b = BLU(*p);
				
				if (r < *min) *min = r; else if (r > *max) *max = r;
				if (g < *min) *min = g; else if (g > *max) *max = g;
				if (b < *min) *min = b; else if (b > *max) *max = b;
			}
			break;
		default:	// INTG
			for (*min = *max = *p, n--, p++; n > 0; n--, p++) {
				if (*p < *min) *min = *p; else if (*p > *max) *max = *p;
			}
			break;
	}
	//EndWaitCursor();

	if (!Quiet && (*min < 0 || *max > 255)) {
		fMessageBox("Warning - " __FUNCTION__, MB_ICONWARNING,
					"Image pixels %d..%d out of display range [0..255] - \n"
					"pixels will be handled as type integer, but colors "
					"will not be RGB until pixels are in range. (See "
					"Histogram > Normalize to remedy this)",
					m_image.minmax.min, m_image.minmax.max);
	}
	m_image.minmax.found = true;
	UpdateAllViews(NULL, ID_SBR_IMAGEMINMAX);	// Pass hint for statusbar via OnUpdate()

	return(true);
}

/*----------------------------------------------------------------------
  This function normalizes the bitmap to the passed range. 
  (For calling interactively from the slider dialog.)
----------------------------------------------------------------------*/
void CImagrDoc::Nrmlz(int nmin, int nmax)
{
	int d;
	float factor;
	byte r = 0, g = 0, b = 0;

	OnDo();		// Save prev. image for Undo

	int *min = &(m_image.minmax.min);
	int *max = &(m_image.minmax.max);

	if (*max - *min == 0)
		factor = 32767.;	// Avoid div. by 0
	else
		factor = (float)((float)(nmax - nmin) / (*max - *min));
	
	int *p = (int *) m_image.GetBits();	// Ptr to bitmap
	unsigned long n = GetImageSize();

	switch (m_image.ptype) {
		case GREY:	// Grey scale pixels
			for ( ; n > 0; n--, p++) {
				r = RED(*p);
				d = (int)((float)(r - *min) * factor + nmin + 0.5);
				r = (byte)THRESH(d);
				*p = BGR(r, r, r);
			}
			break;
		case cRGB:	// Full color RGB pixels
			for ( ; n > 0; n--, p++) {
				r = RED(*p);
				d = (int)((float)(r - *min) * factor + nmin + 0.5);
				r = (byte)THRESH(d);

				g = GRN(*p);
				d = (int)((float)(g - *min) * factor + nmin + 0.5);
				g = (byte)THRESH(d);

				b = BLU(*p);
				d = (int)((float)(b - *min) * factor + nmin + 0.5);
				b = (byte)THRESH(d);
				
				*p = BGR(b, g, r);
			}
			break;
		default:	// INTG pixels
			for ( ; n > 0; n--, p++) {
				r = (int)((float)(*p - *min) * factor + nmin + 0.5);
				*p = BGR(r, r, r);
			}
			m_image.ptype = GREY;	// Changed type
			break;
	}

	//ChkData();					// Re-check range
	*min = nmin;
	*max = nmax;
	UpdateAllViews(NULL, ID_SBR_IMAGEMINMAX);	// Pass hint for statusbar via OnUpdate()
}

/*----------------------------------------------------------------------
  This function normalizes the histogram from the passed range to 0..255. 
  (For calling interactively from the slider dialog.)
----------------------------------------------------------------------*/
void CImagrDoc::NrmlzRange(int nmin, int nmax)
{
	int d;
	float factor;
	byte r = 0, g = 0, b = 0;

	OnDo();		// Save prev. image for Undo

	if (nmax - nmin == 0)
		factor = 32767.;	// Avoid div. by 0
	else
		factor = (float)((float)255 /(float)(nmax - nmin));
	
	int *p = (int *) m_image.GetBits();	// Ptr to bitmap
	unsigned long n = GetImageSize();

	switch (m_image.ptype) {
		case GREY:
			for ( ; n > 0; n--, p++) {
				r = RED(*p);
				d = (int)((float)(r - nmin) * factor + 0.5);
				r = (byte)THRESH(d);
				*p = BGR(r, r, r);
			}
			break;
		case cRGB:
			for ( ; n > 0; n--, p++) {
				r = RED(*p);
				d = (int)((float)(r - nmin) * factor + 0.5);
				r = (byte)THRESH(d);

				g = GRN(*p);
				d = (int)((float)(g - nmin) * factor + 0.5);
				g = (byte)THRESH(d);

				b = BLU(*p);
				d = (int)((float)(b - nmin) * factor + 0.5);
				b = (byte)THRESH(d);
				*p = BGR(b, g, r);
			}
			break;
		default:	// INTG
			for ( ; n > 0; n--, p++) {
				r = (int)((float)(*p - nmin) * factor + 0.5);
				*p = BGR(r, r, r);
			}
			m_image.ptype = GREY;	// Changed type
			break;
	}

	ChkData();					// Re-check range
}

/*----------------------------------------------------------------------
  This function thresholds (i.e. truncates) pixel values to the passed 
  range. (For calling interactively from the slider dialog.)
----------------------------------------------------------------------*/
void CImagrDoc::Thresh(int tmin, int tmax)
{
	OnDo();		// Save image state

	int r, g, b;
	int *p = (int *) m_image.GetBits();	// Ptr to bitmap
	unsigned long n = GetImageSize();

	switch (m_image.ptype) {
		case GREY:
			for ( ; n > 0; n--, p++) {
				r = RED(*p);
				if (r > tmax) r = tmax; else if (r < tmin) r = tmin;
				*p = BGR((byte)r, (byte)r, (byte)r);
			}
			break;
		case cRGB:
			for ( ; n > 0; n--, p++) {
				r = RED(*p);
				g = GRN(*p);
				b = BLU(*p);
				
				if (r > tmax) r = tmax; else if (r < tmin) r = tmin;
				if (g > tmax) g = tmax; else if (g < tmin) g = tmin;
				if (b > tmax) b = tmax; else if (b < tmin) b = tmin;
				
				*p = BGR((byte)b, (byte)g, (byte)r);
			}
			break;
		default:	// INTG
			for ( ; n > 0; n--, p++) {
				if (*p > tmax) *p = tmax; else if (*p < tmin) *p = tmin;
			}
			break;
	}

	ChkData();	// Re-check range for Hist() update
}

/*----------------------------------------------------------------------
  This function redistributes the image data so an equalized histogram 
  is obtained. This results in an image with similar contrast over
  the full range of intensities. Note that the hist must be computed 1st.
  (Thanks to Frank Hoogterp and Steve Caito for the original FORTRAN code).
----------------------------------------------------------------------*/
void CImagrDoc::Eqliz()
{
	if (!Hist()) return;	// Need histogram

	// Check min..max within range
	if (!INRANGE()) {	
		fMessageBox("Error - " __FUNCTION__, MB_ICONERROR, 
					"Image pixels are not within range 0..255\n"
					"(try Normalize or Threshold first)");
		return;	
	}

	OnDo();		// Save image for Undo

	BeginWaitCursor();
	int *p = (int *) m_image.GetBits();	// Ptr to bitmap

	unsigned long n = GetImageSize();
	unsigned long i;
	unsigned ncol = 0;
	unsigned long sum = 0;
	double xnext, step;
	const unsigned Histg_steps = 256; //_countof(m_image.histg);

	step = (float) n / Histg_steps; 
	xnext = step;

	unsigned long d;
	byte r, g, b;

	switch (m_image.ptype) {
		case GREY:
			/* Remap histg[] */
			for (i = 0; i < Histg_steps; i++) { 
				sum += m_image.histg[0][i];                 
				while (sum >= xnext) {
					ncol++;
					xnext += step;
				}
				m_image.histg[0][i] = ncol;
			}
	
			for ( ; n > 0; n--, p++) {
				d = m_image.histg[0][RED(*p)];
				if (d > 255) 
					r = 255;
				else 
					r = (byte) d;
				*p = BGR(r, r, r);
			}
			break;
		case cRGB:
			/* Remap histg[] */
			for (i = 0; i < Histg_steps; i++) {
				// Avg. of RGB's
				sum += (m_image.histg[0][i] + m_image.histg[1][i] +
						m_image.histg[2][i]) / 3;                 
				while (sum >= xnext) {
					ncol++;
					xnext += step;
				}
				m_image.histg[0][i] = ncol;
				m_image.histg[1][i] = ncol;
				m_image.histg[2][i] = ncol;
			}
	
			for ( ; n > 0; n--, p++) {
				d = m_image.histg[0][RED(*p)];
				if (d > 255) 
					r = 255;
				else 
					r = (byte) d;

				d = m_image.histg[1][GRN(*p)];
				if (d > 255) 
					g = 255;
				else 
					g = (byte) d;

				d = m_image.histg[2][BLU(*p)];
				if (d > 255) 
					b = 255;
				else 
					b = (byte) d;
				
				*p = BGR(b, g, r);
			}
			break;
		default:	// INTG
			for ( ; n > 0; n--, p++) {
				d = m_image.histg[0][*p];
				if (d > 255) d = 255;
				*p = d;
			}
			break;
	}
	EndWaitCursor();

	ChkData();		// Re-check range
	SetModifiedFlag(true);			
	UpdateAllViews(NULL);	// Still needed even though called by ChkData()
}

/*----------------------------------------------------------------------
  This function inverts the colors of pixels.
----------------------------------------------------------------------*/
void CImagrDoc::ColorInvert()
{
	if (!m_image.minmax.found) if (!ChkData()) return;	

	OnDo("Invert");		// Save image for Undo

	byte r, g, b;
	int *p = (int *) m_image.GetBits();	// Ptr to bitmap
	unsigned long n = GetImageSize();
	int *min = &(m_image.minmax.min);	// Ptr to member min
	int *max = &(m_image.minmax.max);

	BeginWaitCursor();
	switch (m_image.ptype) {
		case GREY:
			for ( ; n > 0; n--, p++) {
				r = *max - RED(*p) + *min;
				*p = BGR(r, r, r);
			}
			break;
		case cRGB:
			for ( ; n > 0; n--, p++) {
				r = *max - RED(*p) + *min;
				g = *max - GRN(*p) + *min;
				b = *max - BLU(*p) + *min;
				*p = BGR(b, g, r);
			}
			break;
		default:	// INTG
			for ( ; n > 0; n--, p++) {
				*p = *max - *p + *min;
			}
			break;
	}
	EndWaitCursor();

	SetModifiedFlag(true);			
	UpdateAllViews(NULL);
}

/*----------------------------------------------------------------------
  This function converts the pixels to grey.
----------------------------------------------------------------------*/
void CImagrDoc::ConvertGrey()
{
	int *p = (int *) m_image.GetBits();	// Ptr to bitmap
	unsigned long n = GetImageSize();

	switch (m_image.ptype) {
		case GREY:
			((CMainFrame*)AfxGetMainWnd())->SetSBRpane0("Image pixels are already grey scale");
			return;
		case cRGB:
			unsigned t;
			
			OnDo();		// Save image for Undo

			BeginWaitCursor();
			for ( ; n > 0; n--, p++) {
				//t = NINT((GetRValue(*p) + GetGValue(*p) + GetBValue(*p)) / 3.0);
				t = NINT(YRGB(*p));
				*p = BGR((byte)t, (byte)t, (byte)t);
			}
			EndWaitCursor();
			break;
		default:	// INTG
			if (!m_image.minmax.found) if (!ChkData(false)) return;

			OnDo();		// Save image for Undo

			BeginWaitCursor();
			for ( ; n > 0; n--, p++) {
				*p = BGR((byte)*p, (byte)*p, (byte)*p);
			}
			EndWaitCursor();
			break;
	}

	m_image.ptype = GREY;
	ChkData();
	SetModifiedFlag(true);			
	UpdateAllViews(NULL);	// Still needed even though called by ChkData()
}

/*----------------------------------------------------------------------
  This function adds, subtracts, multiples, or divides the image by a
  constant (called by dialog). Returns false on nop.
----------------------------------------------------------------------*/
bool CImagrDoc::ConstArith(char op, float d)
{
	//ATLTRACE2("***"__FUNCTION__" %c %f\n", op, d);	
	long t;
	byte r, g, b;

	int *p = (int *) m_image.GetBits();	// Ptr to bitmap
	unsigned long n = GetImageSize();

	switch (op) {
		case '-':
			d = -d;
		case '+':
			if (d == 0) return(false);	// No need to do this)

			BeginWaitCursor();
			switch (m_image.ptype) {
				case GREY:
					for ( ; n > 0; n--, p++) {
						t = NINT((float)RED(*p) + d);
						r = (byte)THRESH(t);
						*p = BGR(r, r, r);
					}
					break;
				case cRGB:
					for ( ; n > 0; n--, p++) {
						t = NINT((float)RED(*p) + d);
						r = (byte)THRESH(t);

						t = NINT((float)GRN(*p) + d);
						g = (byte)THRESH(t);

						t = NINT((float)BLU(*p) + d);
						b = (byte)THRESH(t);

						*p = BGR(b, g, r);
					}
					break;
				default:	// INTG
					for ( ; n > 0; n--, p++) {
						*p = NINT((float)*p + d);
					}
					break;
			}
			EndWaitCursor();
			break;
		case '/':
			if (d == 0) {
				fMessageBox("Error - " __FUNCTION__, MB_ICONERROR, 
							"Division by zero is undefined\n");
				return(false);
			}
			d = (float)1.0 / d;		// Invert to implement as mult.
		case '*':
			if (d == 1.0) return(false);	// No need to do this)

			BeginWaitCursor();
			switch (m_image.ptype) {
				case GREY:
					for ( ; n > 0; n--, p++) {
						t = NINT((float)RED(*p) * d);
						r = (byte)THRESH(t);
						*p = BGR(r, r, r);
					}
					break;
				case cRGB:
					for ( ; n > 0; n--, p++) {
						t = NINT((float)RED(*p) * d);
						r = (byte)THRESH(t);

						t = NINT((float)GRN(*p) * d);
						g = (byte)THRESH(t);

						t = NINT((float)BLU(*p) * d);
						b = (byte)THRESH(t);

						*p = BGR(b, g, r);
					}
					break;
				default:	// INTG
					for ( ; n > 0; n--, p++) {
						*p = NINT((float)*p * d);
					}
					break;
			}
			EndWaitCursor();
			break;
	}
	
	ChkData();		// Re-check range
	return(true);
}