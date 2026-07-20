#pragma once

#include <QImage>
#include <QMetaType>
#include <QObject>
#include <QSize>
#include <QStringList>

template <typename T>
class QFutureWatcher;

namespace verbuno {

enum class PhotoOcrLayout {
    Automatic = 0,
    SingleBlock = 1,
    SparseText = 2,
};

enum class PhotoOcrError {
    None = 0,
    Busy,
    InvalidFile,
    FileTooLarge,
    UnsupportedImage,
    ImageTooLarge,
    NoLanguageData,
    LanguageUnavailable,
    InitializationFailed,
    RecognitionFailed,
    NoText,
};

struct PhotoOcrResult {
    PhotoOcrError error = PhotoOcrError::None;
    QString detail;
    QString text;
    QString sourceLabel;
    QString language;
    int confidence = 0;
    QSize sourceSize;
    QSize processedSize;
    QImage image;
    bool enhancedPassUsed = false;
    bool outputTruncated = false;

    [[nodiscard]] bool succeeded() const { return error == PhotoOcrError::None; }
};

class PhotoOcrEngine final : public QObject {
    Q_OBJECT

public:
    explicit PhotoOcrEngine(QObject* parent = nullptr);

    [[nodiscard]] bool isBusy() const;
    [[nodiscard]] const QStringList& availableLanguages() const;

    void recognizeFile(const QString& path,
                       const QString& language,
                       PhotoOcrLayout layout);
    void recognizeImage(const QImage& image,
                        const QString& sourceLabel,
                        const QString& language,
                        PhotoOcrLayout layout);

    [[nodiscard]] static QString ocrCodeForTranslationLanguage(const QString& code);
    [[nodiscard]] static QString preferredLanguage(const QStringList& installed,
                                                   const QString& sourceLanguage,
                                                   const QString& interfaceLanguage);
    [[nodiscard]] static QString languageDisplayName(const QString& ocrCode);
    [[nodiscard]] static QString errorMessage(PhotoOcrError error,
                                              const QString& detail = {});

signals:
    void recognitionStarted();
    void recognitionFinished(const verbuno::PhotoOcrResult& result);

private:
    void startRecognition(const QString& path,
                          const QImage& image,
                          const QString& sourceLabel,
                          const QString& language,
                          PhotoOcrLayout layout);

    QStringList m_availableLanguages;
    QFutureWatcher<PhotoOcrResult>* m_watcher = nullptr;
};

} // namespace verbuno

Q_DECLARE_METATYPE(verbuno::PhotoOcrResult)
