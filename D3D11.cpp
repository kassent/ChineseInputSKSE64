#pragma comment (lib, "D3D11.lib")
#include <D3D11.h>
#include "HookUtil.h"
#include "RimeManager.h"
#include "Utilities.h"
#include "ConfigLoader.h"
#include "skse64_common/Relocation.h"
#include "skse64_common/Utilities.h"
#include "skse64_common/SafeWrite.h"
#include "imgui.h"
#include "examples/imgui_impl_win32.h"
#include "examples/imgui_impl_dx11.h"

using namespace rime;
class DX11
{
public:
	static 	IDXGISwapChain	*			pSwapChain;
	static  ID3D11Device *				pDevice;
	static  ID3D11DeviceContext *		pContext;

	using FnD3D11CreateDeviceAndSwapChain = decltype(D3D11CreateDeviceAndSwapChain)*;
	using FnPresent = HRESULT(*)(IDXGISwapChain*, UINT, UINT);
	using FnSetViewports = HRESULT(*)(ID3D11DeviceContext*, UINT, const D3D11_VIEWPORT*);

	static FnPresent					Present_RealFunc;
	static FnSetViewports				SetViewports_RealFunc;
	static FnD3D11CreateDeviceAndSwapChain	D3D11CreateDeviceAndSwapChain_RealFunc;

	static HRESULT SetViewports_Hook(ID3D11DeviceContext * pContext, UINT NumViewports, const D3D11_VIEWPORT *pViewports)
	{
		_MESSAGE("[CI] numViewports: %d    pViewports: %016I64X    width: %.2f    height: %.2f    X: %.2f    Y: %.2f", NumViewports, (uintptr_t)pViewports, pViewports->Width, pViewports->Height, pViewports->TopLeftX, pViewports->TopLeftY);
		return SetViewports_RealFunc(pContext, NumViewports, pViewports);
	}

	static HRESULT Present_Hook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		auto & rime = RimeManager::GetSingleton();
		if (rime.IsRimeComposing()) {
			if (rime.IsRimeEnabled()) {
				RimeManager::RimeIndicator indicator;
				RimeManager::GetSingleton().QueryIndicator(indicator);
				if (indicator.IsComposing())
				{
					ImGui::NewFrame();
					ImGui::SetNextWindowPos(ImVec2(ConfigLoader::GetSingleton().uiOffsetX, ConfigLoader::GetSingleton().uiOffsetY), ImGuiCond_Always); //x, y
					ImGui::SetNextWindowSize(ImVec2(330, 0), ImGuiCond_Always);
					int flag = /*ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | */ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar /*| ImGuiWindowFlags_NoMouseInputs*/;
					ImGui::Begin(indicator.schemaName.c_str(), nullptr, flag);                          // Create a window called "Hello, world!" and append into it.
					ImGui::Text(/*ImVec4(0.26f, 0.59f, 0.98f, 1.00f), */indicator.composition.c_str()); //ImGui::TextColored(ImVec4(1.0f,0.0f,1.0f,1.0f), "Pink");
					auto & candidateList = indicator.candidateList;
					size_t highlightIdx = indicator.curHighlightIndex;
					for (size_t i = 0; i < candidateList.size(); ++i) {
						ImGui::Selectable(candidateList[i].c_str(), i == highlightIdx, 0, ImVec2(0, 0), i != highlightIdx);
					}
					ImGui::Dummy(ImVec2(0, 0));
					ImGui::End();
					ImGui::Render();
					ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
				}
			}
			else
				rime.ClearComposition();
		}

