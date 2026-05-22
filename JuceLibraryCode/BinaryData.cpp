/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#include <cstring>

namespace BinaryData
{

//================== LICENSE.txt ==================
static const unsigned char temp_binary_data_0[] =
"The MIT License (MIT)\r\n"
"\r\n"
"Copyright (c) <year> Adam Veldhousen\r\n"
"\r\n"
"Permission is hereby granted, free of charge, to any person obtaining a copy\r\n"
"of this software and associated documentation files (the \"Software\"), to deal\r\n"
"in the Software without restriction, including without limitation the rights\r\n"
"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\r\n"
"copies of the Software, and to permit persons to whom the Software is\r\n"
"furnished to do so, subject to the following conditions:\r\n"
"\r\n"
"The above copyright notice and this permission notice shall be included in\r\n"
"all copies or substantial portions of the Software.\r\n"
"\r\n"
"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\r\n"
"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\r\n"
"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\r\n"
"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\r\n"
"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\r\n"
"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN\r\n"
"THE SOFTWARE.";

const char* LICENSE_txt = (const char*) temp_binary_data_0;

//================== README.md ==================
static const unsigned char temp_binary_data_1[] =
"# GuitarTuner\r\n"
"\r\n"
"A lightweight, offline guitar tuner for Windows built with C++ and JUCE.\r\n"
"\r\n"
"## Features\r\n"
"\r\n"
"- Real-time pitch detection using the YIN algorithm\r\n"
"- Analog-style arc needle with smooth 60Hz refresh\r\n"
"- Detects all six standard guitar strings (E2 A2 D3 G3 B3 E4)\r\n"
"- Visual feedback \xe2\x80\x94 needle turns green when in tune (\xc2\xb1""5 cents)\r\n"
"- Six string buttons mirroring guitar orientation (low to high, left to right)\r\n"
"- Custom tuning presets \xe2\x80\x94 create, edit, and delete your own tunings\r\n"
"- Presets persist to disk between sessions\r\n"
"- Standard tuning protected from deletion\r\n"
"- Silence gating \xe2\x80\x94 needle holds on note decay, clears on silence\r\n"
"- No internet connection required, no telemetry, no ads\r\n"
"\r\n"
"## License\r\n"
"\r\n"
"MIT \xe2\x80\x94 free to use, modify, and distribute with credit.\r\n";

const char* README_md = (const char*) temp_binary_data_1;


const char* getNamedResource (const char* resourceNameUTF8, int& numBytes);
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;

    if (resourceNameUTF8 != nullptr)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0x5a320952:  numBytes = 1103; return LICENSE_txt;
        case 0x64791dc8:  numBytes = 780; return README_md;
        default: break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] =
{
    "LICENSE_txt",
    "README_md"
};

const char* originalFilenames[] =
{
    "LICENSE.txt",
    "README.md"
};

const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)
{
    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)
        if (strcmp (namedResourceList[i], resourceNameUTF8) == 0)
            return originalFilenames[i];

    return nullptr;
}

}
