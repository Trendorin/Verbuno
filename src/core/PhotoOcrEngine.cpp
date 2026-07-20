#include "core/PhotoOcrEngine.h"

#include "core/LanguageCatalog.h"

#include <QFileInfo>
#include <QFutureWatcher>
#include <QHash>
#include <QImageReader>
#include <QPainter>
#include <QRegularExpression>
#include <QtConcurrent/QtConcurrentRun>

#include <tesseract/baseapi.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

namespace verbuno {

namespace {
constexpr qint64 kMaximumFileBytes = 32LL * 1024LL * 1024LL;
constexpr qint64 kMaximumSourcePixels = 64LL * 1000LL * 1000LL;
constexpr qint64 kMaximumProcessedPixels = 20LL * 1000LL * 1000LL;
constexpr int kMaximumSourceEdge = 12000;
constexpr int kMaximumProcessedEdge = 6000;
constexpr int kMinimumUsefulLongEdge = 1600;
constexpr qsizetype kMaximumOcrCharacters = 200000;

struct RecognitionPass {
    QString text;
    int confidence = 0;
    bool valid = false;
};

QStringList queryAvailableLanguages() {
    std::vector<std::string> reported;
    for (const char* probe : {"eng", "deu", "rus", "ukr"}) {
        tesseract::TessBaseAPI api;
        if (api.Init(nullptr, probe) == 0) {
            api.GetAvailableLanguagesAsVector(&reported);
            break;
        }
    }

    QStringList result;
    result.reserve(static_cast<qsizetype>(reported.size()));
    for (const std::string& value : reported) {
        const QString language = QString::fromStdString(value).trimmed();
        if (!language.isEmpty() && !result.contains(language)) {
            result.append(language);
        }
    }
    std::sort(result.begin(), result.end(), [](const QString& left, const QString& right) {
        return QString::localeAwareCompare(PhotoOcrEngine::languageDisplayName(left),
                                           PhotoOcrEngine::languageDisplayName(right)) < 0;
    });
    return result;
}

const QStringList& cachedAvailableLanguages() {
    static const QStringList languages = queryAvailableLanguages();
    return languages;
}

bool languageIsInstalled(const QStringList& installed, const QString& languageSpec) {
    const QStringList requested = languageSpec.split(QChar('+'), Qt::SkipEmptyParts);
    if (requested.isEmpty()) {
        return false;
    }
    return std::all_of(requested.cbegin(), requested.cend(), [&](const QString& language) {
        return installed.contains(language.trimmed(), Qt::CaseSensitive);
    });
}

QSize boundedProcessingSize(const QSize& source) {
    if (!source.isValid() || source.isEmpty()) {
        return {};
    }

    const qreal edgeScale =
        std::min<qreal>(1.0, static_cast<qreal>(kMaximumProcessedEdge) /
                                static_cast<qreal>(std::max(source.width(), source.height())));
    const qreal pixelScale =
        std::min<qreal>(1.0,
                        std::sqrt(static_cast<qreal>(kMaximumProcessedPixels) /
                                  static_cast<qreal>(static_cast<qint64>(source.width()) *
                                                     static_cast<qint64>(source.height()))));
    qreal scale = std::min(edgeScale, pixelScale);

    const int longEdge = std::max(source.width(), source.height());
    if (longEdge < kMinimumUsefulLongEdge) {
        const qreal upscale = std::min<qreal>(2.5,
                                              static_cast<qreal>(kMinimumUsefulLongEdge) /
                                                  static_cast<qreal>(longEdge));
        const qreal upscaledPixels = static_cast<qreal>(source.width()) *
                                     static_cast<qreal>(source.height()) * upscale * upscale;
        if (upscaledPixels <= static_cast<qreal>(kMaximumProcessedPixels)) {
            scale = upscale;
        }
    }

    if (qFuzzyCompare(scale, 1.0)) {
        return source;
    }
    return QSize(std::max(1, qRound(static_cast<qreal>(source.width()) * scale)),
                 std::max(1, qRound(static_cast<qreal>(source.height()) * scale)));
}

QImage opaqueRgbImage(const QImage& source) {
    if (!source.hasAlphaChannel()) {
        return source.convertToFormat(QImage::Format_RGB32);
    }
    QImage flattened(source.size(), QImage::Format_RGB32);
    flattened.fill(Qt::white);
    QPainter painter(&flattened);
    painter.drawImage(QPoint(0, 0), source);
    return flattened;
}

PhotoOcrResult loadImage(const QString& path) {
    PhotoOcrResult result;
    const QFileInfo file(path);
    result.sourceLabel = file.fileName();
    if (path.trimmed().isEmpty() || !file.exists() || !file.isFile() || !file.isReadable()) {
        result.error = PhotoOcrError::InvalidFile;
        return result;
    }
    if (file.size() <= 0 || file.size() > kMaximumFileBytes) {
        result.error = PhotoOcrError::FileTooLarge;
        return result;
    }

    QImageReader reader(path);
    reader.setAutoTransform(true);
    reader.setDecideFormatFromContent(true);
    const QSize reportedSize = reader.size();
    if (!reportedSize.isValid() || reportedSize.isEmpty()) {
        result.error = PhotoOcrError::UnsupportedImage;
        result.detail = reader.errorString();
        return result;
    }
    const qint64 sourcePixels = static_cast<qint64>(reportedSize.width()) *
                                static_cast<qint64>(reportedSize.height());
    if (reportedSize.width() > kMaximumSourceEdge ||
        reportedSize.height() > kMaximumSourceEdge || sourcePixels > kMaximumSourcePixels) {
        result.error = PhotoOcrError::ImageTooLarge;
        return result;
    }

    result.sourceSize = reportedSize;
    const QSize processingSize = boundedProcessingSize(reportedSize);
    if (processingSize != reportedSize) {
        reader.setScaledSize(processingSize);
    }
    result.image = reader.read();
    if (result.image.isNull()) {
        result.error = PhotoOcrError::UnsupportedImage;
        result.detail = reader.errorString();
        return result;
    }
    const qint64 decodedPixels = static_cast<qint64>(result.image.width()) *
                                 static_cast<qint64>(result.image.height());
    if (result.image.width() > kMaximumProcessedEdge ||
        result.image.height() > kMaximumProcessedEdge ||
        decodedPixels > kMaximumProcessedPixels) {
        result.image = {};
        result.error = PhotoOcrError::ImageTooLarge;
        return result;
    }
    result.image = opaqueRgbImage(result.image);
    result.processedSize = result.image.size();
    return result;
}

PhotoOcrResult prepareImage(const QImage& source, const QString& sourceLabel) {
    PhotoOcrResult result;
    result.sourceLabel = sourceLabel;
    if (source.isNull() || source.width() <= 0 || source.height() <= 0) {
        result.error = PhotoOcrError::UnsupportedImage;
        return result;
    }
    const qint64 sourcePixels = static_cast<qint64>(source.width()) *
                                static_cast<qint64>(source.height());
    if (source.width() > kMaximumSourceEdge || source.height() > kMaximumSourceEdge ||
        sourcePixels > kMaximumSourcePixels) {
        result.error = PhotoOcrError::ImageTooLarge;
        return result;
    }

    result.sourceSize = source.size();
    const QSize target = boundedProcessingSize(source.size());
    const QImage scaled = target == source.size()
                              ? source
                              : source.scaled(target, Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation);
    result.image = opaqueRgbImage(scaled);
    result.processedSize = result.image.size();
    return result;
}

QImage contrastEnhanced(const QImage& grayscale) {
    std::array<quint64, 256> histogram{};
    quint64 total = 0;
    for (int y = 0; y < grayscale.height(); ++y) {
        const uchar* row = grayscale.constScanLine(y);
        for (int x = 0; x < grayscale.width(); ++x) {
            ++histogram[row[x]];
            ++total;
        }
    }
    if (total == 0) {
        return grayscale;
    }

    const quint64 lowTarget = total / 100;
    const quint64 highTarget = total - lowTarget;
    quint64 accumulated = 0;
    int low = 0;
    int high = 255;
    for (int value = 0; value < 256; ++value) {
        accumulated += histogram[static_cast<std::size_t>(value)];
        if (accumulated >= lowTarget) {
            low = value;
            break;
        }
    }
    accumulated = 0;
    for (int value = 0; value < 256; ++value) {
        accumulated += histogram[static_cast<std::size_t>(value)];
        if (accumulated >= highTarget) {
            high = value;
            break;
        }
    }
    if (high - low < 32) {
        return grayscale;
    }

    QImage enhanced = grayscale.copy();
    const int range = high - low;
    for (int y = 0; y < enhanced.height(); ++y) {
        uchar* row = enhanced.scanLine(y);
        for (int x = 0; x < enhanced.width(); ++x) {
            const int stretched = (static_cast<int>(row[x]) - low) * 255 / range;
            row[x] = static_cast<uchar>(std::clamp(stretched, 0, 255));
        }
    }
    return enhanced;
}

QString normalizeText(QString text) {
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QChar('\r'), QChar('\n'));
    text.remove(QChar('\f'));
    text.replace(QRegularExpression(QStringLiteral("[\\t ]+\\n")), QStringLiteral("\n"));
    text.replace(QRegularExpression(QStringLiteral("\\n{4,}")), QStringLiteral("\n\n\n"));
    return text.trimmed();
}

tesseract::PageSegMode pageSegmentationMode(PhotoOcrLayout layout) {
    switch (layout) {
    case PhotoOcrLayout::SingleBlock:
        return tesseract::PSM_SINGLE_BLOCK;
    case PhotoOcrLayout::SparseText:
        return tesseract::PSM_SPARSE_TEXT;
    case PhotoOcrLayout::Automatic:
    default:
        return tesseract::PSM_AUTO;
    }
}

RecognitionPass recognizePass(tesseract::TessBaseAPI& api,
                              const QImage& image,
                              PhotoOcrLayout layout) {
    RecognitionPass pass;
    if (image.isNull() || image.format() != QImage::Format_Grayscale8) {
        return pass;
    }

    api.Clear();
    api.SetPageSegMode(pageSegmentationMode(layout));
    api.SetImage(image.constBits(), image.width(), image.height(), 1,
                 static_cast<int>(image.bytesPerLine()));
    api.SetSourceResolution(300);
    if (api.Recognize(nullptr) != 0) {
        return pass;
    }

    std::unique_ptr<char[]> utf8(api.GetUTF8Text());
    if (!utf8) {
        return pass;
    }
    pass.text = normalizeText(QString::fromUtf8(utf8.get()));
    pass.confidence = std::clamp(api.MeanTextConf(), 0, 100);
    pass.valid = true;
    return pass;
}

PhotoOcrResult performRecognition(PhotoOcrResult result,
                                  const QStringList& installed,
                                  const QString& language,
                                  PhotoOcrLayout layout) {
    result.language = language;
    if (!result.succeeded()) {
        return result;
    }
    if (installed.isEmpty()) {
        result.error = PhotoOcrError::NoLanguageData;
        return result;
    }
    if (!languageIsInstalled(installed, language)) {
        result.error = PhotoOcrError::LanguageUnavailable;
        return result;
    }

    tesseract::TessBaseAPI api;
    const QByteArray languageBytes = language.toUtf8();
    if (api.Init(nullptr, languageBytes.constData()) != 0) {
        result.error = PhotoOcrError::InitializationFailed;
        return result;
    }
    api.SetVariable("preserve_interword_spaces", "1");
    api.SetVariable("user_defined_dpi", "300");

    const QImage grayscale = result.image.convertToFormat(QImage::Format_Grayscale8);
    const RecognitionPass original = recognizePass(api, grayscale, layout);
    if (!original.valid) {
        result.error = PhotoOcrError::RecognitionFailed;
        return result;
    }

    RecognitionPass selected = original;
    if (original.confidence < 86 || original.text.size() < 24) {
        const QImage enhanced = contrastEnhanced(grayscale);
        const RecognitionPass second = recognizePass(api, enhanced, layout);
        const bool materiallyBetter = second.valid &&
                                      (second.confidence > original.confidence + 2 ||
                                       (original.text.size() < 8 &&
                                        second.text.size() > original.text.size()));
        if (materiallyBetter) {
            selected = second;
            result.enhancedPassUsed = true;
        }
    }

    if (selected.text.isEmpty()) {
        result.error = PhotoOcrError::NoText;
        return result;
    }
    result.outputTruncated = selected.text.size() > kMaximumOcrCharacters;
    result.text = selected.text.left(kMaximumOcrCharacters);
    result.confidence = selected.confidence;
    return result;
}

} // namespace

