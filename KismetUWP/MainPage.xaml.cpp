//
// MainPage.xaml.cpp
// MainPage 类的实现。
//
#include <wrl.h>  
#include "pch.h"
#include <robuffer.h> 
#include "MainPage.xaml.h"

using namespace KismetUWP;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Popups;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::Pickers;
using namespace Microsoft::WRL;
using namespace concurrency;
//“空白页”项模板在 http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409 上有介绍

MainPage::MainPage()
{
	InitializeComponent();
	auto coreTitleBar = Windows::ApplicationModel::Core::CoreApplication::GetCurrentView()->TitleBar;
	coreTitleBar->ExtendViewIntoTitleBar = true;
	Window::Current->SetTitleBar(realTitle);
}


byte * KismetUWP::MainPage::GetPointerToPixelData(Windows::Storage::Streams::IBuffer ^ pixelBuffer, unsigned int * length)
{
	Object^ obj = pixelBuffer;
	ComPtr<IInspectable> insp(reinterpret_cast<IInspectable*>(obj));

	// Query the IBufferByteAccess interface.
	ComPtr<IBufferByteAccess> bufferByteAccess;
	insp.As(&bufferByteAccess);

	// Retrieve the buffer data.
	byte* pixels = nullptr;
	bufferByteAccess->Buffer(&pixels);
	return pixels;
}

void KismetUWP::MainPage::ProcessAsync()
{
	if (buffer->Length == 0) {
		filesum->Final(false);
		std::wstring message(L"Name:\t");
		if (casecheck->IsChecked) {
			std::wstring hash=filesum->Hash();
			std::transform(hash.begin(), hash.end(), hash.begin(), toupper);
			message.append(currentFile).append(L"\r\nHash:\t").append(hash);
		}
		else {
			message.append(currentFile).append(L"\r\nHash:\t").append(filesum->Hash());
		}
		filesum.reset(); /// clear self
		hashsumcontent->Text=ref new String(message.data());
	}else {
		auto bt = GetPointerToPixelData(buffer, nullptr);
		if (bt) {
			filesum->Update((const char*)bt, buffer->Length);
		}
		create_task(stream->ReadAsync(buffer, 8192, Streams::InputStreamOptions::None)).then([this]
		(Windows::Storage::Streams::IBuffer ^buf) {
			this->ProcessAsync();
		});
	}
}

bool KismetUWP::MainPage::InitializeSum()
{
	switch (hashalgm->SelectedIndex) {
	case 0:
		filesum->Initialize(kFilesumMD5, 0);
		break;
	case 1:
		filesum->Initialize(kFilesumSHA1, 0);
		break;
	case 2:
		filesum->Initialize(kFilesumSHA1DC, 0);
		break;
	case 3:
		filesum->Initialize(kFilesumSHA2, 224);
		break;
	case 4:
		filesum->Initialize(kFilesumSHA2, 256);
		break;
	case 5:
		filesum->Initialize(kFilesumSHA2, 384);
		break;
	case 6:
		filesum->Initialize(kFilesumSHA2, 512);
		break;
	case 7:
		filesum->Initialize(kFilesumSHA3, 224);
		break;
	case 8:
		filesum->Initialize(kFilesumSHA3, 256);
		break;
	case 9:
		filesum->Initialize(kFilesumSHA3, 384);
		break;
	case 10:
		filesum->Initialize(kFilesumSHA3, 512);
		break;
	default:
		return false;
	}
	return true;
}

void KismetUWP::MainPage::InvokeFileOpenPicker(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (filesum) {
		
		return;
	}
	if(buffer==nullptr)
		buffer = ref new Buffer(8912);
	auto picker = ref new FileOpenPicker();
	picker->ViewMode = PickerViewMode::Thumbnail;
	picker->SuggestedStartLocation = PickerLocationId::DocumentsLibrary;
	picker->FileTypeFilter->Append("*");
	create_task(picker->PickSingleFileAsync()).then([this]
	(Windows::Storage::StorageFile^ file)
	{
		if (nullptr == file)
			return;
		filesum.reset(new Filesum());
		if (!InitializeSum()) {
			auto messageDialog = ref new MessageDialog("Invalid Hash select");
			// Show the message dialog
			create_task(messageDialog->ShowAsync()).then([this](IUICommand^ command)
			{

			});
			return;
		}
		currentFile = file->Name->Data();
		create_task(file->OpenSequentialReadAsync()).then([this]
		(Windows::Storage::Streams::IInputStream^ stream_)
		{
			stream = stream_;
			create_task(stream->ReadAsync(buffer, 8192, Streams::InputStreamOptions::None)).then([this]
			(Windows::Storage::Streams::IBuffer ^buf) {
				this->ProcessAsync();
			});
			//stream->ReadAsync()
			OutputDebugString(L"1. End of OpenReadAsync lambda.\r\n");
		});

		OutputDebugString(L"2. End of PickSingleFileAysnc lambda.\r\n");
	});
}


void KismetUWP::MainPage::ClearFilesumContent(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	
}
