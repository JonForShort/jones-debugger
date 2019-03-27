//
// MIT License
//
// Copyright 2019
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QScrollBar>

#include "command_prompt.hh"

using namespace Ui;

CommandPrompt::CommandPrompt(QWidget *parent)
    : QTextEdit(parent), completerModel_({QString("test"), QString("doggy")}),
      completer_(std::make_unique<QCompleter>(&completerModel_)) {}

CommandPrompt::~CommandPrompt() {}

void CommandPrompt::setCompleter(std::unique_ptr<QCompleter> completer) {
  if (completer_ != nullptr) {
    QObject::disconnect(completer_.get(), nullptr, this, nullptr);
    completer_ = nullptr;
  }
  completer_ = std::move(completer);
  if (!completer_) {
    return;
  }
  completer_->setWidget(this);
  completer_->setCompletionMode(QCompleter::PopupCompletion);
  completer_->setCaseSensitivity(Qt::CaseInsensitive);
  QObject::connect(completer_.get(), SIGNAL(activated(QString)), this,
                   SLOT(insertCompletion(QString)));
}

QCompleter *CommandPrompt::completer() const { return completer_.get(); }

void CommandPrompt::insertCompletion(const QString &completion) {
  if (completer_->widget() != this) {
    return;
  }
  QTextCursor cursor = textCursor();
  int extra = completion.length() - completer_->completionPrefix().length();
  cursor.movePosition(QTextCursor::Left);
  cursor.movePosition(QTextCursor::EndOfWord);
  cursor.insertText(completion.right(extra));
  setTextCursor(cursor);
}

QString CommandPrompt::textUnderCursor() const {
  QTextCursor cursor = textCursor();
  cursor.select(QTextCursor::WordUnderCursor);
  return cursor.selectedText();
}

void CommandPrompt::focusInEvent(QFocusEvent *e) {
  if (completer_) {
    completer_->setWidget(this);
  }
  QTextEdit::focusInEvent(e);
}

void CommandPrompt::keyPressEvent(QKeyEvent *event) {
  if (completer_ && completer_->popup()->isVisible()) {
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
    case Qt::Key_Backtab:
      event->ignore();
      return;
    default:
      break;
    }
  }
  const bool isShortcut = ((event->key() == Qt::Key_Tab));
  if (!completer_ || !isShortcut) {
    QTextEdit::keyPressEvent(event);
  }
  const bool ctrlOrShift =
      event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
  if (!completer_ || (ctrlOrShift && event->text().isEmpty())) {
    return;
  }
  static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
  bool hasModifier = (event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
  QString completionPrefix = textUnderCursor();
  if (!isShortcut &&
      (hasModifier || event->text().isEmpty() ||
       completionPrefix.length() < 3 || eow.contains(event->text().right(1)))) {
    completer_->popup()->hide();
    return;
  }
  if (completionPrefix != completer_->completionPrefix()) {
    completer_->setCompletionPrefix(completionPrefix);
    completer_->popup()->setCurrentIndex(
        completer_->completionModel()->index(0, 0));
  }
  QRect cr = cursorRect();
  cr.setWidth(completer_->popup()->sizeHintForColumn(0) +
              completer_->popup()->verticalScrollBar()->sizeHint().width());
  completer_->complete(cr);
}
