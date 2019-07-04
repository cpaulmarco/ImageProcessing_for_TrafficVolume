#include "stdafx.h"
#include "Imagr.h"
#include "ImagrDoc.h"
#include "ImagrView.h"

/*----------------------------------------------------------------------
  This function performs a 3 x 3 convolution on the active image. The 
  kernel array is passed externally. Edges are added (doubly weighted)
  for the computation. (Thanks to Frank Hoogterp and Steve Caito for the 
  original FORTRAN code).
----------------------------------------------------------------------*/
void CImagrDoc::Convl(float k1, float k2, float k3,
					  float k4, float k5, float k6,
					  float k7, float k8, float k9)
{
	int *p;						/* Image ptr */
	unsigned long i, j, nx, ny;
	int *m1, *m2, *m3;			// Pointers to buffers to free()
	int *old_r1, *r1, *r2, *r3; /* Cycling pointers to rows */
	float s, fsum;
	int t;
	byte r, g, b;

	//if (!ChkDIB()) return;

	nx = m_image.GetWidth();
	ny = m_image.GetHeight();

	p = (int *) m_image.GetBits();	// Ptr to bitmap

	/* Allocate row buffers */
	if (!(m1 = (int *) malloc((nx+2) * sizeof(*m1)))) {
		fMessageBox("Error - " __FUNCTION__, MB_ICONERROR, "malloc() error m1");
		return;
	}
	if (!(m2 = (int *) malloc((nx+2) * sizeof(*m2)))) {
		fMessageBox("Error - " __FUNCTION__, MB_ICONERROR, "malloc() error m2");
		free(m1);
		return;
	}
	if (!(m3 = (int *) malloc((nx+2) * sizeof(*m3)))) {
		fMessageBox("Error - " __FUNCTION__, MB_ICONERROR, "malloc() error m3");
		free(m1);
		free(m2);
		return;
	}
	r1 = m1;
	r2 = m2;
	r3 = m3;

	// Initialize rows
	memcpy_s(&r1[1], nx * sizeof(int), p, nx * sizeof(int));
	r1[0] = r1[1];                      /* Doubly weight edges */
	r1[nx+1] = r1[nx];

	/* Start r2 = r1 (doubly weight 1st row) */
	memcpy_s(r2, (nx+2) * sizeof(int), r1, (nx+2) * sizeof(int));

	// Calc. sum of kernel
	fsum = k1 + k2 + k3 + k4 + k5 + k6 + k7 + k8 + k9;
	if (fsum == 0) 
		fsum = 1;			// Avoid div. by 0
	else
		fsum = 1/fsum;		// Invert so can mult. 

	OnDo();		// Save image for Undo

	BeginWaitCursor(); 
	switch (m_image.ptype) {
		case GREY:
			for (j = 1; j <= ny; j++, p += nx) {
				if (j == ny) {				/* Last row */
					r3 = r2;				/* Last row doubly weighted */
				}
				else { 	/* Read next row (into the 3rd row) */
					memcpy_s(&r3[1], nx * sizeof(int), p + nx, nx * sizeof(int));
					r3[0] = r3[1];			/* Doubly weight edges */
					r3[nx+1] = r3[nx];
				}

				for (i = 0; i < nx; i++) {
					s = k1 * (float)RED(r1[i]) 
					  + k2 * (float)RED(r1[i+1])
					  + k3 * (float)RED(r1[i+2]) 
					  + k4 * (float)RED(r2[i])
					  + k5 * (float)RED(r2[i+1])
					  + k6 * (float)RED(r2[i+2])
					  + k7 * (float)RED(r3[i])
					  + k8 * (float)RED(r3[i+1])
					  + k9 * (float)RED(r3[i+2]);

					t = NINT(s * fsum);
					r = (byte)THRESH(t);

					p[i] = RGB(r, r, r);      
				}

				/* Cycle row pointers */
				old_r1 = r1;	// To save addr. for r3
				r1 = r2;
				r2 = r3;
				r3 = old_r1;
			}
			break;
		case cRGB:
			for (j = 1; j <= ny; j++, p += nx) {
				if (j == ny) {				/* Last row */
					r3 = r2;				/* Last row doubly weighted */
				}
				else { /* Read next row (into the 3rd row) */
					memcpy_s(&r3[1], nx * sizeof(int), p + nx, nx * sizeof(int));
					r3[0] = r3[1];			/* Doubly weight edges */
					r3[nx+1] = r3[nx];
				}

				for (i = 0; i < nx; i++) {
					// Red component
					s = k1 * (float)RED(r1[i]) 
					  + k2 * (float)RED(r1[i+1])
					  + k3 * (float)RED(r1[i+2]) 
					  + k4 * (float)RED(r2[i])
					  + k5 * (float)RED(r2[i+1])
					  + k6 * (float)RED(r2[i+2])
					  + k7 * (float)RED(r3[i])
					  + k8 * (float)RED(r3[i+1])
					  + k9 * (float)RED(r3[i+2]);

					t = NINT(s * fsum);
					r = (byte)THRESH(t);

					// Green component
					s = k1 * (float)GRN(r1[i]) 
					  + k2 * (float)GRN(r1[i+1])
					  + k3 * (float)GRN(r1[i+2]) 
					  + k4 * (float)GRN(r2[i])
					  + k5 * (float)GRN(r2[i+1])
					  + k6 * (float)GRN(r2[i+2])
					  + k7 * (float)GRN(r3[i])
					  + k8 * (float)GRN(r3[i+1])
					  + k9 * (float)GRN(r3[i+2]);

					t = NINT(s * fsum);
					g = (byte)THRESH(t);
					
					// Blue component
					s = k1 * (float)BLU(r1[i]) 
					  + k2 * (float)BLU(r1[i+1])
					  + k3 * (float)BLU(r1[i+2]) 
					  + k4 * (float)BLU(r2[i])
					  + k5 * (float)BLU(r2[i+1])
					  + k6 * (float)BLU(r2[i+2])
					  + k7 * (float)BLU(r3[i])
					  + k8 * (float)BLU(r3[i+1])
					  + k9 * (float)BLU(r3[i+2]);

					t = NINT(s * fsum);
					b = (byte)THRESH(t);

					p[i] = BGR(b, g, r);
				}

				/* Cycle row pointers */
				old_r1 = r1;
				r1 = r2;
				r2 = r3;
				r3 = old_r1;
			}
			break;
		default:	// INTG
			for (j = 1; j <= ny; j++, p += nx) {
				if (j == ny) {				/* Last row */
					r3 = r2;				/* Last row doubly weighted */
				}
				else { /* Read next row (into the 3rd row) */
					memcpy_s(&r3[1], nx * sizeof(int), p + nx, nx * sizeof(int));
					r3[0] = r3[1];			/* Doubly weight edges */
					r3[nx+1] = r3[nx];
				}

				for (i = 0; i < nx; i++) {
					s = k1 * (float)r1[i] + k2 * (float)r1[i+1]
					  + k3 * (float)r1[i+2] + k4 * (float)r2[i]
					  + k5 * (float)r2[i+1] + k6 * (float)r2[i+2]
					  + k7 * (float)r3[i] + k8 * (float)r3[i+1]
					  + k9 * (float)r3[i+2];
					p[i] = NINT(s * fsum);
				}

				/* Cycle row pointers */
				old_r1 = r1;
				r1 = r2;
				r2 = r3;
				r3 = old_r1;
			}
			break;
	}
	EndWaitCursor();

	free(m1);                   
	free(m2);
	free(m3);				

	ChkData();				// Re-check range
	SetModifiedFlag(true);	// Set flag
	UpdateAllViews(NULL);	// Still needed even though called by ChkData()
}

