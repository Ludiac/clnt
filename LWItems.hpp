#pragma once

#include "qlistwidget.h"

class GeneralLWItem : public QListWidgetItem {
public:
  GeneralLWItem(qint32 id, QString text, QListWidget *parent, int type);
  virtual QVariant data(int role) const;

private:
  qint32 id;
};

class PrivateRoomLWItem : public QListWidgetItem {
public:
  PrivateRoomLWItem(qint32 room_id, qint32 client_id, QString text,
                    QListWidget *parent, int type);
  virtual QVariant data(int role) const;

private:
  qint32 room_id, client_id;
};