PhotoOcrEngine::PhotoOcrEngine(QObject* parent)
    : QObject(parent)
    , m_availableLanguages(cachedAvailableLanguages()) {
    qRegisterMetaType<PhotoOcrResult>();
}

bool PhotoOcrEngine::isBusy() const {
    return m_watcher != nullptr;
}

const QStringList& PhotoOcrEngine::availableLanguages() const {
    return m_availableLanguages;
}

void PhotoOcrEngine::recognizeFile(const QString& path,
                                   const QString& language,
                                   PhotoOcrLayout layout) {
    startRecognition(path, {}, QFileInfo(path).fileName(), language, layout);
}

void PhotoOcrEngine::recognizeImage(const QImage& image,
                                    const QString& sourceLabel,
                                    const QString& language,
                                    PhotoOcrLayout layout) {
    startRecognition({}, image, sourceLabel, language, layout);
}

void PhotoOcrEngine::startRecognition(const QString& path,
                                      const QImage& image,
                                      const QString& sourceLabel,
                                      const QString& language,
                                      PhotoOcrLayout layout) {
    if (m_watcher) {
        PhotoOcrResult busy;
        busy.error = PhotoOcrError::Busy;
        emit recognitionFinished(busy);
        return;
    }

    const QStringList installed = m_availableLanguages;
    auto* watcher = new QFutureWatcher<PhotoOcrResult>(this);
    m_watcher = watcher;
    connect(watcher, &QFutureWatcher<PhotoOcrResult>::finished, this, [this, watcher] {
        const PhotoOcrResult result = watcher->result();
        m_watcher = nullptr;
        watcher->deleteLater();
        emit recognitionFinished(result);
    });
    emit recognitionStarted();
    watcher->setFuture(QtConcurrent::run([path, image, sourceLabel, language, layout, installed] {
        PhotoOcrResult prepared = path.isEmpty() ? prepareImage(image, sourceLabel) : loadImage(path);
        return performRecognition(std::move(prepared), installed, language, layout);
    }));
}

