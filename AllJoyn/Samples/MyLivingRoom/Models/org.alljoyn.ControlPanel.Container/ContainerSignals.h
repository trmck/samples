//-----------------------------------------------------------------------------
// <auto-generated> 
//   This code was generated by a tool. 
// 
//   Changes to this file may cause incorrect behavior and will be lost if  
//   the code is regenerated.
//
//   Tool: AllJoynCodeGenerator.exe
//
//   This tool is located in the Windows 10 SDK and the Windows 10 AllJoyn 
//   Visual Studio Extension in the Visual Studio Gallery.  
//
//   The generated code should be packaged in a Windows 10 C++/CX Runtime  
//   Component which can be consumed in any UWP-supported language using 
//   APIs that are available in Windows.Devices.AllJoyn.
//
//   Using AllJoynCodeGenerator - Invoke the following command with a valid 
//   Introspection XML file and a writable output directory:
//     AllJoynCodeGenerator -i <INPUT XML FILE> -o <OUTPUT DIRECTORY>
// </auto-generated>
//-----------------------------------------------------------------------------
#pragma once

namespace org { namespace alljoyn { namespace ControlPanel { namespace Container {

ref class ContainerSignals;

public interface class IContainerSignals
{
    event Windows::Foundation::TypedEventHandler<ContainerSignals^, ContainerMetadataChangedReceivedEventArgs^>^ MetadataChangedReceived;
};

public ref class ContainerSignals sealed : [Windows::Foundation::Metadata::Default] IContainerSignals
{
public:
    // Calling this method will send the MetadataChanged signal to every member of the session.
    void MetadataChanged();

    // This event fires whenever the MetadataChanged signal is sent by another member of the session.
    virtual event Windows::Foundation::TypedEventHandler<ContainerSignals^, ContainerMetadataChangedReceivedEventArgs^>^ MetadataChangedReceived 
    { 
        Windows::Foundation::EventRegistrationToken add(Windows::Foundation::TypedEventHandler<ContainerSignals^, ContainerMetadataChangedReceivedEventArgs^>^ handler) 
        { 
            return _MetadataChangedReceived += ref new Windows::Foundation::EventHandler<Platform::Object^>
            ([handler](Platform::Object^ sender, Platform::Object^ args)
            {
                handler->Invoke(safe_cast<ContainerSignals^>(sender), safe_cast<ContainerMetadataChangedReceivedEventArgs^>(args));
            }, Platform::CallbackContext::Same);
        } 
        void remove(Windows::Foundation::EventRegistrationToken token) 
        { 
            _MetadataChangedReceived -= token; 
        } 
    internal: 
        void raise(ContainerSignals^ sender, ContainerMetadataChangedReceivedEventArgs^ args) 
        { 
            _MetadataChangedReceived(sender, args);
        } 
    }

internal:
    void Initialize(_In_ alljoyn_busobject busObject, _In_ alljoyn_sessionid sessionId);
    void CallMetadataChangedReceived(_In_ ContainerSignals^ sender, _In_ ContainerMetadataChangedReceivedEventArgs^ args);

private:
    alljoyn_busobject m_busObject;
    alljoyn_sessionid m_sessionId;

    virtual event Windows::Foundation::EventHandler<Platform::Object^>^ _MetadataChangedReceived;

    alljoyn_interfacedescription_member m_memberMetadataChanged;
};

} } } } 
