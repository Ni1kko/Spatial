#pragma once
class AntiDetection
{
public:
	AntiDetection();
	bool install(DWORD reason, LPVOID reserved) noexcept;
private:
	HMODULE moduleHandle;
	
	void cleanPEheader() noexcept;
	void HideModule() noexcept;
};