QString PhotoOcrEngine::ocrCodeForTranslationLanguage(const QString& code) {
    QString normalized = code.trimmed().toLower().replace(QChar('_'), QChar('-'));
    if (normalized == QStringLiteral("zh-hans") || normalized == QStringLiteral("zh")) {
        return QStringLiteral("chi_sim");
    }
    if (normalized == QStringLiteral("zh-hant")) {
        return QStringLiteral("chi_tra");
    }
    const qsizetype regionSeparator = normalized.indexOf(QChar('-'));
    if (regionSeparator > 0) {
        normalized.truncate(regionSeparator);
    }
    static const QHash<QString, QString> mapping = {
        {QStringLiteral("af"), QStringLiteral("afr")}, {QStringLiteral("am"), QStringLiteral("amh")},
        {QStringLiteral("ar"), QStringLiteral("ara")}, {QStringLiteral("as"), QStringLiteral("asm")},
        {QStringLiteral("az"), QStringLiteral("aze")}, {QStringLiteral("be"), QStringLiteral("bel")},
        {QStringLiteral("bn"), QStringLiteral("ben")}, {QStringLiteral("bo"), QStringLiteral("bod")},
        {QStringLiteral("bs"), QStringLiteral("bos")}, {QStringLiteral("br"), QStringLiteral("bre")},
        {QStringLiteral("bg"), QStringLiteral("bul")}, {QStringLiteral("ca"), QStringLiteral("cat")},
        {QStringLiteral("cs"), QStringLiteral("ces")}, {QStringLiteral("cy"), QStringLiteral("cym")},
        {QStringLiteral("da"), QStringLiteral("dan")}, {QStringLiteral("de"), QStringLiteral("deu")},
        {QStringLiteral("dv"), QStringLiteral("div")}, {QStringLiteral("dz"), QStringLiteral("dzo")},
        {QStringLiteral("el"), QStringLiteral("ell")}, {QStringLiteral("en"), QStringLiteral("eng")},
        {QStringLiteral("eo"), QStringLiteral("epo")}, {QStringLiteral("es"), QStringLiteral("spa")},
        {QStringLiteral("et"), QStringLiteral("est")}, {QStringLiteral("eu"), QStringLiteral("eus")},
        {QStringLiteral("fa"), QStringLiteral("fas")}, {QStringLiteral("fi"), QStringLiteral("fin")},
        {QStringLiteral("fo"), QStringLiteral("fao")}, {QStringLiteral("fr"), QStringLiteral("fra")},
        {QStringLiteral("ga"), QStringLiteral("gle")}, {QStringLiteral("gd"), QStringLiteral("gla")},
        {QStringLiteral("gl"), QStringLiteral("glg")}, {QStringLiteral("gu"), QStringLiteral("guj")},
        {QStringLiteral("he"), QStringLiteral("heb")}, {QStringLiteral("hi"), QStringLiteral("hin")},
        {QStringLiteral("hr"), QStringLiteral("hrv")}, {QStringLiteral("ht"), QStringLiteral("hat")},
        {QStringLiteral("hu"), QStringLiteral("hun")}, {QStringLiteral("hy"), QStringLiteral("hye")},
        {QStringLiteral("id"), QStringLiteral("ind")}, {QStringLiteral("is"), QStringLiteral("isl")},
        {QStringLiteral("it"), QStringLiteral("ita")}, {QStringLiteral("ja"), QStringLiteral("jpn")},
        {QStringLiteral("jv"), QStringLiteral("jav")}, {QStringLiteral("ka"), QStringLiteral("kat")},
        {QStringLiteral("kk"), QStringLiteral("kaz")}, {QStringLiteral("km"), QStringLiteral("khm")},
        {QStringLiteral("kn"), QStringLiteral("kan")}, {QStringLiteral("ko"), QStringLiteral("kor")},
        {QStringLiteral("ky"), QStringLiteral("kir")}, {QStringLiteral("la"), QStringLiteral("lat")},
        {QStringLiteral("lb"), QStringLiteral("ltz")}, {QStringLiteral("lo"), QStringLiteral("lao")},
        {QStringLiteral("lt"), QStringLiteral("lit")}, {QStringLiteral("lv"), QStringLiteral("lav")},
        {QStringLiteral("mi"), QStringLiteral("mri")}, {QStringLiteral("mk"), QStringLiteral("mkd")},
        {QStringLiteral("ml"), QStringLiteral("mal")}, {QStringLiteral("mn"), QStringLiteral("mon")},
        {QStringLiteral("mr"), QStringLiteral("mar")}, {QStringLiteral("ms"), QStringLiteral("msa")},
        {QStringLiteral("mt"), QStringLiteral("mlt")}, {QStringLiteral("my"), QStringLiteral("mya")},
        {QStringLiteral("ne"), QStringLiteral("nep")}, {QStringLiteral("nl"), QStringLiteral("nld")},
        {QStringLiteral("no"), QStringLiteral("nor")}, {QStringLiteral("oc"), QStringLiteral("oci")},
        {QStringLiteral("or"), QStringLiteral("ori")}, {QStringLiteral("pa"), QStringLiteral("pan")},
        {QStringLiteral("pl"), QStringLiteral("pol")}, {QStringLiteral("ps"), QStringLiteral("pus")},
        {QStringLiteral("pt"), QStringLiteral("por")}, {QStringLiteral("qu"), QStringLiteral("que")},
        {QStringLiteral("ro"), QStringLiteral("ron")}, {QStringLiteral("ru"), QStringLiteral("rus")},
        {QStringLiteral("sa"), QStringLiteral("san")}, {QStringLiteral("sd"), QStringLiteral("snd")},
        {QStringLiteral("si"), QStringLiteral("sin")}, {QStringLiteral("sk"), QStringLiteral("slk")},
        {QStringLiteral("sl"), QStringLiteral("slv")}, {QStringLiteral("sq"), QStringLiteral("sqi")},
        {QStringLiteral("sr"), QStringLiteral("srp")}, {QStringLiteral("sv"), QStringLiteral("swe")},
        {QStringLiteral("sw"), QStringLiteral("swa")}, {QStringLiteral("ta"), QStringLiteral("tam")},
        {QStringLiteral("te"), QStringLiteral("tel")}, {QStringLiteral("tg"), QStringLiteral("tgk")},
        {QStringLiteral("th"), QStringLiteral("tha")}, {QStringLiteral("ti"), QStringLiteral("tir")},
        {QStringLiteral("tl"), QStringLiteral("tgl")}, {QStringLiteral("tr"), QStringLiteral("tur")},
        {QStringLiteral("tt"), QStringLiteral("tat")}, {QStringLiteral("ug"), QStringLiteral("uig")},
        {QStringLiteral("uk"), QStringLiteral("ukr")}, {QStringLiteral("ur"), QStringLiteral("urd")},
        {QStringLiteral("uz"), QStringLiteral("uzb")}, {QStringLiteral("vi"), QStringLiteral("vie")},
        {QStringLiteral("yi"), QStringLiteral("yid")}, {QStringLiteral("yo"), QStringLiteral("yor")},
    };
    const auto found = mapping.constFind(normalized);
    if (found != mapping.cend()) {
        return found.value();
    }
    return normalized.size() == 3 ? normalized : QString{};
}

