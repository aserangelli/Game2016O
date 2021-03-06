#include "stdafx.h"
#include "SGameOver.h"
#include "SMain.h"
#include "Graphics\DXBasicPainter.h"
#include "Graphics\DXManager.h"
#include "SGame.h"
#include "HSM\EventWin32.h"
#include "HSM\StateMachineManager.h"
#include "Graphics\ImageBMP.h"
#include "SGameOver.h"
#include "SMainMenu.h"
#include "SCredits.h"

CSGameOver::CSGameOver()
{
}


CSGameOver::~CSGameOver()
{
}


void CSGameOver::OnEntry(void)
{
	CSMain* p_main = (CSMain*)GetSuperState();

	m_pDXManager = p_main->m_pDXManager;
	m_pDXPainter = p_main->m_pDXPainter;
	
	MAIN->m_pDXManager->GetContext()->OMSetBlendState(NULL, NULL, -1);

	m_pEffects = new CFX(m_pDXManager);

	if (!m_pEffects->Initialize())
	{
		MessageBox(NULL, L"No se pudo iniciar el FX",
			L"No se pudo Iniciar", MB_ICONERROR);
		// Preguntar al corni si nos regreamos al estado nulo
	}
	int a = MAIN->winner;
	switch (a)
	{
	case 0:
		g_pSysTextur = CImageBMP::CreateBitmapFromFile("..\\Assets\\00WinnerBackground.bmp", NULL);
		break;

	case 1:
		g_pSysTextur = CImageBMP::CreateBitmapFromFile("..\\Assets\\01WinnerBackground.bmp", NULL);
		break;

	case 2:
		g_pSysTextur = CImageBMP::CreateBitmapFromFile("..\\Assets\\02WinnerBackground.bmp", NULL);
		break;

	case 3:
		g_pSysTextur = CImageBMP::CreateBitmapFromFile("..\\Assets\\03WinnerBackground.bmp", NULL);
		break;
	default:
		g_pSysTextur = CImageBMP::CreateBitmapFromFile("..\\Assets\\03WinnerBackground.bmp", NULL);
		break;
	}
	
	m_pImag = g_pSysTextur->CreateTexture(m_pDXManager);
	MAIN->m_pSndManager->ClearEngine();
	auto fx = MAIN->m_pSndManager->LoadSoundFx(L"..\\Assets\\Explosion.wav", SND_EXPLOSION);
	if (fx)
	{
		printf("Explosion load success...\n");
	}
	else
	{
		printf("No se pudo cargar el sonido...\n");
	}
	fflush(stdout);
	m_pSndBackground = MAIN->m_pSndManager->LoadSoundFx(L"..\\Assets\\WinSound.wav", SND_BACKGROUND);
	if (m_pSndBackground)
	{
		m_pSndBackground->Play(true);
	}
	SetTimer(MAIN->m_hWnd, 1, 5000, NULL); // el 1 es el identificador del timer es local entre ventanas
	SetTimer(MAIN->m_hWnd, 2, 1000, NULL);
}

unsigned long CSGameOver::OnEvent(CEventBase * pEvent)
{
	static float speed = 1.0f;
	// Todo lo que hagamos dentro de este if es nuestro tiempo libre
	if (APP_LOOP == pEvent->m_ulEventType)
	{
		if (MAIN->m_pSndManager)
		{
			MAIN->m_pSndManager->RemoveAllSndFxStopped();
		}

		ID3D11Texture2D* pBackBuffer = 0;
		m_pDXManager->GetSwapChain()->GetBuffer(0, IID_ID3D11Texture2D, (void**)&pBackBuffer);

		D3D11_TEXTURE2D_DESC dtd;
		// En estos momentos ya tengo la resolucion y ipo exactos
		pBackBuffer->GetDesc(&dtd);

		ID3D11ShaderResourceView* pSRV = NULL;
		m_pDXManager->GetDevice()->CreateShaderResourceView(m_pImag, NULL, &pSRV);
		m_pDXManager->GetContext()->PSSetShaderResources(0, 1, &pSRV);

		m_pEffects->SetRenderTarget(m_pDXManager->GetMainRTV());
		m_pEffects->SetInput(pSRV);
		m_pEffects->Process(0, dtd.Width, dtd.Height);

		m_pDXManager->GetSwapChain()->Present(1, 0);

		SAFE_RELEASE(pSRV);

		//return 0;
	}
	if (EVENT_WIN32 == pEvent->m_ulEventType)
	{
		CEventWin32* pWin32 = (CEventWin32*)pEvent;
		switch (pWin32->m_msg)
		{
		case WM_CHAR:
		{
			if ('b' == pWin32->m_wParam) // si es igual hacemos una transcicion
			{
				//MessageBoxA(NULL, "Vamos a transitar de S11 a S121", GetClassName(), MB_ICONINFORMATION);
				m_pSMOwner->Transition(CLSID_CSGame);
				return 0; // nunca se nosolvide retornar 0 para que no existe access violation
			}
			switch (pWin32->m_wParam)
			{
			case 'i':
				speed = min(4.0, speed + 0.1);
				this->m_pSndBackground->SetSpeed(speed);
				break;
			case 'k':
				speed = max(4.0, speed + 0.1);
				this->m_pSndBackground->SetSpeed(speed);
				break;
			}
		}

		break;
		
		case WM_TIMER:
			if (1 == pWin32->m_wParam)
			{
				m_pSMOwner->Transition(CLSID_CSCredits);
				InvalidateRect(MAIN->m_hWnd, NULL, false);
				return 0;
			}
			if (2 == pWin32->m_wParam)
			{
				KillTimer(MAIN->m_hWnd, 2);
				MAIN->m_pSndManager->PlayFx(SND_EXPLOSION, 1, 0, 0.3);
			}
			break;
		case WM_CLOSE:
			m_pSMOwner->Transition(CLSID_CStateNull);
			return 0;
		}
	}
	return __super::OnEvent(pEvent);
}

void CSGameOver::OnExit(void)
{
	KillTimer(MAIN->m_hWnd, 1);
	SAFE_DELETE(m_pEffects);
}