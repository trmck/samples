#pragma once

ref class TypeConversionHelpers
{
internal:
    // Get the array of primitive values stored in the given alljoyn_msgarg.  The values in the array will be returned as
    // a Windows::Foundation::Collections::IVector.
    template<class T>
    static _Check_return_ int32 GetPrimitiveArrayMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Windows::Foundation::Collections::IVector<T>^* value)
    {
        size_t elementCount = 0;
        T* arrayContents = nullptr;
        RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(argument, signature, &elementCount, &arrayContents));

        if (arrayContents == nullptr)
        {
            *value = ref new Platform::Collections::Vector<T>();
        }
        else
        {
            *value = ref new Platform::Collections::Vector<T>(ref new Platform::Array<T>(arrayContents, static_cast<unsigned int>(elementCount)));
        }

        return ER_OK;
    }

    // Get the array of primitive values stored in the given alljoyn_msgarg.  The values in the array will be returned as
    // a Windows::Foundation::Collections::IVectorView.
    template<class T>
    static _Check_return_ int32 GetPrimitiveArrayMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Windows::Foundation::Collections::IVectorView<T>^* value)
    {
        Windows::Foundation::Collections::IVector<T>^ result;
        int32 status = GetPrimitiveArrayMessageArg(argument, signature, &result);
        *value = result->GetView();
        return status;
    }

    // Check whether the value passed in is an AllJoyn type signature for an array of primitive values,
    // such as an array of int32s or an array of booleans.
    static _Check_return_ bool IsArrayOfPrimitives(_In_ PCSTR signature)
    {
        return (strlen(signature) == 2) && (signature[1] != 's') && (signature[1] != 'v');
    }

    // Find the next complete type in the given AllJoyn type signature and append it to typeSignature.
    //
    // Examples:
    //   If signature is "ii", it will append "i", because that string describes the integer type
    //   If signature is "a(is)si" it will append "a(is)", because that string fully describes an array of structures.
    static _Check_return_ int32 AppendNextCompleteType(_In_ PCSTR signature, _Out_ std::vector<char>* typeSignature);

    // Get the key and value types from the type AllJoyn type signature for a dictionary.
    static _Check_return_ int32 GetDictionaryTypeSignatures(_In_ PCSTR signature, _Out_ std::vector<char>* keySignature, _Out_ std::vector<char>* valueSignature);

    // Get the value of an alljoyn_msgarg whose value matches WinRT type T.
    //
    // The default implementation of this function passes the value directly to alljoyn_msgarg_get, which
    // will work for any primitive types (int32, byte, etc.)  More complex types that require additional
    // work to convert between the WinRT type and the AllJoyn type must have a specialization of this template
    // function to perform the conversion.
    template<class T>
    static _Check_return_ int32 GetAllJoynMessageArg(alljoyn_msgarg argument, PCSTR signature, _Out_ T* value)
    {
        return alljoyn_msgarg_get(argument, signature, value);
    }

    // Set the value of an alljoyn_msgarg to the value of WinRT type T.
    //
    // The default implementation of this function passes the value directly to alljoyn_msgarg_set, which
    // will work for any primitive types (int32, byte, etc.)  More complex types that require additional
    // work to convert between the WinRT type and the AllJoyn type must have a specialization of this template
    // function to perform the conversion.
    template<class T>
    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ T value)
    {
        return alljoyn_msgarg_set(argument, signature, value);
    }

    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Platform::String^* value)
    {
        PSTR allJoynValue;
        QStatus status = alljoyn_msgarg_get(argument, signature, &allJoynValue);
        *value = AllJoynHelpers::MultibyteToPlatformString(allJoynValue);
        return static_cast<int32>(status);
    }

    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ Platform::String^ value)
    {
        std::vector<char> inputArg = AllJoynHelpers::PlatformToMultibyteString(value);
        return alljoyn_msgarg_set_and_stabilize(argument, signature, inputArg.data());
    }

    template<class T>
    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Windows::Foundation::Collections::IVector<T>^* value)
    {
        *value = ref new Platform::Collections::Vector<T>();
        if (signature[0] != 'a')
        {
            return ER_BUS_BAD_SIGNATURE;
        }

        if (IsArrayOfPrimitives(signature))
        {
            return GetPrimitiveArrayMessageArg(argument, signature, value);
        }

        // Remove the 'a' to get the signature of an array element.
        PCSTR elementSignature = signature + 1;
        size_t elementCount = 0;
        alljoyn_msgarg arrayContents = nullptr;
        RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(argument, signature, &elementCount, &arrayContents));

        if (arrayContents != nullptr)
        {
            for (size_t i = 0; i < elementCount; i++)
            {
                T elementValue;
                RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(alljoyn_msgarg_array_element(arrayContents, i), elementSignature, &elementValue));
                (*value)->Append(elementValue);
            }
        }

        return ER_OK;
    }

    template<class T>
    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ Windows::Foundation::Collections::IVectorView<T>^ value)
    {
        if (signature[0] != 'a')
        {
            return ER_BUS_BAD_SIGNATURE;
        }

        // Remove the 'a' to get the signature of an array element.
        PCSTR elementSignature = signature + 1;
        alljoyn_msgarg arrayArgument = alljoyn_msgarg_array_create(value->Size);

        for (size_t i = 0; i < value->Size; i++)
        {
            RETURN_IF_QSTATUS_ERROR(SetAllJoynMessageArg(alljoyn_msgarg_array_element(arrayArgument, i), elementSignature, value->GetAt(static_cast<unsigned int>(i))));
        }

        QStatus status = alljoyn_msgarg_set_and_stabilize(argument, "a*", (size_t)value->Size, arrayArgument);
        alljoyn_msgarg_destroy(arrayArgument);
        return static_cast<int32>(status);
    }

    template<class T>
    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Windows::Foundation::Collections::IVectorView<T>^* value)
    {
        Windows::Foundation::Collections::IVector<T>^ result;
        int32 status = GetAllJoynMessageArg(argument, signature, &result);
        *value = result->GetView();
        return status;
    }

    template<class T>
    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ Windows::Foundation::Collections::IVector<T>^ value)
    {
        return SetAllJoynMessageArg(argument, signature, value->GetView());
    }

    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ Windows::Foundation::Collections::IVectorView<Platform::String^>^ value)
    {
        std::vector<std::vector<char>> strings;
        for (unsigned int i = 0; i < value->Size; i++)
        {
            strings.push_back(AllJoynHelpers::PlatformToMultibyteString(value->GetAt(i)));
        }

        // alljoyn_msgarg_set expects the strings to be in the form of an array of char*.
        std::vector<char*> allJoynArgument;
        for (unsigned int i = 0; i < value->Size; i++)
        {
            allJoynArgument.push_back(strings.at(i).data());
        }

        return alljoyn_msgarg_set_and_stabilize(argument, signature, value->Size, allJoynArgument.data());
    }

    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Windows::Foundation::Collections::IVector<Platform::String^>^* value)
    {
        *value = ref new Platform::Collections::Vector<Platform::String^>();
        size_t elementCount = 0;
        alljoyn_msgarg arrayContents = nullptr;
        RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(argument, signature, &elementCount, &arrayContents));

        if (arrayContents != nullptr)
        {
            for (size_t i = 0; i < elementCount; i++)
            {
                Platform::String^ elementValue;
                RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(alljoyn_msgarg_array_element(arrayContents, i), "s", &elementValue));
                (*value)->Append(elementValue);
            }
        }

        return S_OK;
    }

    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Windows::Foundation::Collections::IVectorView<Platform::String^>^* value)
    {
        Windows::Foundation::Collections::IVector<Platform::String^>^ result;
        int32 status = GetAllJoynMessageArg(argument, signature, &result);
        *value = result->GetView();
        return status;
    }

    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ Windows::Foundation::Collections::IVector<Platform::String^>^ value)
    {
        return SetAllJoynMessageArg(argument, signature, value->GetView());
    }

    template<class T, class U>
    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Windows::Foundation::Collections::IMap<T, U>^* value)
    {
        std::vector<char> keyType;
        std::vector<char> valueType;
        RETURN_IF_QSTATUS_ERROR(GetDictionaryTypeSignatures(signature, &keyType, &valueType));

        *value = ref new Platform::Collections::Map<T, U>();

        size_t elementCount = 0;
        alljoyn_msgarg dictionaryContents = nullptr;
        RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(argument, signature, &elementCount, &dictionaryContents));

        if (dictionaryContents != nullptr)
        {
            for (size_t i = 0; i < elementCount; i++)
            {
                alljoyn_msgarg keyArg, valueArg;
                RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(alljoyn_msgarg_array_element(dictionaryContents, i), "{**}", &keyArg, &valueArg));
                T dictionaryKey;
                RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(keyArg, keyType.data(), &dictionaryKey));
                U dictionaryValue;
                RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(valueArg, valueType.data(), &dictionaryValue));

                (*value)->Insert(dictionaryKey, dictionaryValue);
            }
        }

        return ER_OK;
    }

    template<class T>
    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Windows::Foundation::Collections::IMap<T, Platform::Object^>^* value)
    {
        std::vector<char> keyType;
        std::vector<char> valueType;
        RETURN_IF_QSTATUS_ERROR(GetDictionaryTypeSignatures(signature, &keyType, &valueType));

        *value = ref new Platform::Collections::Map<T, Platform::Object^>();

        size_t elementCount = 0;
        alljoyn_msgarg dictionaryContents = nullptr;
        RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(argument, signature, &elementCount, &dictionaryContents));

        if (dictionaryContents != nullptr)
        {
            for (size_t i = 0; i < elementCount; i++)
            {
                alljoyn_msgarg keyArg, valueArg;
                RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(alljoyn_msgarg_array_element(dictionaryContents, i), "{*v}", &keyArg, &valueArg));
                T dictionaryKey;
                RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(keyArg, keyType.data(), &dictionaryKey));
                Platform::Object^ dictionaryValue;
                RETURN_IF_QSTATUS_ERROR(GetValueFromVariant(valueArg, &dictionaryValue));

                (*value)->Insert(dictionaryKey, dictionaryValue);
            }
        }

        return ER_OK;
    }

    template<class T, class U>
    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, Windows::Foundation::Collections::IMapView<T, U>^ value)
    {
        std::vector<char> keyType;
        std::vector<char> valueType;
        RETURN_IF_QSTATUS_ERROR(GetDictionaryTypeSignatures(signature, &keyType, &valueType));

        alljoyn_msgarg dictionaryArg = alljoyn_msgarg_array_create(value->Size);

        int i = 0;
        for (auto dictionaryElement : value)
        {
            alljoyn_msgarg keyArg = alljoyn_msgarg_create();
            alljoyn_msgarg valueArg = alljoyn_msgarg_create();
            RETURN_IF_QSTATUS_ERROR(SetAllJoynMessageArg(keyArg, keyType.data(), dictionaryElement->Key));
            RETURN_IF_QSTATUS_ERROR(SetAllJoynMessageArg(valueArg, valueType.data(), dictionaryElement->Value));

            RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_set_and_stabilize(alljoyn_msgarg_array_element(dictionaryArg, i++), "{**}", keyArg, valueArg));
            alljoyn_msgarg_destroy(keyArg);
            alljoyn_msgarg_destroy(valueArg);
        }

        QStatus status = alljoyn_msgarg_set_and_stabilize(argument, signature, (size_t)value->Size, dictionaryArg);
        alljoyn_msgarg_destroy(dictionaryArg);
        return static_cast<int32>(status);
    }

    template<class T, class U>
    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Windows::Foundation::Collections::IMapView<T, U>^* value)
    {
        Windows::Foundation::Collections::IMap<T, U>^ result;
        int32 status = GetAllJoynMessageArg(argument, signature, &result);
        *value = result->GetView();
        return status;
    }

    template<class T, class U>
    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ Windows::Foundation::Collections::IMap<T, U>^ value)
    {
        return SetAllJoynMessageArg(argument, signature, value->GetView());
    }

    static _Check_return_ int32 SetVariantStructureArg(_In_ alljoyn_msgarg argument, _In_ PROJECT_NAMESPACE::AllJoynMessageArgStructure^ value)
    {
        alljoyn_msgarg variantArg = alljoyn_msgarg_create();
        alljoyn_msgarg fields = alljoyn_msgarg_array_create(value->Size);
        for (unsigned int i = 0; i < value->Size; i++)
        {
            RETURN_IF_QSTATUS_ERROR(SetAllJoynMessageArg(alljoyn_msgarg_array_element(fields, i), "v", value->GetAt(i)));
        }

        RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_setstruct(variantArg, fields, value->Size));
        alljoyn_msgarg_destroy(fields);

        QStatus status = alljoyn_msgarg_set_and_stabilize(argument, "v", variantArg);
        alljoyn_msgarg_destroy(variantArg);

        return static_cast<int32>(status);
    }

    static _Check_return_ int32 GetVariantStructureArg(_In_ alljoyn_msgarg argument, _Out_ Platform::Object^* value)
    {
        size_t memberCount = alljoyn_msgarg_getnummembers(argument);
        auto result = ref new PROJECT_NAMESPACE::AllJoynMessageArgStructure();

        for (size_t i = 0; i < memberCount; i++)
        {
            Platform::Object^ variantValue;
            RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(alljoyn_msgarg_getmember(argument, i), "v", &variantValue));
            result->Append(variantValue);
        }

        *value = result;
        return ER_OK;
    }

    template<class T>
    static _Check_return_ int32 SetVariantArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ T value)
    {
        alljoyn_msgarg variantArg = alljoyn_msgarg_create();
        RETURN_IF_QSTATUS_ERROR(SetAllJoynMessageArg(variantArg, signature, value));
        QStatus status = alljoyn_msgarg_set_and_stabilize(argument, "v", variantArg);
        alljoyn_msgarg_destroy(variantArg);
        return status;
    }

    template<class T>
    static _Check_return_ int32 GetVariantArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ Platform::Object^* value)
    {
        T innerValue;
        RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(argument, signature, &innerValue));
        *value = innerValue;
        return ER_OK;
    }

    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ Platform::Object^ value)
    {
        UNREFERENCED_PARAMETER(signature);

        auto byteValue = dynamic_cast<Platform::IBox<byte>^>(value);
        if (byteValue != nullptr)
        {
            return SetVariantArg(argument, "y", byteValue->Value);
        }
        auto boolValue = dynamic_cast<Platform::IBox<bool>^>(value);
        if (boolValue != nullptr)
        {
            return SetVariantArg(argument, "b", boolValue->Value);
        }
        auto int16Value = dynamic_cast<Platform::IBox<int16>^>(value);
        if (int16Value != nullptr)
        {
            return SetVariantArg(argument, "n", int16Value->Value);
        }
        auto uint16Value = dynamic_cast<Platform::IBox<uint16>^>(value);
        if (uint16Value != nullptr)
        {
            return SetVariantArg(argument, "q", uint16Value->Value);
        }
        auto int32Value = dynamic_cast<Platform::IBox<int32>^>(value);
        if (int32Value != nullptr)
        {
            return SetVariantArg(argument, "i", int32Value->Value);
        }
        auto uint32Value = dynamic_cast<Platform::IBox<uint32>^>(value);
        if (uint32Value != nullptr)
        {
            return SetVariantArg(argument, "u", uint32Value->Value);
        }
        auto int64Value = dynamic_cast<Platform::IBox<int64>^>(value);
        if (int64Value != nullptr)
        {
            return SetVariantArg(argument, "x", int64Value->Value);
        }
        auto uint64Value = dynamic_cast<Platform::IBox<uint64>^>(value);
        if (uint64Value != nullptr)
        {
            return SetVariantArg(argument, "t", uint64Value->Value);
        }
        auto doubleValue = dynamic_cast<Platform::IBox<double>^>(value);
        if (doubleValue != nullptr)
        {
            return SetVariantArg(argument, "d", doubleValue->Value);
        }
        auto stringValue = dynamic_cast<Platform::String^>(value);
        if (stringValue != nullptr)
        {
            return SetVariantArg(argument, "s", stringValue);
        }
        auto structValue = dynamic_cast<PROJECT_NAMESPACE::AllJoynMessageArgStructure^>(value);
        if (structValue != nullptr)
        {
            return SetVariantStructureArg(argument, structValue);
        }
        auto vectorValue = dynamic_cast<Windows::Foundation::Collections::IVector<Platform::Object^>^>(value);
        if (vectorValue != nullptr)
        {
            return SetVariantArg(argument, "av", vectorValue);
        }
        auto byteMapValue = dynamic_cast<Windows::Foundation::Collections::IMap<byte, Platform::Object^>^>(value);
        if (byteMapValue != nullptr)
        {
            return SetVariantArg(argument, "a{yv}", byteMapValue);
        }
        auto boolMapValue = dynamic_cast<Windows::Foundation::Collections::IMap<bool, Platform::Object^>^>(value);
        if (boolMapValue != nullptr)
        {
            return SetVariantArg(argument, "a{bv}", boolMapValue);
        }
        auto int16MapValue = dynamic_cast<Windows::Foundation::Collections::IMap<int16, Platform::Object^>^>(value);
        if (int16MapValue != nullptr)
        {
            return SetVariantArg(argument, "a{nv}", int16MapValue);
        }
        auto uint16MapValue = dynamic_cast<Windows::Foundation::Collections::IMap<uint16, Platform::Object^>^>(value);
        if (uint16MapValue != nullptr)
        {
            return SetVariantArg(argument, "a{qv}", int16MapValue);
        }
        auto int32MapValue = dynamic_cast<Windows::Foundation::Collections::IMap<int32, Platform::Object^>^>(value);
        if (int32MapValue != nullptr)
        {
            return SetVariantArg(argument, "a{iv}", int32MapValue);
        }
        auto uint32MapValue = dynamic_cast<Windows::Foundation::Collections::IMap<uint32, Platform::Object^>^>(value);
        if (uint32MapValue != nullptr)
        {
            return SetVariantArg(argument, "a{uv}", uint32MapValue);
        }
        auto int64MapValue = dynamic_cast<Windows::Foundation::Collections::IMap<int64, Platform::Object^>^>(value);
        if (int64MapValue != nullptr)
        {
            return SetVariantArg(argument, "a{xv}", int64MapValue);
        }
        auto uint64MapValue = dynamic_cast<Windows::Foundation::Collections::IMap<uint64, Platform::Object^>^>(value);
        if (uint64MapValue != nullptr)
        {
            return SetVariantArg(argument, "a{tv}", uint64MapValue);
        }
        auto doubleMapValue = dynamic_cast<Windows::Foundation::Collections::IMap<double, Platform::Object^>^>(value);
        if (doubleMapValue != nullptr)
        {
            return SetVariantArg(argument, "a{dv}", doubleMapValue);
        }
        auto stringMapValue = dynamic_cast<Windows::Foundation::Collections::IMap<Platform::String^, Platform::Object^>^>(value);
        if (stringMapValue != nullptr)
        {
            return SetVariantArg(argument, "a{sv}", stringMapValue);
        }

        return ER_BUS_BAD_VALUE_TYPE;
    }

    static _Check_return_ int32 GetMapFromVariant(_In_ alljoyn_msgarg argument, _In_ char mapSignature, _Out_ Platform::Object^* value)
    {
        switch (mapSignature)
        {
        case 'y':
        {
            return GetVariantArg<Windows::Foundation::Collections::IMap<byte, Platform::Object^>^>(argument, "a{yv}", value);
        }
        case 'b':
        {
            return GetVariantArg<Windows::Foundation::Collections::IMap<bool, Platform::Object^>^>(argument, "a{bv}", value);
        }
        case 'n':
        {
            return GetVariantArg<Windows::Foundation::Collections::IMap<int16, Platform::Object^>^>(argument, "a{nv}", value);
        }
        case 'q':
        {
            return GetVariantArg<Windows::Foundation::Collections::IMap<uint16, Platform::Object^>^>(argument, "a{qv}", value);
        }
        case 'i':
        {
            return GetVariantArg<Windows::Foundation::Collections::IMap<int32, Platform::Object^>^>(argument, "a{iv}", value);
        }
        case 'u':
        {
            return GetVariantArg<Windows::Foundation::Collections::IMap<uint32, Platform::Object^>^>(argument, "a{uv}", value);
        }
        case 'x':
        {
            return GetVariantArg<Windows::Foundation::Collections::IMap<int64, Platform::Object^>^>(argument, "a{xv}", value);
        }
        case 't':
        {
            return GetVariantArg<Windows::Foundation::Collections::IMap<uint64, Platform::Object^>^>(argument, "a{tv}", value);
        }
        case 'd':
        {
            return GetVariantArg<Windows::Foundation::Collections::IMap<double, Platform::Object^>^>(argument, "a{dv}", value);
        }
        case 's':
        {
            return GetVariantArg<Windows::Foundation::Collections::IMap<Platform::String^, Platform::Object^>^>(argument, "a{sv}", value);
        }
        default:
            return ER_BUS_BAD_SIGNATURE;
        }
    }

    static _Check_return_ int32 GetValueFromVariant(_In_ alljoyn_msgarg argument, _Out_ Platform::Object^* value)
    {
        char variantSignature[c_MaximumSignatureLength];
        alljoyn_msgarg_signature(argument, variantSignature, c_MaximumSignatureLength);

        switch (variantSignature[0])
        {
        case 'y':
            return GetVariantArg<byte>(argument, "y", value);
        case 'b':
            return GetVariantArg<bool>(argument, "b", value);
        case 'n':
            return GetVariantArg<int16>(argument, "n", value);
        case 'q':
            return GetVariantArg<uint16>(argument, "q", value);
        case 'i':
            return GetVariantArg<int32>(argument, "i", value);
        case 'u':
            return GetVariantArg<uint32>(argument, "u", value);
        case 'x':
            return GetVariantArg<int64>(argument, "x", value);
        case 't':
            return GetVariantArg<uint64>(argument, "t", value);
        case 'd':
            return GetVariantArg<double>(argument, "d", value);
        case 's':
            return GetVariantArg<Platform::String^>(argument, "s", value);
        case '(':
            return GetVariantStructureArg(argument, value);
        case 'a':
            if (strlen(variantSignature) < 2)
            {
                return ER_BUS_BAD_SIGNATURE;
            }
            if (variantSignature[1] == '{')
            {
                if (strlen(variantSignature) < 3)
                {
                    return ER_BUS_BAD_SIGNATURE;
                }

                return GetMapFromVariant(argument, variantSignature[2], value);
            }
            else
            {
                return GetVariantArg<Windows::Foundation::Collections::IVector<Platform::Object^>^>(argument, "av", value);
            }
        }

        return ER_BUS_BAD_SIGNATURE;
    }

    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Platform::Object^* value)
    {
        UNREFERENCED_PARAMETER(signature);

        alljoyn_msgarg variantArg;
        RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(argument, "v", &variantArg));
        return GetValueFromVariant(variantArg, value);
    }
};