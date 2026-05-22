// Logger.cpp
#include "Logger.hpp"

Logger::Logger(QTextEdit *textEdit, QObject *parent) : QObject(parent), m_edit(textEdit) {}

void Logger::log(const QString &msg, const QString &color) {
    QString html = QString("<span style='color:%1;'>%2</span>").arg(color, msg.toHtmlEscaped());
    m_edit->append(html);
}