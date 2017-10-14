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
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Microsoft::Graphics::Canvas::Effects;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::Pickers;
using namespace Windows::ApplicationModel::DataTransfer;
using namespace Microsoft::WRL;
using namespace concurrency;


MainPage::MainPage()
{
	InitializeComponent();

	Windows::ApplicationModel::Core::CoreApplication::GetCurrentView()->TitleBar->ExtendViewIntoTitleBar=true;
	auto coreTitleBar = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->TitleBar;
	coreTitleBar->ButtonBackgroundColor =Windows::UI::Colors::Transparent;
	coreTitleBar->ButtonInactiveBackgroundColor = Windows::UI::Colors::Transparent;
	Window::Current->SetTitleBar(realTitle);

}

void KismetUWP::MainPage::UpdateUiWindowState(UiWindowState state)
{
	switch (state) {
	case kWindowNone:
		droppanel->Visibility = Windows::UI::Xaml::Visibility::Visible;
		showpanel->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		progresspanel->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		break;
	case kWindowProgress:
		progresspanel->Visibility = Windows::UI::Xaml::Visibility::Visible;
		droppanel->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		showpanel->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		break;
	case kWindowShow:
		showpanel->Visibility = Windows::UI::Xaml::Visibility::Visible;
		droppanel->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		progresspanel->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		break;
	default:
		break;
	}
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

void KismetUWP::MainPage::CheckFilesum(Windows::Storage::StorageFile ^ file)
{
	if (nullptr == file)
		return;
	if (buffer == nullptr)
		buffer = ref new Buffer(8912);
	filesum.reset(new Filesum());
	if (!InitializeSum()) {
		auto messageDialog = ref new MessageDialog("Invalid Hash select");
		// Show the message dialog
		create_task(messageDialog->ShowAsync()).then([this](IUICommand^ command)
		{

		});
		return;
	}
	UpdateUiWindowState(kWindowProgress);
	rdsize = 0;
	create_task(file->GetBasicPropertiesAsync()).then([this, file](FileProperties::BasicProperties^ basicProperties)
	{
		filesize = basicProperties->Size;
		progressbar->Maximum = (double)filesize;
		currentFile = file->Path->Data();
		create_task(file->OpenSequentialReadAsync()).then([this]
		(Windows::Storage::Streams::IInputStream^ stream_)
		{
			//progressRing->IsActive = true;
			stream = stream_;
			create_task(stream->ReadAsync(buffer, 8192, Streams::InputStreamOptions::None)).then([this]
			(Windows::Storage::Streams::IBuffer ^buf) {
				this->ProcessAsync();
			});
		});
	});
}

void KismetUWP::MainPage::ProcessAsync()
{
	progressbar->Value = (double)rdsize;
	wchar_t buf[20];
	_snwprintf_s(buf, sizeof(buf), L"%0.2f%%", 100 * (double)rdsize / filesize);
	progresstb->Text = ref new String(buf);
	if (buffer->Length == 0) {
		UpdateUiWindowState(kWindowShow);
		filesum->Final(casecheck->IsChecked->Equals(true));
		filenametext->Text = ref new String(currentFile.data());
		hashsumcontent->Text = ref new String(filesum->Hash().data());
		std::wstring h(L"Hash (");
		h.append(filesum->Name()).push_back(')');
		hnblock->Text = ref new String(h.data());
		filesum.reset(); /// clear self
	}else {
		rdsize += buffer->Length;
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
	auto picker = ref new FileOpenPicker();
	picker->ViewMode = PickerViewMode::Thumbnail;
	picker->SuggestedStartLocation = PickerLocationId::DocumentsLibrary;
	picker->FileTypeFilter->Append("*");
	create_task(picker->PickSingleFileAsync()).then([this]
	(Windows::Storage::StorageFile^ file)
	{
		CheckFilesum(file);
	});
}


void KismetUWP::MainPage::ClearFilesumContent(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	UpdateUiWindowState(kWindowNone);
	if (!filesum) {
		hashsumcontent->Text = L"";
	}
}


void KismetUWP::MainPage::OnDragOver(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e)
{
	e->AcceptedOperation = Windows::ApplicationModel::DataTransfer::DataPackageOperation::Copy;
	e->DragUIOverride->Caption = L"Hashsum file";
}


void KismetUWP::MainPage::OnDrop(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e)
{
	auto def = e->GetDeferral();
	auto dv = e->DataView;
	if (dv->Contains(StandardDataFormats::StorageItems)) {
		create_task(dv->GetStorageItemsAsync()).then([this,def,dv](IVectorView<IStorageItem^> ^items) {
			if (items->Size > 0) {
				auto item = items->GetAt(0);
				if (item->IsOfType(StorageItemTypes::File)) {
					CheckFilesum((StorageFile ^)item);
				}
			}
			VisualStateManager::GoToState(this, "Generic", false);
			def->Complete();
		});
	}
}

