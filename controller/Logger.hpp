// Logger.hpp
#pragma once
#include <QObject>
#include <QTextEdit>

class Logger : public QObject {
    Q_OBJECT
public:
    explicit Logger(QTextEdit *textEdit, QObject *parent = nullptr);
    void log(const QString &msg, const QString &color = "black");

private:
    QTextEdit *m_edit;
};