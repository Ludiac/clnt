#ifndef PROMPT_QDIALOG_HPP
#define PROMPT_QDIALOG_HPP

#include <QDialog>

namespace Ui {
class PromptQDialog;
}

class PromptQDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PromptQDialog(QWidget *parent = nullptr);
  ~PromptQDialog();

  QString group_name;

private slots:
  void on_buttonBox_accepted();

private:
  Ui::PromptQDialog *ui;
};

#endif // PROMPT_QDIALOG_HPP
