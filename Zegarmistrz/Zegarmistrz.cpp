#include <windows.h> //biblioteka WinApi (win32)
#include <tchar.h> //biblioteka MBCS - tworzy jednobajtowe, wielobajtowe zestawy znaków i aplikacje Unicode z tych samych źródeł
#include <stdio.h> //biblioteka standardowa C służąca do wykonywania operacji wejścia-wyjścia na strumieniach
#include <time.h> //biblioteka służąca do obsługi czasu (timerów)
#include <mmsystem.h> //biblioteka służąca jako plik nagłówkowy dla funkcji multimedialnych systemu Windows

#pragma comment(lib, "winmm.lib") //dodanie biblioteki do obsługi dźwięku sdo ustawień projektu

LPCWSTR WindowsClassName = L"ApiWindowClass"; //prefix L, aby w C literał string został przypisany do LPCWSTR, który jest odpowiednikiem char* i const char*, nazwa klasy
LPCWSTR WindowsApplicationName = L"Zegarmistrz"; //nazwa naszej aplikacji

HBITMAP obrazek; //zapisywane obrazki do formatu bitmapy
HBITMAP alarm;

#define BUTTON_ID_1 1000 //ta dyrektywa przypisuje podaną wartość do określonej nazwy, zdefiniowane przyciski i czas
#define BUTTON_ID_2 2000
#define BUTTON_ID_3 3000
#define BUTTON_ID_4 4000

#define TIMER_ID_1 1001

