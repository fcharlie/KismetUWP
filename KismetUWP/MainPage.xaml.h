//
// MainPage.xaml.h
// MainPage 类的声明。
//

#pragma once
#include <memory>
#include "../Hashsum/Hashsum.h"
#include "MainPage.g.h"


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
		byte* GetPointerToPixelData(Windows::Storage::Streams::IBuffer^ pixelBuffer, unsigned int *length);
		void ProcessAsync();
		bool InitializeSum();
		void InvokeFileOpenPicker(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ClearFilesumContent(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
