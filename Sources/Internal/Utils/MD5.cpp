#include "Utils/MD5.h"
#include "FileSystem/File.h"
#include "FileSystem/FileList.h"
#include "FileSystem/FileSystem.h"
#include "Logger/Logger.h"

/*
	This file is based on original MD5 implementation from RSA Data Security.
*/
/*
 **********************************************************************
 ** md5.c                                                            **
 ** RSA Data Security, Inc. MD5 Message Digest Algorithm             **
 ** Created: 2/17/90 RLR                                             **
 ** Revised: 1/91 SRD,AJ,BSK,JT Reference C Version                  **
 **********************************************************************
 */

/*
 **********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved. **
 **                                                                  **
 ** License to copy and use this software is granted provided that   **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message     **
 ** Digest Algorithm" in all material mentioning or referencing this **
 ** software or this function.                                       **
 **                                                                  **
 ** License is also granted to make and use derivative works         **
 ** provided that such works are identified as "derived from the RSA **
 ** Data Security, Inc. MD5 Message Digest Algorithm" in all         **
 ** material mentioning or referencing the derived work.             **
 **                                                                  **
 ** RSA Data Security, Inc. makes no representations concerning      **
 ** either the merchantability of this software or the suitability   **
 ** of this software for any particular purpose.  It is provided "as **
 ** is" without express or implied warranty of any kind.             **
 **                                                                  **
 ** These notices must be retained in any copies of any part of this **
 ** documentation and/or software.                                   **
 **********************************************************************
 */

