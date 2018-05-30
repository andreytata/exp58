/****************************************************************************
** Hi guys,
**
**    It was very "smart" decision to remove QHttpHeader classes from Qt5
**    see http://doc.qt.io/qt-5/sourcebreaks.html#changes-to-qt-network
**         The QHttp, QHttpHeader, QHttpResponseHeader, and QHttpRequestHeader
**         classes are removed. Use QNetworkAccessManager instead.
**
**    Crazy enough. Steering will is removed from your car. Use car instead.
**    Thank you very much but I prefer my steering will back.
**
**    So it is very fast and dirty porting of QHttpHeader classes from Qt 4.8
**    Note that it is no QHttp class just QHttpHeader and its friends
**
****************************************************************************/

#ifndef HTTP_HEADERS_H
#define HTTP_HEADERS_H

#include <QList>
#include <QString>
#include <QScopedPointer>

class QHttpPrivate;
class QHttpHeaderPrivate;
class QHttpResponseHeaderPrivate;
class QHttpRequestHeaderPrivate;
class QHttpHeader
{
public:
    QHttpHeader();
    QHttpHeader(const QHttpHeader &header);
    QHttpHeader(const QString &str);
    virtual ~QHttpHeader();

    QHttpHeader &operator=(const QHttpHeader &h);

    void setValue(const QString &key, const QString &value);
    void setValues(const QList<QPair<QString, QString> > &values);
    void addValue(const QString &key, const QString &value);
    QList<QPair<QString, QString> > values() const;
    bool hasKey(const QString &key) const;
    QStringList keys() const;
    QString value(const QString &key) const;
    QStringList allValues(const QString &key) const;
    void removeValue(const QString &key);
    void removeAllValues(const QString &key);

    // ### Qt 5: change to qint64
    bool hasContentLength() const;
    uint contentLength() const;
    void setContentLength(int len);

    bool hasContentType() const;
    QString contentType() const;
    void setContentType(const QString &type);

    virtual QString toString() const;
    bool isValid() const;

    virtual int majorVersion() const = 0;
    virtual int minorVersion() const = 0;

protected:
    virtual bool parseLine(const QString &line, int number);
    bool parse(const QString &str);
    void setValid(bool);

    QHttpHeader(QHttpHeaderPrivate &dd, const QString &str = QString());
    QHttpHeader(QHttpHeaderPrivate &dd, const QHttpHeader &header);
    QScopedPointer<QHttpHeaderPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(QHttpHeader)
};

class QHttpResponseHeader : public QHttpHeader
{
public:
    QHttpResponseHeader();
    QHttpResponseHeader(const QHttpResponseHeader &header);
    QHttpResponseHeader(const QString &str);
    QHttpResponseHeader(int code, const QString &text = QString(), int majorVer = 1, int minorVer = 1);
    QHttpResponseHeader &operator=(const QHttpResponseHeader &header);

    void setStatusLine(int code, const QString &text = QString(), int majorVer = 1, int minorVer = 1);

    int statusCode() const;
    QString reasonPhrase() const;

    int majorVersion() const;
    int minorVersion() const;

    QString toString() const;

protected:
    bool parseLine(const QString &line, int number);

private:
    Q_DECLARE_PRIVATE(QHttpResponseHeader)
    friend class QHttpPrivate;
};

class QHttpRequestHeader : public QHttpHeader
{
public:
    QHttpRequestHeader();
    QHttpRequestHeader(const QString &method, const QString &path, int majorVer = 1, int minorVer = 1);
    QHttpRequestHeader(const QHttpRequestHeader &header);
    QHttpRequestHeader(const QString &str);
    QHttpRequestHeader &operator=(const QHttpRequestHeader &header);

    void setRequest(const QString &method, const QString &path, int majorVer = 1, int minorVer = 1);

    QString method() const;
    QString path() const;

    int majorVersion() const;
    int minorVersion() const;

    QString toString() const;

protected:
    bool parseLine(const QString &line, int number);

private:
    Q_DECLARE_PRIVATE(QHttpRequestHeader)
};

#endif
