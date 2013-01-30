/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-*/
/* vim: set ts=2 sw=2 et tw=79: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef qgraphicsmozview_h
#define qgraphicsmozview_h

#include <QGraphicsWidget>
#include <QUrl>

class QMozContext;
class QSyncMessage;
class QGraphicsMozViewPrivate;

class QSyncMessageResponse : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariant message READ getMessage WRITE setMessage FINAL)

public:
    QSyncMessageResponse(QObject* parent = 0) : QObject(parent) {}
    QSyncMessageResponse(const QSyncMessageResponse& aMsg) { mMessage = aMsg.mMessage; }
    virtual ~QSyncMessageResponse() {}

    QVariant getMessage() const { return mMessage; }
    void setMessage(const QVariant& msg) { mMessage = msg; }

private:
    QVariant mMessage;
};

Q_DECLARE_METATYPE(QSyncMessageResponse)

class QGraphicsMozView : public QGraphicsWidget
{
    Q_OBJECT

    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navigationHistoryChanged FINAL)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY navigationHistoryChanged FINAL)
    Q_PROPERTY(int loadProgress READ loadProgress NOTIFY loadProgressChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged FINAL)

public:
    QGraphicsMozView(QGraphicsItem* parent = 0);

    virtual ~QGraphicsMozView();

    QUrl url() const;
    void setUrl(const QUrl&);
    QString title() const;
    int loadProgress() const;
    bool canGoBack() const;
    bool canGoForward() const;
    bool loading() const;

public Q_SLOTS:
    void loadHtml(const QString& html, const QUrl& baseUrl = QUrl());
    void goBack();
    void goForward();
    void stop();
    void reload();
    void load(const QString&);
    void sendAsyncMessage(const QString& name, const QString& message);
    void sendAsyncMessage(const QString& name, const QVariant& variant);

Q_SIGNALS:
    void viewInitialized();
    void urlChanged();
    void titleChanged();
    void loadProgressChanged();
    void navigationHistoryChanged();
    void loadingChanged();
    void viewDestroyed();
    void recvAsyncMessage(QString message, QString data);
    bool recvSyncMessage(QString message, QString data, QSyncMessageResponse* response);
    void loadRedirect();
    void securityChanged(QString status, uint32_t state);
    void firstPaint(int32_t offx, int32_t offy);
    void contentLoaded(QString docuri);
    void observeNotification(QString topic, QString data);
    void alert(QVariant data);
    void confirm(QVariant data);
    void prompt(QVariant data);
    void authRequired(QVariant data);

protected:
    virtual void setGeometry(const QRectF& rect);
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint) const;

protected:
    virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = 0);
    virtual bool event(QEvent*);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
    virtual void keyPressEvent(QKeyEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);
    virtual void inputMethodEvent(QInputMethodEvent*);
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery aQuery) const;
    virtual void EraseBackgroundGL(const QRect&);

private Q_SLOTS:
    void onInitialized();

private:
    void forceActiveFocus();

    QGraphicsMozViewPrivate* d;
    friend class QGraphicsMozViewPrivate;
};

#endif /* qgraphicsmozview_h */