		return Present_RealFunc(pSwapChain, SyncInterval, Flags);
	}

	static HRESULT D3D11CreateDeviceAndSwapChain_Hook(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppContext)
	{
		HRESULT result = (*D3D11CreateDeviceAndSwapChain_RealFunc)(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppContext);
		pSwapChain = *ppSwapChain;
		pDevice = *ppDevice;
		pContext = *ppContext;
		if (!Present_RealFunc)
		{
			Present_RealFunc = HookUtil::SafeWrite64(*(uintptr_t*)pSwapChain + 8 * 0x8, Present_Hook);
		}
		DXGI_SWAP_CHAIN_DESC sd;
		pSwapChain->GetDesc(&sd);

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGuiStyle * style = &ImGui::GetStyle();

		style->WindowPadding = ImVec2(15, 7);
		style->WindowRounding = 5.0f;
		style->WindowBorderSize = 0.0f;
		style->FramePadding = ImVec2(5, 5);
		style->FrameRounding = 4.0f;
		style->ItemSpacing = ImVec2(12, 10);
		style->ItemInnerSpacing = ImVec2(8, 6);
		style->IndentSpacing = 25.0f;
		style->ScrollbarSize = 15.0f;
		style->ScrollbarRounding = 9.0f;
		style->GrabMinSize = 5.0f;
		style->GrabRounding = 3.0f;
		style->WindowTitleAlign = ImVec2(0.5f, 0.5f); //标题居中对齐。

		style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 0.80f); //ImVec4(0.06f, 0.05f, 0.07f, 0.70f);
		style->Colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f); //ImVec4(0.16f, 0.29f, 0.48f, 1.00f); 一种偏暗的浅蓝色。
		style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
		style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
		style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f); //ImVec4(0.06f, 0.05f, 0.07f, 1.00f); //ImVec4(0.10f, 0.09f, 0.12f, 0.70f); //selected...
		style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f); //tree background...
		style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.14f, 0.17f, 0.70f);
		style->Colors[ImGuiCol_Separator] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
		style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
		//style->Colors[ImGuiCol_Tab] = //ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
		style->Colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		//style->Colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
		//style->Colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
		//style->Colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
		auto fonts = io.Fonts;
		auto font_path = GetProfileDirectory() + "Font.ttf";
		switch (ConfigLoader::GetSingleton().characterSet)
		{
		case kCharacterSet_Chinese:
			fonts->AddFontFromFileTTF(font_path.c_str(), 13.0f, NULL, fonts->GetGlyphRangesChineseFull());
			break;
		case kCharacterSet_Cyrillic:
			fonts->AddFontFromFileTTF(font_path.c_str(), 13.0f, NULL, fonts->GetGlyphRangesCyrillic());
			break;
		case KCharacterSet_Japanese:
			fonts->AddFontFromFileTTF(font_path.c_str(), 13.0f, NULL, fonts->GetGlyphRangesJapanese());
			break;
		case kCharacterSet_Korean:
			fonts->AddFontFromFileTTF(font_path.c_str(), 13.0f, NULL, fonts->GetGlyphRangesKorean());
			break;
		case kCharacterSet_Thai:
			fonts->AddFontFromFileTTF(font_path.c_str(), 13.0f, NULL, fonts->GetGlyphRangesThai());
			break;
		case kCharacterSet_Vietnamese:
			fonts->AddFontFromFileTTF(font_path.c_str(), 13.0f, NULL, fonts->GetGlyphRangesVietnamese());
			break;
		default:
			fonts->AddFontFromFileTTF(font_path.c_str(), 13.0f, NULL, NULL);
			break;
		}
		//fonts->AddFontFromFileTTF(font_path.c_str(), 13.0f, NULL, fonts->GetGlyphRangesChineseFull());
		ImGui_ImplWin32_Init(sd.OutputWindow);
		ImGui_ImplDX11_Init(pDevice, pContext);
		return result;
	}

	static void InitHook()
	{
		uintptr_t thunkAddress = (uintptr_t)GetIATAddr((UInt8 *)GetModuleHandle(NULL), "d3d11.dll", "D3D11CreateDeviceAndSwapChain");

		D3D11CreateDeviceAndSwapChain_RealFunc = (FnD3D11CreateDeviceAndSwapChain)*(uintptr_t *)thunkAddress;
		SafeWrite64(thunkAddress, (uintptr_t)D3D11CreateDeviceAndSwapChain_Hook);

		//g_branchTrampoline.Write6Branch(RelocAddr<uintptr_t*>(0x28E9686).GetUIntPtr(), (uintptr_t)InitDevice_Hook);
	}
};
IDXGISwapChain *				DX11::pSwapChain = nullptr;
ID3D11Device *					DX11::pDevice = nullptr;
ID3D11DeviceContext *			DX11::pContext = nullptr;
DX11::FnPresent					DX11::Present_RealFunc = nullptr;
DX11::FnSetViewports			DX11::SetViewports_RealFunc = nullptr;
DX11::FnD3D11CreateDeviceAndSwapChain	DX11::D3D11CreateDeviceAndSwapChain_RealFunc = nullptr;


void D3D11_Init() {
	DX11::InitHook();
}