QString PhotoOcrEngine::preferredLanguage(const QStringList& installed,
                                          const QString& sourceLanguage,
                                          const QString& interfaceLanguage) {
    const QString source = ocrCodeForTranslationLanguage(sourceLanguage);
    if (!source.isEmpty() && installed.contains(source)) {
        return source;
    }
    const QString interface = ocrCodeForTranslationLanguage(interfaceLanguage);
    if (!interface.isEmpty() && installed.contains(interface)) {
        return interface;
    }
    if (installed.contains(QStringLiteral("eng"))) {
        return QStringLiteral("eng");
    }
    const auto fallback = std::find_if(installed.cbegin(), installed.cend(), [](const QString& code) {
        return code != QStringLiteral("osd") && code != QStringLiteral("equ");
    });
    return fallback == installed.cend() ? QString{} : *fallback;
}

QString PhotoOcrEngine::languageDisplayName(const QString& ocrCode) {
    if (ocrCode.contains(QChar('+'))) {
        QStringList names;
        for (const QString& part : ocrCode.split(QChar('+'), Qt::SkipEmptyParts)) {
            names.append(languageDisplayName(part));
        }
        return names.join(QStringLiteral(" + "));
    }
    static const QHash<QString, QString> special = {
        {QStringLiteral("osd"), QStringLiteral("Orientation and script detection")},
        {QStringLiteral("equ"), QStringLiteral("Mathematical equations")},
        {QStringLiteral("chi_sim"), QStringLiteral("Chinese, Simplified")},
        {QStringLiteral("chi_tra"), QStringLiteral("Chinese, Traditional")},
        {QStringLiteral("srp_latn"), QStringLiteral("Serbian, Latin")},
    };
    if (const auto named = special.constFind(ocrCode); named != special.cend()) {
        return named.value();
    }
    for (const Language& language : LanguageCatalog::all()) {
        if (ocrCodeForTranslationLanguage(language.code) == ocrCode) {
            return language.name;
        }
    }
    return ocrCode;
}

