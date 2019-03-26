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

CommandPrompt::CommandPrompt(QWidget *parent) : QTextEdit(parent), c(nullptr) {
  setPlainText(
      tr("This TextEdit provides autocompletions for words that have more than"
         " 3 characters. You can trigger autocompletion using ") +
      QKeySequence("Ctrl+E").toString(QKeySequence::NativeText));
}

CommandPrompt::~CommandPrompt() {}

void CommandPrompt::setCompleter(QCompleter *completer) {
  if (c)
    QObject::disconnect(c, 0, this, 0);

  c = completer;

  if (!c)
    return;

  c->setWidget(this);
  c->setCompletionMode(QCompleter::PopupCompletion);
  c->setCaseSensitivity(Qt::CaseInsensitive);
  QObject::connect(c, SIGNAL(activated(QString)), this,
                   SLOT(insertCompletion(QString)));
}

QCompleter *CommandPrompt::completer() const { return c; }

void CommandPrompt::insertCompletion(const QString &completion) {
  if (c->widget() != this)
    return;
  QTextCursor tc = textCursor();
  int extra = completion.length() - c->completionPrefix().length();
  tc.movePosition(QTextCursor::Left);
  tc.movePosition(QTextCursor::EndOfWord);
  tc.insertText(completion.right(extra));
  setTextCursor(tc);
}

QString CommandPrompt::textUnderCursor() const {
  QTextCursor tc = textCursor();
  tc.select(QTextCursor::WordUnderCursor);
  return tc.selectedText();
}

void CommandPrompt::focusInEvent(QFocusEvent *e) {
  if (c)
    c->setWidget(this);
  QTextEdit::focusInEvent(e);
}

void CommandPrompt::keyPressEvent(QKeyEvent *e) {
  if (c && c->popup()->isVisible()) {
    // The following keys are forwarded by the completer to the widget
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
      e->ignore();
      return; // let the completer do default behavior
    default:
      break;
    }
  }

  bool isShortcut = ((e->modifiers() & Qt::ControlModifier) &&
                     e->key() == Qt::Key_E); // CTRL+E
  if (!c || !isShortcut) // do not process the shortcut when we have a completer
    QTextEdit::keyPressEvent(e);
  const bool ctrlOrShift =
      e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
  if (!c || (ctrlOrShift && e->text().isEmpty()))
    return;

  static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
  bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
  QString completionPrefix = textUnderCursor();

  if (!isShortcut &&
      (hasModifier || e->text().isEmpty() || completionPrefix.length() < 3 ||
       eow.contains(e->text().right(1)))) {
    c->popup()->hide();
    return;
  }

  if (completionPrefix != c->completionPrefix()) {
    c->setCompletionPrefix(completionPrefix);
    c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
  }
  QRect cr = cursorRect();
  cr.setWidth(c->popup()->sizeHintForColumn(0) +
              c->popup()->verticalScrollBar()->sizeHint().width());
  c->complete(cr); // popup it up!
}
