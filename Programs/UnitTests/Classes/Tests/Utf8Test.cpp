#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

// ALL you want to know about UTF8, UTF16, UTF32
// http://utf8everywhere.org/

using namespace DAVA;

#ifdef __DAVAENGINE_WINDOWS__

// As soon as microsoft will fully support C++11(string_literals, ) remove WIN32 part of this test

DAVA_TESTCLASS (Utf8Test)
{
    DAVA_TEST (TestUtf8ToUtf16Function)
    {
        String utf8String = "это текст на русском внутри в utf8 в исходном файле тоже в utf8";
        WideString result = UTF8Utils::EncodeToWideString(utf8String);

        TEST_VERIFY(result.size() < utf8String.size());

        // http://unicode-table.com
        Vector<int> unicodeCodes = { 0x044D, 0x0442, 0x043E, 0x0020, 0x0442, 0x0435 };
        for (size_t i = 0; i < unicodeCodes.size(); ++i)
        {
            TEST_VERIFY(result[i] == unicodeCodes[i]);
        }

        String empty;
        WideString emptyWide = UTF8Utils::EncodeToWideString(empty);
        TEST_VERIFY(0 == emptyWide.size());
    }

    DAVA_TEST (TestWideStringToUtf8String)
    {
        String binaryContentUTF16LE = FileSystem::Instance()->ReadFileContents("~res:/TestData/Utf8Test/utf16le.txt");
        TEST_VERIFY(binaryContentUTF16LE.size() > 0);
        if (binaryContentUTF16LE.empty())
        {
            TEST_VERIFY(false && "no input file");
        }
        else
        {
            const wchar_t* wideCStr = reinterpret_cast<const wchar_t*>(binaryContentUTF16LE.c_str());
            WideString wideString(wideCStr, binaryContentUTF16LE.size() / sizeof(wchar_t));

            String str = UTF8Utils::EncodeToUTF8(wideString);

            TEST_VERIFY(str.size() > wideString.size());

            // 'э'
            TEST_VERIFY(0xd1 == static_cast<uint8_t>(str[0]));
            TEST_VERIFY(0x8d == static_cast<uint8_t>(str[1]));
            // 'т'
            TEST_VERIFY(0xd1 == static_cast<uint8_t>(str[2]));
            TEST_VERIFY(0x82 == static_cast<uint8_t>(str[3]));
            // 'о'
            TEST_VERIFY(0xd0 == static_cast<uint8_t>(str[4]));
            TEST_VERIFY(0xbe == static_cast<uint8_t>(str[5]));

            WideString empty;
            str = UTF8Utils::EncodeToUTF8(empty);
            TEST_VERIFY(str == "");
        }
    }
}
;

#else

DAVA_TESTCLASS (Utf8Test)
{
    DAVA_TEST (TestUtf8ToUtf32Function)
    {
        String utf8String = u8"это текст на русском внутри в utf8 в исходном файле тоже в utf8";
        WideString result = UTF8Utils::EncodeToWideString(utf8String);

        TEST_VERIFY(result.size() < utf8String.size());

        // http://unicode-table.com
        Vector<int> unicodeCodes = { 0x044D, 0x0442, 0x043E, 0x0020, 0x0442, 0x0435 };
        for (int i = 0; i < unicodeCodes.size(); ++i)
        {
            TEST_VERIFY(result[i] == unicodeCodes[i]);
        }

        String empty;
        WideString emptyWide = UTF8Utils::EncodeToWideString(empty);
        TEST_VERIFY(0 == emptyWide.size());
    }

    DAVA_TEST (TestWideStringToUtf8String)
    {
        static_assert(sizeof(wchar_t) == 4, "is it unix?");
        static_assert(sizeof(wchar_t) == sizeof(uint32_t), "std c++ 14 use uint32_t for unicode");

        std::u32string str32bit = U"это текст на русском в utf32 в исходном файле в utf8";
        const wchar_t* wideCStr = reinterpret_cast<const wchar_t*>(str32bit.c_str());

        WideString wideString(wideCStr);

        String str = UTF8Utils::EncodeToUTF8(wideString);

        TEST_VERIFY(str.size() > wideString.size());

        // 'э'
        TEST_VERIFY(0xd1 == static_cast<uint8_t>(str[0]));
        TEST_VERIFY(0x8d == static_cast<uint8_t>(str[1]));
        // 'т'
        TEST_VERIFY(0xd1 == static_cast<uint8_t>(str[2]));
        TEST_VERIFY(0x82 == static_cast<uint8_t>(str[3]));
        // 'о'
        TEST_VERIFY(0xd0 == static_cast<uint8_t>(str[4]));
        TEST_VERIFY(0xbe == static_cast<uint8_t>(str[5]));

        String empty;
        WideString emptyWide = UTF8Utils::EncodeToWideString(empty);
        TEST_VERIFY(0 == emptyWide.size());
    }
}
;
#endif
