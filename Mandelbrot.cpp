#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
//#include <winbase.h>
#include <ddraw.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

LPDIRECTDRAW        lpDirectDrawObject; //DD object
LPDIRECTDRAWSURFACE lpPrimary;          //DD primary surface
BOOL                ActiveApp;          //Is this program active?

int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
						LPSTR lpCmdLine, int nCmdShow);
long FAR PASCAL WindowProc ( HWND hwnd, UINT message,
							WPARAM wParam, LPARAM lParam );

enum
{
	SCREEN_MODE_W = 1024,
	SCREEN_MODE_H = 768,
	SCREEN_MODE_D = 8
};

const double aspect_ratio = (double)SCREEN_MODE_H / (double)SCREEN_MODE_W;
const double width = 4.;
const double height = width * aspect_ratio;
const double center_x = 0.;
const double center_y = 0.;
const double dx = width/SCREEN_MODE_W;
const double dy = height / SCREEN_MODE_H;



//*********************************************************
//WinMain - mandatory windows init function
//
//*********************************************************
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
						LPSTR lpCmdLine, int nCmdShow)
{
	MSG         msg;
	HWND        hwnd;
	WNDCLASS    wc;
	static char ClassName[] = "Mandelbrot";
	DDSURFACEDESC   ddsd;
	HRESULT         ddreturn;

	lpCmdLine = lpCmdLine;
	hPrevInstance = hPrevInstance;

	//register and realize our display window
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon( hInstance, IDI_APPLICATION );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = NULL;
	wc.lpszMenuName = ClassName;
	wc.lpszClassName = ClassName;
	RegisterClass( &wc );

	//create a full screen window so that GDI won't ever be
	//called
	hwnd = CreateWindowEx(
		0,//WS_EX_TOPMOST,
		ClassName,
		ClassName,
		WS_OVERLAPPEDWINDOW,//WS_POPUP,
		0,
		0,
		700,//GetSystemMetrics(SM_CXSCREEN),
		500,//GetSystemMetrics(SM_CYSCREEN),
		NULL,
		NULL,
		hInstance,
		NULL );

	if( !hwnd )
		return FALSE;

	ShowWindow( hwnd, nCmdShow );
	UpdateWindow( hwnd );
	SetFocus( hwnd );
	//ShowCursor( FALSE );

	//Instanciate our DirectDraw object
	ddreturn = DirectDrawCreate( NULL,
							&lpDirectDrawObject, NULL );
	if ( ddreturn != DD_OK )
	{
		DestroyWindow( hwnd );
		return FALSE;
	}

	ddreturn = lpDirectDrawObject->SetCooperativeLevel( hwnd,
										DDSCL_NORMAL);//DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
	if ( ddreturn != DD_OK )
	{
		DestroyWindow( hwnd );
		return FALSE;
	}

	// Set the video mode to 640x480x8
//	ddreturn = lpDirectDrawObject->SetDisplayMode(SCREEN_MODE_W, SCREEN_MODE_H,
//												  SCREEN_MODE_D);
//	if ( ddreturn != DD_OK )
//	{
//		DestroyWindow( hwnd );
//		return FALSE;
//	}
	// Create the primary surface
	ddsd.dwSize = sizeof ( ddsd );
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	ddreturn = lpDirectDrawObject->CreateSurface( &ddsd, &lpPrimary, NULL );
	if ( ddreturn != DD_OK )
	{
		DestroyWindow( hwnd );
		return FALSE;
	}



		// Get the address of the video memory

	BYTE	*lpDDMemory;

	memset ( &ddsd, 0, sizeof( DDSURFACEDESC ) );
	ddsd.dwSize = sizeof( ddsd );
	ddreturn = lpPrimary->Lock( NULL, &ddsd, 0, NULL );

	if( ddreturn != DD_OK )
	{
		DestroyWindow( hwnd );
		return FALSE;
	}

	lpDDMemory = (BYTE *) ddsd.lpSurface;

		// Set the palette

	PALETTEENTRY        pe[256];
	LPDIRECTDRAWPALETTE lpDDPalette;

		//create a DirectDraw palette for the surface

	for(int i=0; i<256; i++ )
	{
		pe[i].peRed = pe[i].peGreen = pe[i].peBlue = i;
	}
//	ddreturn = lpDirectDrawObject->CreatePalette( DDPCAPS_8BIT,
//												  pe,
//												  &lpDDPalette,
//												  NULL );
	if ( ddreturn != DD_OK )
	{
		DestroyWindow( hwnd );
		return FALSE;
	}
//	lpPrimary->SetPalette ( lpDDPalette );
	
		// Draw the mandelbrot

	const double start_x = center_x - width/2.;
	const double start_y = center_y - height/2.;

	double y = start_y;

	for(int k = 0; k < SCREEN_MODE_H; k++ )
	{
		double x = start_x;
		BYTE* p = lpDDMemory;

		for (int j = 0; j < SCREEN_MODE_W; j++)
		{
			int count = 0;
			double nx = x;
			double ny = y;
			double xx, yy;

			do
			{
				xx = nx*nx;
				yy = ny*ny;

				ny = 2. * nx * ny + y;
				nx = xx - yy + x;
				++count;
			} while (((xx + yy) < 4.) && (count < 255));

			*p++ = count;
			x += dx;
		}

		lpDDMemory += ddsd.lPitch;
		y += dy;
	}

	Sleep(5000);

	lpPrimary->Unlock ( NULL );






	while ( 1 )
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		{
			if( !GetMessage( &msg, NULL, 0, 0 ) )
				return msg.wParam;
			TranslateMessage ( &msg );
			DispatchMessage ( &msg );
		}
		else if( !ActiveApp )
		{
			WaitMessage ();
		}
	}

}


//WinMain

//*********************************************************
//WindowProc - receive and handle windows messages
//
//
//*********************************************************
long FAR PASCAL WindowProc( HWND hwnd, UINT message,
							WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{

		case WM_ACTIVATEAPP:
			ActiveApp = wParam;
			break;

		case WM_CREATE:
			break;

		case WM_KEYDOWN:
			switch( wParam )
			{
				case VK_ESCAPE:
					DestroyWindow( hwnd );
					break;
			}
			break;

		case WM_DESTROY:
			if ( lpDirectDrawObject != NULL )
			{
				if ( lpPrimary != NULL )
					lpPrimary->Release();
				lpDirectDrawObject->Release();
			}

			ShowCursor( TRUE );
			PostQuitMessage( 0 );
			break;

		default:
			return DefWindowProc (hwnd,message,wParam,lParam);
	}

	return 0L;

}