/*----------------------------------------------------------------------
  This function performs a 3 x 3 median filter on the active image. 
  Edges are added (doubly weighted) for the computation. (Thanks to 
  Frank Hoogterp and Steve Caito for the original FORTRAN code).
----------------------------------------------------------------------*/
void CImagrDoc::MedFilter()
{
	int *p;						/* Image ptr */
	unsigned long i, j, nx, ny;
	int bubble[9], m, l;		/* For med. filter */
	int *m1, *m2, *m3;			// Pointers to buffers to free()
	int *old_r1, *r1, *r2, *r3; /* Cycling pointers to rows */
	int t;
	byte r, g, b;

	nx = m_image.GetWidth();
	ny = m_image.GetHeight();

	p = (int *) m_image.GetBits();	// Ptr to bitmap

	/* Allocate row buffers */
	if (!(m1 = (int *) malloc((nx+2) * sizeof(*m1)))) {
		fMessageBox("Error - " __FUNCTION__, MB_ICONERROR, "malloc() error m1");
		return;
	}
	if (!(m2 = (int *) malloc((nx+2) * sizeof(*m2)))) {
		fMessageBox("Error - " __FUNCTION__, MB_ICONERROR, "malloc() error m2");
		free(m1);
		return;
	}
	if (!(m3 = (int *) malloc((nx+2) * sizeof(*m3)))) {
		fMessageBox("Error - " __FUNCTION__, MB_ICONERROR, "malloc() error m3");
		free(m1);
		free(m2);
		return;
	}
	r1 = m1;
	r2 = m2;
	r3 = m3;

	// Initialize rows
	memcpy_s(&r1[1], nx * sizeof(int), p, nx * sizeof(int));
	r1[0] = r1[1];                      /* Doubly wait edges */
	r1[nx+1] = r1[nx];

	// Start r2 same as r1 (doubly weight 1st row) 
	memcpy_s(r2, (nx+2) * sizeof(int), r1, (nx+2) * sizeof(int));

	OnDo();		// Save image for Undo

	BeginWaitCursor();
	switch (m_image.ptype) {
		case GREY:
			for (j = 1; j <= ny; j++, p += nx) {
				if (j == ny) {				/* Last row */
					r3 = r2;				/* Last row doubly weighted */
				}
				else { /* Read next row (into the 3rd row) */
					memcpy_s(&r3[1], nx * sizeof(int), p + nx, nx * sizeof(int));
					r3[0] = r3[1];			/* Doubly wait edges */
					r3[nx+1] = r3[nx];
				}

				for (i = 0; i < nx; i++) {	// Row loop
					bubble[0] = RED(r1[i]);
					bubble[1] = RED(r1[i+1]);
					bubble[2] = RED(r1[i+2]);
					bubble[3] = RED(r2[i]);
					bubble[4] = RED(r2[i+1]);
					bubble[5] = RED(r2[i+2]);
					bubble[6] = RED(r3[i]);
					bubble[7] = RED(r3[i+1]);
					bubble[8] = RED(r3[i+2]);

					// Bubble sort just to 5 since only median is needed
					for (m = 0; m < 5; m++) {
						t = bubble[m];
						for (l = m+1; l < 9; l++) {
							if (t > bubble[l]) {
								t = bubble[l];
								bubble[l] = bubble[m];
								bubble[m] = t;
							}
						}
					}

					r = (byte)bubble[4];	/* The median value */
					p[i] = RGB(r, r, r);      
				}

				/* Cycle row pointers */
				old_r1 = r1;	// To save addr. for r3
				r1 = r2;
				r2 = r3;
				r3 = old_r1;
			}
			break;
		case cRGB:
			for (j = 1; j <= ny; j++, p += nx) {
				if (j == ny) {				/* Last row */
					r3 = r2;				/* Last row doubly weighted */
				}
				else { /* Read next row (into the 3rd row) */
					memcpy_s(&r3[1], nx * sizeof(int), p + nx, nx * sizeof(int));
					r3[0] = r3[1];			/* Doubly wait edges */
					r3[nx+1] = r3[nx];
				}

				for (i = 0; i < nx; i++) {	// Row loop
					// Red component
					bubble[0] = RED(r1[i]);
					bubble[1] = RED(r1[i+1]);
					bubble[2] = RED(r1[i+2]);
					bubble[3] = RED(r2[i]);
					bubble[4] = RED(r2[i+1]);
					bubble[5] = RED(r2[i+2]);
					bubble[6] = RED(r3[i]);
					bubble[7] = RED(r3[i+1]);
					bubble[8] = RED(r3[i+2]);

					// Bubble sort just to 5 since only median is needed
					for (m = 0; m < 5; m++) {
						t = bubble[m];
						for (l = m+1; l < 9; l++) {
							if (t > bubble[l]) {
								t = bubble[l];
								bubble[l] = bubble[m];
								bubble[m] = t;
							}
						}
					}

					r = (byte)bubble[4];	/* The median value */

					// Green component
					bubble[0] = GRN(r1[i]);
					bubble[1] = GRN(r1[i+1]);
					bubble[2] = GRN(r1[i+2]);
					bubble[3] = GRN(r2[i]);
					bubble[4] = GRN(r2[i+1]);
					bubble[5] = GRN(r2[i+2]);
					bubble[6] = GRN(r3[i]);
					bubble[7] = GRN(r3[i+1]);
					bubble[8] = GRN(r3[i+2]);

					// Bubble sort just to 5 since only median is needed
					for (m = 0; m < 5; m++) {
						t = bubble[m];
						for (l = m+1; l < 9; l++) {
							if (t > bubble[l]) {
								t = bubble[l];
								bubble[l] = bubble[m];
								bubble[m] = t;
							}
						}
					}

					g = (byte)bubble[4];	/* The median value */

					// Blue component
					bubble[0] = BLU(r1[i]);
					bubble[1] = BLU(r1[i+1]);
					bubble[2] = BLU(r1[i+2]);
					bubble[3] = BLU(r2[i]);
					bubble[4] = BLU(r2[i+1]);
					bubble[5] = BLU(r2[i+2]);
					bubble[6] = BLU(r3[i]);
					bubble[7] = BLU(r3[i+1]);
					bubble[8] = BLU(r3[i+2]);

					// Bubble sort just to 5 since only median is needed
					for (m = 0; m < 5; m++) {
						t = bubble[m];
						for (l = m+1; l < 9; l++) {
							if (t > bubble[l]) {
								t = bubble[l];
								bubble[l] = bubble[m];
								bubble[m] = t;
							}
						}
					}

					b = (byte)bubble[4];	/* The median value */

					p[i] = BGR(b, g, r);
				}

				/* Cycle row pointers */
				old_r1 = r1;	// To save addr. for r3
				r1 = r2;
				r2 = r3;
				r3 = old_r1;
			}
			break;
		default:	// INTG
			for (j = 1; j <= ny; j++, p += nx) {
				if (j == ny) {				/* Last row */
					r3 = r2;				/* Last row doubly weighted */
				}
				else { /* Read next row (into the 3rd row) */
					memcpy_s(&r3[1], nx * sizeof(int), p + nx, nx * sizeof(int));
					r3[0] = r3[1];			/* Doubly wait edges */
					r3[nx+1] = r3[nx];
				}

				for (i = 0; i < nx; i++) {	// Row loop
					bubble[0] = r1[i];
					bubble[1] = r1[i+1];
					bubble[2] = r1[i+2];
					bubble[3] = r2[i];
					bubble[4] = r2[i+1];
					bubble[5] = r2[i+2];
					bubble[6] = r3[i];
					bubble[7] = r3[i+1];
					bubble[8] = r3[i+2];

					// Bubble sort just to 5 since only median is needed
					for (m = 0; m < 5; m++) {
						t = bubble[m];
						for (l = m+1; l < 9; l++) {
							if (t > bubble[l]) {
								t = bubble[l];
								bubble[l] = bubble[m];
								bubble[m] = t;
							}
						}
					}

					p[i] = bubble[4];       /* The median value */
				}

				/* Cycle row pointers */
				old_r1 = r1;	// To save addr. for r3
				r1 = r2;
				r2 = r3;
				r3 = old_r1;
			}
			break;
	}
	EndWaitCursor();

	free(m1);                   
	free(m2);
	free(m3);				

	ChkData();				// Re-check range
	SetModifiedFlag(true);	// Set flag
	UpdateAllViews(NULL);	// Still needed even though called by ChkData()
}

