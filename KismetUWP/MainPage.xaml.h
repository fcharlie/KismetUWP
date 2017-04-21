//
// MainPage.xaml.h
// MainPage 类的声明。
//

#pragma once
#include <memory>
#include <cstdint>
#include "../Hashsum/Hashsum.h"
#include "MainPage.g.h"
using namespace Windows::UI::Xaml::Controls;

namespace KismetUWP
{
	/// <summary>
	/// 可用于自身或导航至 Frame 内部的空白页。
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	private:
		std::wstring currentFile;
		std::shared_ptr<Filesum> filesum;
		Windows::Storage::Streams::IBuffer ^buffer;
		Windows::Storage::Streams::IInputStream^ stream;
		int64_t filesize = 0;
		int64_t rdsize = 0;
		void ProcessAsync();
		bool InitializeSum();
		byte* GetPointerToPixelData(Windows::Storage::Streams::IBuffer^ pixelBuffer, unsigned int *length);
		void CheckFilesum(Windows::Storage::StorageFile^ file);
		void InvokeFileOpenPicker(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ClearFilesumContent(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