//funkcja do załadowania obrazka zegarmistrza
void ObrazekZegarmistrz()
{
	obrazek = static_cast<HBITMAP>(LoadImageW(NULL, L"C:\\Users\\PC\\Desktop\\Studia\\Semestr 3\\Notatki i zadania\\Programowanie aplikacyjne\\Projekt\\Zegarmistrz\\Zegarmistrz\\Zegarmistrz.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE)); //konwersja wskaźnika do klasy bazowej na wskaźnik do klasy pochodnej
}

//funkcja do załadowania obrazka do alarmu
void ObrazekAlarm()
{
	alarm = static_cast<HBITMAP>(LoadImageW(NULL, L"C:\\Users\\PC\\Desktop\\Studia\\Semestr 3\\Notatki i zadania\\Programowanie aplikacyjne\\Projekt\\Zegarmistrz\\Zegarmistrz\\alarm.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE)); //konwersja wskaźnika do klasy bazowej na wskaźnik do klasy pochodnej
}

//funkcja na program Zegar
void DrawClock(HWND hWnd) //HWND - identyfikator okna
{
	HDC hdc; //oznacza uchwyt do kontekstu urządzenia, ta struktura służy do uzyskiwania elementów graficznych
	PAINTSTRUCT ps; //ta struktura pozwala rysować kształty
	RECT rect; //definiuje prostokąt na podstawie współrzędnych jego lewego górnego i prawego dolnego rogu

	GetClientRect(hWnd, &rect); //pobiera współrzędne obszaru klienta okna
	InvalidateRect(hWnd, &rect, true); //dodaje prostokąt do regionu aktualizacji określonego okna

	hdc = BeginPaint(hWnd, &ps); //zaczyna rysować
	int lfh = -MulDiv(36, GetDeviceCaps(hdc, LOGPIXELSY), 72); //low-fragmentation heap - fragmentacja sterty, określa wysokość czcionki o określonym rozmiarze w punktach/pikselach
	HFONT font = CreateFont(lfh, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, 0, L"Lato"); //tworzenie czcionki z wygładzaniem

	SelectObject(hdc, font); //wybranie czcionki, ustawienie koloru z palety barw rgb, tło nie ulega modyfikacji, wycentrowanie tekstu
	SetTextColor(hdc, RGB(40, 140, 40));
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_CENTER);

	wchar_t time[32]; //pobranie czasu systemowego
	SYSTEMTIME st;
	GetLocalTime(&st);
	GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &st, NULL, time, 32);

	const int x = (rect.right - rect.left) / 2; //ustawia koordynaty, w którym konkretnie miejscu ma narysować zegar, a wynik w postaci "tekstu"
	const int y = (rect.bottom - rect.top + lfh) / 2;
	TextOut(hdc, x, y, time, lstrlen(time));

	EndPaint(hWnd, &ps); //kończy rysować
}

//funkcje na chodzenie i zatrzymanie programu Zegar
void RestartTimer(HWND hWnd)
{
	SetTimer(hWnd, TIMER_ID_1, 1000, NULL); //tworzy licznik czasu z określoną wartością limitu czasu, odstęp czasowy 1000ms
}

void StopTimer(HWND hWnd)
{
	KillTimer(hWnd, TIMER_ID_1); //niszczy określony licznik czasu
}

//funkcje na chodzenie i zatrzymanie programu Alarm
void RestartTimer2(HWND hWnd)
{
	SetTimer(hWnd, TIMER_ID_1, 1000, NULL); //tworzy licznik czasu z określoną wartością limitu czasu, odstęp czasowy 1000ms
}

void StopTimer2(HWND hWnd)
{
	KillTimer(hWnd, TIMER_ID_1); //niszczy określony licznik czasu
}

//funkcja na program Minutnik
int Minutnik(int s, int ileSekund) //w jakim odstępie czasowym ma odliczać ilość sekund w dół
{
	HANDLE hTimer = NULL; //token reprezentujący zasób zarządzany przez jądro systemu Windows
	LARGE_INTEGER liDueTime; //struktura reprezentuje 64-bitową liczbę całkowitą ze znakiem, inicjuje obiekt sekcji krytycznej i ustawia liczbę obrotów dla sekcji krytycznej

	liDueTime.QuadPart = -150000 * (ULONGLONG)s; //sygnalizuje jeden obiekt i czeka na inny obiekt jako pojedynczą operację, czas, po którym stan timera ma być sygnalizowany w odstępach 150000 sekund

	//tworzy ,,czekający'' timer
	hTimer = CreateWaitableTimer(NULL, TRUE, L"WaitableTimer");
	if (NULL == hTimer)
	{
		printf("CreateWaitableTimer failed (%lu)\n", GetLastError());
		return 1;
	}

	printf("Odliczanie %d s\n", s);

	while (ileSekund--)
	{
		//ustawia timer, aby zaczekał s sekund
		if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
		{
			printf("SetWaitableTimer failed (%lu)\n", GetLastError());
			return 2;
		}

		//czekanie na timer
		if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
		{
			printf("WaitForSingleObject failed (%lu)\n", GetLastError());
		}
		else
		{
			printf("%d \n", ileSekund);
		}
	}
	return 0;
}

//funkcja na program Stoper
int Stoper(int s, int ileSekund) //w jakim odstępie czasowym ma odliczać ilość sekund w górę
{
	HANDLE hTimer = NULL; //token reprezentujący zasób zarządzany przez jądro systemu Windows
	LARGE_INTEGER liDueTime; //struktura reprezentuje 64-bitową liczbę całkowitą ze znakiem, inicjuje obiekt sekcji krytycznej i ustawia liczbę obrotów dla sekcji krytycznej

	liDueTime.QuadPart = -150000 * (ULONGLONG)s; //sygnalizuje jeden obiekt i czeka na inny obiekt jako pojedynczą operację, czas, po którym stan timera ma być sygnalizowany w odstępach 150000 sekund

	//tworzy ,,czekający'' timer
	hTimer = CreateWaitableTimer(NULL, TRUE, L"WaitableTimer");
	if (NULL == hTimer)
	{
		printf("CreateWaitableTimer failed (%lu)\n", GetLastError());
		return 1;
	}

	printf("Stoper: %d s\n", ileSekund);

	while (ileSekund++)
	{
		//ustawia timer, aby zaczekał s sekund
		if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
		{
			printf("SetWaitableTimer failed (%lu)\n", GetLastError());
			return 2;
		}

		//czekanie na timer
		if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
		{
			printf("WaitForSingleObject failed (%lu)\n", GetLastError());
		}
		else
		{
			printf("%d \n", ileSekund);
		}
	}
	return 0;
}

HWND Tekst1; //HWND - identyfikator okna

/* 
	WindowProcedure - funkcja procedury okna,
	LRESULT - wartość całkowita, którą program zwraca do systemu Windows,
	CALLBACK - jest konwencją wywoływania funkcji,
	HWND hWnd - identyfikator okna,
	UINT msg - predefiniowana wartość wiadomości,
	WPARAM wParam - dodatkowe informacje o wiadomości zależne od wiadomości,
	LPARAM lParam - dodatkowe informacje o wiadomości zależne od wiadomości.
*/

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) //funkcja procedury okna
{
	HWND hStatic; //element sterujący, który umożliwia aplikacji dostarczanie użytkownikowi tekstu informacyjnego i grafiki
	switch (msg)
	{
	case WM_CREATE: //tworzenie obrazka, okienka, przesłanie ,,wiadomości" oraz tworzenie przycisków
		ObrazekZegarmistrz();
		hStatic = CreateWindowW(L"Static", L"", WS_CHILD | WS_VISIBLE | SS_BITMAP, 300, 150, 800, 600, hWnd, NULL, NULL, NULL);
		SendMessage(hStatic, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)obrazek);
		CreateWindowExW(0, L"Button", L"Zegar", WS_CHILD | WS_VISIBLE, 30, 500, 150, 40, hWnd, (HMENU)BUTTON_ID_1, NULL, NULL);
		CreateWindowExW(0, L"Button", L"Alarm", WS_CHILD | WS_VISIBLE, 300, 500, 150, 40, hWnd, (HMENU)BUTTON_ID_2, NULL, NULL);
		CreateWindowExW(0, L"Button", L"Minutnik", WS_CHILD | WS_VISIBLE, 730, 500, 150, 40, hWnd, (HMENU)BUTTON_ID_3, NULL, NULL);
		CreateWindowExW(0, L"Button", L"Stoper", WS_CHILD | WS_VISIBLE, 1000, 500, 150, 40, hWnd, (HMENU)BUTTON_ID_4, NULL, NULL);
		break;
	case WM_COMMAND: //użytkownik wybiera element polecenia z menu, gdy formant wysyła powiadomienie do swojego okna nadrzędnego
		switch (wParam)
		{
		case BUTTON_ID_1: //funkcja Zegar - wznawianie i zatrzymywanie czasu systemowego
			DeleteObject(obrazek);
			DeleteObject(alarm);
			ObrazekZegarmistrz();
			hStatic = CreateWindowW(L"Static", L"", WS_CHILD | WS_VISIBLE | SS_BITMAP, 300, 5, 800, 600, hWnd, NULL, NULL, NULL);
			SendMessage(hStatic, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)obrazek);
			int value_1;
			int value_2;
			value_1 = MessageBox(hWnd, L"Pokazuje Zegar.", L"Zegar", MB_ICONINFORMATION);
			if (value_1 == IDOK)
			{
				DrawClock(hWnd);
			}
			value_2 = MessageBox(hWnd, L"Uruchomić funkcję Zegara?", L"Zegar", MB_YESNO | MB_ICONINFORMATION);
			if (value_2 == IDYES)
			{
				RestartTimer(hWnd);
			}
			else if (value_2 == IDNO)
			{
				StopTimer(hWnd);
			}
			break;
		case BUTTON_ID_2: //funkcja Alarm - po naciśnięciu przycisku pojawia się niespodzianka :D
			DeleteObject(obrazek);
			DeleteObject(alarm);
			int value_3;
			int value_4;
			int value_4a;
			value_3 = MessageBox(hWnd, L"Pokazuje Alarm.", L"Alarm", MB_ICONINFORMATION);
			if (value_3 == IDOK)
			{
				DrawClock(hWnd);
			}
			value_4 = MessageBox(hWnd, L"Uruchomić funkcję Alarmu?", L"Alarm", MB_ICONINFORMATION);
			if (value_4 == IDOK)
			{
				RestartTimer2(hWnd);
				DeleteObject(alarm);
			}
			value_4a = MessageBox(hWnd, L"Kliknij, aby aktywować Alarm.", L"Alarm", MB_ICONWARNING);
			if (value_4a == IDOK)
			{
				StopTimer2(hWnd);
				ObrazekAlarm();
				PlaySoundA((LPCSTR)"C:\\Users\\PC\\Desktop\\Studia\\Semestr 3\\Notatki i zadania\\Programowanie aplikacyjne\\Projekt\\Zegarmistrz\\Zegarmistrz\\alarm.wav", NULL, SND_FILENAME | SND_ASYNC);
				getchar(); //pobiera znak, aby wstrzymać program na tyle długo, aby odtworzyć plik dźwiękowy
				hStatic = CreateWindowW(L"Static", L"", WS_CHILD | WS_VISIBLE | SS_BITMAP, 150, -80, 100, 100, hWnd, NULL, NULL, NULL);
				SendMessage(hStatic, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)alarm);
			}
			break;
		case BUTTON_ID_3: //funkcja Minutnik - odlicza minutę, czyli zlicza od 60 sekund od 0
			DeleteObject(obrazek);
			DeleteObject(alarm);
			ObrazekZegarmistrz();
			hStatic = CreateWindowW(L"Static", L"", WS_CHILD | WS_VISIBLE | SS_BITMAP, 300, 70, 800, 600, hWnd, NULL, NULL, NULL);
			SendMessage(hStatic, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)obrazek);
			int value_5;
			int value_6;
			value_5 = MessageBox(hWnd, L"Pokazuje Minutnik.", L"Minutnik", MB_ICONINFORMATION);
			if (value_5 == IDOK)
			{
				/*
				LPCWSTR text = L"Odliczanie:\n";
				Tekst1 = CreateWindowEx(0, L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_CENTER, 450, 350, 300, 50, hWnd, NULL, NULL, NULL);
				wchar_t text_buffer[20] = { 0 }; //tymczasowy bufor
				swprintf(text_buffer, _countof(text_buffer), L"Czas: %d", Minutnik); //konwertowanie
				OutputDebugString(text_buffer); //wypisanie liczb z debugowania konsoli do okna
				*/

				FILE* fp;
				AllocConsole();
				freopen_s(&fp, "CONIN$", "r", stdin);
				freopen_s(&fp, "CONOUT$", "w", stdout);
				freopen_s(&fp, "CONOUT$", "w", stderr);
				printf("Koniec!", Minutnik(60, 60));
			}
			value_6 = MessageBox(hWnd, L"Upłynęła minuta!", L"Minutnik", MB_ICONINFORMATION);
			break;
		case BUTTON_ID_4: //funkcja Stoper - czas liczony od 0 do momentu, aż go wyłączymy
			DeleteObject(obrazek);
			DeleteObject(alarm);
			ObrazekZegarmistrz();
			hStatic = CreateWindowW(L"Static", L"", WS_CHILD | WS_VISIBLE | SS_BITMAP, 300, 70, 800, 600, hWnd, NULL, NULL, NULL);
			SendMessage(hStatic, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)obrazek);
			int value_7;
			int value_8;
			value_7 = MessageBox(hWnd, L"Pokazuje Stoper.", L"Stoper", MB_ICONINFORMATION);
			if (value_7 == IDOK)
			{
				/*
				LPCWSTR text = L"Odliczanie:\n";
				Tekst1 = CreateWindowEx(0, L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_CENTER, 450, 350, 300, 50, hWnd, NULL, NULL, NULL);
				wchar_t text_buffer[20] = { 0 }; //tymczasowy bufor
				swprintf(text_buffer, _countof(text_buffer), L"Czas: %d", Stoper); //konwertowanie
				OutputDebugString(text_buffer); //wypisanie liczb z debugowania konsoli do okna
				*/

				FILE* fp;
				AllocConsole();
				freopen_s(&fp, "CONIN$", "r", stdin);
				freopen_s(&fp, "CONOUT$", "w", stdout);
				freopen_s(&fp, "CONOUT$", "w", stderr);
				printf("Odliczanie:", Stoper(60, 1));
			}
			value_8 = MessageBox(hWnd, L"Czas Stop!", L"Stoper", MB_ICONINFORMATION);
			break;
		default:
			MessageBox(hWnd, L"Oj, zrobiłeś coś innego", L"Problem", MB_ICONERROR);
		}
		break;
	case WM_TIMER: //wskaźnik do funkcji wywołania zwrotnego zdefiniowanej przez aplikację, która została przekazana do funkcji SetTimer podczas instalowania czasomierza
		if (wParam == TIMER_ID_1)
		{
			RestartTimer(hWnd);
			SendMessage(hWnd, WM_PAINT, 0, 0);
		}
		break;
	case WM_PAINT: //system lub inna aplikacja wysyła żądanie pomalowania części okna aplikacji
		{
		PAINTSTRUCT ps; //ta struktura pozwala rysować kształty
		HDC hdc; //oznacza uchwyt do kontekstu urządzenia, ta struktura służy do uzyskiwania elementów graficznych
		hdc = BeginPaint(hWnd, &ps); //zaczyna rysować

		DrawClock(hWnd); //rysuje zegar
		
		EndPaint(hWnd, &ps); //kończy rysować
		break;
		}
		return 0;
	case WM_CLOSE: //sygnał, że okno lub aplikacja powinna zakończyć działanie
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY: //okno jest niszczone wraz z innymi elementami
		DeleteObject(obrazek);
		DeleteObject(alarm);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam); //domyślne przetwarzanie komunikatów zdefiniowanych w podsystemie win32
	}
	return 0;
}

 /*
	 WinApi instancja okna w C++ z obsługą Unicode:
	 WinMain - dostarczony przez użytkownika punkt wejścia dla graficznej aplikacji opartej na systemie Windows,
	 HINSTANCE hInstance - uchwyt instancji aplikacji,
	 HINSTANCE hPrevInstance - uchwyt poprzedniego wywołania programu,
	 LPSTR szCmdLine - linia poleceń, z jakiej został uruchomiony nasz program, argumenty,
	 int iCmdShow - stan okna, flaga czy okno ma być normalne, zminimalizowane czy zmaksymalizowane.
 */

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) //instancja okna
{
	WNDCLASSEX WindowsClass; //struktura klasy
	WindowsClass.cbSize = sizeof(WNDCLASSEX); //rozmiar struktury w bajtach
	WindowsClass.style = 0; //styl klasy
	WindowsClass.lpfnWndProc = WindowProcedure; //wskaźnik do procedury obsługi okna
	WindowsClass.cbClsExtra = 0; //dodatkowe bajty dla klasy
	WindowsClass.cbWndExtra = 0; //dodatkowe bajty dla klasy
	WindowsClass.hInstance = hInstance; //identyfikator właściciela
	WindowsClass.hIcon = LoadIcon(NULL, IDI_APPLICATION); //ikona aplikacji
	WindowsClass.hCursor = LoadCursor(NULL, IDC_ARROW); //kursor myszki
	WindowsClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);; //kolor tła
	WindowsClass.lpszMenuName = NULL; //nazwa identyfikująca menu
	WindowsClass.lpszClassName = WindowsClassName; //nazwa klasy
	WindowsClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION); //mała ikona

	if (!RegisterClassEx(&WindowsClass)) //rejestrowanie klasy okna
	{
		MessageBox(NULL, L"Rejestracja klasy zakończona niepowodzeniem", L"Error", 0);
		return 0;
	}

	HWND hWnd = CreateWindowEx(WS_EX_WINDOWEDGE, WindowsClassName, WindowsApplicationName, WS_OVERLAPPEDWINDOW, 150, 100, 1200, 600, NULL, NULL, hInstance, NULL);
	if (!hWnd) //tworzenie okna
	{
		MessageBoxW(NULL, L"Nie udało się utworzyć okna!", L"Error!", 0);
	}

	ShowWindow(hWnd,iCmdShow); //wyświetla okno w bieżącym rozmiarze i położeniu
	UpdateWindow(hWnd); //aktualizuje obszar klienta określonego okna, wysyłając wiadomość WM_PAINT do okna, jeśli region aktualizacji okna nie jest pusty

	MessageBox(NULL, L"Zegarmistrz został uruchomiony!", L"Następuje kalkulacja czasu.", MB_ICONINFORMATION | MB_OK); //tworzy, wyświetla i obsługuje okno komunikatu

	MSG Message; //system operacyjny komunikuje się z oknem aplikacji, przekazując do niego komunikaty
	while (GetMessage(&Message, NULL, 0, 0)) //pętla komunikatów
	{
		TranslateMessage(&Message); //tłumaczy komunikaty klucza wirtualnego na komunikaty znakowe
		DispatchMessage(&Message); //wysyła komunikat do procedury okna
	}
	return (int) Message.wParam; //zwraca typ int jako wiadomość umieszczaną na górze kolejki powiązanej z wątkiem, który zarejestrował klawisz skrótu
}
