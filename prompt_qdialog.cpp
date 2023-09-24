#include "prompt_qdialog.hpp"
#include "ui_prompt_qdialog.h"

PromptQDialog::PromptQDialog(QWidget *parent) :
                                                QDialog(parent),
                                                ui(new Ui::PromptQDialog)
{
  ui->setupUi(this);
}

PromptQDialog::~PromptQDialog()
{
  delete ui;
}

void PromptQDialog::on_buttonBox_accepted()
{
  group_name = ui->lineEdit->text();
}

