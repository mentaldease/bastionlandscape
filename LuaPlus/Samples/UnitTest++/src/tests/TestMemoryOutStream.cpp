#include "../UnitTest++.h"
#include "../MemoryOutStream.h"
#include <cstring>

using namespace UnitTest;

namespace {

TEST (DefaultIsEmptyString)
{
    MemoryOutStream stream;
    CHECK (stream.GetText() != 0);
    CHECK_EQUAL ("", stream.GetText());
}

TEST (StreamingTextCopiesCharacters)
{
    MemoryOutStream stream;
    stream << "Lalala";
    CHECK_EQUAL ("Lalala", stream.GetText());
}

TEST (StreamingMultipleTimesConcatenatesResult)
{
    MemoryOutStream stream;
    stream << "Bork" << "Foo" << "Bar";
    CHECK_EQUAL ("BorkFooBar", stream.GetText());
}

TEST (StreamingIntWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << 123;
    CHECK_EQUAL ("123", stream.GetText());
}

TEST (StreamingFloatWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << 3.1415f;
    CHECK (std::strstr(stream.GetText(), "3.1415"));
}

TEST (StreamingPointerWritesCorrectCharacters)
{
    MemoryOutStream stream;
    int* p = (int*)0x1234;
    stream << p;
    CHECK (std::strstr(stream.GetText(), "1234"));
}

TEST (StreamingSizeTWritesCorrectCharacters)
{
    MemoryOutStream stream;
    size_t s = 53124;
    stream << s;
    CHECK_EQUAL ("53124", stream.GetText());
}

TEST (StreamInitialCapacityIsCorrect)
{
    MemoryOutStream stream(MemoryOutStream::GROW_CHUNK_SIZE);
    CHECK_EQUAL ((int)MemoryOutStream::GROW_CHUNK_SIZE, stream.GetCapacity());
}

TEST (StreamInitialCapacityIsMultipleOfGrowChunkSize)
{
    MemoryOutStream stream(MemoryOutStream::GROW_CHUNK_SIZE + 1);
    CHECK_EQUAL ((int)MemoryOutStream::GROW_CHUNK_SIZE * 2, stream.GetCapacity());
}


TEST (ExceedingCapacityGrowsBuffer)
{
    MemoryOutStream stream(MemoryOutStream::GROW_CHUNK_SIZE);
    stream << "012345678901234567890123456789";
    char const* const oldBuffer = stream.GetText();
    stream << "0123456789";
    CHECK (oldBuffer != stream.GetText());
}

TEST (ExceedingCapacityGrowsBufferByGrowChunk)
{
    MemoryOutStream stream(MemoryOutStream::GROW_CHUNK_SIZE);
    stream << "0123456789012345678901234567890123456789";
    CHECK_EQUAL (MemoryOutStream::GROW_CHUNK_SIZE * 2, stream.GetCapacity());
}

TEST (WritingStringLongerThanCapacityFitsInNewBuffer)
{
    MemoryOutStream stream(8);
    stream << "0123456789ABCDEF";
    CHECK_EQUAL ("0123456789ABCDEF", stream.GetText());
}

TEST (WritingIntLongerThanCapacityFitsInNewBuffer)
{
    MemoryOutStream stream(8);
    stream << "aaaa" << 123456;;
    CHECK_EQUAL ("aaaa123456", stream.GetText());
}

TEST (WritingFloatLongerThanCapacityFitsInNewBuffer)
{
    MemoryOutStream stream(8);
    stream << "aaaa" << 123456.0f;;
    CHECK_EQUAL ("aaaa123456.000000", stream.GetText());
}

TEST (WritingSizeTLongerThanCapacityFitsInNewBuffer)
{
    MemoryOutStream stream(8);
    stream << "aaaa" << size_t(32145);
    CHECK_EQUAL ("aaaa32145", stream.GetText());
}

}