QString PhotoOcrEngine::errorMessage(PhotoOcrError error, const QString& detail) {
    switch (error) {
    case PhotoOcrError::None:
        return {};
    case PhotoOcrError::Busy:
        return tr("Photo recognition is already running.");
    case PhotoOcrError::InvalidFile:
        return tr("The selected image cannot be read.");
    case PhotoOcrError::FileTooLarge:
        return tr("The image file is empty or larger than 32 MiB.");
    case PhotoOcrError::UnsupportedImage:
        return detail.isEmpty()
                   ? tr("The selected file is not a supported image.")
                   : tr("The selected image could not be decoded: %1").arg(detail);
    case PhotoOcrError::ImageTooLarge:
        return tr("The image dimensions are too large to process safely.");
    case PhotoOcrError::NoLanguageData:
        return tr("No Tesseract OCR language data is installed.");
    case PhotoOcrError::LanguageUnavailable:
        return tr("The selected OCR language is not installed.");
    case PhotoOcrError::InitializationFailed:
        return tr("Tesseract could not initialize the selected language.");
    case PhotoOcrError::RecognitionFailed:
        return tr("Text recognition failed for this image.");
    case PhotoOcrError::NoText:
        return tr("No readable text was found. Try another OCR language or layout.");
    }
    return tr("Text recognition failed for this image.");
}

} // namespace verbuno
