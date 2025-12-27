#pragma once

#include "Core/Common.hpp"
#include "Core/Meta/TypeTraits.hpp"

MOE_BEGIN_NAMESPACE

struct FileReader {
    static void initReader(FileReader* reader) {
        MOE_ASSERT(s_instance == nullptr, "FileReader already initialized");
        s_instance = reader;
    }

    static void destroyReader() {
        delete s_instance;
        s_instance = nullptr;
    }

    static FileReader* s_instance;

    virtual ~FileReader() = default;

    virtual Optional<Vector<uint8_t>> readFile(
            StringView filename, size_t& outFileSize) = 0;
};

struct DefaultFileReader : public FileReader {
    Optional<Vector<uint8_t>> readFile(
            StringView filename, size_t& outFileSize) override;
};

struct DefaultDebugFilenamePrinter {
    void operator()(StringView filename) {
        //Logger::debug("Reading file: {}", filename);
    }
};

template<typename InnerReaderT, typename FilenamePrinterT = DefaultDebugFilenamePrinter>
struct DebugFileReader : public FileReader {
public:
    template<
            typename... Args,
            typename = Meta::EnableIfT<
                    (Meta::IsConstructibleV<InnerReaderT, Args...> &&
                     Meta::IsBaseOfV<FileReader, InnerReaderT>)>>
    DebugFileReader(Args&&... args) {
        m_innerReader = new InnerReaderT(std::forward<Args>(args)...);
        m_filenamePrinter = FilenamePrinterT{};
    }

    Optional<Vector<uint8_t>> readFile(
            StringView filename, size_t& outFileSize) override {
        m_filenamePrinter(filename);
        return m_innerReader->readFile(filename, outFileSize);
    }

    ~DebugFileReader() override {
        delete m_innerReader;
    }

private:
    FilenamePrinterT m_filenamePrinter;
    FileReader* m_innerReader{nullptr};
};

MOE_END_NAMESPACE