/*----------------------------------------------------------------------
  This function performs a 3 x 3 avg. filter on the active image. 
  Edges are added (doubly weighted) for the computation.
----------------------------------------------------------------------*/
void CImagrDoc::AvgFilter()
{
	int *p;						/* Image ptr */
	unsigned long i, j, nx, ny;
	int *m1, *m2, *m3;			// Pointers to buffers to free()
	int *old_r1, *r1, *r2, *r3; /* Cycling pointers to rows */
	byte r, g, b;

	nx = m_image.GetWidth();
	ny = m_image.GetHeight();

	p = (int *) m_image.GetBits();	// Ptr to bitmap

	/* Allocate row buffers */
	if (!(m1 = (int *) malloc((nx+2) * sizeof(*m1)))) {
		fMessageBox("Error - " __FUNCTION__, MB_ICONERROR, "malloc() error m1");
		return;
	}
	if (!(m2 = (int *) malloc((nx+2) * sizeof(*m2)))) {
		fMessageBox("Error - " __FUNCTION__, MB_ICONERROR, "malloc() error m2");
		free(m1);
		return;
	}
	if (!(m3 = (int *) malloc((nx+2) * sizeof(*m3)))) {
		fMessageBox("Error - " __FUNCTION__, MB_ICONERROR, "malloc() error m3");
		free(m1);
		free(m2);
		return;
	}
	r1 = m1;
	r2 = m2;
	r3 = m3;

	// Initialize rows
	memcpy_s(&r1[1], nx * sizeof(int), p, nx * sizeof(int));
	r1[0] = r1[1];                      /* Doubly wait edges */
	r1[nx+1] = r1[nx];

	// Start r2 same as r1 (doubly weight 1st row) 
	memcpy_s(r2, (nx+2) * sizeof(int), r1, (nx+2) * sizeof(int));

	OnDo();		// Save image for Undo

	BeginWaitCursor();
	switch (m_image.ptype) {
		case GREY:
			for (j = 1; j <= ny; j++, p += nx) {
				if (j == ny) {				/* Last row */
					r3 = r2;				/* Last row doubly weighted */
				}
				else { /* Read next row (into the 3rd row) */
					memcpy_s(&r3[1], nx * sizeof(int), p + nx, nx * sizeof(int));
					r3[0] = r3[1];			/* Doubly wait edges */
					r3[nx+1] = r3[nx];
				}

				for (i = 0; i < nx; i++) {	// Row loop
					r = (byte)(((RED(r1[i]) + RED(r1[i+1]) + RED(r1[i+2]) +
								 RED(r2[i]) + RED(r2[i+1]) + RED(r2[i+2]) +
								 RED(r3[i]) + RED(r3[i+1]) + RED(r3[i+2])) / 9.) + 0.5);
					p[i] = RGB(r, r, r);      
				}

				/* Cycle row pointers */
				old_r1 = r1;	// To save addr. for r3
				r1 = r2;
				r2 = r3;
				r3 = old_r1;
			}
			break;
		case cRGB:
			for (j = 1; j <= ny; j++, p += nx) {
				if (j == ny) {				/* Last row */
					r3 = r2;				/* Last row doubly weighted */
				}
				else { /* Read next row (into the 3rd row) */
					memcpy_s(&r3[1], nx * sizeof(int), p + nx, nx * sizeof(int));
					r3[0] = r3[1];			/* Doubly wait edges */
					r3[nx+1] = r3[nx];
				}

				for (i = 0; i < nx; i++) {	// Row loop
					// Red component
					r = (byte)(((RED(r1[i]) + RED(r1[i+1]) + RED(r1[i+2]) +
								 RED(r2[i]) + RED(r2[i+1]) + RED(r2[i+2]) +
								 RED(r3[i]) + RED(r3[i+1]) + RED(r3[i+2])) / 9.) + 0.5);

					// Green component
					g = (byte)(((GRN(r1[i]) + GRN(r1[i+1]) + GRN(r1[i+2]) +
								 GRN(r2[i]) + GRN(r2[i+1]) + GRN(r2[i+2]) +
								 GRN(r3[i]) + GRN(r3[i+1]) + GRN(r3[i+2])) / 9.) + 0.5);

					// Blue component
					b = (byte)(((BLU(r1[i]) + BLU(r1[i+1]) + BLU(r1[i+2]) +
								 BLU(r2[i]) + BLU(r2[i+1]) + BLU(r2[i+2]) +
								 BLU(r3[i]) + BLU(r3[i+1]) + BLU(r3[i+2])) / 9.) + 0.5);

					p[i] = BGR(b, g, r);
				}

				/* Cycle row pointers */
				old_r1 = r1;	// To save addr. for r3
				r1 = r2;
				r2 = r3;
				r3 = old_r1;
			}
			break;
		default:	// INTG
			for (j = 1; j <= ny; j++, p += nx) {
				if (j == ny) {				/* Last row */
					r3 = r2;				/* Last row doubly weighted */
				}
				else { /* Read next row (into the 3rd row) */
					memcpy_s(&r3[1], nx * sizeof(int), p + nx, nx * sizeof(int));
					r3[0] = r3[1];			/* Doubly wait edges */
					r3[nx+1] = r3[nx];
				}

				for (i = 0; i < nx; i++) {	// Row loop
					p[i] = NINT((r1[i] + r1[i+1] + r1[i+2] + 
							r2[i] + r2[i+1] + r2[i+2] +
							r3[i] + r3[i+1] + r3[i+2]) / 9.);

				}

				/* Cycle row pointers */
				old_r1 = r1;	// To save addr. for r3
				r1 = r2;
				r2 = r3;
				r3 = old_r1;
			}
			break;
	}
	EndWaitCursor();

	free(m1);                   
	free(m2);
	free(m3);				

	ChkData();				// Re-check range
	SetModifiedFlag(true);	// Set flag
	UpdateAllViews(NULL);	// Still needed even though called by ChkData()
}
