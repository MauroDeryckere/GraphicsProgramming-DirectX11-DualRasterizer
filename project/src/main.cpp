#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
			"DirectX - Mauro Deryckere/2DAE19N",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	static bool displayFPS = true;

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.scancode == SDL_SCANCODE_F1)
				{
					pRenderer->ToggleRasterizerMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					pRenderer->ToggleRotationMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F3)
				{
					pRenderer->ToggleFireMesh();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F4)
				{
					pRenderer->ChangeSamplerState();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					pRenderer->ChangeShadingMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F6)
				{
					pRenderer->ToggleNormalMap();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F7)
				{
					pRenderer->ToggleDisplayDepthBuffer();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F8)
				{
					pRenderer->ToggleBoundingBoxes();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F9)
				{
					pRenderer->ChangeCullMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F10)
				{
					pRenderer->ToggleUniformClearColor();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F11)
				{
					displayFPS = !displayFPS;
					if (displayFPS)
					{
						std::cout << "Display FPS -> Enabled\n";
					}
					else
					{
						std::cout << "Display FPS -> Disabled\n";
					}
				}
				break;
			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			if (displayFPS)
			{
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}