namespace DAVA
{
void MD5::ForData(const uint8* data, uint32 dataSize, MD5Digest& digest)
{
    MD5 md5;
    md5.Init();
    md5.Update(data, dataSize);
    md5.Final();

    digest = md5.GetDigest();
}

void MD5::ForFile(const FilePath& pathName, MD5Digest& digest)
{
    MD5 md5;
    md5.Init();

    ScopedPtr<File> file(File::Create(pathName, File::OPEN | File::READ));
    if (file)
    {
        Array<uint8, 1024> readBuffer;
        uint32 readBytes = 0;
        const uint32 bufSize = static_cast<uint32>(readBuffer.size());

        while ((readBytes = file->Read(readBuffer.data(), bufSize)) != 0)
        {
            md5.Update(readBuffer.data(), readBytes);
        }
    }

    md5.Final();
    digest = md5.GetDigest();
}

void MD5::ForDirectory(const FilePath& pathName, MD5Digest& digest, bool isRecursive, bool includeHidden)
{
    MD5 md5;
    md5.Init();
    MD5::CalculateDirectoryMD5(pathName, md5, isRecursive, includeHidden);
    md5.Final();

    digest = md5.GetDigest();
}

void MD5::CalculateDirectoryMD5(const FilePath& pathName, MD5& md5, bool isRecursive, bool includeHidden)
{
    ScopedPtr<FileList> fileList(new FileList(pathName, includeHidden));
    fileList->Sort();

    for (uint32 i = 0; i < fileList->GetCount(); ++i)
    {
        if (fileList->IsHidden(i) && !includeHidden)
        {
            continue;
        }

        if (fileList->IsDirectory(i))
        {
            if (!fileList->IsNavigationDirectory(i))
            {
                if (isRecursive)
                {
                    String name = fileList->GetPathname(i).GetLastDirectoryName();
                    md5.Update(reinterpret_cast<const uint8*>(name.c_str()), static_cast<uint32>(name.size()));

                    CalculateDirectoryMD5(fileList->GetPathname(i), md5, isRecursive, includeHidden);
                }
            }
        }
        else
        {
            // update MD5 according to the file
            String name = fileList->GetPathname(i).GetFilename();
            md5.Update(reinterpret_cast<const uint8*>(name.c_str()), static_cast<uint32>(name.size()));

            MD5Digest fileDigest;
            MD5::ForFile(fileList->GetPathname(i), fileDigest);
            md5.Update(fileDigest.digest.data(), static_cast<uint32>(fileDigest.digest.size()));
        }
    }
}

void MD5::HashToChar(const MD5Digest& digest, char8* buffer, uint32 bufferSize)
{
    HashToChar(digest.digest.data(), static_cast<uint32>(digest.digest.size()), buffer, bufferSize);
}

void MD5::HashToChar(const uint8* hash, uint32 hashSize, char8* buffer, uint32 bufferSize)
{
    DVASSERT((hashSize * 2 + 1) == bufferSize && "To small buffer. Must be enought to put all characters of hash and \0");

    for (uint32 i = 0; i < hashSize; ++i)
    {
        buffer[2 * i] = GetCharacterFromNumber(hash[i] & 0x0F);
        buffer[2 * i + 1] = GetCharacterFromNumber((hash[i] & 0xF0) >> 4);
    }

    buffer[bufferSize - 1] = 0;
}

void MD5::CharToHash(const char8* buffer, MD5Digest& digest)
{
    const int32 bufferSize = Min(static_cast<int32>(strlen(buffer)), MD5Digest::DIGEST_SIZE * 2);
    CharToHash(buffer, bufferSize, digest.digest.data(), static_cast<uint32>(digest.digest.size()));
}

void MD5::CharToHash(const char8* buffer, uint32 bufferSize, uint8* hash, uint32 hashSize)
{
    if (bufferSize != hashSize * 2)
    {
        Logger::Error("[MD5::CharToHash] char string has wrong size (%d). Must be %d characters", bufferSize, hashSize * 2);
        return;
    }

    for (uint32 i = 0; i < hashSize; ++i)
    {
        uint8 low = GetNumberFromCharacter(buffer[2 * i]);
        uint8 high = GetNumberFromCharacter(buffer[2 * i + 1]);

        hash[i] = (high << 4) | (low);
    }
}

uint8 MD5::GetNumberFromCharacter(char8 character)
{
    if ('0' <= character && character <= '9')
    {
        return (character - '0');
    }
    else if ('a' <= character && character <= 'f')
    {
        return (character - 'a' + 10);
    }
    else if ('A' <= character && character <= 'F')
    {
        return (character - 'A' + 10);
    }

    Logger::Error("[MD5::GetNumberFromCharacter] hash has wrong symbol (%c).", character);
    return 0;
}

char8 MD5::GetCharacterFromNumber(uint8 number)
{
    if (0 <= number && number <= 9)
    {
        return (number + '0');
    }

    return (number + 'A' - 10);
}

static void Transform(uint32* buf, uint32* in);

static unsigned char PADDING[64] = {
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* F, G and H are basic MD5 functions: selection, majority, parity */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F((b), (c), (d)) + (x) + static_cast<uint32>(ac); \
   (a) = ROTATE_LEFT((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G((b), (c), (d)) + (x) + static_cast<uint32>(ac); \
   (a) = ROTATE_LEFT((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H((b), (c), (d)) + (x) + static_cast<uint32>(ac); \
   (a) = ROTATE_LEFT((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I((b), (c), (d)) + (x) + static_cast<uint32>(ac); \
   (a) = ROTATE_LEFT((a), (s)); \
   (a) += (b); \
  }

void MD5::Init()
{
    this->i[0] = this->i[1] = 0;

    /* Load magic initialization constants.
   */
    this->buf[0] = 0x67452301;
    this->buf[1] = 0xefcdab89;
    this->buf[2] = 0x98badcfe;
    this->buf[3] = 0x10325476;
}

void MD5::Update(const uint8* inBuf, uint32 inLen)
{
    uint32 in[16];
    int mdi;
    unsigned int i, ii;

    /* compute number of bytes mod 64 */
    mdi = static_cast<int>((this->i[0] >> 3) & 0x3F);

    /* update number of bits */
    if ((this->i[0] + (inLen << 3)) < this->i[0])
        this->i[1]++;
    this->i[0] += (inLen << 3);
    this->i[1] += (inLen >> 29);

    while (inLen--)
    {
        /* add new character to buffer, increment mdi */
        this->in[mdi++] = *inBuf++;

        /* transform if necessary */
        if (mdi == 0x40)
        {
            for (i = 0, ii = 0; i < 16; i++, ii += 4)
                in[i] = ((static_cast<uint32>(this->in[ii + 3])) << 24) |
                ((static_cast<uint32>(this->in[ii + 2])) << 16) |
                ((static_cast<uint32>(this->in[ii + 1])) << 8) |
                (static_cast<uint32>(this->in[ii]));
            Transform(this->buf, in);
            mdi = 0;
        }
    }
}

void MD5::Final()
{
    uint32 in[16];
    int mdi;
    unsigned int i, ii;
    unsigned int padLen;

    /* save number of bits */
    in[14] = this->i[0];
    in[15] = this->i[1];

    /* compute number of bytes mod 64 */
    mdi = static_cast<int>((this->i[0] >> 3) & 0x3F);

    /* pad out to 56 mod 64 */
    padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
    Update(PADDING, padLen);

    /* append length in bits and transform */
    for (i = 0, ii = 0; i < 14; i++, ii += 4)
        in[i] = ((static_cast<uint32>(this->in[ii + 3])) << 24) |
        ((static_cast<uint32>(this->in[ii + 2]) << 16)) |
        ((static_cast<uint32>(this->in[ii + 1]) << 8)) |
        (static_cast<uint32>(this->in[ii]));
    Transform(this->buf, in);

    /* store buffer in digest */
    for (i = 0, ii = 0; i < 4; i++, ii += 4)
    {
        this->digest.digest[ii] = static_cast<unsigned char>(this->buf[i] & 0xFF);
        this->digest.digest[ii + 1] =
        static_cast<unsigned char>((this->buf[i] >> 8) & 0xFF);
        this->digest.digest[ii + 2] =
        static_cast<unsigned char>((this->buf[i] >> 16) & 0xFF);
        this->digest.digest[ii + 3] =
        static_cast<unsigned char>((this->buf[i] >> 24) & 0xFF);
    }
}

/*
	Basic MD5 step. Transform buf based on in.
 */
static void Transform(uint32* buf, uint32* in)
{
    uint32 a = buf[0], b = buf[1], c = buf[2], d = buf[3];

/* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
    FF(a, b, c, d, in[0], S11, 3614090360U); /* 1 */
    FF(d, a, b, c, in[1], S12, 3905402710U); /* 2 */
    FF(c, d, a, b, in[2], S13, 606105819U); /* 3 */
    FF(b, c, d, a, in[3], S14, 3250441966U); /* 4 */
    FF(a, b, c, d, in[4], S11, 4118548399U); /* 5 */
    FF(d, a, b, c, in[5], S12, 1200080426U); /* 6 */
    FF(c, d, a, b, in[6], S13, 2821735955U); /* 7 */
    FF(b, c, d, a, in[7], S14, 4249261313U); /* 8 */
    FF(a, b, c, d, in[8], S11, 1770035416U); /* 9 */
    FF(d, a, b, c, in[9], S12, 2336552879U); /* 10 */
    FF(c, d, a, b, in[10], S13, 4294925233U); /* 11 */
    FF(b, c, d, a, in[11], S14, 2304563134U); /* 12 */
    FF(a, b, c, d, in[12], S11, 1804603682U); /* 13 */
    FF(d, a, b, c, in[13], S12, 4254626195U); /* 14 */
    FF(c, d, a, b, in[14], S13, 2792965006U); /* 15 */
    FF(b, c, d, a, in[15], S14, 1236535329U); /* 16 */

/* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
    GG(a, b, c, d, in[1], S21, 4129170786U); /* 17 */
    GG(d, a, b, c, in[6], S22, 3225465664U); /* 18 */
    GG(c, d, a, b, in[11], S23, 643717713U); /* 19 */
    GG(b, c, d, a, in[0], S24, 3921069994U); /* 20 */
    GG(a, b, c, d, in[5], S21, 3593408605U); /* 21 */
    GG(d, a, b, c, in[10], S22, 38016083U); /* 22 */
    GG(c, d, a, b, in[15], S23, 3634488961U); /* 23 */
    GG(b, c, d, a, in[4], S24, 3889429448U); /* 24 */
    GG(a, b, c, d, in[9], S21, 568446438U); /* 25 */
    GG(d, a, b, c, in[14], S22, 3275163606U); /* 26 */
    GG(c, d, a, b, in[3], S23, 4107603335U); /* 27 */
    GG(b, c, d, a, in[8], S24, 1163531501U); /* 28 */
    GG(a, b, c, d, in[13], S21, 2850285829U); /* 29 */
    GG(d, a, b, c, in[2], S22, 4243563512U); /* 30 */
    GG(c, d, a, b, in[7], S23, 1735328473U); /* 31 */
    GG(b, c, d, a, in[12], S24, 2368359562U); /* 32 */

/* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
    HH(a, b, c, d, in[5], S31, 4294588738U); /* 33 */
    HH(d, a, b, c, in[8], S32, 2272392833U); /* 34 */
    HH(c, d, a, b, in[11], S33, 1839030562U); /* 35 */
    HH(b, c, d, a, in[14], S34, 4259657740U); /* 36 */
    HH(a, b, c, d, in[1], S31, 2763975236U); /* 37 */
    HH(d, a, b, c, in[4], S32, 1272893353U); /* 38 */
    HH(c, d, a, b, in[7], S33, 4139469664U); /* 39 */
    HH(b, c, d, a, in[10], S34, 3200236656U); /* 40 */
    HH(a, b, c, d, in[13], S31, 681279174U); /* 41 */
    HH(d, a, b, c, in[0], S32, 3936430074U); /* 42 */
    HH(c, d, a, b, in[3], S33, 3572445317U); /* 43 */
    HH(b, c, d, a, in[6], S34, 76029189U); /* 44 */
    HH(a, b, c, d, in[9], S31, 3654602809U); /* 45 */
    HH(d, a, b, c, in[12], S32, 3873151461U); /* 46 */
    HH(c, d, a, b, in[15], S33, 530742520U); /* 47 */
    HH(b, c, d, a, in[2], S34, 3299628645U); /* 48 */

/* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
    II(a, b, c, d, in[0], S41, 4096336452U); /* 49 */
    II(d, a, b, c, in[7], S42, 1126891415U); /* 50 */
    II(c, d, a, b, in[14], S43, 2878612391U); /* 51 */
    II(b, c, d, a, in[5], S44, 4237533241U); /* 52 */
    II(a, b, c, d, in[12], S41, 1700485571U); /* 53 */
    II(d, a, b, c, in[3], S42, 2399980690U); /* 54 */
    II(c, d, a, b, in[10], S43, 4293915773U); /* 55 */
    II(b, c, d, a, in[1], S44, 2240044497U); /* 56 */
    II(a, b, c, d, in[8], S41, 1873313359U); /* 57 */
    II(d, a, b, c, in[15], S42, 4264355552U); /* 58 */
    II(c, d, a, b, in[6], S43, 2734768916U); /* 59 */
    II(b, c, d, a, in[13], S44, 1309151649U); /* 60 */
    II(a, b, c, d, in[4], S41, 4149444226U); /* 61 */
    II(d, a, b, c, in[11], S42, 3174756917U); /* 62 */
    II(c, d, a, b, in[2], S43, 718787259U); /* 63 */
    II(b, c, d, a, in[9], S44, 3951481745U); /* 64 */

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}
};
/*
 **********************************************************************
 ** End of md5.c                                                     **
 ******************************* (cut) ********************************
 */

// /*
//  **********************************************************************
//  ** md5driver.c -- sample routines to test                           **
//  ** RSA Data Security, Inc. MD5 message digest algorithm.            **
//  ** Created: 2/16/90 RLR                                             **
//  ** Updated: 1/91 SRD                                                **
//  **********************************************************************
//  */
//
// /*
//  **********************************************************************
//  ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved. **
//  **                                                                  **
//  ** RSA Data Security, Inc. makes no representations concerning      **
//  ** either the merchantability of this software or the suitability   **
//  ** of this software for any particular purpose.  It is provided "as **
//  ** is" without express or implied warranty of any kind.             **
//  **                                                                  **
//  ** These notices must be retained in any copies of any part of this **
//  ** documentation and/or software.                                   **
//  **********************************************************************
//  */
//
// #include <stdio.h>
// #include <sys/types.h>
// #include <time.h>
// #include <string.h>
// /* -- include the following file if the file md5.h is separate -- */
// /* #include "md5.h" */
//
// /* Prints message digest buffer in mdContext as 32 hexadecimal digits.
//    Order is from low-order byte to high-order byte of digest.
//    Each byte is printed with high-order hexadecimal digit first.
//  */
// static void MDPrint (mdContext)
// MD5_CTX *mdContext;
// {
//   int i;
//
//   for (i = 0; i < 16; i++)
//     printf ("%02x", mdContext->digest[i]);
// }
//
// /*
//
// /* size of test block */
// #define TEST_BLOCK_SIZE 1000
//
// /* number of blocks to process */
// #define TEST_BLOCKS 10000
//
// /* number of test bytes = TEST_BLOCK_SIZE * TEST_BLOCKS */
// static long TEST_BYTES = (long)TEST_BLOCK_SIZE * (long)TEST_BLOCKS;
//
// /* A time trial routine, to measure the speed of MD5.
//    Measures wall time required to digest TEST_BLOCKS * TEST_BLOCK_SIZE
//    characters.
//  */
// static void MDTimeTrial ()
// {
//   MD5_CTX mdContext;
//   time_t endTime, startTime;
//   unsigned char data[TEST_BLOCK_SIZE];
//   unsigned int i;
//
//   /* initialize test data */
//   for (i = 0; i < TEST_BLOCK_SIZE; i++)
//     data[i] = (unsigned char)(i & 0xFF);
//
//   /* start timer */
//   printf ("MD5 time trial. Processing %ld characters...\n", TEST_BYTES);
//   time (&startTime);
//
//   /* digest data in TEST_BLOCK_SIZE byte blocks */
//   MD5Init (&mdContext);
//   for (i = TEST_BLOCKS; i > 0; i--)
//     MD5Update (&mdContext, data, TEST_BLOCK_SIZE);
//   MD5Final (&mdContext);
//
//   /* stop timer, get time difference */
//   time (&endTime);
//   MDPrint (&mdContext);
//   printf (" is digest of test input.\n");
//   printf
//     ("Seconds to process test input: %ld\n", (long)(endTime-startTime));
//   printf
//     ("Characters processed per second: %ld\n",
//      TEST_BYTES/(endTime-startTime));
// }
//
// /* Computes the message digest for string inString.
//    Prints out message digest, a space, the string (in quotes) and a
//    carriage return.
//  */
// static void MDString (inString)
// char *inString;
// {
//   MD5_CTX mdContext;
//   unsigned int len = strlen (inString);
//
//   MD5Init (&mdContext);
//   MD5Update (&mdContext, inString, len);
//   MD5Final (&mdContext);
//   MDPrint (&mdContext);
//   printf (" \"%s\"\n\n", inString);
// }
//
// /* Computes the message digest for a specified file.
//    Prints out message digest, a space, the file name, and a carriage
//    return.
//  */
// static void MDFile (filename)
// char *filename;
// {
//   FILE *inFile = fopen (filename, "rb");
//   MD5_CTX mdContext;
//   int bytes;
//   unsigned char data[1024];
//
//   if (inFile == NULL) {
//     printf ("%s can't be opened.\n", filename);
//     return;
//   }
//
//   MD5Init (&mdContext);
//   while ((bytes = fread (data, 1, 1024, inFile)) != 0)
//     MD5Update (&mdContext, data, bytes);
//   MD5Final (&mdContext);
//   MDPrint (&mdContext);
//   printf (" %s\n", filename);
//   fclose (inFile);
// }
//
// /* Writes the message digest of the data from stdin onto stdout,
//    followed by a carriage return.
//  */
// static void MDFilter ()
// {
//   MD5_CTX mdContext;
//   int bytes;
//   unsigned char data[16];
//
//   MD5Init (&mdContext);
//   while ((bytes = fread (data, 1, 16, stdin)) != 0)
//     MD5Update (&mdContext, data, bytes);
//   MD5Final (&mdContext);
//   MDPrint (&mdContext);
//   printf ("\n");
// }
//
// /* Runs a standard suite of test data.
//  */
// static void MDTestSuite ()
// {
//   printf ("MD5 test suite results:\n\n");
//   MDString ("");
//   MDString ("a");
//   MDString ("abc");
//   MDString ("message digest");
//   MDString ("abcdefghijklmnopqrstuvwxyz");
//   MDString
//     ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
//   MDString
//     ("1234567890123456789012345678901234567890\
// 1234567890123456789012345678901234567890");
//   /* Contents of file foo are "abc" */
//   MDFile ("foo");
// }
//
// void main (argc, argv)
// int argc;
// char *argv[];
// {
//   int i;
//
//   /* For each command line argument in turn:
//   ** filename          -- prints message digest and name of file
//   ** -sstring          -- prints message digest and contents of string
//   ** -t                -- prints time trial statistics for 1M characters
//   ** -x                -- execute a standard suite of test data
//   ** (no args)         -- writes messages digest of stdin onto stdout
//   */
//   if (argc == 1)
//     MDFilter ();
//   else
//     for (i = 1; i < argc; i++)
//       if (argv[i][0] == '-' && argv[i][1] == 's')
//         MDString (argv[i] + 2);
//       else if (strcmp (argv[i], "-t") == 0)
//         MDTimeTrial ();
//       else if (strcmp (argv[i], "-x") == 0)
//         MDTestSuite ();
//       else MDFile (argv[i]);
// }
//
// /*
//  **********************************************************************
//  ** End of md5driver.c                                               **
//  ******************************* (cut) ********************************
//  */
